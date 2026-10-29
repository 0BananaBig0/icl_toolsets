// Self-constructed: one lane of a vector one-hot return feeds a scalar chain.
Module VectorReturn {
  ScanInPort si[3:2];
  DataInPort enabled;
  ScanRegister data[3:0] { ScanInSource si[3]; ResetValue 4'b0; }
  ScanOutPort so[9:8] { Source data[0], si[2]; Enable enabled; }
}
Module VectorScanTop {
  ScanInPort si[5:2];
  LogicSignal a_enabled { select == 1'b0; }
  LogicSignal b_enabled { select == 1'b1; }
  Instance a Of VectorReturn {
    InputPort si = si[2], si[3];
    InputPort enabled = a_enabled;
  }
  Instance b Of VectorReturn {
    InputPort si = ~si[4], si[5];
    InputPort enabled = b_enabled;
  }
  OneHotScanGroup returns[7:6] { Port a.so; Port b.so; }
  ScanRegister select { ScanInSource returns[7]; ResetValue 1'b0; }
  ScanOutPort so { Source select; }
}
