NameSpace ;
UseNameSpace ;
Module TDR_FOR_RESET {
   ResetPort ijtag_reset {
      ActivePolarity 0;
      Attribute connection_rule_option = "allowed_no_source";
   }
   SelectPort ijtag_sel;
   TCKPort ijtag_tck;
   ScanInPort ijtag_si;
   ScanOutPort ijtag_so {
      Source tdr[0];
   }
   ShiftEnPort ijtag_se;
   UpdateEnPort ijtag_ue;
   CaptureEnPort ijtag_ce;
   DataOutPort EN[1:0] {
      Source tdr[1:0];
      Attribute connection_rule_option = "allowed_no_destination";
   }
   ScanRegister tdr[1:0] {
      ScanInSource ijtag_si;
      CaptureSource tdr[1:0];
      ResetValue 2'b10;
   }
}
