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
  timeArray: array(int),
  memory: int
};

[@bs.deriving jsConverter]
type t = {isClosePage: bool};

type state = {
  config:t,
  page: Page.t,
  browser: Browser.t,
  scriptFilePath: string,
  name: string,
  cases: array(caseItem),
  result: option(result)
};

type resultTimeData = {
  timestamp: int,
  timeArray: array(int)
};

type compareConfig = {errorRate: float};

type config = {. "isClosePage": bool};