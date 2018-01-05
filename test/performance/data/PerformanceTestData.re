open PerformanceTestDataType;

let correctPerformanceTestData = {
  commonData: {
    isClosePage: true,
    execCountWhenTest: 10,
    execCountWhenGenerateBenchmark: 20,
    benchmarkPath: "./test/performance/benchmark/",
    scriptFilePathList: ["./test/res/script1.js"]
  },
  testDataList: [
    {
      name: "basic1",
      caseList: [
        {
          name: "pf_test1",
          bodyFuncStr: {|function test(){
                        var arr = [];
                        for(var i = 0; i <= 100000; i++){
                        arr[i] = wd.add(1, 2);
                        }
                    }
var n1 = performance.now();
test();

var n2 = performance.now();

test();


var n3 = performance.now();


test();
test();


var n4 = performance.now();


/* return {"errorRate": 5, "textArray": ["prepare", "init", "loopBody"], "timeArray": [n1, n2, n3, n4] } */
return {"textArray": ["prepare", "init", "loopBody"], "timeArray": [n1, n2, n3, n4] }
|},
          scriptFilePathList: None,
          errorRate: 5
        },
        {
          name: "pf_test2",
          bodyFuncStr: {|function test(){
                        var arr = [];
                        for(var i = 0; i <= 100000; i++){
                        arr[i] = minus(3, wd.add(1, 2));
                        }
                    }
var n1 = performance.now();
test();

var n2 = performance.now();

test();


var n3 = performance.now();


test();
test();


var n4 = performance.now();


return {"textArray": ["prepare", "init", "loopBody"], "timeArray": [n1, n2, n3, n4] }
|},
          scriptFilePathList: Some(["./test/res/script2.js"]),
          errorRate: 5
        }
      ]
    }
  ]
};
let wrongPerformanceTestData = {
  commonData: {
    isClosePage: true,
    execCountWhenTest: 10,
    execCountWhenGenerateBenchmark: 20,
    benchmarkPath: "./test/performance/benchmark/",
    scriptFilePathList: ["./test/res/script1.js"]
  },
  testDataList: [
    {
      name: "basic1",
      caseList: [
        {
          name: "pf_test1",
          bodyFuncStr: {|function test(){
                        var arr = [];
                        for(var i = 0; i <= 100000; i++){
                        arr[i] = wd.add(1, 2);
                        }
                    }
var n1 = performance.now();
test();

var n2 = performance.now();

test();


var n3 = performance.now();


test();
test();


var n4 = performance.now();


/* return {"errorRate": 5, "textArray": ["prepare", "init", "loopBody"], "timeArray": [n1, n2, n3, n4] } */
return {"textArray": ["prepare", "init", "loopBody"], "timeArray": [n1, n2, n3, n4] }
|},
          scriptFilePathList: None,
          errorRate: 5
        },
        {
          name: "pf_test2",
          bodyFuncStr: {|function test(){
                        var arr = [];
                        for(var i = 0; i <= 300000; i++){
                        arr[i] = minus(3, wd.add(1, 2));
                        }
                    }
var n1 = performance.now();
test();

var n2 = performance.now();

test();


var n3 = performance.now();


test();
test();


var n4 = performance.now();


return {"textArray": ["prepare", "init", "loopBody"], "timeArray": [n1, n2, n3, n4] }
|},
          scriptFilePathList: Some(["./test/res/script2.js"]),
          errorRate: 5
        }
      ]
    }
  ]
};