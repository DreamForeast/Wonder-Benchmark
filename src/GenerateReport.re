open PerformanceTestDataType;

open Node;

open Js.Promise;

let _buildHeadStr = () => GenerateHtmlFile.buildHeadStr("performance test");

let _buildFootStr = () => {|</body>
        </html>|};

let _buildFailCaseListHtmlStr = (targetAbsoluteFilePath, compareResultList, performanceTestData) =>
  compareResultList
  |> Comparer.getFailCaseTextList
  |> List.fold_left(
       (resultStr, (testName, caseName, text)) => {
         let targetDebugFilePath =
           "./" ++ GenerateDebug.buildDebugHtmlFileName(testName, caseName);
         let baseDebugFilePath =
           "./" ++ GenerateBaseDebugUtils.buildDebugHtmlFileName(testName, caseName);
         let title = PerformanceTestDataUtils.buildCaseTitle(testName, caseName);
         let htmlText = text |> Js.String.replaceByRe([%re {|/\n/g|}], "<br/>");
         resultStr
         ++ {j|<section>
                    <h3>$title</h3>
                    <a href="$baseDebugFilePath" target="_blank"><h4>baseDebug</h4></a>
                    <a href="$targetDebugFilePath" target="_blank"><h4>targetDebug</h4></a>
                    <p>$htmlText</p>
                </section>
                    |j}
       },
       ""
     );

let _generateCssFile = (filePath) => {||} |> WonderCommonlib.NodeExtend.writeFile(filePath);

let _getAllScriptFilePathList = (commonData) =>
  ScriptFileUtils.getAllScriptFilePathList(commonData);

let removeFile = (reportFilePath) =>
  Fs.existsSync(reportFilePath) ?
    WonderCommonlib.NodeExtend.rmdirFilesSync(reportFilePath |> Path.dirname) : ();

let generateHtmlFile =
    (targetAbsoluteFilePath: string, ({commonData} as performanceTestData, compareResultList)) => {
  let htmlStr =
    _buildHeadStr()
    ++ "\n<body>\n"
    ++ GenerateHtmlFile.buildImportScriptStr(
         targetAbsoluteFilePath,
         ScriptFileUtils.getAllScriptFilePathList(commonData)
       )
    ++ _buildFailCaseListHtmlStr(targetAbsoluteFilePath, compareResultList, performanceTestData)
    ++ GenerateHtmlFile.buildFootStr();
  htmlStr |> WonderCommonlib.NodeExtend.writeFile(targetAbsoluteFilePath);
  _generateCssFile(GenerateHtmlFile.buildDebugCssFilePath(targetAbsoluteFilePath |> Path.dirname))
};