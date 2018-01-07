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
      afterAll(
        () =>
          WonderCommonlib.NodeExtend.rmdirFilesSync(Path.join([|Process.cwd(), "./test/debug"|]))
      );
      beforeAllPromise(
        () =>
          WonderBsPuppeteer.PuppeteerUtils.launchHeadlessBrowser()
          |> then_((browser) => GenerateBenchmark.generate(browser, correctPerformanceTestData))
      );
      testPromise(
        "only generate target debug html files",
        () => {
          let debugFileDir = Path.join([|Process.cwd(), "./test/debug/"|]);
          WonderBsPuppeteer.PuppeteerUtils.launchHeadlessBrowser()
          |> then_(
               (browser) =>
                 [@bs] Comparer.compare(browser, wrongPerformanceTestData)
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
                              "./test/debug/basic1_pf_test1_target_debug.html"
                            |])
                          ),
                          Fs.existsSync(
                            Path.join([|
                              Process.cwd(),
                              "./test/debug/basic1_pf_test2_target_debug.html"
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
      testPromise(
        "generate base and target debug html files",
        () => {
          let debugFileDir = Path.join([|Process.cwd(), "./test/debug/"|]);
          WonderBsPuppeteer.PuppeteerUtils.launchHeadlessBrowser()
          |> then_(
               (browser) =>
                 [@bs] Comparer.compare(browser, wrongPerformanceTestData)
                 |> then_(
                      (failList) => {
                        GenerateDebug.generateHtmlFiles(
                          debugFileDir,
                          wrongPerformanceTestData2,
                          failList
                        );
                        (
                          (
                            Fs.existsSync(
                              Path.join([|
                                Process.cwd(),
                                "./test/debug/basic1_pf_test1_base_debug.html"
                              |])
                            ),
                            Fs.readFileAsUtf8Sync(
                              Path.join([|
                                Process.cwd(),
                                "./test/debug/basic1_pf_test1_base_debug.html"
                              |])
                            )
                            |> Js.String.includes("../base/wd_base.js")
                          ),
                          (
                            Fs.existsSync(
                              Path.join([|
                                Process.cwd(),
                                "./test/debug/basic1_pf_test1_target_debug.html"
                              |])
                            ),
                            Fs.readFileAsUtf8Sync(
                              Path.join([|
                                Process.cwd(),
                                "./test/debug/basic1_pf_test1_target_debug.html"
                              |])
                            )
                            |> Js.String.includes("../res/wd.js")
                          ),
                          Fs.existsSync(
                            Path.join([|
                              Process.cwd(),
                              "./test/debug/basic1_pf_test2_base_debug.html"
                            |])
                          ),
                          Fs.existsSync(
                            Path.join([|
                              Process.cwd(),
                              "./test/debug/basic1_pf_test2_target_debug.html"
                            |])
                          )
                        )
                        |> expect == ((true, true), (true, true), false, false)
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
              WonderBsPuppeteer.PuppeteerUtils.launchHeadlessBrowser()
              |> then_(
                   (browser) =>
                     [@bs] Comparer.compare(browser, wrongPerformanceTestData)
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
                                  "./test/debug/basic1_pf_test1_target_debug.html"
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