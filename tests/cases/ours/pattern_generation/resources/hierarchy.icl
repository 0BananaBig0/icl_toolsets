Module ReservedLeaf {
  ScanInPort si;
  ScanRegister gate { ScanInSource si; ResetValue 1'b0; }
  ScanRegister target[3:0] { ScanInSource gate; ResetValue 4'b0; }
  ScanMux route SelectedBy gate { 1'b0: gate; 1'b1: target[0]; }
  ScanOutPort so { Source route; }
}
Module ResourceNetwork {
  ScanInPort si;
  Instance core Of ReservedLeaf { InputPort si = si; }
  ScanOutPort so { Source core.so; }
}
