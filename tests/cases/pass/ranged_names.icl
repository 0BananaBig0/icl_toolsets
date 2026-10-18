Module m {
  // Names that may carry a bit range. The published grammar gives
  // LogicSignal, the one-hot groups and the muxes `reg_port_signal_id`
  // (SCALAR_ID | vector_id), so `name[7:0]` is legal on each. They used to be
  // rejected because the keyword groups did not allow a range.
  LogicSignal ls[7:0] {
    a && b;
  }
  OneHotScanGroup ohs[3:0] {
    Port a;
  }
  OneHotDataGroup ohd[1:0] {
    Port d;
  }
  ScanMux mx[1:0] SelectedBy s {
    1'b0 : si;
  }
  DataMux dx[1:0] SelectedBy s {
    1'b0 : d;
  }
  ClockMux cx[1:0] SelectedBy s {
    1'b0 : c;
  }
}
