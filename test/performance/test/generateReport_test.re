open Wonder_jest;

let _ =
  describe(
    "test generate report",
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
          WonderBsPuppeteer.PuppeteerUtils.launchHeadlessBrowser()
          |> then_((browser) => GenerateBenchmark.generate(browser, correctPerformanceTestData))
      );
      testPromise(
        "generate report html file and css file",
        () => {
          let reportFilePath = Path.join([|Process.cwd(), "./test/report/report.html"|]);
          WonderBsPuppeteer.PuppeteerUtils.launchHeadlessBrowser()
          |> then_(
               (browser) =>
                 [@bs] Comparer.compare(browser, wrongPerformanceTestData)
                 |> then_(
                      (compareResultData) => {
                        Tester.generateReport(
                          reportFilePath,
                          compareResultData,
                          wrongPerformanceTestData
                        );
                        (
                          Fs.existsSync(reportFilePath),
                          Fs.existsSync(
                            Path.join([|reportFilePath |> Path.dirname, "report.css"|])
                          )
                        )
                        |> expect == (true, true)
                        |> resolve
                      }
                    )
             )
        }
      )
    }
  );