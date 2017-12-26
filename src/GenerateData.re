open BenchmarkType;

open Node;

let unsafeGetFilePath = (state) =>
  BenchmarkStateUtils.getConfig(state).generateDataFilePath |> Js.Option.getExn;

let getConfig = (config) => {
  ...config,
  execCount: config.execCount * 2,
  extremeCount: config.extremeCount * 2
};

let needGenerateData = (generateDataFilePath) => generateDataFilePath |> Js.Option.isSome;

let _buildTimeCaseStr = (actualTimeDataArray) =>
  /* actualTimeDataArray |> Js.Array.reduce((caseArr, (name, time)) => {
         caseArr |> Js.Array.push({
             "name": name,
             "time": time
         }) |> ignore;
         caseArr;
     }, [||]) |> Json.Encode.object_ |> Js.Json.stringify */
  Json.Encode.(
    actualTimeDataArray
    /* |> WonderCommonlib.DebugUtils.logJson */
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

let buildTimeArr = (timeTextArray:array(string), actualTimeArray) =>
  actualTimeArray
  |> Js.Array.mapi((time, index) => (timeTextArray[index], time));

let _getActualCaseDataList = (state) => state.actualCaseDataList;

/* let buildCaseDataStr = (name, actualTimeDataArray, actualMemory, errorRate, state) => { */
let writeCaseDataStr = (name, actualTimeDataArray, actualMemory, errorRate, state) => {
  let filePath = unsafeGetFilePath(state);
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
  /* {j|
         {
             "name": "$name",
             "time": [
                 $timeCaseStr
             ],
             "memory": "$actualMemory",
             "error_rate": "$errorRate"
         }
     |j} */
};

let generateDataFile = (state) => {
  let filePath = unsafeGetFilePath(state);
  /* WonderCommonlib.DebugUtils.log("111") |> ignore; */
  /* WonderCommonlib.DebugUtils.logJson(state) |> ignore; */
  let fileName = Path.basename_ext(filePath, ".json");
  /* let casesArr =
     "[" ++ (state.actualCaseDataList |> Array.of_list |> Js.Array.joinWith(",")) ++ "]"; */
  /* failwith(state.actualCaseDataList |> String.of_list); */
  /* WonderCommonlib.DebugUtils.log(casesArr) |> ignore; */
  let caseDataStr = Fs.readFileAsUtf8Sync(filePath);
  /* failwith(caseDataStr); */

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

let cleanDataFile = (generateDataFilePath: string) =>
  Fs.writeFileAsUtf8Sync(generateDataFilePath, "");