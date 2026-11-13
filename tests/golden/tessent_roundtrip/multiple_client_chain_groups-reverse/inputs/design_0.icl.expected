Module MultipleGroupedCell {
  Parameter W = 4;
  ScanInPort si; SelectPort sel;
  ScanRegister value[$W-1:0] { ScanInSource si; CaptureSource value; ResetValue 0; }
  ScanOutPort so { Source value[0]; }
}
Module Chains {
 ScanInPort si[3:0]; SelectPort sel[1:0];
 CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue; TCKPort ck;
 Instance c0_0 Of MultipleGroupedCell { Parameter W = 4; InputPort si = si[0]; InputPort sel = sel[0]; }
 ScanOutPort so0 { Source c0_0.so; }
 Instance c0_1 Of MultipleGroupedCell { Parameter W = 2; InputPort si = si[1]; InputPort sel = sel[0]; }
 ScanOutPort so1 { Source c0_1.so; }
 ScanInterface bus0 { Port sel[0]; Port ce; Port se; Port ue; Port ck; Chain chain0 { Port si[0]; Port so0; } Chain chain1 { Port si[1]; Port so1; } }
 Instance c1_0 Of MultipleGroupedCell { Parameter W = 4; InputPort si = si[2]; InputPort sel = sel[1]; }
 ScanOutPort so2 { Source c1_0.so; }
 Instance c1_1 Of MultipleGroupedCell { Parameter W = 2; InputPort si = si[3]; InputPort sel = sel[1]; }
 ScanOutPort so3 { Source c1_1.so; }
 ScanInterface bus1 { Port sel[1]; Port ce; Port se; Port ue; Port ck; Chain chain0 { Port si[2]; Port so2; } Chain chain1 { Port si[3]; Port so3; } }
}
