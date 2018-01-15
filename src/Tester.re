open PerformanceTestDataType;

open Js.Promise;

let generateBenchmark = (performanceTestData) =>
  WonderBsPuppeteer.PuppeteerUtils.launchHeadlessBrowser()
  |> then_(
       (browser) =>
         GenerateBenchmark.generate(
           browser,
           ScriptFileUtils.getAllScriptFilePathList(performanceTestData.commonData),
           performanceTestData
         )
     );

let generateBenchmarkWithBrowser = (browser, performanceTestData) =>
  GenerateBenchmark.generate(
    browser,
    ScriptFileUtils.getAllScriptFilePathList(performanceTestData.commonData),
    performanceTestData
  );

let copyBaseScript = (performanceTestData) =>
  /* GenerateBaseDebugUtils.isGenerateBaseDebugData(performanceTestData) ?
     GenerateBaseDebugUtils.copyBaseScript(performanceTestData) : (); */
  GenerateBaseDebugUtils.copyBaseScript(performanceTestData);

let generateReport = (reportFilePath, failList, performanceTestData) =>
  make(
    (~resolve, ~reject) => {
      GenerateReport.generateHtmlFile(reportFilePath, (performanceTestData, failList));
      [@bs]
      resolve(
        GenerateDebug.generateHtmlFiles(
          Node.Path.dirname(reportFilePath),
          performanceTestData,
          failList
        )
      )
    }
  );

let compare = (browser, {commonData, testDataList} as performanceTestData) =>
  [@bs]
  Comparer.compare(
    browser,
    ScriptFileUtils.getAllScriptFilePathList(commonData),
    performanceTestData
  );

/* let generateReport = (reportFilePath, compareResultData) =>
   GenerateReport.generateHtmlFile(reportFilePath, compareResultData)
   |> then_(
        (htmlStr) => {
          GenerateDebug.generateHtmlFiles(reportFilePath, compareResultData);
          reportFilePath |> resolve
        }
      ); */
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

let _compareSpecificCount =
    (
      browser,
      compareCount,
      allTargetScriptFilePathList,
      allBaseScriptFilePathList,
      compareFunc,
      generateCaseFunc,
      performanceTestData
    ) => {
  let rec _compare =
          (
            browser,
            compareCount,
            allTargetScriptFilePathList,
            allBaseScriptFilePathList,
            {commonData} as performanceTestData,
            needReCompareFailList,
            resultFailList
          ) =>
    switch compareCount {
    | count when compareCount === 0 => resultFailList @ needReCompareFailList |> resolve
    | _ =>
      WonderLog.Log.log("compare...") |> ignore;
      let {execCountWhenGenerateBenchmark, maxAllowDiffTimePercent, maxAllowDiffMemoryPercent} = commonData;
      [@bs] compareFunc(browser, allTargetScriptFilePathList, performanceTestData)
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
                 switch (needReCompareFailList |> List.length) {
                 | 0 => resultFailList @ notNeedReCompareFailList |> resolve
                 | _ =>
                   notNeedReCompareFailList |> List.length > 0 ?
                     {
                       WonderLog.Log.log("fail cases which not need re compare:")
                       |> ignore;
                       WonderLog.Log.log(
                         Comparer.getFailText(notNeedReCompareFailList)
                       )
                       |> ignore
                     } :
                     ();
                   needReCompareFailList |> List.length > 0 ?
                     {
                       WonderLog.Log.log("fail cases which need re compare:") |> ignore;
                       WonderLog.Log.log(Comparer.getFailText(needReCompareFailList))
                       |> ignore
                     } :
                     ();
                   needReCompareFailList
                   |> List.fold_left(
                        (promise, (_, (testName, case, _, _))) =>
                          promise
                          |> then_(
                               (browser) =>
                                 [@bs]
                                 generateCaseFunc(
                                   browser,
                                   execCountWhenGenerateBenchmark,
                                   allBaseScriptFilePathList,
                                   commonData,
                                   testName,
                                   case
                                 )
                             ),
                        browser |> resolve
                      )
                   |> then_(
                        (browser) =>
                          _compare(
                            browser,
                            compareCount - 1,
                            allTargetScriptFilePathList,
                            allBaseScriptFilePathList,
                            _buildPerformanceTestDataFromFailList(
                              commonData,
                              needReCompareFailList
                            ),
                            needReCompareFailList,
                            resultFailList @ notNeedReCompareFailList
                          )
                      )
                 }
               }
         )
    };
  _compare(
    browser,
    compareCount,
    allTargetScriptFilePathList,
    allBaseScriptFilePathList,
    performanceTestData,
    [],
    []
  )
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
         |> _compareSpecificCount(
              browser,
              commonData.compareCount,
              ScriptFileUtils.getAllScriptFilePathList(commonData),
              GenerateBaseDebugUtils.getAllScriptFilePathList(performanceTestData),
              Comparer.compare,
              GenerateBenchmark.generateCase
            )
         |> then_(
              (data) =>
                WonderBsPuppeteer.PuppeteerUtils.closeBrowser(browser)
                |> then_((_) => data |> resolve)
            )
         |> then_(
              (failList) =>
                Comparer.isPass(failList) ?
                  failList |> resolve :
                  (Comparer.getFailText(failList), failList) |> Obj.magic |> reject
            )
     );