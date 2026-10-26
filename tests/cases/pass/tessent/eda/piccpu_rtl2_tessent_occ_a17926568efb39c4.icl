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
Module piccpu_rtl2_tessent_occ {
   // ICL module read from source on or near line 1452 of file '/home/lianghuaxiao/workspaces/tessent_2026.2_usage_examples/tessent_example_hybrid_tk_lbist_flow_v2026.2/tsdb_outdir/dft_inserted_designs/piccpu_rtl2.dft_inserted_design/piccpu.icl'
   ClockPort fast_clock {
      Attribute icl_extraction_port_trigger_list = "clock_out";
   }
   ToClockPort clock_out {
      Source tck_mux;
      Attribute exclude_from_sdc = "on";
   }
   TCKPort ijtag_tck;
   ScanInPort ijtag_si;
   ScanOutPort ijtag_so {
      Source tdr_sib.scan_out;
   }
   ResetPort ijtag_reset {
      ActivePolarity 0;
   }
   CaptureEnPort ijtag_ce;
   ShiftEnPort ijtag_se;
   UpdateEnPort ijtag_ue;
   SelectPort ijtag_sel;
   DataInPort static_clock_control_mode {
      Attribute connection_rule_option = "allowed_tied";
   }
   DataInPort clock_sequence[2:0] {
      Attribute connection_rule_option = "allowed_no_source";
      Attribute function_modifier = "tessent_clock_sequence";
   }
   Attribute keep_active_during_scan_test = "true";
   Attribute tessent_use_in_dft_specification = "false";
   Attribute tessent_instrument_type = "mentor::occ";
   Attribute tessent_instrument_subtype = "standard";
   Attribute tessent_instrument_container = "piccpu_rtl2_occ.instrument";
   Attribute tessent_signature = "3352e3b25ca0babc5451969537f9888f";
   Attribute tdr_parent_instance = "ijtag_control";
   Alias test_mode = tdr[0] {
   }
   Alias fast_capture_mode = tdr[1] {
   }
   Alias active_upstream_parent_occ = tdr[2] {
   }
   Alias capture_cycle_width[1:0] = tdr[4:3] {
   }
   Alias inject_tck = tdr[5] {
   }
   Alias ijtag_static_clock_control_mode = tdr[6] {
   }
   Alias ijtag_clock_sequence[2:0] = tdr[9:7] {
   }
   ScanRegister tdr[9:0] {
      ScanInSource ijtag_si;
      CaptureSource 10'b0000000000;
      ResetValue 10'b0000000000;
   }
   ClockMux tck_mux SelectedBy inject_tck {
      1'b1 : ijtag_tck;
      1'b0 : fast_clock;
   }
   Instance tdr_sib Of piccpu_rtl2_tessent_occ_sib_int {
      InputPort clock = ijtag_tck;
      InputPort reset = ijtag_reset;
      InputPort scan_in = ijtag_si;
      InputPort capture_en = ijtag_ce;
      InputPort shift_en = ijtag_se;
      InputPort update_en = ijtag_ue;
      InputPort enable = ijtag_sel;
      InputPort from_scan_out = tdr[0];
   }
}
