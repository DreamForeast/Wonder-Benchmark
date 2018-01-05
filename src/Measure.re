open Node;

open NodeExtend;

open Performance;

open Root;

open Contract;

open Js.Promise;

open Puppeteer;

open PerformanceTestDataType;

open BenchmarkDataType;

let _getCaseName = (resultDataTimeList) => List.hd(resultDataTimeList).name;

let _getErrorRate = (resultDataTimeList) => List.hd(resultDataTimeList).errorRate;

let _getTimeTextList = (resultDataTimeList) =>
  List.hd(resultDataTimeList).timeTextArray |> Array.to_list;

let _getMedianValue = (list) =>
  List.nth(list |> List.sort((a1, a2) => a1 - a2), List.length(list) / 2);

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

let _addScript = (commonScriptFilePathList, scriptFilePathList, promise) =>
  (
    switch scriptFilePathList {
    | None => commonScriptFilePathList
    | Some(scriptFilePathList) => scriptFilePathList @ commonScriptFilePathList
    }
  )
  |> List.fold_left(
       (promise, scriptFilePath) =>
         promise
         |> then_(
              ((page, resultData)) =>
                page
                |> Page.addScriptTag({
                     "url": Js.Nullable.empty,
                     "content": Js.Nullable.empty,
                     "path": Js.Nullable.return(scriptFilePath)
                   })
                |> then_((_) => (page, resultData) |> resolve)
            ),
       promise
     );

let _execBodyFunc: string => bodyFuncReturnValue = [%bs.raw
  {|
   function(bodyFuncStr) {
      var bodyFunc = new Function(bodyFuncStr);

      return bodyFunc();
   }
    |}
];

let _execFunc =
    (
      isClosePage,
      commonScriptFilePathList,
      name,
      scriptFilePathList,
      bodyFuncStr,
      errorRate,
      browser,
      promise
    ) =>
  promise
  |> then_(
       (resultData) => browser |> Browser.newPage |> then_((page) => (page, resultData) |> resolve)
     )
  |> _addScript(commonScriptFilePathList, scriptFilePathList)
  |> then_(
       ((page, resultData)) =>
         page |> Page.metrics() |> then_((data) => (page, resultData, data) |> resolve)
     )
  |> then_(
       ((page, resultData, data)) =>
         page
         |> Page.evaluateWithArg([@bs] _execBodyFunc, bodyFuncStr)
         |> then_((timeData: bodyFuncReturnValue) => resolve((page, resultData, data, timeData)))
     )
  |> then_(
       ((page, resultData, lastData, timeData)) =>
         page
         |> Page.metrics()
         |> then_((data) => (page, resultData, lastData, data, timeData) |> resolve)
     )
  |> then_(
       ((page, (resultDataTimeList, resultDataMemoryList), lastData, data, timeData)) =>
         (
           page,
           (
             [
               {
                 name,
                 errorRate,
                 timestamp: _computeTimestamp(lastData, data),
                 timeTextArray: timeData##textArray,
                 timeArray: _computePerformanceTime(timeData##timeArray)
               },
               ...resultDataTimeList
             ],
             [_computeMemory(lastData, data), ...resultDataMemoryList]
           )
         )
         |> resolve
     )
  |> then_(
       ((page, resultData)) =>
         switch isClosePage {
         | false => resultData |> resolve
         | true => page |> Page.close |> then_((_) => resultData |> resolve)
         }
     );

let _execSpecificCount =
    (
      isClosePage,
      execCount,
      commonScriptFilePathList,
      name,
      scriptFilePathList,
      bodyFuncStr,
      errorRate,
      browser
    ) =>
  ArraySystem.range(0, execCount - 1)
  |> Js.Array.reduce(
       (promise, _) =>
         promise
         |> _execFunc(
              isClosePage,
              commonScriptFilePathList,
              name,
              scriptFilePathList,
              bodyFuncStr,
              errorRate,
              browser
            ),
       ([], []) |> resolve
     );

let _getMedian = (promise) =>
  promise
  |> then_(
       ((resultDataTimeList, resultDataMemoryList)) =>
         (
           _getCaseName(resultDataTimeList),
           _getErrorRate(resultDataTimeList),
           resultDataTimeList
           |> List.map(({timestamp}: resultTimeData) => timestamp)
           |> _getMedianValue,
           _getTimeTextList(resultDataTimeList),
           resultDataTimeList
           |> List.map(({timeArray}: resultTimeData) => timeArray)
           |> Array.of_list
           |> ArraySystem.zip
           |> Array.to_list
           |> List.map((arr) => arr |> Array.to_list |> _getMedianValue),
           resultDataMemoryList |> _getMedianValue
         )
         |> resolve
     );

let _exec =
    (
      isClosePage,
      execCount,
      commonScriptFilePathList,
      name,
      scriptFilePathList,
      bodyFuncStr,
      errorRate,
      case,
      browser,
      promise
    ) =>
  promise
  |> then_(
       (resultList) =>
         browser
         |> _execSpecificCount(
              isClosePage,
              execCount,
              commonScriptFilePathList,
              name,
              scriptFilePathList,
              bodyFuncStr,
              errorRate
            )
         |> _getMedian
         |> then_(
              ((caseName, errorRate, timestamp, timeTextList, timeList, memory)) =>
                (caseName, errorRate, timestamp, timeTextList, timeList, memory, case) |> resolve
            )
         |> then_((data) => [data, ...resultList] |> resolve)
     );

let measure = (browser, {commonData, testDataList}) => {
  let {isClosePage, execCountWhenTest, scriptFilePathList: commonScriptFilePathList} = commonData;
  testDataList
  |> List.fold_left(
       (promise, {name: testName, caseList}) =>
         promise
         |> then_(
              (resultTestList) =>
                caseList
                |> List.fold_left(
                     (
                       promise,
                       {name: caseName, bodyFuncStr, scriptFilePathList, errorRate} as case
                     ) =>
                       promise
                       |> then_(
                            (resultList) => {
                              let title =
                                PerformanceTestDataUtils.buildCaseTitle(testName, caseName);
                              WonderCommonlib.DebugUtils.log({j|measure $(title)...|j}) |> ignore;
                              resultList |> resolve
                            }
                          )
                       |> _exec(
                            isClosePage,
                            execCountWhenTest,
                            commonScriptFilePathList,
                            caseName,
                            scriptFilePathList,
                            bodyFuncStr,
                            errorRate,
                            case,
                            browser
                          ),
                     [] |> resolve
                   )
                |> then_(
                     (resultCaseList) => [(testName, resultCaseList), ...resultTestList] |> resolve
                   )
            ),
       [] |> resolve
     )
};