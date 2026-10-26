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
Module piccpu_rtl2_tessent_edt_internal_scan_registers {
   // ICL module read from source on or near line 1738 of file '/home/lianghuaxiao/workspaces/tessent_2026.2_usage_examples/tessent_example_hybrid_tk_lbist_flow_v2026.2/tsdb_outdir/dft_inserted_designs/piccpu_rtl2.dft_inserted_design/piccpu.icl'
   ScanInPort ijtag_si;
   ScanOutPort ijtag_so {
      Source blk1_sib_i.ijtag_so;
   }
   TCKPort ijtag_tck;
   ResetPort ijtag_reset {
      ActivePolarity 0;
   }
   SelectPort ijtag_sel;
   CaptureEnPort ijtag_ce;
   ShiftEnPort ijtag_se;
   UpdateEnPort ijtag_ue;
   DataInPort edt_single_bypass_chain {
      Attribute tessent_no_input_constraints = "on";
   }
   Attribute keep_active_during_scan_test = "true";
   Attribute tessent_instrument_type = "mentor::edt";
   Attribute tessent_instrument_subtype = "edt_internal_scan_registers";
   Attribute tessent_signature = "c33d7ea4b4922788dd23fcc6a8210100";
   ScanRegister edt_lbist_chain1[1:0] {
      ScanInSource edt_lbist_chain1_single_chain_mode_mux;
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
   }
   ScanRegister edt_lbist_chain2[1:0] {
      ScanInSource edt_lbist_chain2_bypass_mux;
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
   }
   ScanRegister edt_lbist_chain3[1:0] {
      ScanInSource edt_lbist_chain3_bypass_mux;
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
   }
   ScanRegister edt_lbist_chain4[1:0] {
      ScanInSource edt_lbist_chain4_bypass_mux;
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
   }
   ScanRegister edt_lbist_chain5[1:0] {
      ScanInSource edt_lbist_chain5_bypass_mux;
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
   }
   ScanRegister edt_lbist_chain6[1:0] {
      ScanInSource edt_lbist_chain6_bypass_mux;
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
   }
   ScanRegister edt_lbist_chain7[1:0] {
      ScanInSource edt_lbist_chain7_bypass_mux;
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
   }
   ScanRegister edt_lbist_chain8[1:0] {
      ScanInSource edt_lbist_chain8_bypass_mux;
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
   }
   ScanRegister edt_lbist_chain9[1:0] {
      ScanInSource edt_lbist_chain9_bypass_mux;
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
   }
   ScanRegister edt_lbist_chain10[1:0] {
      ScanInSource edt_lbist_chain10_bypass_mux;
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
   }
   ScanMux edt_lbist_chain1_single_chain_mode_mux SelectedBy 
       edt_single_bypass_chain {
      1'b1 : ijtag_si;
   }
   ScanMux edt_lbist_chain2_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1 : edt_lbist_chain1[0];
   }
   ScanMux edt_lbist_chain3_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1 : edt_lbist_chain2[0];
   }
   ScanMux edt_lbist_chain4_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1 : edt_lbist_chain3[0];
   }
   ScanMux edt_lbist_chain5_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1 : edt_lbist_chain4[0];
   }
   ScanMux edt_lbist_chain6_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1 : edt_lbist_chain5[0];
   }
   ScanMux edt_lbist_chain7_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1 : edt_lbist_chain6[0];
   }
   ScanMux edt_lbist_chain8_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1 : edt_lbist_chain7[0];
   }
   ScanMux edt_lbist_chain9_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1 : edt_lbist_chain8[0];
   }
   ScanMux edt_lbist_chain10_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1 : edt_lbist_chain9[0];
   }
   Instance blk1_sib_i Of piccpu_rtl2_tessent_single_chain_mode_logic_sib {
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_si = ijtag_si;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_sel = ijtag_sel;
      InputPort ijtag_from_so = edt_lbist_chain10[0];
   }
}
