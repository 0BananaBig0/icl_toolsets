// Self-constructed alternative reset-release paths; Leaf is in nested.icl.
Module ResetChoice {
  ScanInPort si;
  ResetPort global;
  ScanRegister a { ScanInSource si; ResetValue 1'b0; }
  ScanRegister b { ScanInSource a; ResetValue 1'b0; }
  LogicSignal release_reset { ~(a | b); }
  DataMux reset_gate SelectedBy release_reset { 1'b0: global; 1'b1: 1'b1; }
  Instance child Of Leaf {
    InputPort si = b;
    InputPort rst = reset_gate;
  }
  ScanOutPort so { Source child.so; }
}
