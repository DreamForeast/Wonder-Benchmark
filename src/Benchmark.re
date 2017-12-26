open Node;

open NodeVal;

open Performance;

open Root;

open Contract;

open Js.Promise;

open BenchmarkType;

let _convertStateJsonToRecord = (page, browser, scriptFilePath, config: config, stateJson) =>
  Json.(
    Decode.{
      config,
      page,
      browser,
      scriptFilePathList: [scriptFilePath],
      name: stateJson |> field("name", string),
      caseList:
        stateJson
        |> field(
             "cases",
             (json) =>
               json
               |> array(
                    (stateJson) => {
                      name: stateJson |> field("name", string),
                      time:
                        stateJson
                        |> field(
                             "time",
                             (json) =>
                               json
                               |> array(
                                    (json) => {
                                      name: json |> field("name", string),
                                      time: json |> field("time", int)
                                    }
                                  )
                               |> Array.to_list
                           ),
                      memory: stateJson |> field("memory", int),
                      errorRate: stateJson |> optional(field("error_rate", int))
                    }
                  )
               |> Array.to_list
           ),
      result: None,
      actualCaseDataList: []
    }
  );

let _getFilePath = (jsonFileName: string) =>
  Path.join([|Process.cwd(), "test/performance/data", jsonFileName|]);

