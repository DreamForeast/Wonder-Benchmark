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
                                /* WonderLog.Log.log(failText) |> ignore; */
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
                  (
                    testTitle,
                    testName,
                    case,
                    diffTimePercentList,
                    passedTimeList,
                    diffMemory,
                    failList
                  ) =>
                failList
                @ [(testTitle, (testName, case, diffTimePercentList, passedTimeList, diffMemory))];
              describe(
                "if one item of timeList is already passed in previous compare, not compare it in the later compare ",
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
                           [wrongPerformanceTestData.commonData.maxAllowDiffTimePercent - 1],
                           [false],
                           1
                         )
                      |> _buildFailList(
                           "test1_title1",
                           "test2",
                           failCase2,
                           [wrongPerformanceTestData.commonData.maxAllowDiffTimePercent - 1],
                           [false],
                           1
                         );
                    (failCase1, failCase2, failList)
                  };
                  test(
                    "test Tester->_updatePassdTimeListMapFromFailList",
                    () => {
                      let failCase1 = _buildFailCase(~name="case1", ());
                      let failCase2 = _buildFailCase(~name="case2", ());
                      let failList1 =
                        []
                        |> _buildFailList(
                             "test1_title1",
                             "test1",
                             failCase1,
                             [1, 1],
                             [false, false],
                             1
                           )
                        |> _buildFailList(
                             "test1_title1",
                             "test1",
                             failCase2,
                             [1, 1],
                             [true, false],
                             1
                           );
                      let failList2 =
                        []
                        |> _buildFailList(
                             "test1_title1",
                             "test1",
                             failCase1,
                             [1, 1],
                             [true, false],
                             1
                           )
                        |> _buildFailList(
                             "test1_title1",
                             "test1",
                             failCase2,
                             [1, 1],
                             [false, true],
                             1
                           );
                      let passedTimeListMap =
                        WonderCommonlib.HashMapSystem.createEmpty()
                        |> Tester._updatePassdTimeListMapFromFailList(failList1)
                        |> Tester._updatePassdTimeListMapFromFailList(failList2);
                      (
                        passedTimeListMap
                        |> WonderCommonlib.HashMapSystem.unsafeGet("test1")
                        |> WonderCommonlib.HashMapSystem.unsafeGet("case1"),
                        passedTimeListMap
                        |> WonderCommonlib.HashMapSystem.unsafeGet("test1")
                        |> WonderCommonlib.HashMapSystem.unsafeGet("case2")
                      )
                      |> expect == ([true, false], [true, true])
                    }
                  );
                  describe(
                    "test Comparer->_compare",
                    () => {
                      let _buildActuablCaseList =
                          (caseName, errorRate, timestamp, timeTextList, timeList, memory, list) =>
                        list
                        @ [
                          (
                            caseName,
                            errorRate,
                            timestamp,
                            timeTextList,
                            timeList,
                            memory,
                            Obj.magic(1)
                          )
                        ];
                      let _buildBenchmarkCaseList =
                          (caseName, errorRate, timestamp, timeTextList, timeList, memory, list) =>
                        list @ [(caseName, errorRate, timestamp, timeTextList, timeList, memory)];
                      test(
                        "if item of time list is already pass before, not compare it",
                        () => {
                          let actualResultCaseList =
                            [] |> _buildActuablCaseList("c1", 1, 1, ["t1", "t2"], [10, 25], 10);
                          let benchmarkResultCaseList =
                            [] |> _buildBenchmarkCaseList("c1", 1, 2, ["t1", "t2"], [15, 20], 10);
                          let passedTimeListMap =
                            WonderCommonlib.HashMapSystem.createEmpty()
                            |> WonderCommonlib.HashMapSystem.set(
                                 "test1",
                                 WonderCommonlib.HashMapSystem.createEmpty()
                                 |> WonderCommonlib.HashMapSystem.set("c1", [true, false])
                               );
                          let failList =
                            Comparer._compare(
                              passedTimeListMap,
                              "test1",
                              actualResultCaseList,
                              benchmarkResultCaseList
                            );
                          let (
                            failMessage,
                            (
                              actualTestName,
                              actualCase,
                              diffTimePercentList,
                              passedTimeList,
                              diffMemoryPercent
                            )
                          ) =
                            failList |> List.hd;
                          passedTimeList |> expect == [true, false]
                        }
                      );
                      test(
                        "else, compare it",
                        () => {
                          let actualResultCaseList =
                            [] |> _buildActuablCaseList("c1", 1, 1, ["t1", "t2"], [10, 20], 10);
                          let benchmarkResultCaseList =
                            [] |> _buildBenchmarkCaseList("c1", 1, 2, ["t1", "t2"], [15, 20], 10);
                          let passedTimeListMap = WonderCommonlib.HashMapSystem.createEmpty();
                          let failList =
                            Comparer._compare(
                              passedTimeListMap,
                              "test1",
                              actualResultCaseList,
                              benchmarkResultCaseList
                            );
                          let (
                            failMessage,
                            (
                              actualTestName,
                              actualCase,
                              diffTimePercentList,
                              passedTimeList,
                              diffMemoryPercent
                            )
                          ) =
                            failList |> List.hd;
                          passedTimeList |> expect == [false, true]
                        }
                      )
                    }
                  )
                }
              );
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
                                        WonderLog.Log.log(err) |> ignore;
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
                           [false],
                           1
                         )
                      |> _buildFailList(
                           "test2_title1",
                           "test2",
                           failCase2,
                           [wrongPerformanceTestData.commonData.maxAllowDiffTimePercent - 1],
                           [false],
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
                      _fakeCompare
                      |> onCall(1)
                      |> returns(make((~resolve, ~reject) => [@bs] resolve([])));
                      let _fakeGenerateCaseBenchmark = createEmptyStubWithJsObjSandbox(sandbox);
                      TesterTool.compareSpecificCount(
                        Obj.magic(1),
                        2,
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
                  );
                  test(
                    "only replace fail cases' benchmark, not change correct ones' benchmark",
                    () => {
                      let benchmarkFilePath = "./test/benchmark/test.json";
                      WonderCommonlib.NodeExtend.writeFile(
                        benchmarkFilePath,
                        {|
                 {
                   "name": "test.json",
                   "cases": [
                     {
                       "name": "case1",
                       "timestamp": 112,
                       "time_detail": [
                         {"name": "loopBody", "time": 25}
                       ],
                       "memory": 2,
                       "error_rate": 10
                     },
                     {
                       "name": "case2",
                       "timestamp": 5,
                       "time_detail": [
                         {"name": "loopBody", "time": 10}
                       ],
                       "memory": 5,
                       "error_rate": 5
                     }
                     ]
                   }
                                         |}
                      );
                      GenerateBenchmarkTool.changeCaseBenchmark(
                        "./test/benchmark/",
                        "test",
                        ("case2", 1, 2, ["loopBody"], [3], 4, "")
                      );
                      Node.Fs.readFileAsUtf8Sync(benchmarkFilePath)
                      |> Js.Json.parseExn
                      |>
                      expect == (
                                  {|
                                         {
                                           "name": "test.json",
                                           "cases": [
                                             {
                                                 "name": "case1",
                                                 "timestamp":112,
                                                 "time_detail": [
                                                     {"name":"loopBody","time":25}
                                                 ],
                                                 "memory": 2,
                                                 "error_rate": 10
                                             },

                                             {
                                                 "name": "case2",
                                                 "timestamp":2,
                                                 "time_detail": [
                                                     {"name":"loopBody","time":3}
                                                 ],
                                                 "memory": 4,
                                                 "error_rate": 1
                                             }]
                                           }
                   |}
                                  |> Js.Json.parseExn
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
                    /* |> returns(make((~resolve, ~reject) => [@bs] resolve([List.nth(failList, 1)]))); */
                    |> returns(make((~resolve, ~reject) => [@bs] resolve([])));
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
                               List.nth(_fakeCompare |> getCall(1) |> getArgs, 3).testDataList
                               |> List.hd
                             ).
                               name
                           )
                           |> expect == ([List.nth(failList, 0)], "test2")
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
                               [false],
                               1
                             )
                          |> _buildFailList(
                               "test2_title1",
                               "test2",
                               failCase2,
                               [wrongPerformanceTestData.commonData.maxAllowDiffTimePercent - 1],
                               [false],
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
                               [false],
                               wrongPerformanceTestData.commonData.maxAllowDiffMemoryPercent
                             )
                          |> _buildFailList(
                               "test2_title1",
                               "test2",
                               failCase2,
                               [1],
                               [false],
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
                        [] |> _buildFailList("test1_title1", "test1", failCase1, [1], [false], 1);
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
                        |> _buildFailList("test1_title1", "test1", failCase1, [1], [false], 1)
                        |> _buildFailList("test1_title2", "test1", failCase2, [1], [false], 1);
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
                        |> _buildFailList("test1_title1", "test1", failCase1, [1], [false], 1)
                        |> _buildFailList("test2_title1", "test2", failCase2, [1], [false], 1)
                        |> _buildFailList("test2_title2", "test2", failCase3, [1], [false], 1)
                        |> _buildFailList("test3_title1", "test3", failCase4, [1], [false], 1);
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