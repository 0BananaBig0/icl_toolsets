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
Module picdram {
   // ICL module read from source on or near line 17 of file '/data/workspaces/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hybrid_tk_lbist_flow_v2026.2/tsdb_outdir/instruments/piccpu_rtl1_mbist.instrument/picdram.icl'
   DataInPort address[6:0] {
      Attribute connection_rule_option = "allowed_no_source";
      Attribute tessent_memory_bist_function = "address";
   }
   ClockPort clk;
   Attribute tessent_use_in_dft_specification = "false";
   Attribute keep_active_during_scan_test = "false";
   Attribute tessent_memory_module = "without_internal_scan_logic";
}
