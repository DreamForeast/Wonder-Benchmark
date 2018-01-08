open PerformanceTestDataType;

open Node;

let buildDebugHtmlFileName = (testName, caseName, debugFileType) => {j|$(testName)_$(caseName)_$(debugFileType)_debug.html|j};

let _buildDebugHtmlFilePath = (targetAbsoluteFileDir, testName, caseName, debugFileType) =>
  Path.join([|targetAbsoluteFileDir, buildDebugHtmlFileName(testName, caseName, debugFileType)|]);

let _buildDebugScriptStr = (bodyFuncStr) => {
  let newBodyFuncStr =
    bodyFuncStr
    |> Js.String.replaceByRe([%re {|/^.+wd\.initDirector.+/im|}], "wd.startDirector(state);")
    |> Js.String.replaceByRe([%re {|/^.+wd\.loopBody.+/img|}], "");
  {j|<script>
            window.onload = function () {
                $newBodyFuncStr
            };
           </script>|j}
};

let removeDebugFiles = (targetAbsoluteFileDir) =>
  Fs.existsSync(targetAbsoluteFileDir) ?
    WonderCommonlib.NodeExtend.rmdirFilesSync(targetAbsoluteFileDir) : ();

let generate =
    (targetAbsoluteFileDir, (testName, name, scriptFilePathList, bodyFuncStr, debugFileType)) => {
  let htmlStr =
    GenerateHtmlFile.buildHeadStr(buildDebugHtmlFileName(testName, name))
    ++ "\n<body>\n"
    ++ GenerateHtmlFile.buildImportScriptStr(targetAbsoluteFileDir, scriptFilePathList)
    ++ _buildDebugScriptStr(bodyFuncStr)
    ++ GenerateHtmlFile.buildFootStr();
  htmlStr
  |> WonderCommonlib.NodeExtend.writeFile(
       _buildDebugHtmlFilePath(targetAbsoluteFileDir, testName, name, debugFileType)
     )
};