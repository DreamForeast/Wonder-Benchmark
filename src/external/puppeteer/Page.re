[@bs.send.pipe : WonderBsPuppeteer.Page.t]
external evaluateWithArg :
  ('a => 'evaluateFuncReturnValue, 'b) => Js.Promise.t('evaluateFuncReturnValue) =
  "evaluate";