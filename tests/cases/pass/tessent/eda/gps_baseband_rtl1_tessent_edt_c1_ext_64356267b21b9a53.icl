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
Module gps_baseband_rtl1_tessent_edt_c1_ext {
   // ICL module read from source on or near line 17 of file '/data/workspaces/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/gps_baseband/tsdb_outdir/instruments/gps_baseband_rtl1_edt.instrument/gps_baseband_rtl1_tessent_edt_c1_ext.icl'
   DataInPort edt_bypass {
      RefEnum OnOffTable;
      Attribute tessent_no_input_constraints = "on";
   }
   Attribute keep_active_during_scan_test = "true";
   Attribute tessent_instrument_type = "mentor::edt";
   Attribute tessent_signature = "d942dff1af2308c0bafb18f84063b048";
   Enum OnOffTable {
      off = 1'b0;
      on = 1'b1;
   }
}
