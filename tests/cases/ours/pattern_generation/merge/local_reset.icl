Module MergeLeaf {
  ScanInPort si;
  ResetPort rst;
  ScanRegister value[3:0] { ScanInSource si; ResetValue 4'b0; }
  ScanOutPort so { Source value[0]; }
}
Module MergeNetwork {
  ScanInPort si;
  ResetPort global;
  ScanRegister reset { ScanInSource si; ResetValue 1'b1; }
  DataMux local_reset SelectedBy reset { 1'b0: global; 1'b1: 1'b1; }
  Instance a Of MergeLeaf { InputPort si = reset; InputPort rst = local_reset; }
  Instance b Of MergeLeaf { InputPort si = a.so; InputPort rst = local_reset; }
  ScanOutPort so { Source b.so; }
}
