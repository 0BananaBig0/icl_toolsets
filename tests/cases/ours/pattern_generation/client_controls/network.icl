// Self-constructed standalone scan-client qualification fixture.
Module ClientTop {
  ScanInPort si;
  ScanOutPort so { Source value[0]; }
  SelectPort sel;
  CaptureEnPort ce;
  ShiftEnPort se;
  UpdateEnPort ue;
  TCKPort ck;
  ScanRegister value[3:0] {
    ScanInSource si;
    CaptureSource value;
    ResetValue 4'b0;
  }
  ScanInterface client {
    Port si;
    Port so;
    Port sel;
    Port ce;
    Port se;
    Port ue;
    Port ck;
  }
}
