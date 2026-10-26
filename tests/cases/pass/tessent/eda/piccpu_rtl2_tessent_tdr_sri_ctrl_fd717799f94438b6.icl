//-------------------------------------------------
//  File created by: Tessent Shell
//          Version: 2026.2
//       Created on: Mon Oct 26 08:55:11 CST 2026
//-------------------------------------------------





// instanced as piccpu.piccpu_rtl1_tessent_mbist_bap_inst



// instanced as piccpu.piccpu_rtl1_tessent_sib_mbist_inst



// instanced as piccpu.piccpu_rtl1_tessent_sib_sri_inst



// instanced as piccpu.piccpu_rtl1_tessent_sib_sti_inst



// instanced as piccpu.piccpu_rtl1_tessent_tdr_sri_ctrl_inst



// instanced as piccpu.piccpu_rtl2_tessent_edt_lbist_c0_inst



// instanced as piccpu_rtl2_tessent_edt_lbist_c0.piccpu_rtl2_tessent_edt_c0_sib_decompressor_i
// instanced as piccpu_rtl2_tessent_edt_lbist_c0.piccpu_rtl2_tessent_edt_c0_sib_lbist_lp_static_control_i
// instanced as piccpu_rtl2_tessent_edt_lbist_c0.piccpu_rtl2_tessent_edt_c0_sib_lbist_lp_mask_shift_reg_i
// instanced as piccpu_rtl2_tessent_edt_lbist_c0.piccpu_rtl2_tessent_edt_c0_sib_chain_mask_i
// instanced as piccpu_rtl2_tessent_edt_lbist_c0.piccpu_rtl2_tessent_edt_c0_sib_misr_i



// instanced as piccpu.piccpu_rtl2_tessent_edt_lbist_c0_tdr_inst



// instanced as piccpu.piccpu_rtl2_tessent_lbist_inst



// instanced as piccpu_rtl2_tessent_lbist.piccpu_rtl2_tessent_lbist_sib_bist_registers_i
// instanced as piccpu_rtl2_tessent_lbist.piccpu_rtl2_tessent_lbist_sib_ncp_limits_sib_i
// instanced as piccpu_rtl2_tessent_lbist.piccpu_rtl2_tessent_lbist_sib_control_registers_i
// instanced as piccpu_rtl2_tessent_lbist.piccpu_rtl2_tessent_lbist_edt_sib_i



// instanced as piccpu.piccpu_rtl2_tessent_lbist_ncp_index_decoder_inst



// instanced as piccpu.piccpu_rtl2_tessent_occ_clk_inst
// instanced as piccpu.piccpu_rtl2_tessent_occ_ramclk_inst



// instanced as piccpu_rtl2_tessent_occ.tdr_sib



// instanced as piccpu.piccpu_rtl2_tessent_sib_edt_inst
// instanced as piccpu.piccpu_rtl2_tessent_sib_lbist_inst
// instanced as piccpu.piccpu_rtl2_tessent_sib_occ_inst
// instanced as piccpu.piccpu_rtl2_tessent_sib_sri_ctrl_inst



// instanced as piccpu.piccpu_rtl2_tessent_single_chain_mode_logic_inst



// instanced as piccpu_rtl2_tessent_single_chain_mode_logic.tdr_sib_i
// instanced as piccpu_rtl2_tessent_single_chain_mode_logic.single_chain_sib_i
// instanced as piccpu_rtl2_tessent_edt_internal_scan_registers.blk1_sib_i



// instanced as piccpu_rtl2_tessent_single_chain_mode_logic.piccpu_rtl2_tessent_edt_internal_scan_registers_i



// instanced as piccpu.piccpu_rtl2_tessent_tdr_sri_ctrl_inst



// instanced as piccpu.regs_piccpu_rtl1_tessent_mbist_c1_controller_inst



// instanced as piccpu.regs_picdram



// instanced as piccpu.regs_picdram_interface_inst

