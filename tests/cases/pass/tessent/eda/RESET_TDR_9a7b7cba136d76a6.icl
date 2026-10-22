NameSpace ;
UseNameSpace ;
Module RESET_TDR {
   ResetPort ijtag_reset {
      ActivePolarity 0;
      Attribute connection_rule_option = "allowed_no_source";
   }
   SelectPort ijtag_sel;
   TCKPort ijtag_tck;
   ScanInPort ijtag_si;
   ScanOutPort ijtag_so {
      Source tdr[0];
      Attribute connection_rule_option = "allowed_no_destination";
   }
   ShiftEnPort ijtag_se;
   UpdateEnPort ijtag_ue;
   CaptureEnPort ijtag_ce;
   ToResetPort ijtag_to_reset {
      Source reset_mux;
      ActivePolarity 0;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   ScanRegister tdr[1:0] {
      ScanInSource ijtag_si;
      CaptureSource tdr[1:0];
      ResetValue 2'b10;
   }
   DataMux reset_mux SelectedBy tdr[0] {
      1'b0 : ijtag_reset;
      1'b1 : tdr[1];
   }
}
