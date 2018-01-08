open PerformanceTestDataType;

open Node;

open GenerateDebugFileUtils;

let isGenerateBaseDebugData = ({commonData}) =>
  commonData.generateBaseDebugData |> Js.Option.isSome;

let copyBaseScript = ({commonData}) =>
  commonData.generateBaseDebugData
  |> Js.Option.getExn
  |> List.iter(
       ({sourceScriptFilePath, targetScriptFilePath}) =>
         /* WonderCommonlib.DebugUtils.log(targetScriptFilePath) |> ignore */
         WonderCommonlib.NodeExtend.writeFile(
           targetScriptFilePath,
           Fs.readFileAsUtf8Sync(sourceScriptFilePath)
         )
     );

let removeDebugFiles = ({commonData}) =>
  commonData.generateBaseDebugData
  |> Js.Option.getExn
  |> List.iter(
       ({sourceScriptFilePath, targetScriptFilePath}) => {
         WonderCommonlib.DebugUtils.log(("target:", targetScriptFilePath)) |> ignore;
         WonderCommonlib.NodeExtend.rmdirFilesSync(targetScriptFilePath |> Path.dirname)
       }
     );

let _getSourceScriptFilePathList = (generateBaseDebugData) =>
  generateBaseDebugData |> List.map(({sourceScriptFilePath}) => sourceScriptFilePath);

let _getTargetScriptFilePathList = (generateBaseDebugData) =>
  generateBaseDebugData |> List.map(({targetScriptFilePath}) => targetScriptFilePath);

let _getType = () => "base";

let buildDebugHtmlFileName = (testName, caseName) =>
  GenerateDebugFileUtils.buildDebugHtmlFileName(testName, caseName, _getType());

let _getAllScriptFilePathList = (scriptFilePathList, {commonData}) : list(string) =>
  ScriptFileUtils.getAllScriptFilePathList(commonData.scriptFilePathList, scriptFilePathList)
  |> List.map(
       (scriptFilePath) =>
         switch (
           commonData.generateBaseDebugData
           |> Js.Option.getExn
           |> List.filter(
                ({sourceScriptFilePath, targetScriptFilePath}) =>
                  sourceScriptFilePath === scriptFilePath
              )
         ) {
         | matchedList when matchedList |> List.length === 0 => scriptFilePath
         | matchedList => (matchedList |> List.hd).targetScriptFilePath
         }
     );

let generateBaseDebugFile =
    (
      targetAbsoluteFileDir,
      (testName, name, scriptFilePathList, bodyFuncStr),
      {commonData} as performanceTestData
    ) =>
  generate(
    targetAbsoluteFileDir,
    (
      testName,
      name,
      _getAllScriptFilePathList(scriptFilePathList, performanceTestData),
      bodyFuncStr,
      _getType()
    )
  );