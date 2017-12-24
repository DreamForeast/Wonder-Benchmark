open Node;

open NodeVal;

open Performance;

open Root;

open Contract;

open Js.Promise;

type caseTimeItem = {
  name: string,
  time: int
};

type caseItem = {
  name: string,
  time: array(caseTimeItem),
  memory: int,
  errorRate: option(int)
};

/* type compareTarget =
   | TIME
   | MEMORY; */
type result = {
  name: string,
  timeArray: array(int),
  memory: int
  /* compareTarget */
};

type state = {
  page: Page.t,
  browser: Browser.t,
  scriptFilePath: string,
  name: string,
  cases: array(caseItem),
  result: option(result)
};

type resultTimeData = {
  timestamp: int,
  timeArray: array(int)
};

type compareConfig = {errorRate: float};

let _convertStateJsonToRecord = (page, browser, scriptFilePath, stateJson) =>
  Json.(
    Decode.{
      page,
      browser,
      scriptFilePath,
      name: stateJson |> field("name", string),
      cases:
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
                           ),
                      memory: stateJson |> field("memory", int),
                      errorRate: stateJson |> optional(field("error_rate", int))
                    }
                  )
           ),
      result: None
    }
  );

let _getFilePath = (jsonFileName: string) =>
  Path.join([|Process.cwd(), "test/performance/data", jsonFileName|]);

