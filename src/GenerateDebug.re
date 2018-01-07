open PerformanceTestDataType;

open Node;

open Js.Promise;

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

let buildDebugHtmlFileName = (testName, caseName) => {j|$(testName)_$(caseName)_debug.html|j};

let _buildDebugHtmlFilePath = (targetAbsoluteFileDir, testName, caseName) =>
  Path.join([|targetAbsoluteFileDir, buildDebugHtmlFileName(testName, caseName)|]);

let generateHtmlFiles = (targetAbsoluteFileDir: string, performanceTestData, compareResultList) => {
  compareResultList
  |> List.iter(
       ((_, (testName, {name, bodyFuncStr, scriptFilePathList}, _, _))) => {
         let htmlStr =
           GenerateHtmlFile.buildHeadStr(buildDebugHtmlFileName(testName, name))
           ++ "\n<body>\n"
           ++ GenerateHtmlFile.buildImportScriptStr(
                targetAbsoluteFileDir,
                performanceTestData.commonData.scriptFilePathList,
                scriptFilePathList
              )
           ++ _buildDebugScriptStr(bodyFuncStr)
           ++ GenerateHtmlFile.buildFootStr();
         htmlStr
         |> WonderCommonlib.NodeExtend.writeFile(_buildDebugHtmlFilePath(targetAbsoluteFileDir, testName, name))
       }
     );
};