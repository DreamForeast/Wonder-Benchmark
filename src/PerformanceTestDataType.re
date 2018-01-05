type case = {
  name: string,
  bodyFuncStr: string,
  scriptFilePathList: option(list(string)),
  errorRate: int
};

type commonData = {
  isClosePage: bool,
  execCountWhenTest: int,
  execCountWhenGenerateBenchmark: int,
  benchmarkPath: string,
  scriptFilePathList: list(string)
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