open Node;

open NodeExtend;

open Performance;

open Root;

open Contract;

open Js.Promise;

open BenchmarkDataType;

open PerformanceTestDataType;

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

let _compareMemory = (actualMemory, benchmarkMemory, errorRate, failMessage) => {
  let (minMemory, maxMemory) as rangeData = _getRange(benchmarkMemory, errorRate);
  switch (_isInRange(actualMemory, rangeData)) {
  | false =>
    let diff = _getDiffPercentStr(actualMemory, benchmarkMemory);
    failMessage
    ++ {j|expect memory to in [$minMemory, $maxMemory], but actual is $actualMemory. the diff is $diff\n|j}
  | true => failMessage
  }
};

let _compareTime = (actualTimeList, actualTimeTextList, benchmarkTimeList, errorRate, failMessage) =>
  List.fold_left2(
    (failMessage, (actualTime, actualTimeText), benchmarkTime) => {
      let (minTime, maxTime) = _getRange(benchmarkTime, errorRate);
      switch (_isInRange(actualTime, (minTime, maxTime))) {
      | false =>
        let diff = _getDiffPercentStr(actualTime, benchmarkTime);
        failMessage
        ++ {j|expect time:$actualTimeText to in [$minTime, $maxTime], but actual is $actualTime. the diff is $diff\n|j}
      | true => failMessage
      }
    },
    failMessage,
    List.combine(actualTimeList, actualTimeTextList),
    benchmarkTimeList
  );

let _getBenchmarkData = (testName, commonData) =>
  BenchmarkDataConverter.convertToData(
    Fs.readFileAsUtf8Sync(GenerateBenchmark.getDataFilePath(testName, commonData))
    |> Js.Json.parseExn
  );

let _isFail = (failMessage) => failMessage |> Js.String.length > 0;

let isPass = (failList) => failList |> List.length === 0;

let getFailText = (failList) =>
  failList |> List.fold_left((text, (failMessage, _)) => text ++ failMessage, "");

let compare = (browser, {commonData, testDataList} as performanceTestData) =>
  Measure.measure(browser, commonData.execCountWhenTest, performanceTestData)
  |> then_(
       (resultList: list((string, 'a))) =>
         resultList
         |> List.fold_left(
              (compareResultList, (actualTestName, actualResultCaseList)) =>
                switch (_getBenchmarkData(actualTestName, commonData)) {
                | (benchmarkTestName, _) when benchmarkTestName !== actualTestName =>
                  ExceptionHandleSystem.throwMessage(
                    {j|benchmarkTestName:$benchmarkTestName should === actualTestName:$actualTestName|j}
                  )
                | (_, benchmarkResultCaseList) =>
                  List.fold_left2(
                    (
                      failList,
                      (
                        actualCaseName,
                        actualErrorRate,
                        actualTimestamp,
                        actualTimeTextList,
                        actualTimeList,
                        actualMemory,
                        actualCase
                      ),
                      (
                        benchmarkCaseName,
                        benchmarkErrorRate,
                        benchmarkTimestamp,
                        benchmarkTimeTextList,
                        benchmarkTimeList,
                        benchmarkMemory
                      )
                    ) =>
                      actualCaseName !== benchmarkCaseName ?
                        ExceptionHandleSystem.throwMessage(
                          {j|actual caseName:$actualCaseName should === benchmark caseName:$benchmarkCaseName|j}
                        ) :
                        actualErrorRate !== benchmarkErrorRate ?
                          ExceptionHandleSystem.throwMessage(
                            {j|actual errorRate:$actualErrorRate should === benchmark errorRate:$benchmarkErrorRate|j}
                          ) :
                          (
                            switch (
                              ""
                              |> _compareTime(
                                   actualTimeList,
                                   actualTimeTextList,
                                   benchmarkTimeList,
                                   actualErrorRate
                                 )
                              |> _compareMemory(actualMemory, benchmarkMemory, actualErrorRate)
                            ) {
                            | failMessage when _isFail(failMessage) => [
                                (
                                  PerformanceTestDataUtils.buildCaseTitle(
                                    actualTestName,
                                    actualCaseName
                                  )
                                  ++ failMessage,
                                  (actualTestName, actualCase)
                                ),
                                ...failList
                              ]
                            | _ => failList
                            }
                          ),
                    [],
                    actualResultCaseList,
                    benchmarkResultCaseList
                    |> List.filter(
                         ((benchmarkCaseName, _, _, _, _, _)) =>
                           actualResultCaseList
                           |> List.exists(
                                ((actualCaseName, _, _, _, _, _, _)) =>
                                  actualCaseName === benchmarkCaseName
                              )
                       )
                  )
                },
              []
            )
         |> resolve
     );