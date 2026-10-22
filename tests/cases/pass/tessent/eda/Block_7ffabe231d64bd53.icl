NameSpace ;
UseNameSpace ;
Module Block {
   ResetPort ijtag_reset {
      ActivePolarity 0;
      Attribute connection_rule_option = "allowed_no_source";
   }
   TCKPort ijtag_tck;
   ScanInPort ijtag_si;
   ScanOutPort ijtag_so {
      Source SIB_1.ijtag_so;
   }
   SelectPort ijtag_sel;
   ShiftEnPort ijtag_se;
   UpdateEnPort ijtag_ue;
   CaptureEnPort ijtag_ce;
   DataOutPort EN[5:0] {
      Source TDR_2.EN[2:0], TDR_3.EN[2:0];
   }
   Instance TDR_1_RESET Of RESET_TDR {
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_si = ijtag_si;
      InputPort ijtag_sel = ijtag_sel;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_reset = ijtag_reset;
   }
   Instance SIB_1 Of SIB {
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_si = TDR_1_RESET.ijtag_so;
      InputPort ijtag_from_so = SIB_2.ijtag_so;
      InputPort ijtag_sel = ijtag_sel;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_reset = ijtag_reset;
   }
   Instance TDR_2 Of TDR {
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_si = TDR_1_RESET.ijtag_so;
      InputPort ijtag_sel = SIB_1.ijtag_to_sel;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_reset = TDR_1_RESET.ijtag_to_reset;
      InputPort Temp = Sensor_3.Temp;
   }
   Instance SIB_2 Of SIB {
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_si = TDR_2.ijtag_so;
      InputPort ijtag_from_so = TDR_3.ijtag_so;
      InputPort ijtag_sel = SIB_1.ijtag_to_sel;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_reset = TDR_1_RESET.ijtag_to_reset;
   }
   Instance TDR_3 Of TDR {
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_si = TDR_2.ijtag_so;
      InputPort ijtag_sel = SIB_2.ijtag_to_sel;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_reset = TDR_1_RESET.ijtag_to_reset;
      InputPort Temp = Sensor_4.Temp;
   }
   Instance Sensor_3 Of Sensor {
      InputPort ijtag_tck = ijtag_tck;
      InputPort EN = TDR_2.EN;
   }
   Instance Sensor_4 Of Sensor {
      InputPort ijtag_tck = ijtag_tck;
      InputPort EN = TDR_3.EN;
   }

}