NameSpace ;
UseNameSpace ;
Module piccpu_rtl2_tessent_tdr_sri_ctrl {
   // ICL module read from source on or near line 1853 of file '/home/lianghuaxiao/workspaces/tessent_2026.2_usage_examples/tessent_example_hybrid_tk_lbist_flow_v2026.2/tsdb_outdir/dft_inserted_designs/piccpu_rtl2.dft_inserted_design/piccpu.icl'
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
      Source tdr[0];
   }
   DataOutPort ext_mode {
      Source tdr[9];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_dft_signal_name = "ext_mode";
      Attribute tessent_dft_signal_usage = "scan_mode";
      Attribute tessent_dft_signal_scan_mode_type = "external";
      Attribute tessent_dft_signal_reset_value = 0;
      Attribute tessent_persistent_design_pin = 
          "{tessent_persistent_cell_ext_mode/Y}";
   }
   DataOutPort int_mode {
      Source tdr[8];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_dft_signal_name = "int_mode";
      Attribute tessent_dft_signal_usage = "scan_mode";
      Attribute tessent_dft_signal_scan_mode_type = "internal";
      Attribute tessent_dft_signal_reset_value = 0;
      Attribute tessent_persistent_design_pin = 
          "{tessent_persistent_cell_int_mode/Y}";
   }
   DataOutPort ext_ltest_en {
      Source tdr[7];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_dft_signal_name = "ext_ltest_en";
      Attribute tessent_dft_signal_usage = "logic_test_control";
      Attribute tessent_dft_signal_value_in_pre_scan_drc = "x";
      Attribute tessent_dft_signal_reset_value = 0;
      Attribute tessent_persistent_design_pin = 
          "{tessent_persistent_cell_ext_ltest_en/Y}";
   }
   DataOutPort int_ltest_en {
      Source tdr[6];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_dft_signal_name = "int_ltest_en";
      Attribute tessent_dft_signal_usage = "logic_test_control";
      Attribute tessent_dft_signal_value_in_pre_scan_drc = "x";
      Attribute tessent_dft_signal_reset_value = 0;
      Attribute tessent_persistent_design_pin = 
          "{tessent_persistent_cell_int_ltest_en/Y}";
   }
   DataOutPort x_bounding_en {
      Source tdr[5];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_dft_signal_name = "x_bounding_en";
      Attribute tessent_dft_signal_usage = "logic_test_control";
      Attribute tessent_dft_signal_value_in_pre_scan_drc = "x";
      Attribute tessent_dft_signal_reset_value = 0;
      Attribute tessent_persistent_design_pin = 
          "{tessent_persistent_cell_x_bounding_en/Y}";
   }
   DataOutPort observe_test_point_en {
      Source tdr[4];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_dft_signal_name = "observe_test_point_en";
      Attribute tessent_dft_signal_usage = "logic_test_control";
      Attribute tessent_dft_signal_value_in_pre_scan_drc = "x";
      Attribute tessent_dft_signal_reset_value = 0;
      Attribute tessent_persistent_design_pin = 
          "{tessent_persistent_cell_observe_test_point_en/Y}";
   }
   DataOutPort control_test_point_en {
      Source tdr[3];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_dft_signal_name = "control_test_point_en";
      Attribute tessent_dft_signal_usage = "logic_test_control";
      Attribute tessent_dft_signal_value_in_pre_scan_drc = "x";
      Attribute tessent_dft_signal_reset_value = 0;
      Attribute tessent_persistent_design_pin = 
          "{tessent_persistent_cell_control_test_point_en/Y}";
   }
   DataOutPort tck_occ_en {
      Source tdr[2];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_dft_signal_name = "tck_occ_en";
      Attribute tessent_dft_signal_usage = "global_dft_control";
      Attribute tessent_dft_signal_default_value_in_all_test = 0;
      Attribute tessent_dft_signal_reset_value = 0;
      Attribute tessent_persistent_design_pin = 
          "{tessent_persistent_cell_tck_occ_en/Y}";
   }
   DataOutPort memory_bypass_en {
      Source tdr[1];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_dft_signal_name = "memory_bypass_en";
      Attribute tessent_dft_signal_usage = "logic_test_control";
      Attribute tessent_dft_signal_value_in_pre_scan_drc = "1";
      Attribute tessent_dft_signal_reset_value = 0;
      Attribute tessent_persistent_design_pin = 
          "{tessent_persistent_cell_memory_bypass_en/Y}";
   }
   DataOutPort ltest_en {
      Source tdr[0];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_dft_signal_name = "ltest_en";
      Attribute tessent_dft_signal_usage = "logic_test_control";
      Attribute tessent_dft_signal_value_in_pre_scan_drc = "1";
      Attribute tessent_dft_signal_reset_value = 0;
      Attribute tessent_persistent_design_pin = 
          "{tessent_persistent_cell_ltest_en/Y}";
   }
   ScanInterface client {
      Port ijtag_si;
      Port ijtag_so;
      Port ijtag_sel;
      Port ijtag_tck;
      Port ijtag_reset;
      Port ijtag_ce;
      Port ijtag_se;
      Port ijtag_ue;
   }
   Attribute keep_active_during_scan_test = "true";
   Attribute tessent_dft_function = "scan_resource_instrument_dft_control";
   Attribute forced_low_output_port_list = 
       "control_test_point_en observe_test_point_en x_bounding_en";
   Attribute tessent_use_in_dft_specification = "false";
   Attribute tessent_instrument_type = "mentor::ijtag_node";
   Attribute tessent_instrument_subtype = "tessent_tdr";
   Attribute tessent_signature = "509a38ade19ba711df4fdc50fac551ca";
   Attribute tessent_instrument_container = "piccpu_rtl2_ijtag";
   ScanRegister tdr[9:0] {
      ScanInSource ijtag_si;
      CaptureSource 10'b0000000000;
      DefaultLoadValue 10'b0000000000;
      ResetValue 10'b0000000000;
   }
}
