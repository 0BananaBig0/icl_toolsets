Module m {
  // Every signal type in the standard is written `'~'? signal`, and the
  // concatenation rules put that per element:
  //   concat_data_signal : data_signal (',' data_signal)*
  // So `~` is legal on any element, not just the first. A list used to accept
  // an operator only in first position, which rejected both of the below.
  DataOutPort d {
    Source ~a, b;
    Enable ~e;
  }
  ScanRegister sr {
    ScanInSource ~b;
    ResetValue ~1'b0, 2'b11;
    CaptureSource ~1'b0, ~2'b11;
  }
  Alias x = ~p, q;
}
