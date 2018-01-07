open PerformanceTestDataType;

open Node;

/* todo refactor with render test */
let getRelativeFilePath = (targetAbsoluteFileDir, toAbsoluteFilePath) => {
  let relativePath = Path.relative(~from=targetAbsoluteFileDir, ~to_=toAbsoluteFilePath, ());
  if (! Js.String.startsWith("./", relativePath) && ! Js.String.startsWith("../", relativePath)) {
    "./" ++ relativePath
  } else {
    relativePath
  }
};

let buildImportScriptStr = (targetAbsoluteFileDir, scriptFilePathList) =>
  scriptFilePathList
  |> List.map((scriptFilePath) => getRelativeFilePath(targetAbsoluteFileDir, scriptFilePath))
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
   </head>|j};

let buildDebugCssFilePath = (targetAbsoluteFileDir) =>
  Path.join([|targetAbsoluteFileDir, "report.css"|]);

let buildFootStr = () => {|</body>
        </html>|};