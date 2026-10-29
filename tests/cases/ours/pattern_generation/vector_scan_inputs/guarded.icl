// Declared controls remain authoritative even without an explicit ScanInterface.
Module GuardedVectorChild {
  ScanInPort si[1:0];
  SelectPort selected;
  ShiftEnPort shift;
  ScanRegister data { ScanInSource si[0]; ResetValue 1'b0; }
  ScanOutPort so { Source data; }
}
Module VectorScanTop {
  ScanInPort si[1:0];
  Instance child Of GuardedVectorChild {
    InputPort si = si;
    InputPort selected = 1'b0;
  }
  ScanOutPort so { Source child.so; }
}
