open Wonder_jest;

let _ =
  describe(
    "test generate debug",
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
        "generate debug html files",
        () => {
          let reportFilePath = Path.join([|Process.cwd(), "./test/report/report.html"|]);
          PuppeteerUtils.launchHeadlessBrowser()
          |> then_(
               (browser) =>
                 Comparer.compare(browser, wrongPerformanceTestData)
                 |> then_(
                      (failList) => {
                        GenerateDebug.generateHtmlFiles(reportFilePath, wrongPerformanceTestData, failList);
                        (
                          Fs.existsSync(
                            Path.join([|
                              Process.cwd(),
                              "./test/report/basic1_pf_test1_debug.html"
                            |])
                          ),
                          Fs.existsSync(
                            Path.join([|
                              Process.cwd(),
                              "./test/report/basic1_pf_test2_debug.html"
                            |])
                          )
                        )
                        |> expect == (false, true)
                        |> resolve
                      }
                    )
             )
        }
      )
    }
  );