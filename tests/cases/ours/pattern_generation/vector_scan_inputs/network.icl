// Self-constructed: one serial path can use different bits of a scan-input bus.
Module VectorScanTop {
  ScanInPort si[5:2];
  ScanMux serial_in SelectedBy select {
    1'b0: si[2];
    1'b1: ~si[4];
  }
  ScanRegister data[3:0] { ScanInSource serial_in; ResetValue 4'b0; }
  ScanRegister select { ScanInSource data[0]; ResetValue 1'b0; }
  ScanOutPort so[7] { Source select; }
}
