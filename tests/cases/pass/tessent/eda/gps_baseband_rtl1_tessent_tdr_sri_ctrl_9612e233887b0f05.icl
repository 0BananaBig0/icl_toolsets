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
Module gps_baseband_rtl1_tessent_tdr_sri_ctrl {
   // ICL module read from source on or near line 17 of file '/data/workspaces/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/gps_baseband/tsdb_outdir/instruments/gps_baseband_rtl1_ijtag.instrument/gps_baseband_rtl1_tessent_tdr_sri_ctrl.icl'
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
   DataOutPort async_set_reset_static_disable {
      Source tdr[5];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_dft_signal_name = "async_set_reset_static_disable";
      Attribute tessent_dft_signal_usage = "logic_test_control";
      Attribute tessent_dft_signal_value_in_pre_scan_drc = "0";
      Attribute tessent_dft_signal_reset_value = 0;
      Attribute tessent_persistent_design_pin = 
          "{tessent_persistent_cell_async_set_reset_static_disable/Y}";
   }
   DataOutPort ext_edt_mode {
      Source tdr[4];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_dft_signal_name = "ext_edt_mode";
      Attribute tessent_dft_signal_usage = "scan_mode";
      Attribute tessent_dft_signal_scan_mode_type = "external";
      Attribute tessent_dft_signal_reset_value = 0;
      Attribute tessent_persistent_design_pin = 
          "{tessent_persistent_cell_ext_edt_mode/Y}";
   }
   DataOutPort int_edt_mode {
      Source tdr[3];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_dft_signal_name = "int_edt_mode";
      Attribute tessent_dft_signal_usage = "scan_mode";
      Attribute tessent_dft_signal_scan_mode_type = "internal";
      Attribute tessent_dft_signal_reset_value = 0;
      Attribute tessent_persistent_design_pin = 
          "{tessent_persistent_cell_int_edt_mode/Y}";
   }
   DataOutPort ext_ltest_en {
      Source tdr[2];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_dft_signal_name = "ext_ltest_en";
      Attribute tessent_dft_signal_usage = "logic_test_control";
      Attribute tessent_dft_signal_value_in_pre_scan_drc = "x";
      Attribute tessent_dft_signal_reset_value = 0;
      Attribute tessent_persistent_design_pin = 
          "{tessent_persistent_cell_ext_ltest_en/Y}";
   }
   DataOutPort int_ltest_en {
      Source tdr[1];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_dft_signal_name = "int_ltest_en";
      Attribute tessent_dft_signal_usage = "logic_test_control";
      Attribute tessent_dft_signal_value_in_pre_scan_drc = "x";
      Attribute tessent_dft_signal_reset_value = 0;
      Attribute tessent_persistent_design_pin = 
          "{tessent_persistent_cell_int_ltest_en/Y}";
   }
   DataOutPort ltest_en {
      Source tdr[0];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_dft_signal_name = "ltest_en";
      Attribute tessent_dft_signal_usage = "logic_test_control";
      Attribute tessent_dft_signal_value_in_pre_scan_drc = "1";
      Attribute tessent_dft_signal_reset_value = 0;
      Attribute tessent_persistent_design_pin = 
          "{tessent_persistent_cell_ltest_en/Y}";
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
   Attribute tessent_dft_function = "scan_resource_instrument_dft_control";
   Attribute tessent_use_in_dft_specification = "false";
   Attribute tessent_instrument_type = "mentor::ijtag_node";
   Attribute tessent_instrument_subtype = "tessent_tdr";
   Attribute tessent_signature = "a114a1a0ca115237a1431f79975aacec";
   Attribute tessent_instrument_container = "gps_baseband_rtl1_ijtag";
   ScanRegister tdr[5:0] {
      ScanInSource ijtag_si;
      CaptureSource 6'b000000;
      DefaultLoadValue 6'b000000;
      ResetValue 6'b000000;
   }
}
