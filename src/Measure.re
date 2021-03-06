open Node;

open WonderCommonlib.NodeExtend;

open Performance;

open Root;

open Contract;

open Js.Promise;

open WonderBsPuppeteer.Puppeteer;

open PerformanceTestDataType;

open BenchmarkDataType;

let _getCaseName = resultDataTimeList => List.hd(resultDataTimeList).name;

let _getErrorRate = resultDataTimeList =>
  List.hd(resultDataTimeList).errorRate;

let _getTimeTextList = resultDataTimeList =>
  List.hd(resultDataTimeList).timeTextArray |> Array.to_list;

let _getMedianValue = list =>
  List.nth(list |> List.sort((a1, a2) => a1 - a2), List.length(list) / 2);

let _getJSHeapUsedSize: (. WonderBsPuppeteer.Page.metricsData) => int = [%bs.raw
  {|
               function(data) {
                 return data.JSHeapUsedSize
               }
                |}
];

let _getTimestamp: (. WonderBsPuppeteer.Page.metricsData) => float = [%bs.raw
  {|
               function(data) {
                 return data.Timestamp
               }
                |}
];

let _computeMemory =
    (
      data1: WonderBsPuppeteer.Page.metricsData,
      data2: WonderBsPuppeteer.Page.metricsData,
    ) =>
  (_getJSHeapUsedSize(. data2) - _getJSHeapUsedSize(. data1)) / (1024 * 1024);

let _computeTimestamp = (data1, data2) =>
  (_getTimestamp(. data2) -. _getTimestamp(. data1))
  *. 1000.0
  |> Js.Math.floor;

let _computePerformanceTime = (timeArr: array(float)) => {
  requireCheck(() =>
    Contract.Operators.(
      test("timeArr.length should >= 1", () =>
        timeArr |> Js.Array.length >= 1
      )
    )
  );
  let timeArr = timeArr |> Js.Array.map(time => Number.floatToInt(time));
  let lastTime = ref(timeArr[0]);
  timeArr
  |> Js.Array.sliceFrom(1)
  |> Js.Array.map(time => {
       let result = time - lastTime^;
       lastTime := time;
       result;
     });
};

let _addScript = (allScriptFilePathList, promise) =>
  allScriptFilePathList
  |> List.fold_left(
       (promise, scriptFilePath) =>
         promise
         |> then_(((page, resultData)) =>
              page
              |> WonderBsPuppeteer.Page.addScriptTag({
                   "url": Js.Nullable.undefined,
                   "content": Js.Nullable.undefined,
                   "path": Js.Nullable.return(scriptFilePath),
                 })
              |> then_(_ => (page, resultData) |> resolve)
            ),
       promise,
     );

let _execBodyFunc: string => bodyFuncReturnValue = [%bs.raw
  {|
   function(bodyFuncStr) {
      var bodyFunc = new Function(bodyFuncStr);

      return bodyFunc();
   }
    |}
];

let _exposeReadFileAsUtf8Sync = page =>
  page
  |> Page.exposeFunctionWithString("readFileAsUtf8Sync", filePath =>
       Fs.readFileAsUtf8Sync(filePath)
     );

let _exposeReadFileAsBufferDataSync = page =>
  page
  |> Page.exposeFunctionWithString("readFileAsBufferDataSync", filePath =>
       NodeExtend.readFileBufferDataSync(filePath)
     );

let _loadImageSrc = [%bs.raw
  {|
      function(imageSrc){
        var getPixels = require("get-pixels");

        return new Promise((resolve, reject) => {

        getPixels(imageSrc, function(err, pixels) {
            if(err) {
                reject(err);
            }

            resolve([Array.from(pixels.data.slice()), pixels.shape])
          })
        });
      }
      |}
];

let _exposeLoadImage = page =>
  page
  |> Page.exposeFunctionWithString("loadImageSrc", imageSrc =>
       _loadImageSrc(imageSrc)
     );

