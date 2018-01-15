open PerformanceTestDataType;

open Node;

open GenerateDebugFileUtils;

/* |> List.iter(
     ({sourceScriptFilePath, targetScriptFilePath}) => {
       WonderLog.Log.log(
         {j|copy $(sourceScriptFilePath) to $(targetScriptFilePath)...|j}
       )
       |> ignore;
       WonderCommonlib.NodeExtend.writeFile(
         targetScriptFilePath,
         Fs.readFileAsUtf8Sync(sourceScriptFilePath)
       )
     }
   ); */
let removeDebugFiles = (copiedScriptFileDir) =>
  switch copiedScriptFileDir {
  | None => ()
  | Some(fileDir) =>
    Fs.existsSync(fileDir) ? WonderCommonlib.NodeExtend.rmdirFilesSync(fileDir) : ()
  };

/* let _getSourceScriptFilePathList = (generateBaseDebugData) =>
     generateBaseDebugData |> List.map(({sourceScriptFilePath}) => sourceScriptFilePath);

   let _getTargetScriptFilePathList = (generateBaseDebugData) =>
     generateBaseDebugData |> List.map(({targetScriptFilePath}) => targetScriptFilePath); */
let _getType = () => "base";

let buildDebugHtmlFileName = (testName, caseName) =>
  GenerateDebugFileUtils.buildDebugHtmlFileName(testName, caseName, _getType());

let _getBaseDir = ({baseDir}) => baseDir;

let _getBaseScriptFilePath = (baseDir, scriptFilePath) =>
  /* WonderLog.Log.log(baseDir) |> ignore;  */
  /* WonderLog.Log.log(scriptFilePath) |> ignore;  */
  Path.join([|baseDir, scriptFilePath|]);

/* |> WonderLog.Log.log */
/* let _getCaseAllScriptFilePathList = (scriptFilePathList, {commonData}) : list(string) => {
     let baseDir = _getBaseDir(commonData);
     ScriptFileUtils.getAllScriptFilePathList(commonData)
     |> List.map((scriptFilePath) => _getBaseScriptFilePath(baseDir, scriptFilePath))
   }; */
let getAllScriptFilePathList = ({commonData} as performanceTestData) : list(string) => {
  let baseDir = _getBaseDir(commonData);
  ScriptFileUtils.getAllScriptFilePathList(commonData)
  |> List.map((scriptFilePath) => _getBaseScriptFilePath(baseDir, scriptFilePath))
};

let _getAllBaseAndTargetScriptFilePathList = ({commonData} as performanceTestData) => {
  let baseDir = _getBaseDir(commonData);
  ScriptFileUtils.getAllScriptFilePathList(commonData)
  |> List.map(
       (scriptFilePath) => (_getBaseScriptFilePath(baseDir, scriptFilePath), scriptFilePath)
     )
};

/* let isGenerateBaseDebugData = ({commonData}) => commonData.baseDir |> Js.Option.isSome; */
let copyBaseScript = ({commonData, testDataList} as performanceTestData) =>
  performanceTestData
  |> _getAllBaseAndTargetScriptFilePathList
  |> List.iter(
       ((baseScriptFilePath, targetScriptFilePath)) =>
         Fs.readFileAsUtf8Sync(targetScriptFilePath)
         |> WonderCommonlib.NodeExtend.writeFile(baseScriptFilePath)
     );

let generateBaseDebugFile =
    (targetAbsoluteFileDir, (testName, name, bodyFuncStr), {commonData} as performanceTestData) =>
  generate(
    targetAbsoluteFileDir,
    (testName, name, getAllScriptFilePathList(performanceTestData), bodyFuncStr, _getType())
  );