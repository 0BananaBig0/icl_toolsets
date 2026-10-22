//-------------------------------------------------
//  File created by: Tessent Shell
//          Version: 2026.2
//       Created on: Wed Oct 21 16:19:15 CST 2026
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
Module chip {
   // Created by ICL extraction
   DataOutPort EN[11] {
      Source Block_2.EN[5];
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataOutPort EN[10] {
      Source Block_2.EN[4];
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataOutPort EN[9] {
      Source Block_2.EN[3];
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataOutPort EN[8] {
      Source Block_2.EN[2];
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataOutPort EN[7] {
      Source Block_2.EN[1];
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataOutPort EN[6] {
      Source Block_2.EN[0];
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataOutPort EN[5] {
      Source Block_1.EN[5];
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataOutPort EN[4] {
      Source Block_1.EN[4];
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataOutPort EN[3] {
      Source Block_1.EN[3];
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataOutPort EN[2] {
      Source Block_1.EN[2];
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataOutPort EN[1] {
      Source Block_1.EN[1];
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataOutPort EN[0] {
      Source Block_1.EN[0];
      Attribute connection_rule_option = "allowed_no_destination";
   }
   CaptureEnPort ijtag_ce;
   ResetPort ijtag_reset {
      ActivePolarity 0;
      Attribute connection_rule_option = "allowed_no_source";
   }
   ShiftEnPort ijtag_se;
   SelectPort ijtag_sel;
   ScanInPort ijtag_si;
   ScanOutPort ijtag_so {
      Source SIB_TOP.ijtag_so;
   }
   TCKPort ijtag_tck;
   UpdateEnPort ijtag_ue;
   ScanInterface C0 {
      Port ijtag_si;
      Port ijtag_so;
      Port ijtag_sel;
      Port ijtag_se;
      Port ijtag_ce;
      Port ijtag_ue;
      Port ijtag_reset;
      Port ijtag_tck;
   }
   Attribute tessent_design_format = "verilog_2001";
   Attribute forced_high_input_port_list = "{test_enable}";
   Attribute icl_extraction_date = "Wed Oct 21 16:19:15 CST 2026";
   Attribute created_by_tessent_icl_extract = "true";
   Attribute tessent_design_id = "gate";
   Instance Block_1 Of Block {
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_reset = TDR_TOP_RESET.ijtag_to_reset;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_sel = SIB_TOP.ijtag_to_sel;
      InputPort ijtag_si = TDR_TOP_RESET.ijtag_so;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_ue = ijtag_ue;
      Attribute tessent_design_instance = "Block_1";
   }
   Instance Block_2 Of Block {
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_reset = TDR_TOP_RESET.ijtag_to_reset;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_sel = SIB_TOP.ijtag_to_sel;
      InputPort ijtag_si = Block_1.ijtag_so;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_ue = ijtag_ue;
      Attribute tessent_design_instance = "Block_2";
   }
   Instance SIB_TOP Of SIB {
      InputPort ijtag_reset = TDR_TOP_RESET.ijtag_to_reset;
      InputPort ijtag_sel = ijtag_sel;
      InputPort ijtag_si = TDR_TOP_RESET.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_from_so = Block_2.ijtag_so;
      Attribute tessent_design_instance = "SIB_TOP";
   }
   Instance TDR_TOP_RESET Of RESET_TDR {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = ijtag_sel;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_si = ijtag_si;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_ce = ijtag_ce;
      Attribute tessent_design_instance = "TDR_TOP_RESET";
   }
}
