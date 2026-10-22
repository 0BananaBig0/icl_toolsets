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
Module processor_core_rtl1_tessent_mbist_bap {
   // ICL module read from source on or near line 591 of file '/home/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl2.dft_inserted_design/processor_core.icl'
   ResetPort reset {
      ActivePolarity 0;
   }
   SelectPort ijtag_select;
   ScanInPort si;
   CaptureEnPort capture_en;
   ShiftEnPort shift_en;
   ToShiftEnPort BIST_HOLD {
      Attribute connection_rule_option = "allowed_no_destination";
   }
   UpdateEnPort update_en;
   TCKPort tck;
   DataInPort memory_bypass_en {
      Attribute connection_rule_option = "allowed_tied_low";
   }
   DataOutPort memory_bypass_to_en {
      Source memory_bypass_en;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataInPort mcp_bounding_en {
      Attribute connection_rule_option = "allowed_tied_low";
   }
   DataOutPort mcp_bounding_to_en {
      Source mcp_bounding_en;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataInPort ltest_en {
      DefaultLoadValue 1'b0;
      Attribute connection_rule_option = "allowed_tied_low";
   }
   DataOutPort ltest_to_en {
      Source ltest_en;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   ScanOutPort so {
      Source ctl_group_sib;
   }
   ScanOutPort toBist[0:0] {
      Source toBist_int;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   ScanInPort fromBist[0:0] {
      Attribute connection_rule_option = "allowed_tied_low";
   }
   DataInPort MBISTPG_GO[0:0];
   DataInPort MBISTPG_DONE[0:0];
   DataOutPort bistEn[0:0] {
      Source bistEn_int[0:0];
   }
   DataOutPort ENABLE_MEM_RESET {
      Source ENABLE_MEM_RESET_tdr;
      RefEnum OnOff;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataOutPort REDUCED_ADDRESS_COUNT {
      Source REDUCED_ADDRESS_COUNT_tdr;
      RefEnum OnOff;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataOutPort BIST_SELECT_TEST_DATA {
      Source BIST_SELECT_TEST_DATA_tdr;
      RefEnum OnOff;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataOutPort BIST_ALGO_MODE0 {
      Source BIST_ALGO_MODE0_tdr;
      RefEnum OnOff;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataOutPort BIST_ALGO_MODE1 {
      Source BIST_ALGO_MODE1_tdr;
      RefEnum OnOff;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataOutPort BIST_DIAG_EN {
      Source BIST_DIAG_EN_tdr;
      RefEnum OnOff;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataOutPort BIST_ASYNC_RESET {
      Source BIST_ASYNC_RESET_tdr;
      RefEnum OnOff;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataOutPort FL_CNT_MODE0 {
      Source FL_CNT_MODE0_tdr;
      RefEnum OnOff;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataOutPort FL_CNT_MODE1 {
      Source FL_CNT_MODE1_tdr;
      RefEnum OnOff;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataOutPort INCLUDE_MEM_RESULTS_REG {
      Source INCLUDE_MEM_RESULTS_REG_tdr;
      RefEnum OnOff;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataOutPort BIST_SETUP[2:0] {
      Source BIST_SETUP_tdr[2:0];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute explicit_iwrite_only = 3'b010;
   }
   ScanInterface client {
      Port si;
      Port so;
      Port ijtag_select;
   }
   ScanInterface host_0 {
      Port toBist[0];
      Port fromBist[0];
      Port BIST_HOLD;
   }
   Attribute ijtag_logical_connection = 
       "{tck to_interfaces_tck} {tck to_controllers_tck_retime} {tck to_controllers_tck}"
       ;
   Attribute keep_active_during_scan_test = "false";
   Attribute tessent_use_in_dft_specification = "false";
   Attribute tessent_instrument_type = "mentor::memory_bist";
   Attribute tessent_instrument_subtype = "bist_access_port";
   Attribute tessent_signature = "41cb61b9df95f696decdebcd90fe4b71";
   Alias tdr[13:0] = BIST_SETUP_tdr[2:0], CHAIN_BYPASS_EN_tdr, 
       INCLUDE_MEM_RESULTS_REG_tdr, FL_CNT_MODE1_tdr, FL_CNT_MODE0_tdr, 
       BIST_ASYNC_RESET_tdr, BIST_DIAG_EN_tdr, BIST_ALGO_MODE1_tdr, 
       BIST_ALGO_MODE0_tdr, BIST_SELECT_TEST_DATA_tdr, REDUCED_ADDRESS_COUNT_tdr,
       ENABLE_MEM_RESET_tdr {
      RefEnum tdr_symbols;
   }
   Alias ChainBypassMode = CHAIN_BYPASS_EN_tdr {
   }
   Alias bistEn_int[0:0] = mbist_go_0 {
   }
   Alias toBist_int[0:0] = tdr_bypass_sib_inst {
   }
   Enum OnOff {
      ON = 1'b1;
      OFF = 1'b0;
   }
   Enum tdr_symbols {
      idle = 14'b00010001000100;
      ignore = 14'bxxxxxxxxxxxxxx;
      mbist_async_reset = 14'b00010000000000;
   }
   ScanRegister BIST_SETUP_tdr[2:0] {
      ScanInSource si;
      CaptureSource BIST_SETUP_tdr[2], BIST_SETUP_tdr[1], BIST_SETUP_tdr[0];
      DefaultLoadValue 3'b000;
      ResetValue 3'b000;
   }
   ScanRegister CHAIN_BYPASS_EN_tdr {
      ScanInSource BIST_SETUP_tdr[0];
      CaptureSource CHAIN_BYPASS_EN_tdr;
      DefaultLoadValue 1'b1;
      ResetValue 1'b1;
   }
   ScanRegister INCLUDE_MEM_RESULTS_REG_tdr {
      ScanInSource CHAIN_BYPASS_EN_tdr;
      CaptureSource INCLUDE_MEM_RESULTS_REG_tdr;
      DefaultLoadValue 1'b0;
      ResetValue 1'b0;
   }
   ScanRegister FL_CNT_MODE1_tdr {
      ScanInSource INCLUDE_MEM_RESULTS_REG_tdr;
      CaptureSource FL_CNT_MODE1_tdr;
      DefaultLoadValue 1'b0;
      ResetValue 1'b0;
   }
   ScanRegister FL_CNT_MODE0_tdr {
      ScanInSource FL_CNT_MODE1_tdr;
      CaptureSource FL_CNT_MODE0_tdr;
      DefaultLoadValue 1'b0;
      ResetValue 1'b0;
   }
   ScanRegister BIST_ASYNC_RESET_tdr {
      ScanInSource FL_CNT_MODE0_tdr;
      CaptureSource BIST_ASYNC_RESET_tdr;
      DefaultLoadValue 1'b0;
      ResetValue 1'b0;
   }
   ScanRegister BIST_DIAG_EN_tdr {
      ScanInSource BIST_ASYNC_RESET_tdr;
      CaptureSource BIST_DIAG_EN_tdr;
      DefaultLoadValue 1'b0;
      ResetValue 1'b0;
   }
   ScanRegister BIST_ALGO_MODE1_tdr {
      ScanInSource BIST_DIAG_EN_tdr;
      CaptureSource BIST_ALGO_MODE1_tdr;
      DefaultLoadValue 1'b0;
      ResetValue 1'b0;
   }
   ScanRegister BIST_ALGO_MODE0_tdr {
      ScanInSource BIST_ALGO_MODE1_tdr;
      CaptureSource BIST_ALGO_MODE0_tdr;
      DefaultLoadValue 1'b0;
      ResetValue 1'b0;
   }
   ScanRegister BIST_SELECT_TEST_DATA_tdr {
      ScanInSource BIST_ALGO_MODE0_tdr;
      CaptureSource BIST_SELECT_TEST_DATA_tdr;
      DefaultLoadValue 1'b0;
      ResetValue 1'b0;
   }
   ScanRegister REDUCED_ADDRESS_COUNT_tdr {
      ScanInSource BIST_SELECT_TEST_DATA_tdr;
      CaptureSource REDUCED_ADDRESS_COUNT_tdr;
      DefaultLoadValue 1'b0;
      ResetValue 1'b0;
   }
   ScanRegister ENABLE_MEM_RESET_tdr {
      ScanInSource REDUCED_ADDRESS_COUNT_tdr;
      CaptureSource ENABLE_MEM_RESET_tdr;
      DefaultLoadValue 1'b0;
      ResetValue 1'b0;
   }
   ScanRegister tdr_bypass_sib_inst {
      ScanInSource tdr_bypass_sib_mux_inst;
      CaptureSource 1'b0;
      DefaultLoadValue 1'b0;
      ResetValue 1'b0;
   }
   ScanRegister mbist_done_0 {
      ScanInSource fromBistMux_0;
      CaptureSource MBISTPG_DONE[0];
      DefaultLoadValue 1'b0;
      ResetValue 1'b0;
   }
   ScanRegister mbist_go_0 {
      ScanInSource mbist_done_0;
      CaptureSource MBISTPG_GO[0];
      DefaultLoadValue 1'b0;
      ResetValue 1'b0;
   }
   ScanRegister ctl_group_sib {
      ScanInSource ctl_sib_mux_inst;
      CaptureSource 1'b0;
      DefaultLoadValue 1'b0;
      ResetValue 1'b0;
   }
   ScanMux tdr_bypass_sib_mux_inst SelectedBy tdr_bypass_sib_inst {
      1'b0 : si;
      1'b1 : ENABLE_MEM_RESET_tdr;
   }
   ScanMux fromBistMux_0 SelectedBy bistEn_int[0], BIST_SETUP_tdr[1], 
       ChainBypassMode {
      3'b100 : fromBist[0];
      3'bxxx : tdr_bypass_sib_inst;
   }
   ScanMux ctl_sib_mux_inst SelectedBy ctl_group_sib {
      1'b0 : tdr_bypass_sib_inst;
      1'b1 : mbist_go_0;
   }
}
