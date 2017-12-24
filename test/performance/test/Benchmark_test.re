open Wonder_jest;

open Benchmark;

open WonderCommonlib;

let _ =
  describe(
    "basic boxes",
    () => {
      open Expect;
      open Expect.Operators;
      open Sinon;
      open Puppeteer;
      open Js.Promise;
      let sandbox = getSandboxDefaultVal();
      let state = ref(createEmptyState());
      let browser = ref(None);
      let page = ref(None);
      beforeAllPromise(
        () =>
          launch(
            ~options={
              "ignoreHTTPSErrors": Js.Nullable.empty,
              "executablePath": Js.Nullable.empty,
              "slowMo": Js.Nullable.empty,
              "args": Js.Nullable.empty,
              /* "args": Js.Nullable.return([|"--headless", "--hide-scrollbars", "--mute-audio"|]), */
              "handleSIGINT": Js.Nullable.empty,
              "timeout": Js.Nullable.empty,
              "dumpio": Js.Nullable.empty,
              "userDataDir": Js.Nullable.empty,
              "headless": Js.Nullable.return(Js.false_)
            },
            ()
          )
          |> then_(
               (b) => {
                 browser := Some(b);
                 b |> Browser.newPage
               }
             )
          |> then_(
               (p) => {
                 page := Some(p);
                 state :=
                   createState(p, browser^ |> Js.Option.getExn, "./mine/wd.js", "basic_boxes.json");
                 p |> resolve
               }
             )
      );
      afterAllPromise(() => browser^ |> Js.Option.getExn |> Browser.close);
      beforeEach(() => sandbox := createSandbox());
      afterEach(() => restoreSandbox(refJsObjToSandbox(sandbox^)));
      describe(
        "test time",
        () =>
          testPromiseWithTimeout(
            "create 20k boxes",
            () => {
              /* state^
                 |> exec("create_20k_boxes", () => {
                   [|1,2|] |> Js.Array.includes(1) |> ignore;
                 })
                 |> getTime
                 |> compare */
              /* let init = (state: StateDataType.state) => DirectorSystem.init(state);

                 let initSystem = (state: StateDataType.state) => GameObjectAdmin.init(state);

                 let sync = (state: StateDataType.state, ~time=0., ()) => DirectorSystem._sync(time, state);

                 let run = (state: StateDataType.state, ~time=0., ()) => DirectorSystem._run(time, state);

                 let loopBody = (~time: float=0., ()) => DirectorSystem.loopBody(time); */
              let body2 = [%bs.raw
                {| function() {




                var state = wd.setMainConfig({
                    isTest: false
                });

                return initSample(state);


                function createBoxes(count, state) {
                    var boxes = [];

                    var data = createBox(state);
                    var state = data[0];
                    var box = data[1];


                    var data = wd.cloneGameObject(box, count, true, state);
                    var state = data[0];
                    var newBoxes = data[1];


                    let flatten = (arr) => {
                        return arr.reduce((a, b) => {
                            let arr = a.concat(b);
                            return arr;
                        }, []);
                    };
                    newBoxes = flatten(newBoxes);

                    return [state, newBoxes];

                }


                function createBoxesWithoutClone(count, state) {
                    var boxes = [];
                    // var state$ = state;

                    // var [state, box] = wd.createBox(state);


                    for (let i = 0; i < count; i++) {
                        var [state, box] = wd.createBox(state);


                        // var [state, newBoxes] = cloneGameObject(box, 2000, state);

                        boxes.push(box);
                    }

                    return [state, boxes];
                }



                function setPosition(boxes, state) {
                    var playgroundSize = 500;

                    for (let i = 0, len = boxes.length; i < len; i++) {
                        let box = boxes[i];
                        // state = initGameObject(box, state);


                        var transform = wd.getGameObjectTransformComponent(box, state);


                        // var pos = wd.getTransformPosition(transform, state);
                        var localPos = wd.getTransformLocalPosition(transform, state);

                        state = wd.setTransformLocalPosition(transform, [Math.random() * 2 * playgroundSize - playgroundSize, Math.random() * 2 * playgroundSize - playgroundSize, Math.random() * 2 * playgroundSize - playgroundSize], state);


                        // console.log(wd.getTransformLocalPosition(transform, state));
                    }

                    return [state, boxes];
                }


                function initSample(state) {
var n1 = performance.now();
                    var data = createBoxes(20000, state);

                    var state = data[0];
                    var boxes = data[1];

                    var data = setPosition(boxes, state);
                    var state = data[0];
                    var boxes = data[1];

                    var data = createCamera(state);
                    var state = data[0];

var n2 = performance.now();

                    var state = wd.initDirector(state);


var n3 = performance.now();

                    /* var state = wd.setState(state); */

                    var state = wd.loopBody(100.0, state);

                    /* var state = wd.loopBody(200.0, state);
                    var state = wd.loopBody(300.0, state); */


var n4 = performance.now();


                    /* return state; */




return [n1, n2, n3, n4]
                }

                let _getRandom = function (num) {
                    return Math.floor(Math.random() * num);
                }

                let _setData = (boxes, state) => {
                    boxes.forEach(function (box) {
                        var transform = wd.getGameObjectTransformComponent(box, state);


                        var pos = wd.getTransformPosition(transform, state);
                        var localPos = wd.getTransformLocalPosition(transform, state);

                        if (pos[0] >= 500) {
                            state = wd.setTransformLocalPosition(transform, [100, localPos[1], localPos[2]], state);

                        }
                        else if (pos[0] < 500) {
                            state = wd.setTransformPosition(transform, [pos[0] + 10, pos[1], pos[2]], state);
                        }
                    });

                    return state;
                };

                function setData(boxes, state) {
                    return scheduleLoop((elapsed, state) => _setData(boxes, state), state)
                };

                function getRandomParentIndex(num) {
                    return Math.floor(Math.random() * num);
                }


                function setParent(boxes, state) {
                    for (var i = 1, len = 10; i < len; i++) {
                        var box = boxes[i];

                        state = wd.setTransformParent(boxes[i - 1], box, state)
                    }

                    return scheduleLoop((elapsed, state) => {
                        var box = boxes[i];

                        state = wd.setTransformParent(boxes[wd.getRandomParentIndex(10)], box, state);

                        return _setData(boxes, state);
                    }, state)
                }

                window.newBoxes$ = null;

                function createAndDisposeGameObjects(boxes, state) {
                    window.sourceBox = boxes[0];
                    window.boxes = [];


                    return scheduleLoop((elapsed, state) => {
                        // for(let i = 0, len = window.boxes.length; i < len; i++){
                        //     let box = window.boxes[i];
                        //     state = disposeGameObject(box, state);
                        // }

                        var state = wd.batchDisposeGameObject(window.boxes, state);


                        // var [state, newBoxes] = wd.createBoxesWithoutClone(2000, state);

                        var [state, newBoxes] = wd.cloneGameObject(window.sourceBox, 5000, true, state);
                        let flatten = (arr) => {
                            return arr.reduce((a, b) => {
                                let arr = a.concat(b);
                                return arr;
                            }, []);
                        };
                        newBoxes = flatten(newBoxes);


                        var [state, newBoxes] = wd.setPosition(newBoxes, state);



                        window.boxes = newBoxes;



                        for (let i = 0, len = newBoxes.length; i < len; i++) {
                            let box = newBoxes[i];
                            state = wd.initGameObject(box, state);
                        }

                        return state;

                    }, state)
                }

                function createBox(state) {
                    var data = wd.createBasicMaterial(state);
                    var state = data[0];
                    var material = data[1];

                    state = wd.setMaterialColor(material, [0.0, 0.5, 0.2], state);

                    var data = wd.createMeshRenderer(state);
                    var state = data[0];
                    var meshRenderer = data[1];

                    var data = wd.createGameObject(state);
                    var state = data[0];
                    var obj = data[1];

                    state = wd.addGameObjectMaterialComponent(obj, material, state);
                    state = wd.addGameObjectMeshRendererComponent(obj, meshRenderer, state);


                    var data = wd.createBoxGeometry(state);
                    var state = data[0];
                    var geometry = data[1];

                    state = wd.setBoxGeometryConfigData(geometry, {
                        width: 5,
                        height: 5,
                        depth: 5
                    }, state);




                    state = wd.addGameObjectGeometryComponent(obj, geometry, state);

                    return [state, obj];

                }

                function createCamera(state) {

                    var data = wd.createCameraController(state);
                    var state = data[0];
                    var cameraController = data[1];

                    state = wd.setPerspectiveCameraNear(cameraController, 0.1, state);
                    state = wd.setPerspectiveCameraFar(cameraController, 2000, state);
                    state = wd.setPerspectiveCameraFovy(cameraController, 60, state);
                    state = wd.setPerspectiveCameraAspect(cameraController, 1.0, state);


                    state = wd.setCameraControllerPerspectiveCamera(cameraController, state);



                    var data = wd.createGameObject(state);
                    var state = data[0];
                    var obj = data[1];

                    state = wd.addGameObjectCameraControllerComponent(obj, cameraController, state);

                    var transform = wd.getGameObjectTransformComponent(obj, state);

                    state = wd.setTransformLocalPosition(transform, [0, 0, 1500], state);

                    return [state, obj];
                }





}
|}
              ];
              /* let test = () =>
                 launch(
                   ~options={
                     "ignoreHTTPSErrors": Js.Nullable.empty,
                     "executablePath": Js.Nullable.empty,
                     "slowMo": Js.Nullable.empty,
                     "args": Js.Nullable.empty,
                     "handleSIGINT": Js.Nullable.empty,
                     "timeout": Js.Nullable.empty,
                     "dumpio": Js.Nullable.empty,
                     "userDataDir": Js.Nullable.empty,
                     "headless": Js.Nullable.return(false)
                   },
                   ()
                 )
                 |> then_(
                      (browser) =>
                        browser
                        |> Browser.newPage
                        |> then_(
                             (page) =>
                               page
                               |> Page.addScriptTag({
                                    "url": Js.Nullable.empty,
                                    "content": Js.Nullable.empty,
                                    "path": Js.Nullable.return("./lib/js/src/core/api/Main1.js")
                                  })
                               |> then_((_) => page |> Page.metrics())
                               /* |> then_((data) => page |> Page.metrics(body)) */
                               |> then_((data) => (page |> Page.evaluate(body), data) |> resolve)
                               |> then_(((d1, d2)) => (d1, d2, page |> Page.metrics()) |> resolve)
                               |> then_(
                                    (d) => {
                                      Js.log(d) |> ignore;
                                      resolve(d)
                                    }
                                  )
                               |> then_((_) => browser |> Browser.close)
                               |> then_((_) => expect(1) == 1 |> resolve)
                           )
                    ); */
              /* logHtml() |> Obj.magic */
              /* test() */
              /* DebugUtils.log(browser^) |> ignore;
                 DebugUtils.log(page^) |> ignore; */
              /* let page = page^ |> Js.Option.getExn;
                 let metricData0 = ref(None);
                 let metricData1 = ref(None);
                 let metricData2 = ref(None);
                 /* let _ =  */
                 page
                 |> Page.addScriptTag({
                      "url": Js.Nullable.empty,
                      "content": Js.Nullable.empty,
                      "path": Js.Nullable.return("./mine/wd.js")
                    })
                 |> then_((_) => page |> Page.metrics())
                 |> then_(
                      (data) => {
                        metricData0 := Some(data);
                        resolve(data)
                      }
                    )
                 |> then_((_) => page |> Page.evaluate(body2))
                 |> then_((_) => page |> Page.metrics())
                 |> then_(
                      (data) => {
                        metricData1 := Some(data);
                        resolve(data)
                      }
                    )
                 /* |> then_((_) => page |> Page.evaluate(body2))
                     |> then_((bodyReturnVal) => {

                    /* DebugUtils.log(bodyReturnVal) |> ignore;  */
                          resolve(true)
                     }

                     )
                     |> then_((_) => page |> Page.metrics())
                     |> then_(
                          (data) => {
                            metricData2 := Some(data);
                            resolve(data)
                          }
                        ) */
                 |> then_(
                      (_) => {
                        DebugUtils.log(metricData0) |> ignore;
                        DebugUtils.log(metricData1) |> ignore;
                        /* DebugUtils.log(metricData2) |> ignore;    */
                        expect(1) == 1 |> resolve
                      }
                    ); */
              /* let func = () => {
                 let a = 1;
                                     }; */



              state^
              |> exec("create_20k_boxes", [@bs] body2)
              /* |> exec("create_20k_boxes", () => {

                 }) */
              |> compare((expect, toBe))
              /* |> compare(( expect,  pass)) */
              /* |> then_((_) => expect(1) == 1 |> resolve) */
            }, 16000
          )
      )
      /* describe("test memory", () => {}) */
    }
  );