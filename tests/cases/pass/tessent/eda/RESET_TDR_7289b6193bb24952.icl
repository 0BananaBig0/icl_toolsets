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
      Source tdr.ijtag_so;
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
   Instance tdr Of TDR_FOR_RESET {
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_si = ijtag_si;
      InputPort ijtag_sel = ijtag_sel;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_reset = ijtag_reset;
   }
   DataMux reset_mux SelectedBy tdr.EN[0] {
      1'b0 : ijtag_reset;
      1'b1 : tdr.EN[1];
   }
}
