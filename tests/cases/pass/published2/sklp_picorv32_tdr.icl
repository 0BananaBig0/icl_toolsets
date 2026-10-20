Module picorv32_top_ctrl_tdr {
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
  Source tdr[0];
}
DataOutPort ijtag_to_data_in[1:0] {
  Source tdr[1:0];
}
ScanInterface client {
  Port ijtag_si;
  Port ijtag_so;
  Port ijtag_sel;
}
ScanRegister tdr[1:0] {
  ScanInSource ijtag_si;
  CaptureSource 2'b00;
  DefaultLoadValue 2'b00;
  ResetValue 2'b00;
}
  }
