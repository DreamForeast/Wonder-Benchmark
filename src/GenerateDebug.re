open PerformanceTestDataType;

open Node;

open Js.Promise;

let _buildDebugScriptStr = (bodyFuncStr) => {j|<script>
            window.onload = function () {
                $bodyFuncStr
            };
           </script>|j};

let _buildDebugHtmlFileName = (testName, caseName) => {j|$(testName)_$(caseName)_debug.html|j};

let _buildDebugHtmlFilePath = (targetAbsoluteReportFilePath, testName, caseName) =>
  Path.join([|
    Path.dirname(targetAbsoluteReportFilePath),
    _buildDebugHtmlFileName(testName, caseName)
  |]);

let _generateCssFile = (filePath) =>
  {|img.correct-img, img.current-img, img.diff-img{
  width:33%;
  height:33%;
};|}
  |> NodeExtend.writeFile(filePath);

/* todo refactor render test: comparer not return test data! */
let generateHtmlFiles =
    (targetAbsoluteReportFilePath: string, performanceTestData, compareResultList) => {
  compareResultList
  |> List.iter(
       ((_, (testName, {name, bodyFuncStr, scriptFilePathList}))) => {
         let htmlStr =
           GenerateHtmlFile.buildHeadStr(_buildDebugHtmlFileName(testName, name))
           ++ "\n<body>\n"
           ++ GenerateHtmlFile.buildImportScriptStr(
                targetAbsoluteReportFilePath,
                performanceTestData.commonData.scriptFilePathList,
                scriptFilePathList
              )
           ++ _buildDebugScriptStr(bodyFuncStr)
           ++ GenerateHtmlFile.buildFootStr();
         htmlStr
         |> NodeExtend.writeFile(
              _buildDebugHtmlFilePath(targetAbsoluteReportFilePath, testName, name)
            )
       }
     );
  /* todo fix render test: generate css */
  _generateCssFile(GenerateHtmlFile.buildDebugCssFilePath(targetAbsoluteReportFilePath))
};