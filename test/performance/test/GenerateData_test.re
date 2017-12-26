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
      beforeEach(() => sandbox := createSandbox());
      afterEach(() => restoreSandbox(refJsObjToSandbox(sandbox^)));
      testPromiseWithTimeout(
        "test generate data",
        () => {
          let generateDataFilePath = "./test/res/data.json";
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
                     ~config={
                       isClosePage: true,
                       execCount: 3,
                       extremeCount: 1,
                       generateDataFilePath: Some("./test/res/data.json")
                     },
                     p,
                     browser^ |> Js.Option.getExn,
                     "./test/res/script1.js",
                     "data.json"
                   )
                   |> prepareBeforeAll;
                 p |> resolve
               }
             )
          |> then_(
               (_) => {
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
                 state^ |> exec("pf_test1", [@bs] body) |> compare((expect, toBe))
               }
             )
          |> then_(
               (_) => {
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


return {"errorRate": 10, "textArray": ["prepare", "init"], "timeArray": [n1, n2, n3] }
}
|}
                 ];
                 state^ |> exec("pf_test2", [@bs] body) |> compare((expect, toBe))
               }
             )
          |> then_(
               (_) =>
                 browser^
                 |> Js.Option.getExn
                 |> Browser.close
                 |> then_((_) => Benchmark.generateDataFile(state^) |> resolve)
                 |> then_(
                      (_) => {
                        open Node;
                        let content = Fs.readFileAsUtf8Sync(generateDataFilePath);
                        (
                          content |> Js.String.includes({|"name": "pf_test1"|}),
                          content |> Js.String.includes({|"name": "pf_test2"|})
                        )
                        |> expect == (true, true)
                        |> resolve
                      }
                    )
             )
        },
        16000
      )
    }
  );