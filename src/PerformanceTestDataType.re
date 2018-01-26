type case = {
  name: string,
  bodyFuncStr: string,
  errorRate: int
};

/*
 type copiedScriptData = {
   sourceScriptFilePath: string,
   targetScriptFilePath: string
 }; */
/* type reCompare = {
     compareCount: int,
     /* execCountWhenTest: int,
     execCountWhenGenerateBenchmark: int */
   }; */
type commonData = {
  isClosePage: bool,
  execCountWhenTest: int,
  execCountWhenGenerateBenchmark: int,
  compareCount: int,
  maxAllowDiffTimePercent: int,
  maxAllowDiffMemoryPercent: int,
  benchmarkPath: string,
  /* copiedScriptDataListWhenGenerateBenchmark: option(list(copiedScriptData)), */
  /* generateBaseDebugData: option(list(copiedScriptData)), */
  baseDir: string,
  /* generateBaseDebugData: option(Js.Dict.t(string)), */
  scriptFilePathList: list(string),
  replaceBodyFuncStrWhenDebug: option((string => string))
};

type testData = {
  name: string,
  caseList: list(case)
};

type performanceTestData = {
  commonData,
  testDataList: list(testData)
};

type bodyFuncReturnValue = {. "textArray": array(string), "timeArray": array(float)};