let createState = (page, browser, scriptFilePath, jsonFileName: string) => {
  requireCheck(
    () => Contract.Operators.(jsonFileName |> _getFilePath |> Fs.existsSync |> assertTrue)
  );
  let stateJson = Fs.readFileSync(_getFilePath(jsonFileName), `utf8) |> Js.Json.parseExn;
  stateJson |> _convertStateJsonToRecord(page, browser, scriptFilePath)
};

let createEmptyState = () => {
  page: Obj.magic(0),
  browser: Obj.magic(1),
  scriptFilePath: "",
  name: "",
  cases: [||],
  result: None
};

let _findFirst = (arr: array('item), func) => arr |> ArraySystem.unsafeFind(func);

let _buildSumTimeArr = (resultDataTimeArr) =>
  ArraySystem.range(0, Js.Array.length(resultDataTimeArr[0].timeArray) - 1);

let _average = (promise) =>
  promise
  |> then_(
       ((resultDataTimeArr, resultDataMemoryArr)) =>
         (
           resultDataTimeArr
           |> Js.Array.reduce(
                ((sumTimeArr, resultDataArr), {timeArray}) => (
                  sumTimeArr |> Js.Array.mapi((sum, index) => sum + timeArray[index]),
                  resultDataArr
                ),
                (_buildSumTimeArr(resultDataTimeArr), resultDataTimeArr)
              )
           |> (
             ((sumTimeArr, resultDataTimeArr)) =>
               sumTimeArr
               |> Js.Array.map((sumTime) => sumTime / (resultDataTimeArr |> Js.Array.length))
           )
           ,
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
     ) /* resultDataArr
          |> Js.Array.reduce(
               ((sumTimeArr, sumMemory, resultDataArr), ({timeArray}, memory)) => (
                 sumTimeArr |> Js.Array.mapi((sum, index) => sum + timeArray[index]),
                 sumMemory + memory,
                 resultDataArr
               ),
               (_buildSumTimeArr(resultDataArr), 0, resultDataArr)
             )
          |> WonderCommonlib.DebugUtils.log
          |> (
            ((sumTimeArr, sumMemory, resultDataArr)) => (
              sumTimeArr |> Js.Array.map((sumTime) => sumTime / (resultDataArr |> Js.Array.length)),
              sumMemory / (resultDataArr |> Js.Array.length)
            )
          )
          |> resolve */;

/* (arr |> Js.Array.reduce((sum, value) => sum +. value, 0.))
   /. (arr |> Js.Array.length |> Number.intToFloat)
   |> Js.Math.round; */
let _removeExtreme = (count, promise) =>
  promise
  |> then_(
       ((resultDataTimeArr, resultDataMemoryArr)) =>
         (
           resultDataTimeArr
         |> WonderCommonlib.DebugUtils.logJson
           |> Js.Array.sortInPlaceWith(
                ({timestamp: timestamp1}, {timestamp: timestamp2}) => timestamp1 - timestamp2
              )
           |> Js.Array.slice(~start=count, ~end_=Js.Array.length(resultDataTimeArr) - count),
           resultDataMemoryArr
         |> WonderCommonlib.DebugUtils.logJson
           |> Js.Array.sortInPlaceWith((memory1, memory2) => memory1 - memory2)
           |> Js.Array.slice(~start=count, ~end_=Js.Array.length(resultDataMemoryArr) - count)
         )

         |> WonderCommonlib.DebugUtils.logJson
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
  |> Js.Array.slice_start(1)
  |> Js.Array.map(
       (time) => {
         let result = time - lastTime^;
         lastTime := time;
         result
       }
     )
};

let _getScriptFilePath = (state) => state.scriptFilePath;

let _addScript = (scriptFilePath, promise) =>
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
     );

let _execFunc = (browser, func, state, promise) =>
  promise
  |> then_(
       (resultData) => browser |> Browser.newPage |> then_((page) => (page, resultData) |> resolve)
     )
  |> _addScript(_getScriptFilePath(state))
  |> then_(
       ((page, resultData)) =>
         page |> Page.metrics() |> then_((data) => (page, resultData, data) |> resolve)
     )
  |> then_(
       ((page, resultData, data)) =>
         page
         |> Page.evaluate([@bs] func)
         |> then_((timeArr) => resolve((page, resultData, data, timeArr)))
     )
  |> then_(
       ((page, resultData, lastData, timeArr)) =>
         page
         |> Page.metrics()
         |> then_((data) => (page, resultData, lastData, data, timeArr) |> resolve)
     )
  |> then_(
       ((page, (resultDataTimeArr, resultDataMemoryArr), lastData, data, timeArr)) => {
         resultDataTimeArr
         |> Js.Array.push({
              timestamp: _computeTimestamp(lastData, data),
              timeArray: _computePerformanceTime(timeArr)
            })
         |> ignore;
         resultDataMemoryArr |> Js.Array.push(_computeMemory(lastData, data)) |> ignore;


/* WonderCommonlib.DebugUtils.logJson((resultDataTimeArr, resultDataMemoryArr)) |> ignore; */
         (page, (resultDataTimeArr, resultDataMemoryArr)) |> resolve
       }
     )
  |> then_(((page, resultData)) => page |> Page.close |> then_((_) => resultData |> resolve));

let _execSpecificCount = (count, func, browser, state) =>
  ArraySystem.range(0, count - 1)
  |> Js.Array.reduce(
       (promise, _) => promise |> _execFunc(browser, func, state),
       ([||], [||]) |> resolve
     );

/* let _addScript = (scriptFilePath, page) =>
   page
   |> Page.addScriptTag({
        "url": Js.Nullable.empty,
        "content": Js.Nullable.empty,
        "path": Js.Nullable.return(scriptFilePath)
      }); */
let _getPage = (state) => state.page;

let _getBrowser = (state) => state.browser;

let exec = (name: string, func, state) => {
  let page = state |> _getPage;
  let browser = state |> _getBrowser;
  /* let (time, memory) = */
  state
  |> _execSpecificCount(5, func, browser)
  |> _removeExtreme(1)
  |> _average
  |> (
    (promise) =>
      promise
      |> then_(
           ((timeArray, memory)) => {...state, result: Some({name, timeArray, memory})} |> resolve
         )
  )
  /* {...state, result: Some({...state.result |> Js.Option.getExn, name, time, memory})} */
};

/* let getTime = (state) => {
     ...state,
     result: Some({...state.result |> Js.Option.getExn, compareTarget: TIME})
   }; */
let _filterTargetName = (name, targetName) => name == targetName;

let _getRange = (target, errorRate) => (
  target * (100 - errorRate) / 100,
  target * (100 + errorRate) / 100
);

let _isInRange = (actual, (min, max)) => actual >= min && actual <= max;

let _compareMemory = (actualMemory, targetMemory, errorRate) => {
  let (minMemory, maxMemory) = _getRange(targetMemory, errorRate);
  switch (
    /* _isInRange(actualTime, targetTime, errorRate), */
    _isInRange(
      actualMemory,
      (minMemory, maxMemory)
    )
  ) {
  | false =>
    /* failwith({j|expect memory to in [$minMemory, $maxMemory], but actual is $actualMemory|j}) */
    (true, {j|expect memory to in [$minMemory, $maxMemory], but actual is $actualMemory\n|j})
  /* | _ => true |> expect |> toBe(true) |> resolve */
  | true => (false, "")
  }
};

let _compareTime =
    (actualTimeArray, targetTimeDataArray: array(caseTimeItem), errorRate, (isFail, failMessage)) =>
  targetTimeDataArray
  |> Js.Array.reducei(
       ((isFail, failMessage), {name, time: targetTime}: caseTimeItem, index) => {
         let (minTime, maxTime) = _getRange(targetTime, errorRate);
         let actualTime = actualTimeArray[index];
         switch (_isInRange(actualTime, (minTime, maxTime))) {
         | false => (
             true,
             failMessage
             ++ {j|expect time:$name to in [$minTime, $maxTime], but actual is $actualTime\n|j}
           )
         | true => (isFail, failMessage)
         }
       },
       (isFail, failMessage)
     );

/* let compare = (expect, {cases, result}) => { */
let compare = ((expect, toFail), promise) =>
  promise
  |> then_(
       ({cases, result}) => {
         let {name, timeArray: actualTimeArray, memory: actualMemory}: result =
           result |> Js.Option.getExn;
         let {time: targetTimeDataArray, memory: targetMemory, errorRate}: caseItem =
           _findFirst(cases, (item: caseItem) => _filterTargetName(item.name, name));
         let errorRate = errorRate |> Js.Option.getExn;
         let (isFail, failMessage) =
           _compareMemory(actualMemory, targetMemory, errorRate)
           |> _compareTime(actualTimeArray, targetTimeDataArray, errorRate);
         isFail ? failwith(failMessage) : true |> resolve
         /* isFail ? expect() |> toFail(failMessage) : true |> resolve */
       }
     );