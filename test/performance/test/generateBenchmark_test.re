open Wonder_jest;

let _ =
  describe(
    "test generate benchmark data",
    () => {
      open Expect;
      open Expect.Operators;
      open Sinon;
      open Js.Promise;
      open Node;
      open PerformanceTestDataType;
      afterAll(
        () => {
          WonderCommonlib.NodeExtend.rmdirFilesSync(
            Path.join([|Process.cwd(), "./test/performance/benchmark"|])
          );
          WonderCommonlib.NodeExtend.rmdirFilesSync(Path.join([|Process.cwd(), "./test/base"|]))
        }
      );
      testPromise(
        "test generate benchmark data to specific dir",
        () =>
          PerformanceTestData.(
            WonderBsPuppeteer.PuppeteerUtils.launchHeadlessBrowser()
            |> then_(
                 (browser) =>
                   GenerateBenchmark.generate(browser, correctPerformanceTestData)
                   |> then_(
                        (_) =>
                          Fs.existsSync("./test/performance/benchmark/basic1.json")
                          |> expect == true
                          |> resolve
                      )
               )
          )
      );
      testPromise(
        "test copy base scripts for generate base debug files",
        () =>
          PerformanceTestData.(
            WonderBsPuppeteer.PuppeteerUtils.launchHeadlessBrowser()
            |> then_(
                 (browser) =>
                   GenerateBenchmark.generate(browser, wrongPerformanceTestData2)
                   |> then_(
                        (_) => Fs.existsSync("./test/base/wd_base.js") |> expect == true |> resolve
                      )
               )
          )
      )
    }
  );