let _execFunc =
    (
      isClosePage,
      allScriptFilePathList,
      name,
      bodyFuncStr,
      errorRate,
      browser,
      promise,
    ) =>
  promise
  |> then_(resultData =>
       browser
       |> WonderBsPuppeteer.Browser.newPage
       |> then_(page => (page, resultData) |> resolve)
     )
  |> _addScript(allScriptFilePathList)
  |> then_(((page, resultData)) =>
       page
       |> WonderBsPuppeteer.Page.metrics()
       |> then_(data => (page, resultData, data) |> resolve)
     )
  |> then_(((page, resultData, data)) =>
       page
       |> _exposeReadFileAsUtf8Sync
       |> then_(_ => page |> _exposeReadFileAsBufferDataSync)
       |> then_(_ => page |> _exposeLoadImage)
       |> then_(_ =>
            page
            |> Page.evaluateWithArg([@bs] _execBodyFunc, bodyFuncStr)
            |> then_((timeData: bodyFuncReturnValue) =>
                 resolve((page, resultData, data, timeData))
               )
          )
     )
  |> then_(((page, resultData, lastData, timeData)) =>
       page
       |> WonderBsPuppeteer.Page.metrics()
       |> then_(data =>
            (page, resultData, lastData, data, timeData) |> resolve
          )
     )
  |> then_(
       (
         (
           page,
           (resultDataTimeList, resultDataMemoryList),
           lastData,
           data,
           timeData,
         ),
       ) =>
       (
         page,
         (
           [
             {
               name,
               errorRate,
               timestamp: _computeTimestamp(lastData, data),
               timeTextArray: timeData##textArray,
               timeArray: _computePerformanceTime(timeData##timeArray),
             },
             ...resultDataTimeList,
           ],
           [_computeMemory(lastData, data), ...resultDataMemoryList],
         ),
       )
       |> resolve
     )
  |> then_(((page, resultData)) =>
       switch (isClosePage) {
       | false => resultData |> resolve
       | true =>
         page
         |> WonderBsPuppeteer.Page.close
         |> then_(_ => resultData |> resolve)
       }
     );

let _execSpecificCount =
    (
      isClosePage,
      execCount,
      allScriptFilePathList,
      name,
      bodyFuncStr,
      errorRate,
      browser,
    ) =>
  WonderCommonlib.ArrayService.range(0, execCount - 1)
  |> Js.Array.reduce(
       (promise, _) =>
         promise
         |> _execFunc(
              isClosePage,
              allScriptFilePathList,
              name,
              bodyFuncStr,
              errorRate,
              browser,
            ),
       ([], []) |> resolve,
     );

let _getMedian = promise =>
  promise
  |> then_(((resultDataTimeList, resultDataMemoryList)) =>
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
         |> List.map(arr => arr |> Array.to_list |> _getMedianValue),
         resultDataMemoryList |> _getMedianValue,
       )
       |> resolve
     );

let _exec =
    (
      isClosePage,
      execCount,
      allScriptFilePathList,
      name,
      bodyFuncStr,
      errorRate,
      case,
      browser,
      promise,
    ) =>
  promise
  |> then_(resultList =>
       browser
       |> _execSpecificCount(
            isClosePage,
            execCount,
            allScriptFilePathList,
            name,
            bodyFuncStr,
            errorRate,
          )
       |> _getMedian
       |> then_(
            (
              (caseName, errorRate, timestamp, timeTextList, timeList, memory),
            ) =>
            (
              caseName,
              errorRate,
              timestamp,
              timeTextList,
              timeList,
              memory,
              case,
            )
            |> resolve
          )
       |> then_(data => [data, ...resultList] |> resolve)
     );

let measureCase =
    (
      browser,
      execCount,
      allScriptFilePathList,
      commonData,
      testName,
      {name: caseName, bodyFuncStr, errorRate} as case,
      promise,
    ) => {
  let {isClosePage} = commonData;
  promise
  |> then_(resultList => {
       let title =
         PerformanceTestDataUtils.buildCaseTitle(testName, caseName);
       WonderLog.Log.log({j|measure $(title)...|j}) |> ignore;
       resultList |> resolve;
     })
  |> _exec(
       isClosePage,
       execCount,
       allScriptFilePathList,
       caseName,
       bodyFuncStr,
       errorRate,
       case,
       browser,
     );
};

let measure =
    (browser, execCount, allScriptFilePathList, {commonData, testDataList}) => {
  let {execCountWhenTest, isClosePage} = commonData;
  testDataList
  |> List.fold_left(
       (promise, {name: testName, caseList}) =>
         promise
         |> then_(resultTestList =>
              caseList
              |> List.fold_left(
                   (
                     promise,
                     {name: caseName, bodyFuncStr, errorRate} as case,
                   ) =>
                     promise
                     |> measureCase(
                          browser,
                          execCount,
                          allScriptFilePathList,
                          commonData,
                          testName,
                          case,
                        ),
                   [] |> resolve,
                 )
              |> then_(resultCaseList =>
                   [(testName, resultCaseList), ...resultTestList] |> resolve
                 )
            ),
       [] |> resolve,
     );
};