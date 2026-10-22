NameSpace ;
UseNameSpace ;
Module TDR {
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
   DataInPort Temp[5:0] {
      Attribute connection_rule_option = "allowed_no_source";
   }
   DataOutPort EN[2:0] {
      Source tdr[2:0];
      Attribute connection_rule_option = "allowed_no_destination";
   }
   ScanRegister tdr[8:0] {
      ScanInSource ijtag_si;
      CaptureSource Temp, tdr[2:0];
      ResetValue 9'b000000000;
   }
}
