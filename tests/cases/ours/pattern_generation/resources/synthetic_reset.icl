Module ReservedResetLeaf {
  ScanInPort si;
  ResetPort rst;
  ScanRegister value[3:0] { ScanInSource si; ResetValue 4'b0; }
  ScanOutPort so { Source value[0]; }
}
Module ResourceNetwork {
  ScanInPort si;
  ResetPort global;
  ScanRegister reset { ScanInSource si; ResetValue 1'b1; }
  DataMux local_reset SelectedBy reset { 1'b0: global; 1'b1: 1'b1; }
  Instance a Of ReservedResetLeaf { InputPort si = reset; InputPort rst = local_reset; }
  ScanRegister b[3:0] { ScanInSource reset; ResetValue 4'b0; }
  ScanMux route SelectedBy reset { 1'b0: a.so; 1'b1: b[0]; }
  ScanOutPort so { Source route; }
}
