//-------------------------------------------------
//  File created by: Tessent Shell
//          Version: 2024.2
//       Created on: Mon Oct 20 23:31:21 CST 2025
//-------------------------------------------------


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
      Port ijtag_ce;
      Port ijtag_reset;
      Port ijtag_se;
      Port ijtag_sel;
      Port ijtag_si;
      Port ijtag_so;
      Port ijtag_tck;
      Port ijtag_ue;
   }
   Attribute tessent_design_format = "verilog_2001";
   Attribute icl_extraction_date = "Mon Oct 20 23:31:21 CST 2025";
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

// instanced as chip.Block_1
// instanced as chip.Block_2
Module Block {
   // Created by ICL extraction
   DataOutPort EN[5] {
      Source TDR_3.EN[2];
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataOutPort EN[4] {
      Source TDR_3.EN[1];
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataOutPort EN[3] {
      Source TDR_3.EN[0];
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataOutPort EN[2] {
      Source TDR_2.EN[2];
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataOutPort EN[1] {
      Source TDR_2.EN[1];
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataOutPort EN[0] {
      Source TDR_2.EN[0];
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
      Source SIB_1.ijtag_so;
   }
   TCKPort ijtag_tck;
   UpdateEnPort ijtag_ue;
   ScanInterface C0 {
      Port ijtag_ce;
      Port ijtag_reset;
      Port ijtag_se;
      Port ijtag_sel;
      Port ijtag_si;
      Port ijtag_so;
      Port ijtag_tck;
      Port ijtag_ue;
   }
   Attribute tessent_design_format = "verilog_2001";
   Attribute icl_extraction_date = "Mon Oct 20 23:31:21 CST 2025";
   Attribute created_by_tessent_icl_extract = "true";
   Attribute tessent_design_id = "gate";
   Instance SIB_1 Of SIB {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = ijtag_sel;
      InputPort ijtag_si = TDR_1_RESET.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_from_so = SIB_2.ijtag_so;
      Attribute tessent_design_instance = "SIB_1";
   }
   Instance SIB_2 Of SIB {
      InputPort ijtag_reset = TDR_1_RESET.ijtag_to_reset;
      InputPort ijtag_sel = SIB_1.ijtag_to_sel;
      InputPort ijtag_si = TDR_2.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_from_so = TDR_3.ijtag_so;
      Attribute tessent_design_instance = "SIB_2";
   }
   Instance Sensor_3 Of Sensor {
      InputPort ijtag_tck = ijtag_tck;
      InputPort EN[2] = TDR_2.EN[2];
      InputPort EN[1] = TDR_2.EN[1];
      InputPort EN[0] = TDR_2.EN[0];
      Attribute tessent_design_instance = "Sensor_3";
   }
   Instance Sensor_4 Of Sensor {
      InputPort ijtag_tck = ijtag_tck;
      InputPort EN[2] = TDR_3.EN[2];
      InputPort EN[1] = TDR_3.EN[1];
      InputPort EN[0] = TDR_3.EN[0];
      Attribute tessent_design_instance = "Sensor_4";
   }
   Instance TDR_1_RESET Of RESET_TDR {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = ijtag_sel;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_si = ijtag_si;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_ce = ijtag_ce;
      Attribute tessent_design_instance = "TDR_1_RESET";
   }
   Instance TDR_2 Of TDR {
      InputPort ijtag_reset = TDR_1_RESET.ijtag_to_reset;
      InputPort ijtag_sel = SIB_1.ijtag_to_sel;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_si = TDR_1_RESET.ijtag_so;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_ce = ijtag_ce;
      InputPort Temp[5] = Sensor_3.Temp[5];
      InputPort Temp[4] = Sensor_3.Temp[4];
      InputPort Temp[3] = Sensor_3.Temp[3];
      InputPort Temp[2] = Sensor_3.Temp[2];
      InputPort Temp[1] = Sensor_3.Temp[1];
      InputPort Temp[0] = Sensor_3.Temp[0];
      Attribute tessent_design_instance = "TDR_2";
   }
   Instance TDR_3 Of TDR {
      InputPort ijtag_reset = TDR_1_RESET.ijtag_to_reset;
      InputPort ijtag_sel = SIB_2.ijtag_to_sel;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_si = TDR_2.ijtag_so;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_ce = ijtag_ce;
      InputPort Temp[5] = Sensor_4.Temp[5];
      InputPort Temp[4] = Sensor_4.Temp[4];
      InputPort Temp[3] = Sensor_4.Temp[3];
      InputPort Temp[2] = Sensor_4.Temp[2];
      InputPort Temp[1] = Sensor_4.Temp[1];
      InputPort Temp[0] = Sensor_4.Temp[0];
      Attribute tessent_design_instance = "TDR_3";
   }
}

// instanced as Block.SIB_1
// instanced as Block.SIB_2
// instanced as chip.SIB_TOP
Module SIB {
   // ICL module read from source on or near line 1 of file 'SIB.icl'
   ResetPort ijtag_reset {
      ActivePolarity 0;
      Attribute connection_rule_option = "allowed_no_source";
   }
   SelectPort ijtag_sel;
   ScanInPort ijtag_si;
   CaptureEnPort ijtag_ce;
   ShiftEnPort ijtag_se;
   UpdateEnPort ijtag_ue;
   TCKPort ijtag_tck;
   ScanOutPort ijtag_so {
      Source sib;
   }
   ToSelectPort ijtag_to_sel {
      Attribute connection_rule_option = "allowed_no_destination";
   }
   ScanInPort ijtag_from_so {
      Attribute connection_rule_option = "allowed_no_source";
   }
   ScanInterface client {
      Port ijtag_si;
      Port ijtag_so;
      Port ijtag_sel;
      Port ijtag_tck;
      Port ijtag_reset;
      Port ijtag_se;
      Port ijtag_ce;
      Port ijtag_ue;
   }
   ScanInterface host {
      Port ijtag_from_so;
      Port ijtag_to_sel;
   }
   ScanRegister sib {
      ScanInSource scan_in_mux;
      CaptureSource 1'b0;
      ResetValue 1'b0;
   }
   ScanMux scan_in_mux SelectedBy sib {
      1'b0 : ijtag_si;
      1'b1 : ijtag_from_so;
   }
}

// instanced as Block.Sensor_3
// instanced as Block.Sensor_4
Module Sensor {
   // ICL module read from source on or near line 1 of file 'Sensor.icl'
   TCKPort ijtag_tck;
   DataInPort EN[2:0];
   DataOutPort Temp[5:0];
}

// instanced as Block.TDR_1_RESET
// instanced as chip.TDR_TOP_RESET
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

// instanced as RESET_TDR.tdr
Module TDR_FOR_RESET {
   // ICL module read from source on or near line 1 of file 'TDR_FOR_RESET.icl'
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

// instanced as Block.TDR_2
// instanced as Block.TDR_3
Module TDR {
   // ICL module read from source on or near line 1 of file 'TDR.icl'
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
