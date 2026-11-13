Module ShiftOnlyCell {
  Parameter W = 4;
  ScanInPort si; ShiftEnPort se;
  ScanRegister value[$W-1:0] { ScanInSource si; CaptureSource value; ResetValue 0; }
  ScanOutPort so { Source value[0]; }
}
Module Chains {
  ScanInPort si[1:0]; ShiftEnPort shift[1:0];
  CaptureEnPort ce; UpdateEnPort ue; TCKPort ck;
  Instance c0 Of ShiftOnlyCell { InputPort si = si[0]; InputPort se = shift[0]; }
  ScanOutPort so0 { Source c0.so; }
  ScanInterface bus0 { Port si[0]; Port so0; Port shift[0]; Port ce; Port ue; Port ck; }
  Instance c1 Of ShiftOnlyCell { InputPort si = si[1]; InputPort se = shift[1]; }
  ScanOutPort so1 { Source c1.so; }
  ScanInterface bus1 { Port si[1]; Port so1; Port shift[1]; Port ce; Port ue; Port ck; }
}
