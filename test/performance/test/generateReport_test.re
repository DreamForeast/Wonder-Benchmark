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
      afterEach(
        () =>
          GenerateReport.removeFile(Node.Path.join([|Node.Process.cwd(), "./test/report/report.html"|]))
      );
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
      );
      testPromise(
        "test generate report html file with base and target debug file links and generate base and target debug files",
        () => {
          let reportFilePath = Path.join([|Process.cwd(), "./test/report/report.html"|]);
          WonderBsPuppeteer.PuppeteerUtils.launchHeadlessBrowser()
          |> then_(
               (browser) =>
                 [@bs] Comparer.compare(browser, wrongPerformanceTestData2)
                 |> then_(
                      (compareResultData) => {
                        Tester.generateReport(
                          reportFilePath,
                          compareResultData,
                          wrongPerformanceTestData2
                        );
                        (
                          Fs.existsSync(reportFilePath),
                          Fs.readFileAsUtf8Sync(reportFilePath)
                          |> Js.String.includes("base_debug.html"),
                          Fs.readFileAsUtf8Sync(reportFilePath)
                          |> Js.String.includes("target_debug.html"),
                          Fs.existsSync(
                            Path.join([|
                              Process.cwd(),
                              "./test/report/basic1_pf_test2_base_debug.html"
                            |])
                          ),
                          Fs.existsSync(
                            Path.join([|
                              Process.cwd(),
                              "./test/report/basic1_pf_test2_target_debug.html"
                            |])
                          )
                        )
                        |> expect == (true, true, true, true, true)
                        |> resolve
                      }
                    )
             )
        }
      )
    }
  );