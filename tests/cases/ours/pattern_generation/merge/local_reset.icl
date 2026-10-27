Module MergeLeaf {
  ScanInPort si;
  ResetPort rst;
  ScanRegister value[3:0] { ScanInSource si; ResetValue 4'b0; }
  ScanOutPort so { Source value[0]; }
}
Module MergeNetwork {
  ScanInPort si;
  ScanRegister reset { ScanInSource si; ResetValue 1'b1; }
  Instance a Of MergeLeaf { InputPort si = reset; InputPort rst = reset; }
  Instance b Of MergeLeaf { InputPort si = a.so; InputPort rst = reset; }
  ScanOutPort so { Source b.so; }
}
