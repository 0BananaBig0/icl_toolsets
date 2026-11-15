Module Opaque {
  ScanInPort si;
  ScanOutPort so;
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
