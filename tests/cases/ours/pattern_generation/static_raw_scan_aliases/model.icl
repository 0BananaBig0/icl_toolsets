Module AliasedRoot {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 Alias input = ~si;
 ScanRegister a[0:1] { ScanInSource input; CaptureSource a; ResetValue 2'b0; }
 Alias bridge = ~a[1];
 ScanRegister b[2:0] { ScanInSource bridge; CaptureSource b; ResetValue 3'b0; }
 Alias mirror[0:2] = ~b;
 ScanOutPort so { Source mirror[2]; }
 ScanInterface client {
  Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port reset;
  DefaultLoadValue 5'b0;
 }
}
