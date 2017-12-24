open WonderCommonlib.ArraySystem;

open Contract;

[@bs.send.pipe : array('a)] external unsafeFind : ('a => [@bs.uncurry] bool) => 'a = "find";

let range = (a: int, b: int) => {
  let result = createEmpty();
  for (i in a to b) {
    Js.Array.push(i, result) |> ignore
  };
  result
};

let reduceOneParam = (func, param, arr) => {
  let mutableParam = ref(param);
  for (i in 0 to Js.Array.length(arr) - 1) {
    mutableParam := [@bs] func(mutableParam^, Array.unsafe_get(arr, i))
  };
  mutableParam^
};

let reduceOneParami = (func, param, arr) => {
  let mutableParam = ref(param);
  for (i in 0 to Js.Array.length(arr) - 1) {
    mutableParam := [@bs] func(mutableParam^, Array.unsafe_get(arr, i), i)
  };
  mutableParam^
};