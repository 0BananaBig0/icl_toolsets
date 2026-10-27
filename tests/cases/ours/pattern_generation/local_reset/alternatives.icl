// Self-constructed alternative reset-release paths; Leaf is in nested.icl.
Module ResetChoice {
  ScanInPort si;
  ScanRegister a { ScanInSource si; ResetValue 1'b0; }
  ScanRegister b { ScanInSource a; ResetValue 1'b0; }
  LogicSignal release_reset { ~(a | b); }
  Instance child Of Leaf {
    InputPort si = b;
    InputPort rst = release_reset;
  }
  ScanOutPort so { Source child.so; }
}
