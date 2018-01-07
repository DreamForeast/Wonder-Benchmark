open PerformanceTestDataType;

let commonData = {
  isClosePage: true,
  execCountWhenTest: 1,
  execCountWhenGenerateBenchmark: 2,
  compareCount: 3,
  maxAllowDiffTimePercent: 5000,
  maxAllowDiffMemoryPercent: 5000,
  benchmarkPath: "./test/performance/benchmark/",
  generateBaseDebugData: None,
  scriptFilePathList: []
};

let correctPerformanceTestData = {
  commonData,
  testDataList: [
    {
      name: "basic1",
      caseList: [
        {
          name: "pf_test1",
          bodyFuncStr: {|
            var state = wd.setMainConfig({
                isTest: false
            });

            return initSample(state);



            function initSample(state) {
var n1 = performance.now();

                var data = BasicBoxesTool.createBoxesByClone(20, state);

                var state = data[0];
                var boxes = data[1];

                var data = BasicBoxesTool.setPosition(boxes, state);
                var state = data[0];
                var boxes = data[1];

                var data = BasicBoxesTool.createCamera(state);
                var state = data[0];

var n2 = performance.now();

                var state = wd.initDirector(state);



                /* var state = wd.setState(state); */


var n3 = performance.now();
                var state = wd.loopBody(100.0, state);




var n4 = performance.now();


                /* return state; */




return {"textArray": ["prepare", "init", "loopBody"], "timeArray": [n1, n2, n3, n4] }
            }
|},
          scriptFilePathList:
            Some(["./test/res/BasicBoxesTool.js", "./test/res/CameraTool.js", "./test/res/wd.js"]),
          errorRate: 20
        },
        {
          name: "pf_test2",
          bodyFuncStr: {|function test(){
                        var arr = [];
                        for(var i = 0; i <= 100000; i++){
                        arr[i] = minus(1, wd.add(1, 2));
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
          scriptFilePathList: Some(["./test/res/script1.js", "./test/res/script2.js"]),
          errorRate: 50
        }
      ]
    }
  ]
};

let wrongPerformanceTestData = {
  commonData,
  testDataList: [
    {
      name: "basic1",
      caseList: [
        {
          name: "pf_test1",
          bodyFuncStr: {|
            var state = wd.setMainConfig({
                isTest: false
            });

            return initSample(state);



            function initSample(state) {
var n1 = performance.now();

                var data = BasicBoxesTool.createBoxesByClone(1000, state);

                var state = data[0];
                var boxes = data[1];

                var data = BasicBoxesTool.setPosition(boxes, state);
                var state = data[0];
                var boxes = data[1];

                var data = BasicBoxesTool.createCamera(state);
                var state = data[0];

var n2 = performance.now();

                var state = wd.initDirector(state);



                /* var state = wd.setState(state); */


var n3 = performance.now();
                var state = wd.loopBody(100.0, state);




var n4 = performance.now();


                /* return state; */




return {"textArray": ["prepare", "init", "loopBody"], "timeArray": [n1, n2, n3, n4] }
            }
|},
          scriptFilePathList:
            Some(["./test/res/BasicBoxesTool.js", "./test/res/CameraTool.js", "./test/res/wd.js"]),
          errorRate: 20
        },
        {
          name: "pf_test2",
          bodyFuncStr: {|function test(){
                        var arr = [];
                        for(var i = 0; i <= 100000; i++){
                        arr[i] = minus(1, wd.add(1, 2));
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
          scriptFilePathList: Some(["./test/res/script1.js", "./test/res/script2.js"]),
          errorRate: 50
        }
      ]
    }
  ]
};

let wrongPerformanceTestData2 = {
  commonData: {
    isClosePage: true,
    execCountWhenTest: 1,
    execCountWhenGenerateBenchmark: 2,
    compareCount: 3,
    maxAllowDiffTimePercent: 5000,
    maxAllowDiffMemoryPercent: 5000,
    benchmarkPath: "./test/performance/benchmark/",
    generateBaseDebugData:
      Some([
        {sourceScriptFilePath: "./test/res/wd.js", targetScriptFilePath: "./test/base/wd_base.js"}
      ]),
    scriptFilePathList: []
  },
  testDataList: [
    {
      name: "basic1",
      caseList: [
        {
          name: "pf_test1",
          bodyFuncStr: {|
            var state = wd.setMainConfig({
                isTest: false
            });

            return initSample(state);



            function initSample(state) {
var n1 = performance.now();

                var data = BasicBoxesTool.createBoxesByClone(20, state);

                var state = data[0];
                var boxes = data[1];

                var data = BasicBoxesTool.setPosition(boxes, state);
                var state = data[0];
                var boxes = data[1];

                var data = BasicBoxesTool.createCamera(state);
                var state = data[0];

var n2 = performance.now();

                var state = wd.initDirector(state);



                /* var state = wd.setState(state); */


var n3 = performance.now();
                var state = wd.loopBody(100.0, state);




var n4 = performance.now();


                /* return state; */




return {"textArray": ["prepare", "init", "loopBody"], "timeArray": [n1, n2, n3, n4] }
            }
|},
          scriptFilePathList:
            Some(["./test/res/BasicBoxesTool.js", "./test/res/CameraTool.js", "./test/res/wd.js"]),
          errorRate: 20
        },
        {
          name: "pf_test2",
          bodyFuncStr: {|function test(){
                        var arr = [];
                        for(var i = 0; i <= 10000; i++){
                        arr[i] = minus(1, wd.add(1, 2));
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
          scriptFilePathList: Some(["./test/res/script1.js", "./test/res/script2.js"]),
          errorRate: 50
        }
      ]
    }
  ]
};