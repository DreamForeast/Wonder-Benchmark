open Wonder_jest;

open Benchmark;

open WonderCommonlib;

let _ =
  describe(
    "Benchmark",
    () => {
      open Expect;
      open Expect.Operators;
      open Sinon;
      open Puppeteer;
      open Js.Promise;
      let sandbox = getSandboxDefaultVal();
      let state = ref(createEmptyState());
      let browser = ref(None);
      let page = ref(None);
      beforeAllPromise(
        () =>
          launch(
            ~options={
              "ignoreHTTPSErrors": Js.Nullable.empty,
              "executablePath": Js.Nullable.empty,
              "slowMo": Js.Nullable.empty,
              "args": Js.Nullable.empty,
              /* "args": Js.Nullable.return([|"--headless", "--hide-scrollbars", "--mute-audio"|]), */
              "handleSIGINT": Js.Nullable.empty,
              "timeout": Js.Nullable.empty,
              "dumpio": Js.Nullable.empty,
              "userDataDir": Js.Nullable.empty,
              "headless": Js.Nullable.return(Js.false_)
            },
            ()
          )
          |> then_(
               (b) => {
                 browser := Some(b);
                 b |> Browser.newPage
               }
             )
          |> then_(
               (p) => {
                 page := Some(p);
                 state :=
                   createState(
                     p,
                     browser^ |> Js.Option.getExn,
                     ["./test/res/script1.js"],
                     "data.json"
                   );
                 p |> resolve
               }
             )
      );
      afterAllPromise(() => browser^ |> Js.Option.getExn |> Browser.close);
      beforeEach(() => sandbox := createSandbox());
      afterEach(() => restoreSandbox(refJsObjToSandbox(sandbox^)));
      describe(
        "test performance by compare with json data",
        () => {
          testPromiseWithTimeout(
            "test exec and compare",
            () => {
              let body = [%bs.raw
                {| function() {
                    function test(){
                        var arr = [];
                        for(var i = 0; i <= 100000; i++){
                        arr[i] = wd.add(1, 2);
                        }
                    }
var n1 = performance.now();
test();

var n2 = performance.now();

test();


var n3 = performance.now();


test();
test();


var n4 = performance.now();


return {"errorRate": 5, "textArray": ["prepare", "init", "loopBody"], "timeArray": [n1, n2, n3, n4] }
}
|}
              ];
              state^ |> exec("pf_test", [@bs] body) |> compare((expect, toBe))
            },
            16000
          );
          describe(
            "test add more script",
            () => {
              testPromiseWithTimeout(
                "test addScript",
                () => {
                  let body = [%bs.raw
                    {| function() {
                    function test(){
                        var arr = [];
                        for(var i = 0; i <= 100000; i++){
                        arr[i] = minus(3, wd.add(1, 2));
                        }
                    }
var n1 = performance.now();
test();

var n2 = performance.now();

test();


var n3 = performance.now();


test();
test();


var n4 = performance.now();


return {"errorRate": 5, "textArray": ["prepare", "init", "loopBody"], "timeArray": [n1, n2, n3, n4] }
}
|}
                  ];
                  state^
                  |> addScript("./test/res/script2.js")
                  |> exec("pf_test", [@bs] body)
                  |> compare((expect, toBe))
                },
                16000
              );
              testPromiseWithTimeout(
                "test addScriptList",
                () => {
                  let body = [%bs.raw
                    {| function() {
                    function test(){
                        var arr = [];
                        for(var i = 0; i <= 100000; i++){
                        arr[i] = minus2(10, minus(3, wd.add(1, 2)));
                        }
                    }
var n1 = performance.now();
test();

var n2 = performance.now();

test();


var n3 = performance.now();


test();
test();


var n4 = performance.now();


return {"errorRate": 5, "textArray": ["prepare", "init", "loopBody"], "timeArray": [n1, n2, n3, n4] }
}
|}
                  ];
                  state^
                  |> addScriptList(["./test/res/script2.js", "./test/res/script3.js"])
                  |> exec("pf_test", [@bs] body)
                  |> compare((expect, toBe))
                },
                16000
              )
            }
          )
        }
      );
      testPromise(
        "test get median value",
        () =>
          BenchmarkType.
            (
              BenchmarkTool.getMedian(
                (
                  [|
                    {
                      errorRate: 5,
                      timestamp: 100,
                      timeArray: [|3, 1|],
                      timeTextArray: [|"a", "b"|]
                    },
                    {
                      errorRate: 6,
                      timestamp: 10,
                      timeArray: [|98, 2|],
                      timeTextArray: [|"a", "b"|]
                    },
                    {
                      errorRate: 50,
                      timestamp: 11,
                      timeArray: [|5, 88|],
                      timeTextArray: [|"a", "b"|]
                    }
                  |],
                  [|100, 1, 3|]
                )
                |> resolve
              )
              |> then_((data) => data |> expect == (5, 11, [|"a", "b"|], [|5, 2|], 3) |> resolve)
            )
      )
    }
  );