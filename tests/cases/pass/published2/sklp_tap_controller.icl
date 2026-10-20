Module TapController {
TCKPort tck; 
ScanInPort tdi; 
ScanOutPort tdo {
  Source IRMux;
}
TMSPort tms; 
TRSTPort trst_n; 
ToCaptureEnPort sync_capture_dr; 
ToShiftEnPort ShiftDR; 
ToUpdateEnPort sync_update_dr; 
ToResetPort test_logic_reset {
  ActivePolarity 0;
}
ToResetPort ijtag_to_reset; 
ToSelectPort ijtag_to_select; 
ScanInPort host_ijtag_so; 
Instance fsm Of tap_fsm {
  InputPort tms = tms;
  InputPort trst = trst_n;
  InputPort tck = tck;
}
ScanInterface tap_client {
  Port tms;
  Port tdi;
  Port tdo;
}
ScanInterface host_ijtag {
  Port host_ijtag_so;
  Port ijtag_to_select;
}
ScanRegister bypass {
  ScanInSource tdi;
  CaptureSource 1'b0;
}
ScanRegister instruction[3:0] {
  ScanInSource tdi;
  CaptureSource 4'b1101;
  ResetValue 4'b1111;
}
ScanMux IRMux SelectedBy fsm.irSel{
  1'b0 : DRMux;
  1'b1 : instruction[0];
}
ScanMux DRMux SelectedBy instruction{
  4'b1111 : bypass;
  4'b0000 : host_ijtag_so;
}
  }
Module tap_fsm {
TCKPort tck; 
TMSPort tms; 
TRSTPort trst; 
ToIRSelectPort irSel; 
ToResetPort tlr; 
}
