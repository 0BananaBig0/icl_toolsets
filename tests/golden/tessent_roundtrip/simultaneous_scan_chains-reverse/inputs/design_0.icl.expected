// Self-constructed unequal-length simultaneous client chains.
Module Chains {
  ScanInPort si[3:2];
  ScanRegister a[3:0] { ScanInSource si[3]; CaptureSource a; ResetValue 4'b0; }
  ScanRegister b[1:0] { ScanInSource si[2]; CaptureSource b; ResetValue 2'b0; }
  ScanOutPort so[5:4] { Source a[0], b[0]; }
  SelectPort sel; CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue; TCKPort ck;
  ScanInterface client { Chain first { Port si[3]; Port so[5]; } Chain second { Port si[2]; Port so[4]; } Port sel; Port ce; Port se; Port ue; Port ck; }
}
