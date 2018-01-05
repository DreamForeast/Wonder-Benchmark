open PerformanceTestDataType;

open Node;

/* todo refactor with render test */
let getRelativeFilePath = (fromAbsoluteFilePath, toAbsoluteFilePath) => {
  let relativePath =
    Path.relative(~from=Path.dirname(fromAbsoluteFilePath), ~to_=toAbsoluteFilePath, ());
  if (! Js.String.startsWith("./", relativePath) && ! Js.String.startsWith("../", relativePath)) {
    "./" ++ relativePath
  } else {
    relativePath
  }
};

let _getAllScriptFilePathList = (commonScriptFilePathList, scriptFilePathList) : list(string) =>
  switch scriptFilePathList {
  | None => commonScriptFilePathList
  | Some(scriptFilePathList) => commonScriptFilePathList @ scriptFilePathList
  };

let buildImportScriptStr = (targetAbsoluteFilePath, commonScriptFilePathList, scriptFilePathList) =>
  _getAllScriptFilePathList(commonScriptFilePathList, scriptFilePathList)
  |> List.map((scriptFilePath) => getRelativeFilePath(targetAbsoluteFilePath, scriptFilePath))
  |> List.fold_left(
       (resultStr, scriptFilePath) => resultStr ++ {j|<script src="$scriptFilePath"></script>
|j},
       ""
     );

let buildHeadStr = (title) => {j|<!DOCTYPE html>
   <html lang="en">
   <head>
     <meta charset="UTF-8">
     <title>$title</title>
     <link rel="stylesheet" href="./report.css"/>
   </head>|j};

let buildDebugCssFilePath = (targetAbsoluteReportFilePath) =>
  Path.join([|Path.dirname(targetAbsoluteReportFilePath), "report.css"|]);

let buildFootStr = () => {|</body>
        </html>|};