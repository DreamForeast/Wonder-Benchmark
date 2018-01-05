let _buildTimeCaseStr = (timeTextList, timeList) =>
  Json.Encode.(
    List.fold_left2(
      (list, text, time) => [
        [("name", text |> string), ("time", time |> int)] |> object_ |> Js.Json.stringify,
        ...list
      ],
      [],
      timeTextList,
      timeList
    )
    |> Array.of_list
    |> Js.Array.joinWith(",")
  );

let _generateCaseData = ((caseName, errorRate, timestamp, timeTextList, timeList, memory, _)) => {
  let timeCaseStr = _buildTimeCaseStr(timeTextList, timeList);
  {j|
           {
               "name": "$caseName",
               "timestamp":$timestamp,
               "time_detail": [
                   $timeCaseStr
               ],
               "memory": $memory,
               "error_rate": $errorRate
           },
       |j}
};

let buildDataFileName = (name) => name ++ ".json";

let convertToJsonStr = (testName, resultCaseList) => {
  let fileName = buildDataFileName(testName);
  let caseDataStr =
    resultCaseList |> List.fold_left((str, caseData) => str ++ _generateCaseData(caseData), "");
  let caseDataStr =
    "["
    ++ (caseDataStr |> Js.String.slice(~from=0, ~to_=caseDataStr |> Js.String.lastIndexOf(",")))
    ++ "]";
  {j|{
         "name": "$fileName",
         "cases": $caseDataStr
         }|j}
};

/* (caseName, errorRate, timestamp, timeTextList, timeList, memory) */
let convertToData = (json) =>
  Json.Decode.(
    json |> field("name", string),
    json
    |> field(
         "cases",
         (json) =>
           json
           |> array(
                (json) => (
                  json |> field("name", string),
                  json |> field("error_rate", int),
                  json |> field("timestamp", int),
                  json
                  |> field(
                       "time",
                       (json) =>
                         json |> array((json) => json |> field("name", string)) |> Array.to_list
                     ),
                  json
                  |> field(
                       "time",
                       (json) =>
                         json |> array((json) => json |> field("time", int)) |> Array.to_list
                     ),
                  json |> field("memory", int)
                )
              )
           |> Array.to_list
       )
  );