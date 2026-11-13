Module MultipleIndependentCell {
  Parameter W = 4;
  ScanInPort si; SelectPort sel;
  ScanRegister value[$W-1:0] { ScanInSource si; CaptureSource value; ResetValue 0; }
  ScanOutPort so { Source value[0]; }
}
Module Chains {
  ScanInPort si[1:0]; SelectPort sel[1:0];
  CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue; TCKPort ck;
  Instance c0 Of MultipleIndependentCell { InputPort si = si[0]; InputPort sel = sel[0]; }
  ScanOutPort so0 { Source c0.so; }
  ScanInterface bus0 { Port si[0]; Port so0; Port sel[0]; Port ce; Port se; Port ue; Port ck; }
  Instance c1 Of MultipleIndependentCell { InputPort si = si[1]; InputPort sel = sel[1]; }
  ScanOutPort so1 { Source c1.so; }
  ScanInterface bus1 { Port si[1]; Port so1; Port sel[1]; Port ce; Port se; Port ue; Port ck; }
}
