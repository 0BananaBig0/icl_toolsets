Module RawSibLeaf {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 ScanRegister value[7:0] { ScanInSource si; CaptureSource value; ResetValue 8'b0; }
 ScanOutPort so { Source value[0]; }
 ScanInterface local { Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port reset; }
}
Module Top {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 Instance child Of RawSibLeaf {
  InputPort si = si; InputPort sel = open; InputPort ce = ce;
  InputPort se = se; InputPort ue = ue; InputPort tck = tck; InputPort reset = reset;
 }
 ScanMux route SelectedBy open { 1'b0: si; 1'b1: child.so; }
 ScanRegister open { ScanInSource route; CaptureSource open; ResetValue 1'b0; }
 ScanOutPort so { Source open; }
 ScanInterface client {
  Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port reset;
  DefaultLoadValue 1'b0;
 }
}
