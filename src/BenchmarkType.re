type caseTimeItem = {
  name: string,
  time: int
};

type caseItem = {
  name: string,
  time: array(caseTimeItem),
  memory: int,
  errorRate: option(int)
};

type result = {
  name: string,
  timestamp: int,
  timeArray: array(int),
  memory: int
};

[@bs.deriving jsConverter]
type t = {isClosePage: bool, execCount: int, extremeCount:int};

type state = {
  config:t,
  page: Page.t,
  browser: Browser.t,
  scriptFilePathList: list(string),
  name: string,
  cases: array(caseItem),
  result: option(result)
};

type resultTimeData = {
  timestamp: int,
  timeArray: array(int)
};

type compareConfig = {errorRate: float};

type config = {. "isClosePage": bool, "execCount": int, "extremeCount": int};