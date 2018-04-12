open PerformanceTestDataType;

let commonData1 = {
  isClosePage: true,
  execCountWhenTest: 1,
  execCountWhenGenerateBenchmark: 2,
  compareCount: 3,
  maxAllowDiffTimePercent: 5000,
  maxAllowDiffMemoryPercent: 5000,
  benchmarkPath: "./test/performance/benchmark/",
  baseDir: "./test/base",
  scriptFilePathList: [
    "./test/res/BasicBoxesTool.js",
    "./test/res/CameraTool.js",
    "./test/res/wd.js",
    "./test/res/script1.js",
    "./test/res/script2.js"
  ],
  replaceBodyFuncStrWhenDebug: None
};

let commonData2 = {
  isClosePage: true,
  execCountWhenTest: 1,
  execCountWhenGenerateBenchmark: 2,
  compareCount: 3,
  maxAllowDiffTimePercent: 5000,
  maxAllowDiffMemoryPercent: 5000,
  benchmarkPath: "./test/performance/benchmark/",
  baseDir: "./test/base",
  scriptFilePathList: [
    "./test/res/BasicBoxesTool.js",
    "./test/res/CameraTool.js",
    "./test/res/wd.js",
    "./test/res/script1.js",
    "./test/res/script2.js"
  ],
  replaceBodyFuncStrWhenDebug: None
};

let correctPerformanceTestData = {
  commonData: commonData1,
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
          errorRate: 20
        },
        {
          name: "pf_test2",
          bodyFuncStr: {|function test(){
                        var arr = [];
                        for(var i = 0; i <= 100000; i++){
                        arr[i] = minus(1, testScript.add(1, 2));
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
          errorRate: 50
        }
      ]
    }
  ]
};

let correctPerformanceTestData2 = {
  commonData: commonData1,
  testDataList: [
    {
      name: "basic1",
      caseList: [
        {
          name: "pf_test11",
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
          errorRate: 20
        },
        {
          name: "pf_test12",
          bodyFuncStr: {|function test(){
                        var arr = [];
                        for(var i = 0; i <= 100000; i++){
                        arr[i] = minus(1, testScript.add(1, 2));
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
          errorRate: 50
        }
      ]
    },
    {
      name: "basic2",
      caseList: [
        {
          name: "pf_test21",
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
          errorRate: 20
        },
        {
          name: "pf_test22",
          bodyFuncStr: {|function test(){
                        var arr = [];
                        for(var i = 0; i <= 100000; i++){
                        arr[i] = minus(1, testScript.add(1, 2));
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
          errorRate: 50
        }
      ]
    }
  ]
};

let wrongPerformanceTestData = {
  commonData: commonData1,
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
          errorRate: 20
        },
        {
          name: "pf_test2",
          bodyFuncStr: {|function test(){
                        var arr = [];
                        for(var i = 0; i <= 100000; i++){
                        arr[i] = minus(1, testScript.add(1, 2));
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
          errorRate: 50
        }
      ]
    }
  ]
};

let wrongPerformanceTestData2 = {
  commonData: commonData2,
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
          errorRate: 20
        },
        {
          name: "pf_test2",
          bodyFuncStr: {|function test(){
                        var arr = [];
                        for(var i = 0; i <= 10000; i++){
                        arr[i] = minus(1, testScript.add(1, 2));
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
          errorRate: 50
        }
      ]
    }
  ]
};

let wrongPerformanceTestData3 = {
  commonData: commonData1,
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
          errorRate: 20
        },
        {
          name: "pf_test2",
          bodyFuncStr: {|function test(){
                        var arr = [];
                        for(var i = 0; i <= 10000; i++){
                        arr[i] = minus(1, testScript.add(1, 2));
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
          errorRate: 50
        }
      ]
    }
  ]
};

let _getFailCaseBodyFuncStr = () => {|function test(){
                        var arr = [];
                        for(var i = 0; i <= 100; i++){
                        arr[i] = minus(1, testScript.add(1, 2));
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
|};

let wrongPerformanceTestData4 = {
  commonData: commonData1,
  testDataList: [
    {
      name: "basic1",
      caseList: [
        {name: "pf_test11", bodyFuncStr: _getFailCaseBodyFuncStr(), errorRate: 20},
        {name: "pf_test12", bodyFuncStr: _getFailCaseBodyFuncStr(), errorRate: 50}
      ]
    },
    {
      name: "basic2",
      caseList: [
        {name: "pf_test21", bodyFuncStr: _getFailCaseBodyFuncStr(), errorRate: 20},
        {name: "pf_test22", bodyFuncStr: _getFailCaseBodyFuncStr(), errorRate: 50}
      ]
    }
  ]
};