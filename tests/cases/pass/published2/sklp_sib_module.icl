Module picorv32_top_sib {
ResetPort ijtag_reset {
  ActivePolarity 0;
}
SelectPort ijtag_sel; 
ScanInPort ijtag_si; 
CaptureEnPort ijtag_ce; 
UpdateEnPort ijtag_ue; 
ShiftEnPort ijtag_se; 
TCKPort ijtag_tck; 
ScanOutPort ijtag_so {
  Source picorv32_top_sib;
}
ScanInPort ijtag_from_so; 
ToSelectPort ijtag_to_sel; 
ScanInterface client {
  Port ijtag_si;
  Port ijtag_so;
  Port ijtag_sel;
}
ScanInterface host {
  Port ijtag_to_sel;
  Port ijtag_from_so;
}
ScanRegister picorv32_top_sib {
  ScanInSource scan_in_mux;
  CaptureSource 1'b0;
  DefaultLoadValue 1'b0;
  ResetValue 1'b0;
}
ScanMux scan_in_mux SelectedBy picorv32_top_sib{
  1'b0 : ijtag_si;
  1'b1 : ijtag_from_so;
}
  }
