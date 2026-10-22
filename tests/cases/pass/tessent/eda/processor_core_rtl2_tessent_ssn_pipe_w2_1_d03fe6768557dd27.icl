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
Module processor_core_rtl2_tessent_ssn_pipe_w2_1 {
   // ICL module read from source on or near line 1606 of file '/home/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl2.dft_inserted_design/processor_core.icl'
   ResetPort ijtag_reset {
      ActivePolarity 0;
   }
   ClockPort bus_clock {
      Attribute function_modifier = "tessent_ssn_clock";
      Attribute forced_high_dft_signal_list = "ssn_en";
   }
   DataInPort bus_data_in[1:0] {
      Attribute function_modifier = "tessent_ssn_bus_data";
      Attribute forced_high_dft_signal_list = "ssn_en";
   }
   DataOutPort bus_data_out[1:0] {
      Source bus_out_reg;
      Attribute function_modifier = "tessent_ssn_bus_data";
      Attribute forced_high_dft_signal_list = "ssn_en";
   }
   Attribute keep_active_during_scan_test = "true";
   Attribute tessent_instrument_type = "mentor::ssn";
   Attribute tessent_instrument_subtype = "pipeline";
   Attribute tessent_instrument_container = "processor_core_rtl2_ssn.instrument";

   Attribute tessent_signature = "bbad5d7301b96cad8c929872924e7f2b";
   DataRegister bus_out_reg[1:0] {
      WriteEnSource 1'b1;
      WriteDataSource bus_data_in[1:0];
      ResetValue 2'b0;
   }
}
