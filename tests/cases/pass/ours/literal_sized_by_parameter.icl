Module m {
  // The width of a literal may be a parameter reference: the published grammar
  // has `size : pos_int | '$' SCALAR_ID`, so a module parameterised on width
  // writes its literals this way.
  Parameter W = 8;
  DataInPort din[$W:0];
  Reg r {
    ResetValue $W'd5;
    CaptureSource $W'b1;
    DefaultLoadValue $W 'd5;
  }
}
