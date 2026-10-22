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
      Source tdr;
   }
   ShiftEnPort ijtag_se;
   UpdateEnPort ijtag_ue;
   CaptureEnPort ijtag_ce;
   DataOutPort EN {
      Source tdr;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   ScanRegister tdr {
      ScanInSource ijtag_si;
      CaptureSource tdr;
      ResetValue 1'b0;
   }
}
