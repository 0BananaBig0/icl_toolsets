Module EdgeAliasEndStateConcat {
  ScanInPort si;
  ScanRegister value[3:0] { ScanInSource si; ResetValue 4'b1001; }
  Alias saved[3:0] = value[3:2], value[1:0] {
    iApplyEndState 2'b10, /* low bits */ 2'b01;
    AccessTogether;
  }
}
