open PerformanceTestDataType;

open Js.Promise;

let generateBenchmark = (performanceTestData) =>
  PuppeteerUtils.launchHeadlessBrowser()
  |> then_((browser) => GenerateBenchmark.generate(browser, performanceTestData));

let _buildPerformanceTestDataFromFailList = (commonData, failList) => {
  let (_, (firstTestName, firstCase)) = List.hd(failList);
  {
    commonData,
    testDataList:
      failList
      |> List.tl
      |> List.fold_left(
           ((testDataList, caseList, lastTestName), (_, (testName, case))) =>
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

let _compareSpecificCount = (browser, count, performanceTestData) => {
  let rec _compare = (browser, count, {commonData} as performanceTestData, resultFailList) =>
    switch count {
    | count when count === 0 => resultFailList |> resolve
    | _ =>
      Comparer.compare(browser, performanceTestData)
      |> then_(
           (failList) =>
             Comparer.isPass(failList) ?
               failList |> resolve :
               _compare(
                 browser,
                 count - 1,
                 _buildPerformanceTestDataFromFailList(commonData, failList),
                 /* |> WonderCommonlib.DebugUtils.logJson, */
                 failList
               )
         )
    };
  _compare(browser, count, performanceTestData, [])
};

let runTest = (browserArr, performanceTestData) =>
  (
    switch (browserArr |> Js.Array.length) {
    | 0 => PuppeteerUtils.launchHeadlessBrowser()
    | _ => browserArr[0] |> resolve
    }
  )
  |> then_(
       (browser) =>
         performanceTestData
         |> _compareSpecificCount(browser, 3)
         |> then_((data) => PuppeteerUtils.closeBrowser(browser) |> then_((_) => data |> resolve))
         |> then_(
              (failList) =>
                Comparer.isPass(failList) ?
                  failList |> resolve : Comparer.getFailText(failList) |> Obj.magic |> reject
            )
     );