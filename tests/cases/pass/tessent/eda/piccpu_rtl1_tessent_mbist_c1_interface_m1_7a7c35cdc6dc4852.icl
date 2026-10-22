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
Module piccpu_rtl1_tessent_mbist_c1_interface_m1 {
   // ICL module read from source on or near line 25 of file '/data/workspaces/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hybrid_tk_lbist_flow_v2026.2/tsdb_outdir/instruments/piccpu_rtl1_mbist.instrument/piccpu_rtl1_tessent_mbist_c1_interface_m1.icl'
   ClockPort BIST_CLK;
   DataInPort BIST_COLLAR_EN;
   DataInPort BIST_ASYNC_RESETN;
   ScanInPort BIST_SI;
   ScanOutPort BIST_SO {
      Source BIST_SO_INT;
   }
   ShiftEnPort BIST_SHIFT_COLLAR;
   DataInPort BIST_SETUP0;
   DataInPort MEM_BYPASS_EN {
      Attribute connection_rule_option = "allowed_tied_low";
   }
   DataInPort MCP_BOUNDING_EN {
      Attribute connection_rule_option = "allowed_tied_low";
   }
   DataInPort INCLUDE_MEM_RESULTS_REG;
   DataOutPort address[6:0] {
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_memory_bist_function = "address";
   }
   Attribute tessent_instrument_type = "mentor::memory_bist";
   Attribute tessent_instrument_subtype = "memory_interface";
   Attribute tessent_signature = "50faf16f84338daf1edcadbdf99d7fe5";
   Attribute tessent_ignore_during_icl_verification = "on";
   Attribute keep_active_during_scan_test = "false";
   Attribute tessent_use_in_dft_specification = "false";
   Attribute tessent_bist_input_select_persistent_cell_output_list = 
       "tessent_persistent_cell_BIST_INPUT_SELECT_INT/Y";
   Attribute tessent_async_bypass_persistent_cell_input_list = 
       "tessent_persistent_cell_dout_SCAN_IN7/A tessent_persistent_cell_dout_SCAN_IN6/A tessent_persistent_cell_dout_SCAN_IN5/A tessent_persistent_cell_dout_SCAN_IN4/A tessent_persistent_cell_dout_SCAN_IN3/A tessent_persistent_cell_dout_SCAN_IN2/A tessent_persistent_cell_dout_SCAN_IN1/A tessent_persistent_cell_dout_SCAN_IN0/A"
       ;
   Attribute tessent_bist_clk_persistent_cell_output_list = 
       "tessent_persistent_cell_GATING_BIST_CLK/GCK";
   Attribute tessent_memory_output_is_tristate = "false";
   Attribute tessent_memory_control_inputs_list = "";
   Attribute tessent_memory_test_inputs_list = "";
   Attribute tessent_memory_test_outputs_list = "";
   Attribute tessent_memory_control_inputs_di_coverage_list = "partial";
   Alias GO_ID_REG_BYPASS_SOURCE = RA_STATUS_REG[0] {
   }
   Alias BIST_SO_INT = FREEZE_STOP_ERROR_REG[0] {
   }
   ScanRegister RA_STATUS_REG[0:0] {
      ScanInSource BIST_SI;
   }
   ScanRegister GO_ID_REG[7:0] {
      ScanInSource RA_STATUS_REG[0];
   }
   ScanRegister FREEZE_STOP_ERROR_REG[0:0] {
      ScanInSource GO_ID_REG_BYPASS_MUX;
   }
   ScanMux GO_ID_REG_BYPASS_MUX SelectedBy INCLUDE_MEM_RESULTS_REG {
      1'b0 : GO_ID_REG_BYPASS_SOURCE;
      1'b1 : GO_ID_REG[0];
   }
}
