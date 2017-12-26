open BenchmarkType;

open Node;

let getFilePath = (state) => state.dataFilePath;

let getConfig = (config) => {
  ...config,
  execCount: config.execCount * 2,
  extremeCount: config.extremeCount * 2
};

let getIsGenerateDataFile = (state) => BenchmarkStateUtils.getConfig(state).isGenerateDataFile;

let needGenerateData = (isGenerateDataFile) =>
  switch (CommandTool.hasOption("jest_performance_generate.json"), isGenerateDataFile) {
  | (false, _)
  | (_, false) => false
  | _ => true
  };

let _buildTimeCaseStr = (actualTimeDataArray) =>
  Json.Encode.(
    actualTimeDataArray
    |> Js.Array.reduce(
         (arr, (name, time)) => {
           arr
           |> Js.Array.push(
                [("name", name |> string), ("time", time |> int)] |> object_ |> Js.Json.stringify
              )
           |> ignore;
           arr
         },
         [||]
       )
    |> Js.Array.joinWith(",")
  );

let buildTimeArr = (timeTextArray: array(string), actualTimeArray) =>
  actualTimeArray |> Js.Array.mapi((time, index) => (timeTextArray[index], time));

let writeCaseDataStr = (name, actualTimeDataArray, actualMemory, errorRate, state) => {
  let filePath = getFilePath(state);
  let timeCaseStr = _buildTimeCaseStr(actualTimeDataArray);
  Fs.readFileAsUtf8Sync(filePath)
  ++ {j|
           {
               "name": "$name",
               "time": [
                   $timeCaseStr
               ],
               "memory": $actualMemory,
               "error_rate": $errorRate
           },
       |j}
  |> Fs.writeFileAsUtf8Sync(filePath);
  state
};

let generateDataFile = (state) => {
  let filePath = getFilePath(state);
  let fileName = Path.basename_ext(filePath, ".json");
  let caseDataStr = Fs.readFileAsUtf8Sync(filePath);
  let caseDataStr =
    "["
    ++ (caseDataStr |> Js.String.slice(~from=0, ~to_=caseDataStr |> Js.String.lastIndexOf(",")))
    ++ "]";
  {j|{
         "name": "$fileName",
         "cases": $caseDataStr
         }|j}
  |> Fs.writeFileAsUtf8Sync(filePath);
  state
};

let createEmptyDataFile = (generateDataFilePath: string) =>
  Fs.writeFileAsUtf8Sync(generateDataFilePath, "");

let convertStateJsonToRecord = (page, browser, scriptFilePath, dataFilePath, config: config) =>
  Json.(
    Decode.{
      config,
      page,
      browser,
      scriptFilePathList: [scriptFilePath],
      dataFilePath,
      name: "",
      caseList: [],
      result: None
    }
  );