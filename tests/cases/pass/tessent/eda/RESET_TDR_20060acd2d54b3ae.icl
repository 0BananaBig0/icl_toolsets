//-------------------------------------------------
//  File created by: Tessent Shell
//          Version: 2024.2
//       Created on: Mon Oct 20 23:31:21 CST 2025
//-------------------------------------------------





// instanced as chip.Block_1
// instanced as chip.Block_2



// instanced as Block.SIB_1
// instanced as Block.SIB_2
// instanced as chip.SIB_TOP



// instanced as Block.Sensor_3
// instanced as Block.Sensor_4



// instanced as Block.TDR_1_RESET
// instanced as chip.TDR_TOP_RESET



// instanced as RESET_TDR.tdr



// instanced as Block.TDR_2
// instanced as Block.TDR_3

NameSpace ;
UseNameSpace ;
Module RESET_TDR {
   // ICL module read from source on or near line 1 of file 'RESET_TDR.icl'
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
      ActivePolarity 0;
      Source reset_mux;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataMux reset_mux SelectedBy tdr.EN[0] {
      1'b0 : ijtag_reset;
      1'b1 : tdr.EN[1];
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
}
