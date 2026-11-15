Module Visible {
  ScanInPort si;
  ScanRegister value[7:0] { ScanInSource si; ResetValue 8'b0; }
  ScanOutPort so { Source value[0]; }
  SelectPort sel;
  CaptureEnPort ce;
  ShiftEnPort se;
  UpdateEnPort ue;
  TCKPort tck;
  ResetPort reset;
  ScanInterface client {
    Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port reset;
    DefaultLoadValue 8'b00000000;
  }
}
