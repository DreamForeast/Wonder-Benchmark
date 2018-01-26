[@bs.send.pipe : WonderBsPuppeteer.Page.t]
external evaluateWithArg :
  ('a => 'evaluateFuncReturnValue, 'b) => Js.Promise.t('evaluateFuncReturnValue) =
  "evaluate";

  [@bs.send.pipe : WonderBsPuppeteer.Page.t]
external exposeFunctionWithString :
  (string, string => 'exposeFunctionWithStringReturnValue) =>
  Js.Promise.t('exposeFunctionWithStringReturnValue) =
  "exposeFunction";