// Self-constructed mixed scan-data / parallel-enable dependency.
Module MixedScanData {
  ScanInPort si;
  DataInPort enable;
  ScanRegister data[3:0] { ScanInSource si; ResetValue 4'b0; }
  DataRegister value[3:0] {
    WriteDataSource data;
    WriteEnSource enable;
    ResetValue 4'b0;
  }
  ScanOutPort so { Source data[0]; }
  DataOutPort observed[3:0] { Source value; }
}
