open Wonder_jest;

let _ =
  describe(
    "test compare",
    () => {
      open Expect;
      open Expect.Operators;
      open Sinon;
      open Js.Promise;
      open PerformanceTestDataType;
      open PerformanceTestData;
      afterAll(
        () =>
          WonderCommonlib.NodeExtend.rmdirFilesSync(
            Node.Path.join([|Node.Process.cwd(), "./test/performance/benchmark"|])
          )
      );
      beforeAllPromise(
        () =>
          PuppeteerUtils.launchHeadlessBrowser()
          |> then_((browser) => GenerateBenchmark.generate(browser, correctPerformanceTestData))
      );
      describe(
        "Comparer",
        () =>
          describe(
            "compare",
            () =>
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
                                  /* failText |> Js.String.includes("expect time:init"), */
                                  failText |> Js.String.includes("expect time:loopBody"),
                                  failText |> Js.String.includes("expect memory")
                                )
                                |> expect == (false, true, false, true, true, true)
                                |> resolve
                              }
                            )
                     )
              )
          )
      );
      describe(
        "TestPerformance",
        () =>
          describe(
            "runTest",
            () => {
              testPromise(
                "compare at most 3 times",
                () =>
                  PuppeteerUtils.launchHeadlessBrowser()
                  |> then_(
                       (browser) =>
                         TestPerformance.runTest([|browser|], wrongPerformanceTestData)
                         |> then_(
                              (failList) => fail("should be fail, but actual is pass") |> resolve
                            )
                         |> catch(
                              (err) => {
                                let (failText, failList) = err |> Obj.magic;
                                /* WonderCommonlib.DebugUtils.log(failText) |> ignore; */
                                (
                                  /* Comparer.isPass(failList), */
                                  failText |> Js.String.includes("pf_test1"),
                                  failText |> Js.String.includes("pf_test2"),
                                  failText |> Js.String.includes("expect time:prepare"),
                                  failText |> Js.String.includes("expect time:loopBody"),
                                  failText |> Js.String.includes("expect memory")
                                )
                                |> expect == (true, false, true, true, true)
                                /* |> expect == (true, false) */
                                |> resolve
                                |> Obj.magic
                              }
                            )
                     )
              );
              describe(
                "test build performanceTestData from failList",
                () => {
                  test(
                    "test one fail case",
                    () => {
                      let failCase = {
                        name: "case1",
                        bodyFuncStr: "",
                        scriptFilePathList: Some(["./script1"]),
                        errorRate: 5
                      };
                      let failList = [("test1_title", ("test1", failCase))];
                      let data =
                        TestPerformanceTool.buildPerformanceTestDataFromFailList(
                          wrongPerformanceTestData.commonData,
                          failList
                        );
                      data
                      |>
                      expect == {
                                  commonData: wrongPerformanceTestData.commonData,
                                  testDataList: [{name: "test1", caseList: [failCase]}]
                                }
                    }
                  );
                  test(
                    "test two fail case of the same test",
                    () => {
                      let failCase1 = {
                        name: "case1",
                        bodyFuncStr: "",
                        scriptFilePathList: Some(["./script1"]),
                        errorRate: 5
                      };
                      let failCase2 = {
                        name: "case2",
                        bodyFuncStr: "",
                        scriptFilePathList: Some(["./script2"]),
                        errorRate: 6
                      };
                      let failList = [
                        ("test1_title1", ("test1", failCase1)),
                        ("test1_title2", ("test1", failCase2))
                      ];
                      let data =
                        TestPerformanceTool.buildPerformanceTestDataFromFailList(
                          wrongPerformanceTestData.commonData,
                          failList
                        );
                      data
                      |>
                      expect == {
                                  commonData: wrongPerformanceTestData.commonData,
                                  testDataList: [{name: "test1", caseList: [failCase1, failCase2]}]
                                }
                    }
                  );
                  test(
                    "test fail cases of the different test",
                    () => {
                      let failCase1 = {
                        name: "case1",
                        bodyFuncStr: "",
                        scriptFilePathList: Some(["./script1"]),
                        errorRate: 5
                      };
                      let failCase2 = {
                        name: "case2",
                        bodyFuncStr: "",
                        scriptFilePathList: Some(["./script2"]),
                        errorRate: 6
                      };
                      let failCase3 = {
                        name: "case3",
                        bodyFuncStr: "",
                        scriptFilePathList: Some(["./script3"]),
                        errorRate: 7
                      };
                      let failCase4 = {
                        name: "case4",
                        bodyFuncStr: "",
                        scriptFilePathList: Some(["./script4"]),
                        errorRate: 8
                      };
                      let failList = [
                        ("test1_title1", ("test1", failCase1)),
                        ("test2_title1", ("test2", failCase2)),
                        ("test2_title2", ("test2", failCase3)),
                        ("test3_title1", ("test3", failCase4))
                      ];
                      let data =
                        TestPerformanceTool.buildPerformanceTestDataFromFailList(
                          wrongPerformanceTestData.commonData,
                          failList
                        );
                      data
                      |>
                      expect == {
                                  commonData: wrongPerformanceTestData.commonData,
                                  testDataList: [
                                    {name: "test1", caseList: [failCase1]},
                                    {name: "test2", caseList: [failCase2, failCase3]},
                                    {name: "test3", caseList: [failCase4]}
                                  ]
                                }
                    }
                  )
                }
              )
            }
          )
      )
    }
  );