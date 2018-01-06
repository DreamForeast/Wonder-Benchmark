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
      afterAll(() => WonderCommonlib.NodeExtend.rmdirFilesSync(Path.join([|Process.cwd(), "./test/debug"|])));
      beforeAllPromise(
        () =>
          PuppeteerUtils.launchHeadlessBrowser()
          |> then_((browser) => GenerateBenchmark.generate(browser, correctPerformanceTestData))
      );
      testPromise(
        "generate debug html files",
        () => {
          let debugFileDir = Path.join([|Process.cwd(), "./test/debug/"|]);
          PuppeteerUtils.launchHeadlessBrowser()
          |> then_(
               (browser) =>
                 Comparer.compare(browser, wrongPerformanceTestData)
                 |> then_(
                      (failList) => {
                        GenerateDebug.generateHtmlFiles(
                          debugFileDir,
                          wrongPerformanceTestData,
                          failList
                        );
                        (
                          Fs.existsSync(
                            Path.join([|
                              Process.cwd(),
                              "./test/debug/basic1_pf_test1_debug.html"
                            |])
                          ),
                          Fs.existsSync(
                            Path.join([|
                              Process.cwd(),
                              "./test/debug/basic1_pf_test2_debug.html"
                            |])
                          )
                        )
                        |> expect == (true, false)
                        |> resolve
                      }
                    )
             )
        }
      );
      describe(
        "replace body script",
        () =>
          testPromise(
            "use wd.startDirector instead of wd.initDirector, wd.loopBody",
            () => {
              let debugFileDir = Path.join([|Process.cwd(), "./test/debug/"|]);
              PuppeteerUtils.launchHeadlessBrowser()
              |> then_(
                   (browser) =>
                     Comparer.compare(browser, wrongPerformanceTestData)
                     |> then_(
                          (failList) => {
                            GenerateDebug.generateHtmlFiles(
                              debugFileDir,
                              wrongPerformanceTestData,
                              failList
                            );
                            let debugFileContent =
                              Fs.readFileAsUtf8Sync(
                                Path.join([|
                                  Process.cwd(),
                                  "./test/debug/basic1_pf_test1_debug.html"
                                |])
                              );
                            (
                              debugFileContent |> Js.String.includes("wd.startDirector(state);"),
                              debugFileContent |> Js.String.includes("wd.initDirector(state);"),
                              debugFileContent |> Js.String.includes("wd.loopBody")
                            )
                            |> expect == (true, false, false)
                            |> resolve
                          }
                        )
                 )
            }
          )
      )
    }
  );