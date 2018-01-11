open PerformanceTestDataType;

open Sinon;

let buildPerformanceTestDataFromFailList = Tester._buildPerformanceTestDataFromFailList;

let compareSpecificCount =
    (
      browser,
      compareCount,
      compareFunc,
      generateCaseBenchmarkFunc,
      {commonData} as performanceTestData
    ) =>
  Tester._compareSpecificCount(
    browser,
    compareCount,
    ScriptFileUtils.getAllScriptFilePathList(commonData),
    GenerateBaseDebugUtils.getAllScriptFilePathList(performanceTestData),
    compareFunc,
    generateCaseBenchmarkFunc,
    performanceTestData
  );