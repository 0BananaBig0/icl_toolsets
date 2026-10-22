Module m {
  Parameter W = 8;
  // A range bound is an integer expression: the published grammar allows
  // + - * / % and parentheses, over integers and parameter references.
  DataInPort din[$W - 1:0];
  ScanRegister r {
    ScanInSource bus[$W - 1:0];
    CaptureSource part[(1 + 2) * 2:0];
    ResetValue 4'd3;
  }
  // DefaultLoadValue takes an enum symbol as well as a number.
  DataInPort en {
    DefaultLoadValue ON;
    RefEnum E;
  }
}
