open PerformanceTestDataType;

open Node;

open Js.Promise;

let _getType = () => "target";

let buildDebugHtmlFileName = (testName, caseName) =>
  GenerateDebugFileUtils.buildDebugHtmlFileName(testName, caseName, _getType());

let _removeTargetDebugFiles = (targetAbsoluteFileDir) =>
  GenerateDebugFileUtils.removeDebugFiles(targetAbsoluteFileDir);

let removeFiles = (debugFileDir, copiedScriptFileDir: option(string)) => {
  debugFileDir |> _removeTargetDebugFiles;
  GenerateBaseDebugUtils.removeDebugFiles(copiedScriptFileDir)
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