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
      let sandbox = getSandboxDefaultVal();
      beforeEach(() => sandbox := createSandbox());
      afterEach(() => restoreSandbox(refJsObjToSandbox(sandbox^)));
      afterAll(
        () =>
          WonderCommonlib.NodeExtend.rmdirFilesSync(
            Node.Path.join([|Node.Process.cwd(), "./test/performance/benchmark"|])
          )
      );
      beforeAllPromise(
        () =>
          WonderBsPuppeteer.PuppeteerUtils.launchHeadlessBrowser()
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
                  WonderBsPuppeteer.PuppeteerUtils.launchHeadlessBrowser()
                  |> then_(
                       (browser) =>
                         [@bs] Comparer.compare(browser, wrongPerformanceTestData)
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
        "Tester",
        () =>
          describe(
            "runTest",
            () => {
              let _buildFailCase =
                  (
                    ~name,
                    ~bodyFuncStr="",
                    ~scriptFilePathList=Some(["./script"]),
                    ~errorRate=5,
                    ()
                  ) => {
                name,
                bodyFuncStr,
                scriptFilePathList,
                errorRate
              };
              let _buildFailList =
                  (testTitle, testName, case, diffTimePercentList, diffMemory, failList) =>
                failList @ [(testTitle, (testName, case, diffTimePercentList, diffMemory))];
              describe(
                "compare at most 3 times",
                () => {
                  testPromise(
                    "test",
                    () =>
                      WonderBsPuppeteer.PuppeteerUtils.launchHeadlessBrowser()
                      |> then_(
                           (browser) =>
                             Tester.runTest([|browser|], wrongPerformanceTestData)
                             |> then_(
                                  (failList) =>
                                    fail("should be fail, but actual is pass") |> resolve
                                )
                             |> catch(
                                  (err) => {
                                    let (failText, failList) = err |> Obj.magic;
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
                    "if compare diff percent exceed max allow diff percent, not re-compare it and regard it as fail case",
                    () => {
                      let _testCompareOnce = (prepareFunc) => {
                        let (_, _, failList) = prepareFunc();
                        let _fakeCompare = createEmptyStubWithJsObjSandbox(sandbox);
                        _fakeCompare
                        |> onCall(0)
                        |> returns(make((~resolve, ~reject) => [@bs] resolve(failList)));
                        TesterTool.compareSpecificCount(
                          Obj.magic(1),
                          1,
                          _fakeCompare,
                          wrongPerformanceTestData
                        )
                        |> then_(
                             (resultFailList) => resultFailList |> expect == failList |> resolve
                           )
                      };
                      let _testCompareTwice = (prepareFunc) => {
                        let (failCase1, failCase2, failList) = prepareFunc();
                        let _fakeCompare = createEmptyStubWithJsObjSandbox(sandbox);
                        _fakeCompare
                        |> onCall(0)
                        |> returns(make((~resolve, ~reject) => [@bs] resolve(failList)));
                        _fakeCompare
                        |> onCall(1)
                        |> returns(
                             make((~resolve, ~reject) => [@bs] resolve([List.nth(failList, 1)]))
                           );
                        TesterTool.compareSpecificCount(
                          Obj.magic(1),
                          2,
                          _fakeCompare,
                          wrongPerformanceTestData
                        )
                        |> then_(
                             (resultFailList) =>
                               (
                                 resultFailList,
                                 (
                                   List.nth(_fakeCompare |> getCall(1) |> getArgs, 1).testDataList
                                   |> List.hd
                                 ).
                                   name
                               )
                               |> expect == (failList, "test2")
                               |> resolve
                           )
                      };
                      describe(
                        "test diff time exceed",
                        () => {
                          let _prepare = () => {
                            let failCase1 = _buildFailCase(~name="case1", ());
                            let failCase2 = _buildFailCase(~name="case2", ());
                            let failList =
                              []
                              |> _buildFailList(
                                   "test1_title1",
                                   "test1",
                                   failCase1,
                                   [wrongPerformanceTestData.commonData.maxAllowDiffTimePercent],
                                   1
                                 )
                              |> _buildFailList(
                                   "test2_title1",
                                   "test2",
                                   failCase2,
                                   [
                                     wrongPerformanceTestData.commonData.maxAllowDiffTimePercent - 1
                                   ],
                                   1
                                 );
                            (failCase1, failCase2, failList)
                          };
                          testPromise("test run once", () => _testCompareOnce(_prepare));
                          testPromise("test run twice", () => _testCompareTwice(_prepare))
                        }
                      );
                      describe(
                        "test diff memory exceed",
                        () => {
                          let _prepare = () => {
                            let failCase1 = _buildFailCase(~name="case1", ());
                            let failCase2 = _buildFailCase(~name="case2", ());
                            let failList =
                              []
                              |> _buildFailList(
                                   "test1_title1",
                                   "test1",
                                   failCase1,
                                   [1],
                                   wrongPerformanceTestData.commonData.maxAllowDiffMemoryPercent
                                 )
                              |> _buildFailList(
                                   "test2_title1",
                                   "test2",
                                   failCase2,
                                   [1],
                                   wrongPerformanceTestData.commonData.maxAllowDiffMemoryPercent
                                   - 1
                                 );
                            (failCase1, failCase2, failList)
                          };
                          testPromise("test run once", () => _testCompareOnce(_prepare));
                          testPromise("test run twice", () => _testCompareTwice(_prepare))
                        }
                      )
                    }
                  )
                }
              );
              describe(
                "test build performanceTestData from failList",
                () => {
                  test(
                    "test one fail case",
                    () => {
                      let failCase1 = _buildFailCase(~name="case1", ());
                      let failList =
                        [] |> _buildFailList("test1_title1", "test1", failCase1, [1], 1);
                      let data =
                        TesterTool.buildPerformanceTestDataFromFailList(
                          wrongPerformanceTestData.commonData,
                          failList
                        );
                      data
                      |>
                      expect == {
                                  commonData: wrongPerformanceTestData.commonData,
                                  testDataList: [{name: "test1", caseList: [failCase1]}]
                                }
                    }
                  );
                  test(
                    "test two fail case of the same test",
                    () => {
                      let failCase1 = _buildFailCase(~name="case1", ());
                      let failCase2 = _buildFailCase(~name="case2", ());
                      let failList =
                        []
                        |> _buildFailList("test1_title1", "test1", failCase1, [1], 1)
                        |> _buildFailList("test1_title2", "test1", failCase2, [1], 1);
                      let data =
                        TesterTool.buildPerformanceTestDataFromFailList(
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
                      let failCase1 = _buildFailCase(~name="case1", ());
                      let failCase2 = _buildFailCase(~name="case2", ());
                      let failCase3 = _buildFailCase(~name="case3", ());
                      let failCase4 = _buildFailCase(~name="case4", ());
                      let failList =
                        []
                        |> _buildFailList("test1_title1", "test1", failCase1, [1], 1)
                        |> _buildFailList("test2_title1", "test2", failCase2, [1], 1)
                        |> _buildFailList("test2_title2", "test2", failCase3, [1], 1)
                        |> _buildFailList("test3_title1", "test3", failCase4, [1], 1);
                      let data =
                        TesterTool.buildPerformanceTestDataFromFailList(
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