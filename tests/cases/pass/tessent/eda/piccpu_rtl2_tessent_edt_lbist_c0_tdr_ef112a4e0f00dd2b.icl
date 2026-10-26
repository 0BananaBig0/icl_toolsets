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
Module piccpu_rtl2_tessent_edt_lbist_c0_tdr {
   // ICL module read from source on or near line 1107 of file '/home/lianghuaxiao/workspaces/tessent_2026.2_usage_examples/tessent_example_hybrid_tk_lbist_flow_v2026.2/tsdb_outdir/dft_inserted_designs/piccpu_rtl2.dft_inserted_design/piccpu.icl'
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
   DataOutPort edt_bypass {
      Source tdr[0];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_persistent_design_pin = 
          "{tessent_persistent_cell_edt_bypass/Y}";
   }
   DataOutPort edt_low_power_shift_en {
      Source tdr[1];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_persistent_design_pin = 
          "{tessent_persistent_cell_edt_low_power_shift_en/Y}";
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
   Attribute tessent_use_in_dft_specification = "false";
   Attribute tessent_instrument_type = "mentor::ijtag_node";
   Attribute tessent_instrument_subtype = "tessent_tdr";
   Attribute tessent_signature = "7bc0d6f3c5972a874b5e0545f88ecafc";
   Attribute tessent_instrument_container = "piccpu_rtl2_edt_lbist";
   ScanRegister tdr[1:0] {
      ScanInSource ijtag_si;
      CaptureSource tdr[1:0];
      DefaultLoadValue 2'b00;
      ResetValue 2'b00;
   }
}
