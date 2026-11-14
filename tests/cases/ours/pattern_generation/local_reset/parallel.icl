// Leaf is supplied by nested.icl; do not duplicate its definition here.
Module ParallelResetNetwork {
  ScanInPort si;
  ResetPort global;
  ScanRegister ra { ScanInSource si; ResetValue 1'b0; }
  ScanRegister rb { ScanInSource ra; ResetValue 1'b1; }
  DataMux reset_a SelectedBy ra { 1'b0 : 1'b1; 1'b1 : global; }
  DataMux reset_b SelectedBy rb { 1'b0 : global; 1'b1 : 1'b1; }
  Instance a Of Leaf { InputPort si = rb; InputPort rst = reset_a; }
  Instance b Of Leaf { InputPort si = a.so; InputPort rst = reset_b; }
  ScanOutPort so { Source b.so; }
}
