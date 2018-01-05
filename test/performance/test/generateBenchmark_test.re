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
      beforeEach(
        () =>
          NodeExtend.rmdirFilesSync(Path.join([|Process.cwd(), "./test/performance/benchmark"|]))
      );
      testPromise(
        "test generate benchmark data to specific dir",
        () =>
          PerformanceTestData.(
            PuppeteerUtils.launchHeadlessBrowser()
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
      )
    }
  );