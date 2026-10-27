// Leaf is supplied by nested.icl; do not duplicate its definition here.
Module ParallelResetNetwork {
  ScanInPort si;
  ScanRegister ra { ScanInSource si; ResetValue 1'b0; }
  ScanRegister rb { ScanInSource ra; ResetValue 1'b1; }
  Instance a Of Leaf { InputPort si = rb; InputPort rst = ~ra; }
  Instance b Of Leaf { InputPort si = a.so; InputPort rst = rb; }
  ScanOutPort so { Source b.so; }
}
