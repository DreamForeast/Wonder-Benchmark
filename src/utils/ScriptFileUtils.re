let getAllScriptFilePathList = (commonScriptFilePathList, scriptFilePathList) : list(string) =>
  switch scriptFilePathList {
  | None => commonScriptFilePathList
  | Some(scriptFilePathList) => commonScriptFilePathList @ scriptFilePathList
  };