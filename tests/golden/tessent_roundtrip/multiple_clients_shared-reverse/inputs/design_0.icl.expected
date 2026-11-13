Module MultipleSharedCell {
  Parameter W = 4;
  ScanInPort si; SelectPort sel;
  ScanRegister value[$W-1:0] { ScanInSource si; CaptureSource value; ResetValue 0; }
  ScanOutPort so { Source value[0]; }
}
Module Chains {
  ScanInPort si[1:0]; SelectPort sel[1:0]; DataInPort route;
  CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue; TCKPort ck;
  ScanMux input SelectedBy sel[0] { 1'b1: si[0]; 1'b0: si[1]; }
  LogicSignal selected { sel[0] | sel[1]; }
  Instance c Of MultipleSharedCell { InputPort si = input; InputPort sel = selected; }
  ScanOutPort so0 { Source c.so; Enable ~route; }
  ScanOutPort so1 { Source c.so; Enable route; }
  ScanInterface bus0 { Port si[0]; Port so0; Port sel[0]; Port ce; Port se; Port ue; Port ck; }
  ScanInterface bus1 { Port si[1]; Port so1; Port sel[1]; Port ce; Port se; Port ue; Port ck; }
}
