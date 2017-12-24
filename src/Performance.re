[@bs.val] external root : Js.t({..}) = "window";

[@bs.val] external chrome : Js.t({..}) = "";

/* [@bs.module "chrome"] external loadTimes : unit => Js.t({..}) = ""; */
let getNow = (root) : float => {
  /* DebugUtils.log(root##performance) |> ignore; */
  let now = root##performance##now;
  [@bs] now()
};

let getStartLoadTime = [%bs.raw
  {|
function(window){
  console.log(window.performance);
        if (window.PerformanceNavigationTiming && window.performance.timeOrigin) {
          var ntEntry = performance.getEntriesByType('navigation')[0];
          return (ntEntry.startTime + performance.timeOrigin) / 1000;
        } else {
          return window.performance.timing.navigationStart / 1000;
        }
}
  |}
];