open PerformanceTestDataType;

open Node;

open Js.Promise;

let _getType = () => "target";

let buildDebugHtmlFileName = (testName, caseName) =>
  GenerateDebugFileUtils.buildDebugHtmlFileName(testName, caseName, _getType());

let _removeTargetDebugFiles = (targetAbsoluteFileDir) =>
  GenerateDebugFileUtils.removeDebugFiles(targetAbsoluteFileDir);

let removeDebugFiles = (targetAbsoluteFileDir, performanceTestData) => {
  targetAbsoluteFileDir |> _removeTargetDebugFiles;
  GenerateBaseDebugUtils.removeDebugFiles(performanceTestData)
};

let generateHtmlFiles =
    (targetAbsoluteFileDir: string, {commonData} as performanceTestData, compareResultList) =>
  compareResultList
  |> List.iter(
       ((_, (testName, {name, bodyFuncStr, scriptFilePathList}, _, _))) => {
         GenerateDebugFileUtils.generate(
           targetAbsoluteFileDir,
           (
             testName,
             name,
             ScriptFileUtils.getAllScriptFilePathList(
               commonData.scriptFilePathList,
               scriptFilePathList
             ),
             bodyFuncStr,
             _getType()
           )
         );
         GenerateBaseDebugUtils.isGenerateBaseDebugData(performanceTestData) ?
           GenerateBaseDebugUtils.generateBaseDebugFile(
             targetAbsoluteFileDir,
             (testName, name, scriptFilePathList, bodyFuncStr),
             performanceTestData
           ) :
           ()
       }
     );