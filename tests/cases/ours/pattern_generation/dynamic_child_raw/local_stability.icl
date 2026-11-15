Module ProjectionRawLeaf {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 ScanRegister value[7:0] { ScanInSource si; CaptureSource value; ResetValue 8'b0; }
 DataOutPort gate { Source value[0]; }
 ScanOutPort so { Source value[0]; }
 ScanInterface local { Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port reset; }
}
Module Top {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 Instance child Of ProjectionRawLeaf {
  InputPort si = route; InputPort sel = sel; InputPort ce = ce;
  InputPort se = se; InputPort ue = ue; InputPort tck = tck; InputPort reset = reset;
 }
 Instance sibling Of ProjectionRawLeaf {
  InputPort si = si; InputPort sel = child.gate; InputPort ce = ce;
  InputPort se = se; InputPort ue = ue; InputPort tck = tck; InputPort reset = reset;
 }
 ScanMux route SelectedBy child.gate { 1'b0: si; 1'b1: sibling.so; }
 ScanOutPort so { Source child.so; }
 ScanInterface client {
  Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port reset;
  DefaultLoadValue 8'b0;
 }
}
