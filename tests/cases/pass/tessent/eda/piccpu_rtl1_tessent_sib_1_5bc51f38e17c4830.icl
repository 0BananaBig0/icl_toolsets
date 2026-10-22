//-------------------------------------------------
//  File created by: Tessent Shell
//          Version: 2026.2
//       Created on: Wed Aug 12 13:52:01 CST 2026
//-------------------------------------------------





// instanced as piccpu.piccpu_rtl1_tessent_mbist_bap_inst



// instanced as piccpu.piccpu_rtl1_tessent_sib_mbist_inst



// instanced as piccpu.piccpu_rtl1_tessent_sib_sri_inst



// instanced as piccpu.piccpu_rtl1_tessent_sib_sti_inst



// instanced as piccpu.piccpu_rtl1_tessent_tdr_sri_ctrl_inst



// instanced as piccpu.regs_piccpu_rtl1_tessent_mbist_c1_controller_inst



// instanced as piccpu.regs_picdram



// instanced as piccpu.regs_picdram_interface_inst

NameSpace ;
UseNameSpace ;
Module piccpu_rtl1_tessent_sib_1 {
   // ICL module read from source on or near line 17 of file '/data/workspaces/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hybrid_tk_lbist_flow_v2026.2/tsdb_outdir/instruments/piccpu_rtl1_ijtag.instrument/piccpu_rtl1_tessent_sib_1.icl'
   ResetPort ijtag_reset {
      ActivePolarity 0;
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
   ToResetPort ijtag_to_reset {
      ActivePolarity 0;
      Source ijtag_reset;
   }
   ScanOutPort ijtag_to_si {
      Source ijtag_si;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   ToCaptureEnPort ijtag_to_ce {
      Source ijtag_ce;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   ToShiftEnPort ijtag_to_se {
      Source ijtag_se;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   ToUpdateEnPort ijtag_to_ue {
      Source ijtag_ue;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   ToTCKPort ijtag_to_tck {
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataInPort ltest_en {
      Attribute explicit_iwrite_only = 1'b1;
      Attribute connection_rule_option = "allowed_tied_low";
   }
   DataInPort ltest_occ_en {
      Attribute explicit_iwrite_only = 1'b1;
      Attribute connection_rule_option = "allowed_tied_low";
   }
   DataInPort ltest_static_clock_control_mode {
      Attribute connection_rule_option = "allowed_tied_low";
   }
   DataInPort ltest_clock_sequence[1:0] {
      Attribute connection_rule_option = "allowed_no_source";
      Attribute tessent_use_in_dft_specification = "false";
      Attribute function_modifier = "tessent_clock_sequence";
   }
   DataOutPort ltest_to_en {
      Source ltest_en;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataInPort ltest_mem_bypass_en {
      Attribute connection_rule_option = "allowed_tied";
   }
   DataOutPort ltest_to_mem_bypass_en {
      Source ltest_to_mem_bypass_en_int;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataInPort ltest_mcp_bounding_en {
      Attribute explicit_iwrite_only = 1'b1;
      Attribute connection_rule_option = "allowed_tied";
      Attribute forced_low_dft_signal_list = "ltest_en";
   }
   DataOutPort ltest_to_mcp_bounding_en {
      Source ltest_to_mcp_bounding_en_int;
      Attribute connection_rule_option = "allowed_no_destination";
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
      Port ijtag_to_tck;
      Port ijtag_to_reset;
      Port ijtag_to_se;
      Port ijtag_to_ce;
      Port ijtag_to_ue;
      Port ijtag_to_sel;
      Port ijtag_to_si;
   }
   Attribute keep_active_during_scan_test = "true";
   Attribute tessent_dft_function = "scan_tested_instrument_host";
   Attribute tessent_use_in_dft_specification = "false";
   Attribute tessent_instrument_type = "mentor::ijtag_node";
   Attribute tessent_signature = "2c039f215c6e37fa115da9be3899628e";
   Attribute tessent_instrument_container = "piccpu_rtl1_ijtag";
   Attribute tessent_instrument_subtype = "sib";
   ScanRegister sib {
      ScanInSource scan_in_mux;
      CaptureSource sib;
      ResetValue 1'b0;
   }
   ScanMux scan_in_mux SelectedBy sib, ltest_en {
      2'b10 : ijtag_from_so;
      2'b11 : 1'bx;
      2'bxx : ijtag_si;
   }
   LogicSignal ltest_to_mem_bypass_en_int {
      ltest_mem_bypass_en & ltest_en;
   }
   LogicSignal ltest_to_mcp_bounding_en_int {
      ltest_mcp_bounding_en & ltest_en;
   }
}
