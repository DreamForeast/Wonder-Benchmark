open Js.Promise;

open Node;

open PerformanceTestDataType;

open BenchmarkDataType;

/*
 let _buildTimeCaseStr = (timeTextList, timeList) =>
   Json.Encode.(
     List.fold_left2(
       (list, text, time) => [
         [("name", name |> string), ("time", time |> int)] |> object_ |> Js.Json.stringify,
         ...list
       ],
       timeTextList,
       timeList
     )
     |> Array.of_list
     |> Js.Array.joinWith(",")
   ); */
/* let convertStateJsonToRecord = (page, browser, scriptFilePathList, dataFilePath, config: config) =>
   Json.(
     Decode.{
       config,
       page,
       browser,
       scriptFilePathList,
       dataFilePath,
       name: "",
       caseList: [],
       result: None
     }
   ); */
let _buildDataFilePath = (benchmarkPath, fileName) => Path.join([|benchmarkPath, fileName|]);

let getDataFilePath = (testName, {benchmarkPath}) =>
  _buildDataFilePath(benchmarkPath, BenchmarkDataConverter.buildDataFileName(testName));

let removeFiles = (benchmarkDir) =>
  Fs.existsSync(benchmarkDir) ? WonderCommonlib.NodeExtend.rmdirFilesSync(benchmarkDir) : ();

let _generateBenchmark = (benchmarkPath, testName, resultCaseList) => {
  let filePath =
    _buildDataFilePath(benchmarkPath, BenchmarkDataConverter.buildDataFileName(testName));
  WonderCommonlib.DebugUtils.log({j|generate benchmark:$filePath...|j}) |> ignore;
  BenchmarkDataConverter.convertToJsonStr(testName, resultCaseList)
  |> WonderCommonlib.DebugUtils.log
  |> WonderCommonlib.NodeExtend.writeFile(filePath)
};

let generateCase =
  [@bs]
  (
    (browser, execCountWhenGenerateBenchmark, allScriptFilePathList, commonData, testName, case) => {
      let {benchmarkPath} = commonData;
      []
      |> resolve
      |> Measure.measureCase(
           browser,
           execCountWhenGenerateBenchmark,
           allScriptFilePathList,
           commonData,
           testName,
           case
         )
      |> then_(
           (resultCaseList) => {
             _generateBenchmark(benchmarkPath, testName, resultCaseList);
             browser |> resolve
           }
         )
    }
  );

let generate = (browser, allScriptFilePathList, {commonData} as performanceTestData) => {
  let {benchmarkPath, execCountWhenGenerateBenchmark} = commonData;
  Measure.measure(
    browser,
    execCountWhenGenerateBenchmark,
    allScriptFilePathList,
    performanceTestData
  )
  |> then_(
       (resultList) => {
         resultList
         |> List.iter(
              ((testName, resultCaseList)) =>
                _generateBenchmark(benchmarkPath, testName, resultCaseList)
            );
         browser |> resolve
       }
     )
};