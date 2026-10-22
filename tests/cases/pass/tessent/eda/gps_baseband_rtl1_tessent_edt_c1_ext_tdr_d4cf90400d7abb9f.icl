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
Module gps_baseband_rtl1_tessent_edt_c1_ext_tdr {
   // ICL module read from source on or near line 17 of file '/data/workspaces/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/gps_baseband/tsdb_outdir/instruments/gps_baseband_rtl1_edt.instrument/gps_baseband_rtl1_tessent_edt_c1_ext_tdr.icl'
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
   Attribute tessent_signature = "2d1dd78c8fe811dc3ca511970a5efbcd";
   Attribute tessent_instrument_container = "gps_baseband_rtl1_edt";
   ScanRegister tdr[0:0] {
      ScanInSource ijtag_si;
      CaptureSource tdr[0:0];
      DefaultLoadValue 1'b0;
      ResetValue 1'b0;
   }
}
