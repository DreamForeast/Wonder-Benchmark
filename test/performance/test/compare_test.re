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
          GenerateBenchmark.removeFiles(
            Node.Path.join([|Node.Process.cwd(), "./test/performance/benchmark"|])
          )
      );
      beforeAllPromise(() => Tester.generateBenchmark(correctPerformanceTestData));
      describe(
        "Comparer",
        () =>
          describe(
            "compare",
            () => {
              testPromise(
                "test compare current and benchmark",
                () =>
                  WonderBsPuppeteer.PuppeteerUtils.launchHeadlessBrowser()
                  |> then_(
                       (browser) =>
                         Tester.compare(browser, wrongPerformanceTestData)
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
              );
              describe(
                "fix bug",
                () =>
                  testPromise(
                    "if current run faster than benchmark, diff should be -xxx%",
                    () =>
                      WonderBsPuppeteer.PuppeteerUtils.launchHeadlessBrowser()
                      |> then_(
                           (browser) =>
                             Tester.compare(browser, wrongPerformanceTestData2)
                             |> then_(
                                  (failList) => {
                                    let failText = Comparer.getFailText(failList);
                                    (
                                      Comparer.isPass(failList),
                                      failText |> Js.String.includes("pf_test2"),
                                      Js.Re.test(failText, [%re {|/-\d+%/g|}]),
                                      Js.Re.test(failText, [%re {|/--\d+%/g|}])
                                    )
                                    |> expect == (false, true, true, false)
                                    |> resolve
                                  }
                                )
                         )
                  )
              )
            }
          )
      );
      describe(
        "Tester",
        () =>
          describe(
            "runTest",
            () => {
              let _buildFailCase = (~name, ~bodyFuncStr="", ~errorRate=5, ()) => {
                name,
                bodyFuncStr,
                errorRate
              };
              let _buildFailList =
                  (testTitle, testName, case, diffTimePercentList, diffMemory, failList) =>
                failList @ [(testTitle, (testName, case, diffTimePercentList, diffMemory))];
              describe(
                "compare at most 3 times",
                () =>
                  describe(
                    "re-generate benchmark before compare after first compare",
                    () => {
                      let _restoreBenchmark = (promise) =>
                        promise
                        |> then_(
                             (failList) =>
                               Tester.generateBenchmark(correctPerformanceTestData)
                               |> then_((_) => failList |> resolve)
                           );
                      /* afterEach(
                                                 () =>
                         Tester.generateBenchmark(correctPerformanceTestData)
                                   /* GenerateBenchmark.removeFiles(
                                     Node.Path.join([|Node.Process.cwd(), "./test/performance/benchmark"|])
                                   ) */
                                               ); */
                      testPromise(
                        "test pass",
                        () =>
                          WonderBsPuppeteer.PuppeteerUtils.launchHeadlessBrowser()
                          |> then_(
                               (browser) => {
                                 Tester.copyBaseScript(wrongPerformanceTestData);
                                 Tester.runTest([|browser|], wrongPerformanceTestData)
                                 |> _restoreBenchmark
                                 |> then_((failList) => pass |> resolve)
                                 |> catch(
                                      (err) => {
                                        WonderCommonlib.DebugUtils.log(err) |> ignore;
                                        fail("should be pass, but actual is fail") |> resolve
                                      }
                                    )
                               }
                             )
                      );
                      testPromise(
                        "test fail",
                        () =>
                          WonderBsPuppeteer.PuppeteerUtils.launchHeadlessBrowser()
                          |> then_(
                               (browser) => {
                                 Tester.copyBaseScript(wrongPerformanceTestData);
                                 let _setFakeBaseScript = () =>
                                   Node.Fs.writeFileAsUtf8Sync(
                                     "./test/base/test/res/script1.js",
                                     {|

var testScript = {
    add: (a, b) => {
      var sum = 0;
      for(var i = 0; i < 10000; i++){

        sum = sum+ ( a + b );
      }

      return sum;
    }
}
    |}
                                   );
                                 _setFakeBaseScript();
                                 Tester.runTest([|browser|], wrongPerformanceTestData3)
                                 |> _restoreBenchmark
                                 |> then_((failList) => pass |> resolve)
                                 |> catch(
                                      (err) => {
                                        let (failText, failList) = err |> Obj.magic;
                                        (
                                          failText |> Js.String.includes("pf_test1"),
                                          failText |> Js.String.includes("pf_test2")
                                        )
                                        |> expect == (false, true)
                                        |> resolve
                                        |> Obj.magic
                                      }
                                    )
                               }
                             )
                      )
                    }
                  )
              );
              describe(
                "test re-generate fail cases' benchmark",
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
                           [wrongPerformanceTestData.commonData.maxAllowDiffTimePercent - 1],
                           1
                         );
                    (failCase1, failCase2, failList)
                  };
                  let _buildTestData = (baseDir, scriptFilePathList, performanceTestData) => {
                    ...performanceTestData,
                    commonData: {...performanceTestData.commonData, scriptFilePathList, baseDir}
                  };
                  testPromise(
                    "use base scripts",
                    () => {
                      let (_, _, failList) = _prepare();
                      let _fakeCompare = createEmptyStubWithJsObjSandbox(sandbox);
                      _fakeCompare
                      |> onCall(0)
                      |> returns(make((~resolve, ~reject) => [@bs] resolve(failList)));
                      let _fakeGenerateCaseBenchmark = createEmptyStubWithJsObjSandbox(sandbox);
                      TesterTool.compareSpecificCount(
                        Obj.magic(1),
                        1,
                        _fakeCompare,
                        _fakeGenerateCaseBenchmark,
                        _buildTestData(
                          "./test/base",
                          ["./test/res/a.js", "./test/res/aaa/b.js"],
                          wrongPerformanceTestData
                        )
                      )
                      |> then_(
                           (resultFailList) =>
                             List.nth(_fakeGenerateCaseBenchmark |> getCall(0) |> getArgs, 2)
                             |>
                             expect == ["test/base/test/res/a.js", "test/base/test/res/aaa/b.js"]
                             |> resolve
                         )
                    }
                  )
                }
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
                    let _fakeGenerateCaseBenchmark = createEmptyStubWithJsObjSandbox(sandbox);
                    TesterTool.compareSpecificCount(
                      Obj.magic(1),
                      1,
                      _fakeCompare,
                      _fakeGenerateCaseBenchmark,
                      wrongPerformanceTestData
                    )
                    |> then_((resultFailList) => resultFailList |> expect == failList |> resolve)
                  };
                  let _testCompareTwice = (prepareFunc) => {
                    let (failCase1, failCase2, failList) = prepareFunc();
                    let _fakeCompare = createEmptyStubWithJsObjSandbox(sandbox);
                    _fakeCompare
                    |> onCall(0)
                    |> returns(make((~resolve, ~reject) => [@bs] resolve(failList)));
                    _fakeCompare
                    |> onCall(1)
                    |> returns(make((~resolve, ~reject) => [@bs] resolve([List.nth(failList, 1)])));
                    let _fakeGenerateCaseBenchmark = createEmptyStubWithJsObjSandbox(sandbox);
                    TesterTool.compareSpecificCount(
                      Obj.magic(1),
                      2,
                      _fakeCompare,
                      _fakeGenerateCaseBenchmark,
                      wrongPerformanceTestData
                    )
                    |> then_(
                         (resultFailList) =>
                           (
                             resultFailList,
                             (
                               List.nth(_fakeCompare |> getCall(1) |> getArgs, 2).testDataList
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
                               [wrongPerformanceTestData.commonData.maxAllowDiffTimePercent - 1],
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
                               wrongPerformanceTestData.commonData.maxAllowDiffMemoryPercent - 1
                             );
                        (failCase1, failCase2, failList)
                      };
                      testPromise("test run once", () => _testCompareOnce(_prepare));
                      testPromise("test run twice", () => _testCompareTwice(_prepare))
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