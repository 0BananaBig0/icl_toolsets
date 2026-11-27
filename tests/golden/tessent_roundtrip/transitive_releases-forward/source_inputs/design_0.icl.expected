Module Leaf {
  ScanInPort si;
  ResetPort rst;
  ScanRegister value[3:0] { ScanInSource si; ResetValue 4'b0000; }
  ScanOutPort so { Source value[0]; }
}
Module ResetStage {
  ScanInPort si;
  ResetPort rst;
  ScanRegister release { ScanInSource si; ResetValue 1'b0; }
  DataMux reset_gate SelectedBy release { 1'b0: 1'b1; 1'b1: rst; }
  ToResetPort reset_out { Source reset_gate; }
  ScanOutPort so { Source release; }
}
Module Network {
  ScanInPort si;
  ResetPort rst;
  ScanRegister gate { ScanInSource si; ResetValue 1'b0; }
  ScanRegister root { ScanInSource gate; ResetValue 1'b0; }
  DataMux branch_reset SelectedBy root { 1'b0: 1'b1; 1'b1: rst; }
  Instance branch Of ResetStage { InputPort si = root; InputPort rst = branch_reset; }
  Instance a Of Leaf { InputPort si = branch.so; InputPort rst = branch.reset_out; }
  Instance b Of Leaf { InputPort si = a.so; InputPort rst = branch.reset_out; }
  ScanMux route SelectedBy gate { 1'b0: branch.so; 1'b1: b.so; }
  ScanOutPort so { Source route; }
}
