// Self-constructed IEEE 1687-2014 6.4.9 scan-driven selectable register.
Module ScanData {
  ScanInPort si;
  ScanRegister bus[4:0] { ScanInSource si; ResetValue 5'b0; }
  DataRegister value[3:0] {
    WriteDataSource bus[3:0]; WriteEnSource bus[4]; ResetValue 4'b0;
  }
  ScanRegister observed[3:0] {
    ScanInSource bus[0]; CaptureSource value; ResetValue 4'b0;
  }
  ScanOutPort so { Source observed[0]; }
}
