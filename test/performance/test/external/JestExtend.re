/* todo move to Wonder_jest */
[@bs.val] external beforeAllPromise : (unit => Js.Promise.t('a)) => unit = "beforeAll";

[@bs.val] external afterAllPromise : (unit => Js.Promise.t('a)) => unit = "afterAll";