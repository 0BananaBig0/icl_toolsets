NameSpace ;
UseNameSpace ;
Module chip {
   ResetPort ijtag_reset {
      ActivePolarity 0;
      Attribute connection_rule_option = "allowed_no_source";
   }
   TCKPort ijtag_tck;
   ScanInPort ijtag_si;
   ScanOutPort ijtag_so {
      Source SIB_TOP.ijtag_so;
   }
   SelectPort ijtag_sel;
   ShiftEnPort ijtag_se;
   UpdateEnPort ijtag_ue;
   CaptureEnPort ijtag_ce;
   DataOutPort EN[11:0] {
      Source Block_1.EN[5:0], Block_2.EN[5:0];
   }
   Instance TDR_TOP_RESET Of RESET_TDR {
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_si = ijtag_si;
      InputPort ijtag_sel = ijtag_sel;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_reset = ijtag_reset;
   }
   Instance SIB_TOP Of SIB {
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_si = TDR_TOP_RESET.ijtag_so;
      InputPort ijtag_from_so = Block_2.ijtag_so;
      InputPort ijtag_sel = ijtag_sel;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_reset = TDR_TOP_RESET.ijtag_to_reset;
   }
   Instance Block_1 Of Block {
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_si = TDR_TOP_RESET.ijtag_so;
      InputPort ijtag_sel = SIB_TOP.ijtag_to_sel;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_reset = TDR_TOP_RESET.ijtag_to_reset;
   }
   Instance Block_2 Of Block {
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_si = Block_1.ijtag_so;
      InputPort ijtag_sel = SIB_TOP.ijtag_to_sel;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_reset = TDR_TOP_RESET.ijtag_to_reset;
   }
}
