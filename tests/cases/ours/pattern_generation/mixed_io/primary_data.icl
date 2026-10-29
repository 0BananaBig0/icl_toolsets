// Self-constructed mixed parallel-data / scan-enable dependency.
Module MixedPrimaryData {
  ScanInPort si;
  DataInPort data[3:0];
  ScanRegister enable { ScanInSource si; ResetValue 1'b0; }
  DataRegister value[3:0] {
    WriteDataSource data;
    WriteEnSource enable;
    ResetValue 4'b0;
  }
  ScanOutPort so { Source enable; }
  DataOutPort observed[3:0] { Source value; }
}
