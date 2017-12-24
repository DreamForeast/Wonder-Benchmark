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

type state = {
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