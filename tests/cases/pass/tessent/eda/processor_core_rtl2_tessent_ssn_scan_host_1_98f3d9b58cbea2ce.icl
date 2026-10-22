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
Module processor_core_rtl2_tessent_ssn_scan_host_1 {
   // ICL module read from source on or near line 1638 of file '/home/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl2.dft_inserted_design/processor_core.icl'
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
   ResetPort ijtag_reset {
      ActivePolarity 0;
   }
   TCKPort ijtag_tck;
   SelectPort ijtag_sel;
   CaptureEnPort ijtag_ce;
   ShiftEnPort ijtag_se;
   UpdateEnPort ijtag_ue;
   ScanInPort ijtag_si;
   ScanOutPort ijtag_so {
      Source inner.ijtag_so;
   }
   Attribute keep_active_during_scan_test = "true";
   Attribute tessent_instrument_type = "mentor::ssn";
   Attribute tessent_instrument_container = "processor_core_rtl2_ssn.instrument";

   Attribute tessent_instrument_subtype = "scan_host";
   Attribute tessent_signature = "a84d40ff344a30bffc466ccb6e772fa3";
   Attribute tessent_persistent_clock_pins = 
       "{tessent_persistent_cell_bus_clock_buf/A}";
   Attribute tessent_persistent_bus_in_pins = 
       "{tessent_persistent_cell_bus_data_in_buf_0/A} ", 
       "{tessent_persistent_cell_bus_data_in_buf_1/A}";
   Attribute tessent_persistent_bus_out_pins = 
       "{tessent_persistent_cell_bus_data_out_buf_0/Y} ", 
       "{tessent_persistent_cell_bus_data_out_buf_1/Y}";
   DataRegister bus_in_reg[1:0] {
      WriteEnSource 1'b1;
      WriteDataSource bus_data_in;
      ResetValue 2'b0;
      Attribute tessent_matched_scan_register = "inner.bus_in_shift_reg";
   }
   DataRegister bus_out_reg[1:0] {
      WriteEnSource 1'b1;
      WriteDataSource bus_in_reg;
      ResetValue 2'b0;
      Attribute tessent_matched_scan_register = "inner.bus_out_shift_reg";
   }
   Instance inner Of processor_core_rtl2_tessent_ssn_scan_host_1_inner {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_clock = ijtag_tck;
      InputPort ijtag_sel = ijtag_sel;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_si = ijtag_si;
      Attribute tessent_design_instance = "ijtag_registers";
   }
}
