Module RawRoundtrip {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 ScanRegister payload[7:0] {
  ScanInSource si; CaptureSource payload; ResetValue 8'b0;
 }
 ScanOutPort so { Source payload[0]; }
 ScanInterface client {
  Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port reset;
  DefaultLoadValue 8'b0;
 }
}
