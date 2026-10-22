//-------------------------------------------------
//  File created by: Tessent Shell
//          Version: 2026.2
//       Created on: Thu Aug 27 11:28:14 CST 2026
//-------------------------------------------------





// instanced as processor_core.DATA_MEM_1
// instanced as processor_core.PROG_MEM_2



// instanced as processor_core.DATA_MEM_1_interface_inst



// instanced as processor_core.PROG_MEM_2_interface_inst



// instanced as processor_core.processor_core_rtl1_tessent_mbist_bap_inst



// instanced as processor_core.processor_core_rtl1_tessent_mbist_c1_controller_inst



// instanced as processor_core.processor_core_rtl1_tessent_sib_mbist_inst



// instanced as processor_core.processor_core_rtl1_tessent_sib_sti_inst



// instanced as processor_core.processor_core_rtl2_tessent_edt_c1_ext_inst



// instanced as processor_core.processor_core_rtl2_tessent_edt_c1_ext_tdr_inst



// instanced as processor_core.processor_core_rtl2_tessent_edt_c1_int_inst



// instanced as processor_core.processor_core_rtl2_tessent_edt_c1_int_tdr_inst



// instanced as processor_core.processor_core_rtl2_tessent_occ_dco_clk_inst



// instanced as processor_core_rtl2_tessent_occ.tdr_sib



// instanced as processor_core.processor_core_rtl2_tessent_sib_edt_inst
// instanced as processor_core.processor_core_rtl2_tessent_sib_occ_inst
// instanced as processor_core.processor_core_rtl2_tessent_sib_sri_ctrl_inst
// instanced as processor_core.processor_core_rtl2_tessent_sib_ssn_inst



// instanced as processor_core.processor_core_rtl2_tessent_sib_sri_inst



// instanced as processor_core.processor_core_rtl2_tessent_ssn_pipe_1_inst
// instanced as processor_core.processor_core_rtl2_tessent_ssn_pipe_2_inst



// instanced as processor_core.processor_core_rtl2_tessent_ssn_scan_host_1_inst



// instanced as processor_core_rtl2_tessent_ssn_scan_host_1.inner



// instanced as processor_core.processor_core_rtl2_tessent_tdr_sri_ctrl_inst

NameSpace ;
UseNameSpace ;
Module processor_core_rtl1_tessent_sib_1 {
   // ICL module read from source on or near line 1149 of file '/home/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl2.dft_inserted_design/processor_core.icl'
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
   Attribute tessent_signature = "7ae0bd514ccf669a1bbc1f88cba987ee";
   Attribute tessent_instrument_container = "processor_core_rtl1_ijtag";
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
