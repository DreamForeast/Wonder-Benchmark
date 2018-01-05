open Wonder_jest;

let _ =
  describe(
    "test compare",
    () => {
      open Expect;
      open Expect.Operators;
      open Sinon;
      open Js.Promise;
      open Node;
      open PerformanceTestDataType;
      open PerformanceTestData;
      beforeAllPromise(
        () =>
          PuppeteerUtils.launchHeadlessBrowser()
          |> then_((browser) => GenerateBenchmark.generate(browser, correctPerformanceTestData))
      );
      testPromise(
        "test compare current and benchmark",
        () =>
          PuppeteerUtils.launchHeadlessBrowser()
          |> then_(
               (browser) =>
                 Comparer.compare(browser, wrongPerformanceTestData)
                 |> then_(
                      (failList) => {
                        let failText = Comparer.getFailText(failList);
                        /* WonderCommonlib.DebugUtils.log(failText) |> ignore; */
                        (
                          Comparer.isPass(failList),
                          failText |> Js.String.includes("pf_test1"),
                          failText |> Js.String.includes("pf_test2"),
                          failText |> Js.String.includes("expect time:prepare"),
                          failText |> Js.String.includes("expect time:init"),
                          failText |> Js.String.includes("expect time:loopBody"),
                          failText |> Js.String.includes("expect memory")
                        )
                        |> expect == (false, false, true, true, true, true, true)
                        |> resolve
                      }
                    )
             )
      )
    }
  );