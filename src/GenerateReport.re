open PerformanceTestDataType;

open Node;

open Js.Promise;

let _buildHeadStr = () => GenerateHtmlFile.buildHeadStr("performance test");

let _buildFootStr = () => {|</body>
        </html>|};

let _buildFailCaseListHtmlStr = (targetAbsoluteFilePath, compareResultList) =>
  compareResultList
  |> Comparer.getFailCaseText
  |> List.fold_left(
       (resultStr, (testName, caseName, text)) => {
         let debugFilePath = "./" ++ GenerateDebug.buildDebugHtmlFileName(testName, caseName);
         let title = PerformanceTestDataUtils.buildCaseTitle(testName, caseName);
         let htmlText = text |> Js.String.replaceByRe([%re {|/\n/g|}], "<br/>");
         resultStr
         ++ {j|<section>
                    <a href="$debugFilePath" target="_blank"><h3>$title</h3></a>
                    <p>$htmlText</p>
                </section>
                    |j}
       },
       ""
     );

let _generateCssFile = (filePath) => {||} |> WonderCommonlib.NodeExtend.writeFile(filePath);

let _getAllScriptFilePathList = (testDataList) =>
  Some(
    testDataList
    |> List.fold_left(
         (resultList, {caseList}) =>
           caseList
           |> List.fold_left(
                (resultList, {scriptFilePathList}: case) =>
                  switch scriptFilePathList {
                  | None => resultList
                  | Some(scriptFilePathList) => resultList @ scriptFilePathList
                  },
                resultList
              ),
         []
       )
  );

let generateHtmlFile =
    (targetAbsoluteFilePath: string, ({commonData, testDataList}, compareResultList)) => {
  let htmlStr =
    _buildHeadStr()
    ++ "\n<body>\n"
    ++ GenerateHtmlFile.buildImportScriptStr(
         targetAbsoluteFilePath,
         commonData.scriptFilePathList,
         _getAllScriptFilePathList(testDataList)
       )
    ++ _buildFailCaseListHtmlStr(targetAbsoluteFilePath, compareResultList)
    ++ GenerateHtmlFile.buildFootStr();
  htmlStr |> WonderCommonlib.NodeExtend.writeFile(targetAbsoluteFilePath);
  _generateCssFile(GenerateHtmlFile.buildDebugCssFilePath(targetAbsoluteFilePath |> Path.dirname))
};