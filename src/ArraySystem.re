open WonderCommonlib.ArrayService;

let zip = (arr) => {
  let itemArr = range(0, (arr[0] |> Js.Array.length) - 1) |> Js.Array.map((_) => [||]);
  arr
  |> Js.Array.reducei(
       (itemArr, item, index) => {
         item
         |> Js.Array.forEachi((value, i) => itemArr[i] |> Js.Array.push(value) |> ignore)
         |> ignore;
         itemArr
       },
       itemArr
     )
};