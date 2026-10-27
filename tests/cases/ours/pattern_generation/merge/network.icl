Module MergeNetwork {
  ScanInPort si;
  ScanRegister a[3:0] { ScanInSource si; ResetValue 4'b0; }
  ScanRegister b[3:0] { ScanInSource a[0]; ResetValue 4'b0; }
  ScanOutPort so { Source b[0]; }
}
