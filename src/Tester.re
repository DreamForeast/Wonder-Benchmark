open PerformanceTestDataType;

open Js.Promise;

let generateBenchmark = (performanceTestData) =>
  WonderBsPuppeteer.PuppeteerUtils.launchHeadlessBrowser()
  |> then_((browser) => GenerateBenchmark.generate(browser, performanceTestData));

let generateReport = (debugFilePath, failList, performanceTestData) =>
  make(
    (~resolve, ~reject) =>
      [@bs]
      resolve(
        GenerateDebug.generateHtmlFiles(
          Node.Path.dirname(debugFilePath),
          performanceTestData,
          failList
        )
      )
  );

let _isNotExceedMaxDiffPercent =
    (maxAllowDiffTimePercent, maxAllowDiffMemoryPercent, diffTimePercentList, diffMemoryPercent) =>
  ! (
    diffTimePercentList
    |> List.map((diff) => diff |> Js.Math.abs_int)
    |> List.exists((diff) => diff >= maxAllowDiffTimePercent)
  )
  && diffMemoryPercent
  |> Js.Math.abs_int < maxAllowDiffMemoryPercent;

let _filterFailCases = (maxAllowDiffTimePercent, maxAllowDiffMemoryPercent, failList) =>
  failList
  |> List.filter(
       ((_, (testName, case, diffTimePercentList, diffMemoryPercent))) =>
         ! (
           diffTimePercentList
           |> List.map((diff) => diff |> Js.Math.abs_int)
           |> List.exists((diff) => diff >= maxAllowDiffTimePercent)
         )
         && diffMemoryPercent
         |> Js.Math.abs_int < maxAllowDiffMemoryPercent
     );

let _buildPerformanceTestDataFromFailList = (commonData, failList) => {
  let (_, (firstTestName, firstCase, _, _)) = List.hd(failList);
  {
    commonData,
    testDataList:
      failList
      |> List.tl
      |> List.fold_left(
           ((testDataList, caseList, lastTestName), (_, (testName, case, _, _))) =>
             lastTestName === testName ?
               (testDataList, caseList @ [case], testName) :
               (testDataList @ [({name: lastTestName, caseList}: testData)], [case], testName),
           ([], [firstCase], firstTestName)
         )
      |> (
        ((testDataList, caseList, lastTestName)) =>
          testDataList @ [({name: lastTestName, caseList}: testData)]
      )
  }
};

let _compareSpecificCount = (browser, count, compareFunc, performanceTestData) => {
  let rec _compare =
          (
            browser,
            count,
            {commonData} as performanceTestData,
            needReCompareFailList,
            resultFailList
          ) =>
    switch count {
    | count when count === 0 => resultFailList @ needReCompareFailList |> resolve
    | _ =>
      WonderCommonlib.DebugUtils.log("compare...") |> ignore;
      let {maxAllowDiffTimePercent, maxAllowDiffMemoryPercent} = commonData;
      [@bs] compareFunc(browser, performanceTestData)
      |> then_(
           (failList) =>
             Comparer.isPass(failList) ?
               failList |> resolve :
               {
                 let needReCompareFailList =
                   failList
                   |> List.filter(
                        ((_, (testName, case, diffTimePercentList, diffMemoryPercent))) =>
                          _isNotExceedMaxDiffPercent(
                            maxAllowDiffTimePercent,
                            maxAllowDiffMemoryPercent,
                            diffTimePercentList,
                            diffMemoryPercent
                          )
                      );
                 let notNeedReCompareFailList =
                   failList
                   |> List.filter(
                        ((_, (testName, case, diffTimePercentList, diffMemoryPercent))) =>
                          !
                            _isNotExceedMaxDiffPercent(
                              maxAllowDiffTimePercent,
                              maxAllowDiffMemoryPercent,
                              diffTimePercentList,
                              diffMemoryPercent
                            )
                      );
                 /* WonderCommonlib.DebugUtils.logJson(("need:", needReCompareFailList)) |> ignore;
                 WonderCommonlib.DebugUtils.logJson(("not need:", notNeedReCompareFailList))
                 |> ignore; */
                 switch (needReCompareFailList |> List.length) {
                 | 0 => resultFailList @ notNeedReCompareFailList |> resolve
                 | _ =>
                   _compare(
                     browser,
                     count - 1,
                     _buildPerformanceTestDataFromFailList(commonData, needReCompareFailList),
                     needReCompareFailList,
                     resultFailList @ notNeedReCompareFailList
                   )
                 }
               }
         )
    };
  _compare(browser, count, performanceTestData, [], [])
};

let runTest = (browserArr, {commonData} as performanceTestData) =>
  (
    switch (browserArr |> Js.Array.length) {
    | 0 => WonderBsPuppeteer.PuppeteerUtils.launchHeadlessBrowser()
    | _ => browserArr[0] |> resolve
    }
  )
  |> then_(
       (browser) =>
         performanceTestData
         |> _compareSpecificCount(browser, commonData.compareCount, Comparer.compare)
         |> then_(
              (data) =>
                WonderBsPuppeteer.PuppeteerUtils.closeBrowser(browser)
                |> then_((_) => data |> resolve)
            )
         |> then_(
              (failList) => {
                /* WonderCommonlib.DebugUtils.logJson(("failList:", failList)) |> ignore; */
                Comparer.isPass(failList) ?
                  failList |> resolve :
                  (Comparer.getFailText(failList), failList) |> Obj.magic |> reject
              }
            )
     ) /* failList |> resolve */;