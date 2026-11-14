Module MultipleResetHidden {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue; TCKPort ck;
 DataInPort d[3:0];
 ScanRegister value[3:0] { ScanInSource si; CaptureSource d; ResetValue 0; }
 ScanOutPort so { Source value[0]; }
 ScanInterface bus { Port si; Port so; Port sel; Port ce; Port se; Port ue; Port ck; }
}
Module MultipleResetCell {
  Parameter W = 4;
  ScanInPort si; SelectPort sel; ResetPort rst;
  ScanRegister value[$W-1:0] { ScanInSource si; CaptureSource value; ResetValue 0; }
  ScanOutPort so { Source value[0]; }
  DataOutPort q[$W-1:0] { Source value; }
}
Module Chains {
 ResetPort global;
 DataMux reset_c0 SelectedBy sel[1] { 1'b0: global; 1'b1: 1'b1; }
 Instance h Of MultipleResetHidden { InputPort si = si[1]; InputPort sel = sel[1]; InputPort ce = ce; InputPort se = 1'b0; InputPort ue = ue; InputPort ck = ck; InputPort d = c0.q; }

  ScanInPort si[1:0]; SelectPort sel[1:0];
  CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue; TCKPort ck;
  Instance c0 Of MultipleResetCell { InputPort rst = reset_c0; InputPort si = si[0]; InputPort sel = sel[0]; }
  ScanOutPort so0 { Source c0.so; }
  ScanInterface bus0 { Port si[0]; Port so0; Port sel[0]; Port ce; Port se; Port ue; Port ck; }
  Instance c1 Of MultipleResetCell { InputPort rst = 1'b0; InputPort si = si[1]; InputPort sel = sel[1]; }
  ScanOutPort so1 { Source c1.so; }
  ScanInterface bus1 { Port si[1]; Port so1; Port sel[1]; Port ce; Port se; Port ue; Port ck; }
}