let createState =
    (
      ~config={isClosePage: true, execCount: 10, extremeCount: 2, generateDataFilePath: None},
      page,
      browser,
      scriptFilePath,
      jsonFileName: string
    ) => {
  requireCheck(
    () =>
      Contract.(
        Operators.(
          GenerateData.needGenerateData(config.generateDataFilePath) ?
            () : jsonFileName |> _getFilePath |> Fs.existsSync |> assertTrue
        )
      )
  );
  switch (GenerateData.needGenerateData(config.generateDataFilePath)) {
  | false =>
    let stateJson = Fs.readFileSync(_getFilePath(jsonFileName), `utf8) |> Js.Json.parseExn;
    stateJson |> _convertStateJsonToRecord(page, browser, scriptFilePath, config)
  | true =>
    GenerateData.convertStateJsonToRecord(
      page,
      browser,
      scriptFilePath,
      GenerateData.getConfig(config)
    )
  }
};

let prepareBeforeAll = (state) =>
  GenerateData.needGenerateData(BenchmarkStateUtils.getConfig(state).generateDataFilePath) ?
    {
      GenerateData.createEmptyDataFile(GenerateData.unsafeGetFilePath(state));
      state
    } :
    state;

let createEmptyState = () => {
  config: Obj.magic(0),
  page: Obj.magic(0),
  browser: Obj.magic(1),
  scriptFilePathList: [],
  name: "",
  caseList: [],
  result: None,
  actualCaseDataList: []
};

let _findFirst = (list: list('item), func) => list |> List.find(func);

let _buildSumTimeArr = (resultDataTimeArr) =>
  ArraySystem.range(0, Js.Array.length(resultDataTimeArr[0].timeArray) - 1);

let _getErrorRate = (resultDataTimeArr) => resultDataTimeArr[0].errorRate;

let _getTimeTextArray = (resultDataTimeArr) => resultDataTimeArr[0].timeTextArray;

let _average = (promise) =>
  promise
  |> then_(
       ((resultDataTimeArr, resultDataMemoryArr)) =>
         (
           _getErrorRate(resultDataTimeArr),
           resultDataTimeArr
           |> Js.Array.reduce(
                ((sumTime, resultDataArr), {timestamp}: resultTimeData) => (
                  sumTime + timestamp,
                  resultDataArr
                ),
                (0, resultDataTimeArr)
              )
           |> (((sumTime, resultDataTimeArr)) => sumTime / (resultDataTimeArr |> Js.Array.length)),
           _getTimeTextArray(resultDataTimeArr),
           resultDataTimeArr
           |> Js.Array.reduce(
                ((sumTimeArr, resultDataArr), {timeArray}: resultTimeData) => (
                  sumTimeArr |> Js.Array.mapi((sum, index) => sum + timeArray[index]),
                  resultDataArr
                ),
                (_buildSumTimeArr(resultDataTimeArr), resultDataTimeArr)
              )
           |> (
             ((sumTimeArr, resultDataTimeArr)) =>
               sumTimeArr
               |> Js.Array.map((sumTime) => sumTime / (resultDataTimeArr |> Js.Array.length))
           ),
           resultDataMemoryArr
           |> Js.Array.reduce(
                ((sumMemory, resultDataArr), memory) => (sumMemory + memory, resultDataArr),
                (0, resultDataMemoryArr)
              )
           |> (
             ((sumMemory, resultDataMemoryArr)) =>
               sumMemory / (resultDataMemoryArr |> Js.Array.length)
           )
         )
         |> resolve
     );

let _removeExtreme = (count, promise) =>
  promise
  |> then_(
       ((resultDataTimeArr, resultDataMemoryArr)) =>
         (
           resultDataTimeArr
           |> Js.Array.sortInPlaceWith(
                ({timestamp: timestamp1}, {timestamp: timestamp2}) => timestamp1 - timestamp2
              )
           |> Js.Array.slice(~start=count, ~end_=Js.Array.length(resultDataTimeArr) - count),
           resultDataMemoryArr
           |> Js.Array.sortInPlaceWith((memory1, memory2) => memory1 - memory2)
           |> Js.Array.slice(~start=count, ~end_=Js.Array.length(resultDataMemoryArr) - count)
         )
         |> resolve
     );

let _getJSHeapUsedSize: [@bs] (Page.metricsData => int) = [%bs.raw
  {|
               function(data) {
                 return data.JSHeapUsedSize
               }
                |}
];

let _getTimestamp: [@bs] (Page.metricsData => float) = [%bs.raw
  {|
               function(data) {
                 return data.Timestamp
               }
                |}
];

let _computeMemory = (data1: Page.metricsData, data2: Page.metricsData) =>
  ([@bs] _getJSHeapUsedSize(data2) - [@bs] _getJSHeapUsedSize(data1)) / (1024 * 1024);

let _computeTimestamp = (data1, data2) =>
  ([@bs] _getTimestamp(data2) -. [@bs] _getTimestamp(data1)) *. 1000.0 |> Js.Math.floor;

let _computePerformanceTime = (timeArr: array(float)) => {
  requireCheck(
    () =>
      Contract.Operators.(
        test("timeArr.length should >= 1", () => timeArr |> Js.Array.length >= 1)
      )
  );
  let timeArr = timeArr |> Js.Array.map((time) => Number.floatToInt(time));
  let lastTime = ref(timeArr[0]);
  timeArr
  |> Js.Array.sliceFrom(1)
  |> Js.Array.map(
       (time) => {
         let result = time - lastTime^;
         lastTime := time;
         result
       }
     )
};

let _getScriptFilePathList = (state) => state.scriptFilePathList;

let _addScript = (scriptFilePathList, promise) =>
  scriptFilePathList
  |> List.fold_left(
       (promise, scriptFilePath) =>
         promise
         |> then_(
              ((page, resultDataArr)) =>
                page
                |> Page.addScriptTag({
                     "url": Js.Nullable.empty,
                     "content": Js.Nullable.empty,
                     "path": Js.Nullable.return(scriptFilePath)
                   })
                |> then_((_) => (page, resultDataArr) |> resolve)
            ),
       promise
     );

let _execFunc = (browser, func: unit => funcReturnValue, state, promise) =>
  promise
  |> then_(
       (resultData) => browser |> Browser.newPage |> then_((page) => (page, resultData) |> resolve)
     )
  |> _addScript(_getScriptFilePathList(state))
  |> then_(
       ((page, resultData)) =>
         page |> Page.metrics() |> then_((data) => (page, resultData, data) |> resolve)
     )
  |> then_(
       ((page, resultData, data)) =>
         page
         |> Page.evaluate([@bs] func)
         |> then_((timeData: funcReturnValue) => resolve((page, resultData, data, timeData)))
     )
  |> then_(
       ((page, resultData, lastData, timeData)) =>
         page
         |> Page.metrics()
         |> then_((data) => (page, resultData, lastData, data, timeData) |> resolve)
     )
  |> then_(
       ((page, (resultDataTimeArr, resultDataMemoryArr), lastData, data, timeData)) => {
         resultDataTimeArr
         |> Js.Array.push({
              errorRate: timeData##errorRate,
              timestamp: _computeTimestamp(lastData, data),
              timeTextArray: timeData##textArray,
              timeArray: _computePerformanceTime(timeData##timeArray)
            })
         |> ignore;
         resultDataMemoryArr |> Js.Array.push(_computeMemory(lastData, data)) |> ignore;
         (page, (resultDataTimeArr, resultDataMemoryArr)) |> resolve
       }
     )
  |> then_(
       ((page, resultData)) => {
         let isClosePage = BenchmarkStateUtils.getConfig(state).isClosePage;
         switch isClosePage {
         | false => resultData |> resolve
         | true => page |> Page.close |> then_((_) => resultData |> resolve)
         }
       }
     );

let _execSpecificCount = (count, func: unit => funcReturnValue, browser, state) =>
  ArraySystem.range(0, count - 1)
  |> Js.Array.reduce(
       (promise, _) => promise |> _execFunc(browser, func, state),
       ([||], [||]) |> resolve
     );

let _getPage = (state) => state.page;

let _getBrowser = (state) => state.browser;

let _getExecCount = (state) => state.config.execCount;

let _getExtremeCount = (state) => state.config.extremeCount;

let addScript = (scriptFilePath: string, state: state) : state => {
  ...state,
  scriptFilePathList: [scriptFilePath, ...state.scriptFilePathList]
};

let addScriptList = (scriptFilePathList: list(string), state: state) : state => {
  ...state,
  scriptFilePathList: scriptFilePathList @ state.scriptFilePathList
};

let exec = (name: string, func: unit => funcReturnValue, state) => {
  let page = state |> _getPage;
  let browser = state |> _getBrowser;
  state
  |> _execSpecificCount(_getExecCount(state), func, browser)
  |> _removeExtreme(_getExtremeCount(state))
  |> _average
  |> (
    (promise) =>
      promise
      |> then_(
           ((errorRate, timestamp, timeTextArray, timeArray, memory)) =>
             {
               ...state,
               result: Some({name, errorRate, timestamp, timeTextArray, timeArray, memory})
             }
             |> resolve
         )
  )
};

let _filterTargetName = (name, targetName) => name == targetName;

let _getRange = (target, errorRate) => (
  (target |> Number.intToFloat)
  *. (100. -. (errorRate |> Number.intToFloat))
  /. 100.0
  |> Js.Math.floor,
  (target |> Number.intToFloat)
  *. (100. +. (errorRate |> Number.intToFloat))
  /. 100.0
  |> Js.Math.ceil
);

let _isInRange = (actual, (min, max)) => actual >= min && actual <= max;

let _getDiffPercentStr = (actualValue, targetValue) => {
  let percent =
    (actualValue - targetValue |> Js.Math.abs_int |> Number.intToFloat)
    /. (targetValue |> Number.intToFloat)
    *. 100.0
    |> Js.Math.round;
  switch (actualValue - targetValue) {
  | value when value >= 0 => {j|+$percent%|j}
  | _ => {j|-$percent%|j}
  }
};

let _compareMemory = (actualMemory, targetMemory, errorRate) => {
  let (minMemory, maxMemory) = _getRange(targetMemory, errorRate);
  switch (_isInRange(actualMemory, (minMemory, maxMemory))) {
  | false =>
    let diff = _getDiffPercentStr(actualMemory, targetMemory);
    (
      true,
      {j|expect memory to in [$minMemory, $maxMemory], but actual is $actualMemory. the diff is $diff\n|j}
    )
  | true => (false, "")
  }
};

let _compareTime =
    (actualTimeArray, targetTimeDataList: list(caseTimeItem), errorRate, (isFail, failMessage)) =>
  targetTimeDataList
  |> Array.of_list
  |> Js.Array.reducei(
       ((isFail, failMessage), {name, time: targetTime}: caseTimeItem, index) => {
         let (minTime, maxTime) = _getRange(targetTime, errorRate);
         let actualTime = actualTimeArray[index];
         switch (_isInRange(actualTime, (minTime, maxTime))) {
         | false =>
           let diff = _getDiffPercentStr(actualTime, targetTime);
           (
             true,
             failMessage
             ++ {j|expect time:$name to in [$minTime, $maxTime], but actual is $actualTime. the diff is $diff\n|j}
           )
         | true => (isFail, failMessage)
         }
       },
       (isFail, failMessage)
     );

let _pass = (expect, toBe) => true |> expect |> toBe(true) |> resolve;

let compare = ((expect, toBe), promise) =>
  promise
  |> then_(
       ({caseList, result} as state) => {
         let {
           name,
           errorRate,
           timestamp: actualTimestamp,
           timeTextArray,
           timeArray: actualTimeArray,
           memory: actualMemory
         }: result =
           result |> Js.Option.getExn;
         GenerateData.needGenerateData(BenchmarkStateUtils.getConfig(state).generateDataFilePath) ?
           {
             let state =
               GenerateData.writeCaseDataStr(
                 name,
                 GenerateData.buildTimeArr(timeTextArray, actualTimeArray),
                 actualMemory,
                 errorRate,
                 state
               );
             _pass(expect, toBe)
           } :
           {
             let {time: targetTimeDataList, memory: targetMemory, errorRate}: caseItem =
               _findFirst(caseList, (item: caseItem) => _filterTargetName(item.name, name));
             let errorRate = errorRate |> Js.Option.getExn;
             let (isFail, failMessage) =
               _compareMemory(actualMemory, targetMemory, errorRate)
               |> _compareTime(actualTimeArray, targetTimeDataList, errorRate);
             isFail ?
               failwith({j|actualTimestamp is $actualTimestamp\n$failMessage|j}) :
               _pass(expect, toBe)
           }
       }
     );

let generateDataFile = GenerateData.generateDataFile;

let needGenerateData = (state) => GenerateData.needGenerateData(GenerateData.getFilePath(state));