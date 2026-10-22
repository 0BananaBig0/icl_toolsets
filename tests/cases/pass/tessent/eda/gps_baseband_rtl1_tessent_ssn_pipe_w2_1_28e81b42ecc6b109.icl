//-------------------------------------------------
//  File created by: Tessent Shell
//          Version: 2026.2
//       Created on: Wed Aug 12 13:58:46 CST 2026
//-------------------------------------------------





// instanced as gps_baseband.gps_baseband_rtl1_tessent_edt_c1_ext_inst



// instanced as gps_baseband.gps_baseband_rtl1_tessent_edt_c1_ext_tdr_inst



// instanced as gps_baseband.gps_baseband_rtl1_tessent_edt_c1_int_inst



// instanced as gps_baseband.gps_baseband_rtl1_tessent_edt_c1_int_tdr_inst



// instanced as gps_baseband.gps_baseband_rtl1_tessent_occ_clk_inst



// instanced as gps_baseband_rtl1_tessent_occ.tdr_sib



// instanced as gps_baseband.gps_baseband_rtl1_tessent_sib_edt_inst
// instanced as gps_baseband.gps_baseband_rtl1_tessent_sib_occ_inst
// instanced as gps_baseband.gps_baseband_rtl1_tessent_sib_sri_ctrl_inst
// instanced as gps_baseband.gps_baseband_rtl1_tessent_sib_ssn_inst



// instanced as gps_baseband.gps_baseband_rtl1_tessent_sib_sri_inst



// instanced as gps_baseband.gps_baseband_rtl1_tessent_ssn_pipe_1_inst
// instanced as gps_baseband.gps_baseband_rtl1_tessent_ssn_pipe_2_inst



// instanced as gps_baseband.gps_baseband_rtl1_tessent_ssn_scan_host_1_inst



// instanced as gps_baseband_rtl1_tessent_ssn_scan_host_1.inner



// instanced as gps_baseband.gps_baseband_rtl1_tessent_tdr_sri_ctrl_inst

NameSpace ;
UseNameSpace ;
Module gps_baseband_rtl1_tessent_ssn_pipe_w2_1 {
   // ICL module read from source on or near line 17 of file '/data/workspaces/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/gps_baseband/tsdb_outdir/instruments/gps_baseband_rtl1_ssn.instrument/gps_baseband_rtl1_tessent_ssn_pipe_w2_1.icl'
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
   Attribute tessent_instrument_container = "gps_baseband_rtl1_ssn.instrument";
   Attribute tessent_signature = "15ee38d0dbdb41776f8c5dbe146f0a45";
   DataRegister bus_out_reg[1:0] {
      WriteEnSource 1'b1;
      WriteDataSource bus_data_in[1:0];
      ResetValue 2'b0;
   }
}
