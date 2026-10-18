// Merged from the 32 requested ICL files.
// Duplicate module names: first occurrence in the supplied file order retained.

// Source: wrapped_cores/gps_baseband/tsdb_outdir/dft_inserted_designs/gps_baseband_rtl1.dft_inserted_design/gps_baseband.icl
Module gps_baseband {
   // Created by ICL extraction
   ClockPort clk {
      Attribute tessent_clock_domain_labels = "clk clk";
      Attribute tessent_clock_periods = "all 3.00ns";
   }
   CaptureEnPort ijtag_ce;
   ResetPort ijtag_reset {
      ActivePolarity 0;
   }
   ShiftEnPort ijtag_se;
   SelectPort ijtag_sel;
   ScanInPort ijtag_si;
   ScanOutPort ijtag_so {
      Source gps_baseband_rtl1_tessent_sib_sri_inst.ijtag_so;
   }
   TCKPort ijtag_tck;
   UpdateEnPort ijtag_ue;
   ClockPort ssn_bus_clock {
      Attribute function_modifier = "tessent_ssn_clock";
      Attribute forced_high_dft_signal_list = "ssn_en";
      Attribute tessent_ssn_datapath_ids = "1";
   }
   DataInPort ssn_bus_data_in[1] {
      Attribute function_modifier = "tessent_ssn_bus_data";
      Attribute forced_high_dft_signal_list = "ssn_en";
      Attribute tessent_ssn_datapath_ids = "1";
      Attribute tessent_ssn_datapath_bus_index = 1;
      Attribute tessent_ssn_clocks = "ssn_bus_clock";
      Attribute tessent_ssn_data_phases = "1 1";
   }
   DataInPort ssn_bus_data_in[0] {
      Attribute function_modifier = "tessent_ssn_bus_data";
      Attribute forced_high_dft_signal_list = "ssn_en";
      Attribute tessent_ssn_datapath_ids = "1";
      Attribute tessent_ssn_datapath_bus_index = 0;
      Attribute tessent_ssn_clocks = "ssn_bus_clock";
      Attribute tessent_ssn_data_phases = "1 1";
   }
   DataOutPort ssn_bus_data_out[1] {
      Source gps_baseband_rtl1_tessent_ssn_pipe_2_inst.bus_data_out[1];
      Attribute function_modifier = "tessent_ssn_bus_data";
      Attribute forced_high_dft_signal_list = "ssn_en";
      Attribute tessent_ssn_datapath_ids = "1";
      Attribute tessent_ssn_datapath_bus_index = 1;
      Attribute tessent_ssn_clocks = "ssn_bus_clock";
      Attribute tessent_ssn_data_phases = "1 1";
   }
   DataOutPort ssn_bus_data_out[0] {
      Source gps_baseband_rtl1_tessent_ssn_pipe_2_inst.bus_data_out[0];
      Attribute function_modifier = "tessent_ssn_bus_data";
      Attribute forced_high_dft_signal_list = "ssn_en";
      Attribute tessent_ssn_datapath_ids = "1";
      Attribute tessent_ssn_datapath_bus_index = 0;
      Attribute tessent_ssn_clocks = "ssn_bus_clock";
      Attribute tessent_ssn_data_phases = "1 1";
   }
   ScanInterface ijtag {
      Port ijtag_ce;
      Port ijtag_reset;
      Port ijtag_se;
      Port ijtag_sel;
      Port ijtag_si;
      Port ijtag_so;
      Port ijtag_tck;
      Port ijtag_ue;
   }
   Attribute tessent_design_format = "verilog_2001";
   Attribute test_setup_procfile = "";
   Attribute forced_low_internal_input_port_list = 
       "{gps_baseband_rtl1_tessent_ssn_scan_host_1_inst/scan_en}";
   Attribute icl_extraction_date = "Wed Aug 12 13:58:46 CST 2026";
   Attribute created_by_tessent_icl_extract = "true";
   Attribute tessent_design_id = "rtl1";
   Attribute tessent_design_level = "physical_block";
   Attribute tessent_is_physical_module = "true";
   Instance gps_baseband_rtl1_tessent_edt_c1_ext_inst Of 
       gps_baseband_rtl1_tessent_edt_c1_ext {
      InputPort edt_bypass = 
          gps_baseband_rtl1_tessent_edt_c1_ext_tdr_inst.edt_bypass;
      Attribute tessent_design_instance = 
          "gps_baseband_rtl1_tessent_edt_c1_ext_inst";
   }
   Instance gps_baseband_rtl1_tessent_edt_c1_ext_tdr_inst Of 
       gps_baseband_rtl1_tessent_edt_c1_ext_tdr {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = gps_baseband_rtl1_tessent_sib_edt_inst.ijtag_to_sel;
      InputPort ijtag_si = 
          gps_baseband_rtl1_tessent_edt_c1_int_tdr_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      Attribute tessent_design_instance = 
          "gps_baseband_rtl1_tessent_edt_c1_ext_tdr_inst";
   }
   Instance gps_baseband_rtl1_tessent_edt_c1_int_inst Of 
       gps_baseband_rtl1_tessent_edt_c1_int {
      InputPort edt_bypass = 
          gps_baseband_rtl1_tessent_edt_c1_int_tdr_inst.edt_bypass;
      Attribute tessent_design_instance = 
          "gps_baseband_rtl1_tessent_edt_c1_int_inst";
   }
   Instance gps_baseband_rtl1_tessent_edt_c1_int_tdr_inst Of 
       gps_baseband_rtl1_tessent_edt_c1_int_tdr {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = gps_baseband_rtl1_tessent_sib_edt_inst.ijtag_to_sel;
      InputPort ijtag_si = gps_baseband_rtl1_tessent_sib_occ_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      Attribute tessent_design_instance = 
          "gps_baseband_rtl1_tessent_edt_c1_int_tdr_inst";
   }
   Instance gps_baseband_rtl1_tessent_occ_clk_inst Of 
       gps_baseband_rtl1_tessent_occ {
      InputPort fast_clock = clk;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_si = gps_baseband_rtl1_tessent_sib_ssn_inst.ijtag_so;
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_sel = gps_baseband_rtl1_tessent_sib_occ_inst.ijtag_to_sel;
      Attribute tessent_design_instance = 
          "gps_baseband_rtl1_tessent_occ_clk_inst";
   }
   Instance gps_baseband_rtl1_tessent_sib_edt_inst Of 
       gps_baseband_rtl1_tessent_sib_2 {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = gps_baseband_rtl1_tessent_sib_sri_inst.ijtag_to_sel;
      InputPort ijtag_si = gps_baseband_rtl1_tessent_sib_occ_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_from_so = 
          gps_baseband_rtl1_tessent_edt_c1_ext_tdr_inst.ijtag_so;
      Attribute tessent_design_instance = 
          "gps_baseband_rtl1_tessent_sib_edt_inst";
   }
   Instance gps_baseband_rtl1_tessent_sib_occ_inst Of 
       gps_baseband_rtl1_tessent_sib_2 {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = gps_baseband_rtl1_tessent_sib_sri_inst.ijtag_to_sel;
      InputPort ijtag_si = gps_baseband_rtl1_tessent_sib_ssn_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_from_so = gps_baseband_rtl1_tessent_occ_clk_inst.ijtag_so;
      Attribute tessent_design_instance = 
          "gps_baseband_rtl1_tessent_sib_occ_inst";
   }
   Instance gps_baseband_rtl1_tessent_sib_sri_ctrl_inst Of 
       gps_baseband_rtl1_tessent_sib_2 {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = gps_baseband_rtl1_tessent_sib_sri_inst.ijtag_to_sel;
      InputPort ijtag_si = gps_baseband_rtl1_tessent_sib_edt_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_from_so = 
          gps_baseband_rtl1_tessent_tdr_sri_ctrl_inst.ijtag_so;
      Attribute tessent_design_instance = 
          "gps_baseband_rtl1_tessent_sib_sri_ctrl_inst";
   }
   Instance gps_baseband_rtl1_tessent_sib_sri_inst Of 
       gps_baseband_rtl1_tessent_sib_1 {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = ijtag_sel;
      InputPort ijtag_si = ijtag_si;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_from_so = 
          gps_baseband_rtl1_tessent_sib_sri_ctrl_inst.ijtag_so;
      Attribute tessent_design_instance = 
          "gps_baseband_rtl1_tessent_sib_sri_inst";
   }
   Instance gps_baseband_rtl1_tessent_sib_ssn_inst Of 
       gps_baseband_rtl1_tessent_sib_2 {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = gps_baseband_rtl1_tessent_sib_sri_inst.ijtag_to_sel;
      InputPort ijtag_si = ijtag_si;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_from_so = 
          gps_baseband_rtl1_tessent_ssn_scan_host_1_inst.ijtag_so;
      Attribute tessent_design_instance = 
          "gps_baseband_rtl1_tessent_sib_ssn_inst";
   }
   Instance gps_baseband_rtl1_tessent_ssn_pipe_1_inst Of 
       gps_baseband_rtl1_tessent_ssn_pipe_w2_1 {
      InputPort ijtag_reset = ijtag_reset;
      InputPort bus_clock = ssn_bus_clock;
      InputPort bus_data_in[1] = ssn_bus_data_in[1];
      InputPort bus_data_in[0] = ssn_bus_data_in[0];
      Attribute tessent_design_instance = 
          "gps_baseband_rtl1_tessent_ssn_pipe_1_inst";
   }
   Instance gps_baseband_rtl1_tessent_ssn_pipe_2_inst Of 
       gps_baseband_rtl1_tessent_ssn_pipe_w2_1 {
      InputPort ijtag_reset = ijtag_reset;
      InputPort bus_clock = ssn_bus_clock;
      InputPort bus_data_in[1] = 
          gps_baseband_rtl1_tessent_ssn_scan_host_1_inst.bus_data_out[1];
      InputPort bus_data_in[0] = 
          gps_baseband_rtl1_tessent_ssn_scan_host_1_inst.bus_data_out[0];
      Attribute tessent_design_instance = 
          "gps_baseband_rtl1_tessent_ssn_pipe_2_inst";
   }
   Instance gps_baseband_rtl1_tessent_ssn_scan_host_1_inst Of 
       gps_baseband_rtl1_tessent_ssn_scan_host_1 {
      InputPort bus_clock = ssn_bus_clock;
      InputPort bus_data_in[1] = 
          gps_baseband_rtl1_tessent_ssn_pipe_1_inst.bus_data_out[1];
      InputPort bus_data_in[0] = 
          gps_baseband_rtl1_tessent_ssn_pipe_1_inst.bus_data_out[0];
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_sel = gps_baseband_rtl1_tessent_sib_ssn_inst.ijtag_to_sel;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_si = ijtag_si;
      Attribute tessent_design_instance = 
          "gps_baseband_rtl1_tessent_ssn_scan_host_1_inst";
   }
   Instance gps_baseband_rtl1_tessent_tdr_sri_ctrl_inst Of 
       gps_baseband_rtl1_tessent_tdr_sri_ctrl {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = 
          gps_baseband_rtl1_tessent_sib_sri_ctrl_inst.ijtag_to_sel;
      InputPort ijtag_si = gps_baseband_rtl1_tessent_sib_edt_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      Attribute tessent_design_instance = 
          "gps_baseband_rtl1_tessent_tdr_sri_ctrl_inst";
   }
}

// Source: wrapped_cores/gps_baseband/tsdb_outdir/dft_inserted_designs/gps_baseband_rtl1.dft_inserted_design/gps_baseband.icl
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

// Source: wrapped_cores/gps_baseband/tsdb_outdir/dft_inserted_designs/gps_baseband_rtl1.dft_inserted_design/gps_baseband.icl
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

// Source: wrapped_cores/gps_baseband/tsdb_outdir/dft_inserted_designs/gps_baseband_rtl1.dft_inserted_design/gps_baseband.icl
Module gps_baseband_rtl1_tessent_edt_c1_int {
   // ICL module read from source on or near line 17 of file '/data/workspaces/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/gps_baseband/tsdb_outdir/instruments/gps_baseband_rtl1_edt.instrument/gps_baseband_rtl1_tessent_edt_c1_int.icl'
   DataInPort edt_bypass {
      RefEnum OnOffTable;
      Attribute tessent_no_input_constraints = "on";
   }
   Attribute keep_active_during_scan_test = "true";
   Attribute tessent_instrument_type = "mentor::edt";
   Attribute tessent_signature = "6b5c989c9d41ebd86a7c3b161c640cb8";
   Enum OnOffTable {
      off = 1'b0;
      on = 1'b1;
   }
}

// Source: wrapped_cores/gps_baseband/tsdb_outdir/dft_inserted_designs/gps_baseband_rtl1.dft_inserted_design/gps_baseband.icl
Module gps_baseband_rtl1_tessent_edt_c1_int_tdr {
   // ICL module read from source on or near line 17 of file '/data/workspaces/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/gps_baseband/tsdb_outdir/instruments/gps_baseband_rtl1_edt.instrument/gps_baseband_rtl1_tessent_edt_c1_int_tdr.icl'
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
   Attribute tessent_signature = "d0b5561d9511cdb201125e37b3df6710";
   Attribute tessent_instrument_container = "gps_baseband_rtl1_edt";
   ScanRegister tdr[0:0] {
      ScanInSource ijtag_si;
      CaptureSource tdr[0:0];
      DefaultLoadValue 1'b0;
      ResetValue 1'b0;
   }
}

// Source: wrapped_cores/gps_baseband/tsdb_outdir/dft_inserted_designs/gps_baseband_rtl1.dft_inserted_design/gps_baseband.icl
Module gps_baseband_rtl1_tessent_occ {
   // ICL module read from source on or near line 17 of file '/data/workspaces/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/gps_baseband/tsdb_outdir/instruments/gps_baseband_rtl1_occ.instrument/gps_baseband_rtl1_tessent_occ.icl'
   ClockPort fast_clock;
   ToClockPort clock_out {
      Source tck_mux;
      Attribute exclude_from_sdc = "on";
   }
   TCKPort ijtag_tck;
   ScanInPort ijtag_si;
   ScanOutPort ijtag_so {
      Source tdr_sib.scan_out;
   }
   ResetPort ijtag_reset {
      ActivePolarity 0;
   }
   CaptureEnPort ijtag_ce;
   ShiftEnPort ijtag_se;
   UpdateEnPort ijtag_ue;
   SelectPort ijtag_sel;
   Attribute keep_active_during_scan_test = "true";
   Attribute tessent_use_in_dft_specification = "false";
   Attribute tessent_instrument_type = "mentor::occ";
   Attribute tessent_instrument_subtype = "standard";
   Attribute tessent_instrument_container = "gps_baseband_rtl1_occ.instrument";
   Attribute tessent_signature = "981c5e9fffbf40f0adb642b150e3828b";
   Attribute tdr_parent_instance = "ijtag_control";
   Alias test_mode = tdr[0] {
   }
   Alias fast_capture_mode = tdr[1] {
   }
   Alias active_upstream_parent_occ = tdr[2] {
   }
   Alias capture_cycle_width[1:0] = tdr[4:3] {
   }
   Alias inject_tck = tdr[5] {
   }
   ScanRegister tdr[5:0] {
      ScanInSource ijtag_si;
      CaptureSource 6'b000000;
      ResetValue 6'b000000;
   }
   ClockMux tck_mux SelectedBy inject_tck {
      1'b1 : ijtag_tck;
      1'b0 : fast_clock;
   }
   Instance tdr_sib Of gps_baseband_rtl1_tessent_occ_sib_int {
      InputPort clock = ijtag_tck;
      InputPort reset = ijtag_reset;
      InputPort scan_in = ijtag_si;
      InputPort capture_en = ijtag_ce;
      InputPort shift_en = ijtag_se;
      InputPort update_en = ijtag_ue;
      InputPort enable = ijtag_sel;
      InputPort from_scan_out = tdr[0];
   }
}

// Source: wrapped_cores/gps_baseband/tsdb_outdir/dft_inserted_designs/gps_baseband_rtl1.dft_inserted_design/gps_baseband.icl
Module gps_baseband_rtl1_tessent_occ_sib_int {
   // ICL module read from source on or near line 75 of file '/data/workspaces/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/gps_baseband/tsdb_outdir/instruments/gps_baseband_rtl1_occ.instrument/gps_baseband_rtl1_tessent_occ.icl'
   TCKPort clock;
   ResetPort reset {
      ActivePolarity 0;
   }
   ScanInPort scan_in;
   ScanOutPort scan_out {
      Source sib;
   }
   CaptureEnPort capture_en;
   ShiftEnPort shift_en;
   UpdateEnPort update_en;
   SelectPort enable;
   ToSelectPort to_scan_en {
      Source to_scan_en_and;
   }
   ScanInPort from_scan_out;
   ScanInterface client {
      Port scan_in;
      Port scan_out;
      Port enable;
   }
   ScanInterface host {
      Port from_scan_out;
      Port to_scan_en;
   }
   ScanRegister sib {
      ScanInSource scan_in_mux;
      CaptureSource 1'b0;
      ResetValue 1'b0;
   }
   ScanMux scan_in_mux SelectedBy sib {
      1'b0 : scan_in;
      1'b1 : from_scan_out;
   }
   LogicSignal to_scan_en_and {
      enable, sib == 2'b11;
   }
}

// Source: wrapped_cores/gps_baseband/tsdb_outdir/dft_inserted_designs/gps_baseband_rtl1.dft_inserted_design/gps_baseband.icl
Module gps_baseband_rtl1_tessent_sib_2 {
   // ICL module read from source on or near line 17 of file '/data/workspaces/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/gps_baseband/tsdb_outdir/instruments/gps_baseband_rtl1_ijtag.instrument/gps_baseband_rtl1_tessent_sib_2.icl'
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
      Source sib;
   }
   ToSelectPort ijtag_to_sel {
      Attribute connection_rule_option = "allowed_no_destination";
   }
   ScanInPort ijtag_from_so {
      Attribute connection_rule_option = "allowed_no_source";
   }
   ScanInterface client {
      Port ijtag_si;
      Port ijtag_so;
      Port ijtag_sel;
      Port ijtag_tck;
      Port ijtag_reset;
      Port ijtag_se;
      Port ijtag_ce;
      Port ijtag_ue;
   }
   ScanInterface host {
      Port ijtag_from_so;
      Port ijtag_to_sel;
   }
   Attribute keep_active_during_scan_test = "true";
   Attribute tessent_use_in_dft_specification = "false";
   Attribute tessent_instrument_type = "mentor::ijtag_node";
   Attribute tessent_signature = "f58bfe32e6b6fe31c1526a7ea3b22e97";
   Attribute tessent_instrument_container = "gps_baseband_rtl1_ijtag";
   Attribute tessent_instrument_subtype = "sib";
   ScanRegister sib {
      ScanInSource scan_in_mux;
      CaptureSource sib;
      ResetValue 1'b0;
   }
   ScanMux scan_in_mux SelectedBy sib {
      1'b0 : ijtag_si;
      1'b1 : ijtag_from_so;
   }
}

// Source: wrapped_cores/gps_baseband/tsdb_outdir/dft_inserted_designs/gps_baseband_rtl1.dft_inserted_design/gps_baseband.icl
Module gps_baseband_rtl1_tessent_sib_1 {
   // ICL module read from source on or near line 17 of file '/data/workspaces/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/gps_baseband/tsdb_outdir/instruments/gps_baseband_rtl1_ijtag.instrument/gps_baseband_rtl1_tessent_sib_1.icl'
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
      Source sib;
   }
   ToSelectPort ijtag_to_sel {
      Attribute connection_rule_option = "allowed_no_destination";
   }
   ScanInPort ijtag_from_so {
      Attribute connection_rule_option = "allowed_no_source";
   }
   ScanInterface client {
      Port ijtag_si;
      Port ijtag_so;
      Port ijtag_sel;
      Port ijtag_tck;
      Port ijtag_reset;
      Port ijtag_se;
      Port ijtag_ce;
      Port ijtag_ue;
   }
   ScanInterface host {
      Port ijtag_from_so;
      Port ijtag_to_sel;
   }
   Attribute keep_active_during_scan_test = "true";
   Attribute tessent_dft_function = "scan_resource_instrument_host";
   Attribute tessent_use_in_dft_specification = "false";
   Attribute tessent_instrument_type = "mentor::ijtag_node";
   Attribute tessent_signature = "9f3d1936e92abe917dc5ab1df2e0c266";
   Attribute tessent_instrument_container = "gps_baseband_rtl1_ijtag";
   Attribute tessent_instrument_subtype = "sib";
   ScanRegister sib {
      ScanInSource scan_in_mux;
      CaptureSource sib;
      ResetValue 1'b0;
   }
   ScanMux scan_in_mux SelectedBy sib {
      1'b0 : ijtag_si;
      1'b1 : ijtag_from_so;
   }
}

// Source: wrapped_cores/gps_baseband/tsdb_outdir/dft_inserted_designs/gps_baseband_rtl1.dft_inserted_design/gps_baseband.icl
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

// Source: wrapped_cores/gps_baseband/tsdb_outdir/dft_inserted_designs/gps_baseband_rtl1.dft_inserted_design/gps_baseband.icl
Module gps_baseband_rtl1_tessent_ssn_scan_host_1 {
   // ICL module read from source on or near line 17 of file '/data/workspaces/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/gps_baseband/tsdb_outdir/instruments/gps_baseband_rtl1_ssn.instrument/gps_baseband_rtl1_tessent_ssn_scan_host_1.icl'
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
   Attribute tessent_instrument_container = "gps_baseband_rtl1_ssn.instrument";
   Attribute tessent_instrument_subtype = "scan_host";
   Attribute tessent_signature = "e5c83755f9028d12f7259576d0fe1713";
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
   Instance inner Of gps_baseband_rtl1_tessent_ssn_scan_host_1_inner {
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

// Source: wrapped_cores/gps_baseband/tsdb_outdir/dft_inserted_designs/gps_baseband_rtl1.dft_inserted_design/gps_baseband.icl
Module gps_baseband_rtl1_tessent_ssn_scan_host_1_inner {
   // ICL module read from source on or near line 86 of file '/data/workspaces/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/gps_baseband/tsdb_outdir/instruments/gps_baseband_rtl1_ssn.instrument/gps_baseband_rtl1_tessent_ssn_scan_host_1.icl'
   ResetPort ijtag_reset {
      ActivePolarity 0;
   }
   TCKPort ijtag_clock;
   SelectPort ijtag_sel;
   CaptureEnPort ijtag_ce;
   ShiftEnPort ijtag_se;
   UpdateEnPort ijtag_ue;
   ScanInPort ijtag_si;
   ScanOutPort ijtag_so {
      Source ijtag_so_mux;
   }
   DataOutPort enable {
      Source enable_shift_reg;
   }
   DataOutPort on_chip_compare_enable {
      Source on_chip_compare_enable_shift_reg;
   }
   DataOutPort initial_offset[30:0] {
      Source initial_offset_shift_reg;
   }
   DataOutPort relative_initial_offset_enable {
      Source relative_initial_offset_enable_shift_reg;
   }
   DataOutPort bus_width[1:0] {
      Source bus_width_shift_reg;
   }
   DataOutPort bits_per_packet[9:0] {
      Source bits_per_packet_shift_reg;
   }
   DataOutPort to_scan_in_bits[2:0] {
      Source to_scan_in_bits_shift_reg;
   }
   DataOutPort from_scan_out_bits[2:0] {
      Source from_scan_out_bits_shift_reg;
   }
   DataOutPort from_scan_out_le_strobe_en {
      Source from_scan_out_le_strobe_en_shift_reg;
   }
   DataOutPort total_shift_cnt_minus_one[27:0] {
      Source total_shift_cnt_minus_one_shift_reg;
   }
   DataOutPort extra_shift_packets[27:0] {
      Source extra_shift_packets_shift_reg;
   }
   DataOutPort packet_size[15:0] {
      Source packet_size_shift_reg;
   }
   DataOutPort timeslot_sharing_count_minus_one[2:0] {
      Source timeslot_sharing_count_minus_one_shift_reg;
   }
   DataOutPort timeslot_sharing_offset[2:0] {
      Source timeslot_sharing_offset_shift_reg;
   }
   DataOutPort force_suppress_capture {
      Source force_suppress_capture_shift_reg;
   }
   DataOutPort initial_bit0_position[1:0] {
      Source initial_bit0_position_shift_reg;
   }
   DataOutPort disable_on_chip_compare_contribution {
      Source disable_on_chip_compare_contribution_shift_reg;
   }
   DataOutPort on_chip_compare_group_count[7:0] {
      Source on_chip_compare_group_count_shift_reg;
   }
   DataOutPort on_chip_compare_group[7:0] {
      Source on_chip_compare_group_shift_reg;
   }
   DataOutPort loop_back_en {
      Source loop_back_en_shift_reg;
   }
   DataOutPort capture_packets[2:0] {
      Source capture_packets_shift_reg;
   }
   DataOutPort init_scan_load_used {
      Source init_scan_load_used_shift_reg;
   }
   DataOutPort disable_first_scan_load_masking {
      Source disable_first_scan_load_masking_shift_reg;
   }
   DataOutPort packets_per_capture_clock_minus_one[3:0] {
      Source packets_per_capture_clock_minus_one_shift_reg;
   }
   DataOutPort scan_en_transition_packets[6:0] {
      Source scan_en_transition_packets_shift_reg;
   }
   DataOutPort scan_en_launch_packet[6:0] {
      Source scan_en_launch_packet_shift_reg;
   }
   DataOutPort edt_update_falling_transition_words[6:0] {
      Source edt_update_falling_transition_words_shift_reg;
   }
   DataOutPort edt_update_falling_launch_word[6:0] {
      Source edt_update_falling_launch_word_shift_reg;
   }
   DataOutPort min_shift_clock_low_width[5:0] {
      Source min_shift_clock_low_width_shift_reg;
   }
   DataOutPort min_capture_clock_low_width[5:0] {
      Source min_capture_clock_low_width_shift_reg;
   }
   DataOutPort packet_width_is_less_than_two_bus_clock {
      Source packet_width_is_less_than_two_bus_clock_shift_reg;
   }
   DataOutPort infinite_shift_mode {
      Source infinite_shift_mode_signal;
   }
   DataOutPort streaming_through_ijtag_en {
      Source streaming_through_ijtag_en_shift_reg;
   }
   Attribute tessent_instrument_type = "mentor::ssn";
   Attribute tessent_instrument_subtype = "scan_host_ijtag_reg";
   Attribute tessent_signature = "29a1ed13e2bd3903281d434e02abdb9e";
   Alias sticky_status = config_sib_shift_reg {
      RefEnum pass_fail_1bit_wide;
   }
   Enum pass_fail_1bit_wide {
      pass = 1'b0;
      fail = 1'b1;
   }
   ScanRegister config_sib_shift_reg {
      ScanInSource config_sib_si_mux;
      ResetValue 1'b0;
      Attribute tessent_ssn_function = "config_inclusion_bit";
      Attribute tessent_rtl_reg = "config_sib";
   }
   ScanRegister enable_shift_reg {
      ScanInSource streaming_through_ijtag_en_si_mux;
      CaptureSource enable_shift_reg;
      ResetValue 1'b0;
      Attribute tessent_ssn_function = "ssh_enable";
      Attribute tessent_rtl_reg = "enable_update_stage";
   }
   ScanRegister fake_data_bus_reg {
      ScanInSource ijtag_si;
      Attribute tessent_ignore_during_icl_verification = "on";
   }
   ScanRegister streaming_through_ijtag_en_shift_reg {
      ScanInSource enable_shift_reg;
      ResetValue 1'b0;
      Attribute tessent_ssn_function = "streaming_through_ijtag_enable";
      Attribute tessent_rtl_reg = "streaming_through_ijtag_en_update_stage";
      Attribute explicit_iwrite_only = 1'b1;
   }
   ScanRegister on_chip_compare_enable_shift_reg {
      ScanInSource streaming_through_ijtag_en_shift_reg;
      CaptureSource on_chip_compare_enable_shift_reg;
      Attribute tessent_rtl_reg = "on_chip_compare_enable_shift_reg";
      Attribute tessent_ssn_function = "ssh_on_chip_compare_enable";
   }
   ScanRegister initial_offset_shift_reg[30:0] {
      ScanInSource on_chip_compare_enable_shift_reg;
      CaptureSource initial_offset_shift_reg[30:0];
      Attribute tessent_rtl_reg = "initial_offset_shift_reg[30:0]";
   }
   ScanRegister relative_initial_offset_enable_shift_reg {
      ScanInSource initial_offset_shift_reg[0];
      CaptureSource relative_initial_offset_enable_shift_reg;
      Attribute tessent_rtl_reg = "relative_initial_offset_enable_shift_reg";
   }
   ScanRegister bus_width_shift_reg[1:0] {
      ScanInSource relative_initial_offset_enable_shift_reg;
      CaptureSource bus_width_shift_reg[1:0];
      Attribute tessent_rtl_reg = "bus_width_shift_reg[1:0]";
   }
   ScanRegister bits_per_packet_shift_reg[9:0] {
      ScanInSource bus_width_shift_reg[0];
      CaptureSource bits_per_packet_shift_reg[9:0];
      Attribute tessent_rtl_reg = "bits_per_packet_shift_reg[9:0]";
   }
   ScanRegister to_scan_in_bits_shift_reg[2:0] {
      ScanInSource bits_per_packet_shift_reg[0];
      CaptureSource to_scan_in_bits_shift_reg[2:0];
      Attribute tessent_rtl_reg = "to_scan_in_bits_shift_reg[2:0]";
   }
   ScanRegister from_scan_out_bits_shift_reg[2:0] {
      ScanInSource to_scan_in_bits_shift_reg[0];
      CaptureSource from_scan_out_bits_shift_reg[2:0];
      Attribute tessent_rtl_reg = "from_scan_out_bits_shift_reg[2:0]";
   }
   ScanRegister from_scan_out_le_strobe_en_shift_reg {
      ScanInSource from_scan_out_bits_shift_reg[0];
      CaptureSource from_scan_out_le_strobe_en_shift_reg;
      Attribute tessent_rtl_reg = "from_scan_out_le_strobe_en_shift_reg";
   }
   ScanRegister total_shift_cnt_minus_one_shift_reg[27:0] {
      ScanInSource from_scan_out_le_strobe_en_shift_reg;
      CaptureSource total_shift_cnt_minus_one_shift_reg[27:0];
      Attribute tessent_rtl_reg = "total_shift_cnt_minus_one_shift_reg[27:0]";
   }
   ScanRegister extra_shift_packets_shift_reg[27:0] {
      ScanInSource total_shift_cnt_minus_one_shift_reg[0];
      CaptureSource extra_shift_packets_shift_reg[27:0];
      Attribute tessent_rtl_reg = "extra_shift_packets_shift_reg[27:0]";
   }
   ScanRegister packet_size_shift_reg[15:0] {
      ScanInSource extra_shift_packets_shift_reg[0];
      CaptureSource packet_size_shift_reg[15:0];
      Attribute tessent_rtl_reg = "packet_size_shift_reg[15:0]";
   }
   ScanRegister timeslot_sharing_count_minus_one_shift_reg[2:0] {
      ScanInSource packet_size_shift_reg[0];
      CaptureSource timeslot_sharing_count_minus_one_shift_reg[2:0];
      Attribute tessent_rtl_reg = 
          "timeslot_sharing_count_minus_one_shift_reg[2:0]";
   }
   ScanRegister timeslot_sharing_offset_shift_reg[2:0] {
      ScanInSource timeslot_sharing_count_minus_one_shift_reg[0];
      CaptureSource timeslot_sharing_offset_shift_reg[2:0];
      Attribute tessent_rtl_reg = "timeslot_sharing_offset_shift_reg[2:0]";
   }
   ScanRegister force_suppress_capture_shift_reg {
      ScanInSource timeslot_sharing_offset_shift_reg[0];
      CaptureSource force_suppress_capture_shift_reg;
      Attribute tessent_rtl_reg = "force_suppress_capture_shift_reg";
   }
   ScanRegister initial_bit0_position_shift_reg[1:0] {
      ScanInSource force_suppress_capture_shift_reg;
      CaptureSource initial_bit0_position_shift_reg[1:0];
      Attribute tessent_rtl_reg = "initial_bit0_position_shift_reg[1:0]";
   }
   ScanRegister disable_on_chip_compare_contribution_shift_reg {
      ScanInSource initial_bit0_position_shift_reg[0];
      CaptureSource disable_on_chip_compare_contribution_shift_reg;
      Attribute tessent_rtl_reg = 
          "disable_on_chip_compare_contribution_shift_reg";
   }
   ScanRegister on_chip_compare_group_count_shift_reg[7:0] {
      ScanInSource disable_on_chip_compare_contribution_shift_reg;
      CaptureSource on_chip_compare_group_count_shift_reg[7:0];
      Attribute tessent_rtl_reg = "on_chip_compare_group_count_shift_reg[7:0]";
   }
   ScanRegister on_chip_compare_group_shift_reg[7:0] {
      ScanInSource on_chip_compare_group_count_shift_reg[0];
      CaptureSource on_chip_compare_group_shift_reg[7:0];
      Attribute tessent_rtl_reg = "on_chip_compare_group_shift_reg[7:0]";
   }
   ScanRegister loop_back_en_shift_reg {
      ScanInSource on_chip_compare_group_shift_reg[0];
      CaptureSource loop_back_en_shift_reg;
      Attribute tessent_rtl_reg = "loop_back_en_shift_reg";
   }
   ScanRegister capture_packets_shift_reg[2:0] {
      ScanInSource loop_back_en_shift_reg;
      CaptureSource capture_packets_shift_reg[2:0];
      Attribute tessent_rtl_reg = "capture_packets_shift_reg[2:0]";
   }
   ScanRegister init_scan_load_used_shift_reg {
      ScanInSource capture_packets_shift_reg[0];
      CaptureSource init_scan_load_used_shift_reg;
      Attribute tessent_rtl_reg = "init_scan_load_used_shift_reg";
   }
   ScanRegister disable_first_scan_load_masking_shift_reg {
      ScanInSource init_scan_load_used_shift_reg;
      CaptureSource disable_first_scan_load_masking_shift_reg;
      Attribute tessent_rtl_reg = "disable_first_scan_load_masking_shift_reg";
   }
   ScanRegister packets_per_capture_clock_minus_one_shift_reg[3:0] {
      ScanInSource disable_first_scan_load_masking_shift_reg;
      CaptureSource packets_per_capture_clock_minus_one_shift_reg[3:0];
      Attribute tessent_rtl_reg = 
          "packets_per_capture_clock_minus_one_shift_reg[3:0]";
   }
   ScanRegister scan_en_transition_packets_shift_reg[6:0] {
      ScanInSource packets_per_capture_clock_minus_one_shift_reg[0];
      CaptureSource scan_en_transition_packets_shift_reg[6:0];
      Attribute tessent_rtl_reg = "scan_en_transition_packets_shift_reg[6:0]";
   }
   ScanRegister scan_en_launch_packet_shift_reg[6:0] {
      ScanInSource scan_en_transition_packets_shift_reg[0];
      CaptureSource scan_en_launch_packet_shift_reg[6:0];
      Attribute tessent_rtl_reg = "scan_en_launch_packet_shift_reg[6:0]";
   }
   ScanRegister edt_update_falling_transition_words_shift_reg[6:0] {
      ScanInSource scan_en_launch_packet_shift_reg[0];
      CaptureSource edt_update_falling_transition_words_shift_reg[6:0];
      Attribute tessent_rtl_reg = 
          "edt_update_falling_transition_words_shift_reg[6:0]";
   }
   ScanRegister edt_update_falling_launch_word_shift_reg[6:0] {
      ScanInSource edt_update_falling_transition_words_shift_reg[0];
      CaptureSource edt_update_falling_launch_word_shift_reg[6:0];
      Attribute tessent_rtl_reg = 
          "edt_update_falling_launch_word_shift_reg[6:0]";
   }
   ScanRegister min_shift_clock_low_width_shift_reg[5:0] {
      ScanInSource edt_update_falling_launch_word_shift_reg[0];
      CaptureSource min_shift_clock_low_width_shift_reg[5:0];
      Attribute tessent_rtl_reg = "min_shift_clock_low_width_shift_reg[5:0]";
   }
   ScanRegister min_capture_clock_low_width_shift_reg[5:0] {
      ScanInSource min_shift_clock_low_width_shift_reg[0];
      CaptureSource min_capture_clock_low_width_shift_reg[5:0];
      Attribute tessent_rtl_reg = "min_capture_clock_low_width_shift_reg[5:0]";
   }
   ScanRegister packet_width_is_less_than_two_bus_clock_shift_reg {
      ScanInSource min_capture_clock_low_width_shift_reg[0];
      CaptureSource packet_width_is_less_than_two_bus_clock_shift_reg;
      Attribute tessent_rtl_reg = 
          "packet_width_is_less_than_two_bus_clock_shift_reg";
   }
   ScanRegister bus_in_shift_reg[1:0] {
      ScanInSource ijtag_si;
   }
   ScanRegister bus_out_shift_reg[1:0] {
      ScanInSource bus_in_shift_reg[0];
   }
   ScanMux config_sib_si_mux SelectedBy config_sib_shift_reg, enable_shift_reg {
      2'b11 : bus_out_shift_reg[0];
      2'bxx : ijtag_si;
   }
   ScanMux streaming_through_ijtag_en_si_mux SelectedBy 
       streaming_through_ijtag_en_si_mux_select {
      1'b0 : config_sib_shift_reg;
      1'b1 : fake_data_bus_reg;
   }
   ScanMux ijtag_so_mux SelectedBy config_sib_shift_reg, enable_shift_reg {
      Attribute exclude_codes_from_icl_verify = "{2'b01}";
      2'b10 : packet_width_is_less_than_two_bus_clock_shift_reg;
      2'b00 : enable_shift_reg;
      2'b01 : enable_shift_reg;
      2'b11 : enable_shift_reg;
   }
   LogicSignal streaming_through_ijtag_en_si_mux_select {
      streaming_through_ijtag_en_shift_reg && enable_shift_reg;
   }
   LogicSignal infinite_shift_mode_signal {
      total_shift_cnt_minus_one_shift_reg == 28'b0;
   }
}

// Source: wrapped_cores/gps_baseband/tsdb_outdir/dft_inserted_designs/gps_baseband_rtl1.dft_inserted_design/gps_baseband.icl
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

// Source: wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl2.dft_inserted_design/processor_core.icl
Module processor_core {
   // Created by ICL extraction
   ClockPort dco_clk {
      Attribute tessent_clock_domain_labels = "dco_clk dco_clk";
      Attribute tessent_clock_periods = "all 3.00ns";
   }
   CaptureEnPort ijtag_ce;
   ResetPort ijtag_reset {
      ActivePolarity 0;
   }
   ShiftEnPort ijtag_se;
   SelectPort ijtag_sel;
   ScanInPort ijtag_si;
   ScanOutPort ijtag_so {
      Source processor_core_rtl2_tessent_sib_sri_inst.ijtag_so;
   }
   TCKPort ijtag_tck;
   UpdateEnPort ijtag_ue;
   ClockPort ssn_bus_clock {
      Attribute function_modifier = "tessent_ssn_clock";
      Attribute forced_high_dft_signal_list = "ssn_en";
      Attribute tessent_ssn_datapath_ids = "1";
   }
   DataInPort ssn_bus_data_in[1] {
      Attribute function_modifier = "tessent_ssn_bus_data";
      Attribute forced_high_dft_signal_list = "ssn_en";
      Attribute tessent_ssn_datapath_ids = "1";
      Attribute tessent_ssn_datapath_bus_index = 1;
      Attribute tessent_ssn_clocks = "ssn_bus_clock";
      Attribute tessent_ssn_data_phases = "1 1";
   }
   DataInPort ssn_bus_data_in[0] {
      Attribute function_modifier = "tessent_ssn_bus_data";
      Attribute forced_high_dft_signal_list = "ssn_en";
      Attribute tessent_ssn_datapath_ids = "1";
      Attribute tessent_ssn_datapath_bus_index = 0;
      Attribute tessent_ssn_clocks = "ssn_bus_clock";
      Attribute tessent_ssn_data_phases = "1 1";
   }
   DataOutPort ssn_bus_data_out[1] {
      Source processor_core_rtl2_tessent_ssn_pipe_2_inst.bus_data_out[1];
      Attribute function_modifier = "tessent_ssn_bus_data";
      Attribute forced_high_dft_signal_list = "ssn_en";
      Attribute tessent_ssn_datapath_ids = "1";
      Attribute tessent_ssn_datapath_bus_index = 1;
      Attribute tessent_ssn_clocks = "ssn_bus_clock";
      Attribute tessent_ssn_data_phases = "1 1";
   }
   DataOutPort ssn_bus_data_out[0] {
      Source processor_core_rtl2_tessent_ssn_pipe_2_inst.bus_data_out[0];
      Attribute function_modifier = "tessent_ssn_bus_data";
      Attribute forced_high_dft_signal_list = "ssn_en";
      Attribute tessent_ssn_datapath_ids = "1";
      Attribute tessent_ssn_datapath_bus_index = 0;
      Attribute tessent_ssn_clocks = "ssn_bus_clock";
      Attribute tessent_ssn_data_phases = "1 1";
   }
   ScanInterface ijtag {
      Port ijtag_ce;
      Port ijtag_reset;
      Port ijtag_se;
      Port ijtag_sel;
      Port ijtag_si;
      Port ijtag_so;
      Port ijtag_tck;
      Port ijtag_ue;
   }
   Attribute tessent_design_format = "verilog_2001";
   Attribute test_setup_procfile = "";
   Attribute forced_low_internal_input_port_list = 
       "{processor_core_rtl2_tessent_ssn_scan_host_1_inst/scan_en}";
   Attribute icl_extraction_date = "Thu Aug 27 11:22:50 CST 2026";
   Attribute created_by_tessent_icl_extract = "true";
   Attribute tessent_design_id = "rtl2";
   Attribute tessent_design_level = "physical_block";
   Attribute tessent_is_physical_module = "true";
   Instance DATA_MEM_1 Of SYNC_1RW_8Kx16 {
      InputPort A[12] = DATA_MEM_1_interface_inst.A[12];
      InputPort A[11] = DATA_MEM_1_interface_inst.A[11];
      InputPort A[10] = DATA_MEM_1_interface_inst.A[10];
      InputPort A[9] = DATA_MEM_1_interface_inst.A[9];
      InputPort A[8] = DATA_MEM_1_interface_inst.A[8];
      InputPort A[7] = DATA_MEM_1_interface_inst.A[7];
      InputPort A[6] = DATA_MEM_1_interface_inst.A[6];
      InputPort A[5] = DATA_MEM_1_interface_inst.A[5];
      InputPort A[4] = DATA_MEM_1_interface_inst.A[4];
      InputPort A[3] = DATA_MEM_1_interface_inst.A[3];
      InputPort A[2] = DATA_MEM_1_interface_inst.A[2];
      InputPort A[1] = DATA_MEM_1_interface_inst.A[1];
      InputPort A[0] = DATA_MEM_1_interface_inst.A[0];
      InputPort CLK = processor_core_rtl2_tessent_occ_dco_clk_inst.clock_out;
      Attribute tessent_design_instance = "DATA_MEM_1";
   }
   Instance DATA_MEM_1_interface_inst Of 
       processor_core_rtl1_tessent_mbist_c1_interface_m1 {
      InputPort BIST_CLK = 
          processor_core_rtl2_tessent_occ_dco_clk_inst.clock_out;
      InputPort BIST_COLLAR_EN = 
          processor_core_rtl1_tessent_mbist_c1_controller_inst.BIST_COLLAR_EN0;
      InputPort BIST_ASYNC_RESETN = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_ASYNC_RESET;
      InputPort BIST_SI = 
          processor_core_rtl1_tessent_mbist_c1_controller_inst.MEM0_BIST_COLLAR_SI;

      InputPort BIST_SHIFT_COLLAR = 
          processor_core_rtl1_tessent_mbist_c1_controller_inst.BIST_SHIFT_COLLAR;

      InputPort BIST_SETUP0 = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_SETUP[0];
      InputPort MEM_BYPASS_EN = 
          processor_core_rtl1_tessent_mbist_bap_inst.memory_bypass_to_en;
      InputPort MCP_BOUNDING_EN = 
          processor_core_rtl1_tessent_mbist_bap_inst.mcp_bounding_to_en;
      InputPort INCLUDE_MEM_RESULTS_REG = 
          processor_core_rtl1_tessent_mbist_bap_inst.INCLUDE_MEM_RESULTS_REG;
      Attribute tessent_design_instance = "DATA_MEM_1_interface_inst";
   }
   Instance PROG_MEM_2 Of SYNC_1RW_8Kx16 {
      InputPort A[12] = PROG_MEM_2_interface_inst.A[12];
      InputPort A[11] = PROG_MEM_2_interface_inst.A[11];
      InputPort A[10] = PROG_MEM_2_interface_inst.A[10];
      InputPort A[9] = PROG_MEM_2_interface_inst.A[9];
      InputPort A[8] = PROG_MEM_2_interface_inst.A[8];
      InputPort A[7] = PROG_MEM_2_interface_inst.A[7];
      InputPort A[6] = PROG_MEM_2_interface_inst.A[6];
      InputPort A[5] = PROG_MEM_2_interface_inst.A[5];
      InputPort A[4] = PROG_MEM_2_interface_inst.A[4];
      InputPort A[3] = PROG_MEM_2_interface_inst.A[3];
      InputPort A[2] = PROG_MEM_2_interface_inst.A[2];
      InputPort A[1] = PROG_MEM_2_interface_inst.A[1];
      InputPort A[0] = PROG_MEM_2_interface_inst.A[0];
      InputPort CLK = processor_core_rtl2_tessent_occ_dco_clk_inst.clock_out;
      Attribute tessent_design_instance = "PROG_MEM_2";
   }
   Instance PROG_MEM_2_interface_inst Of 
       processor_core_rtl1_tessent_mbist_c1_interface_m2 {
      InputPort BIST_CLK = 
          processor_core_rtl2_tessent_occ_dco_clk_inst.clock_out;
      InputPort BIST_COLLAR_EN = 
          processor_core_rtl1_tessent_mbist_c1_controller_inst.BIST_COLLAR_EN1;
      InputPort BIST_ASYNC_RESETN = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_ASYNC_RESET;
      InputPort BIST_SI = 
          processor_core_rtl1_tessent_mbist_c1_controller_inst.MEM1_BIST_COLLAR_SI;

      InputPort BIST_SHIFT_COLLAR = 
          processor_core_rtl1_tessent_mbist_c1_controller_inst.BIST_SHIFT_COLLAR;

      InputPort BIST_SETUP0 = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_SETUP[0];
      InputPort MEM_BYPASS_EN = 
          processor_core_rtl1_tessent_mbist_bap_inst.memory_bypass_to_en;
      InputPort MCP_BOUNDING_EN = 
          processor_core_rtl1_tessent_mbist_bap_inst.mcp_bounding_to_en;
      InputPort INCLUDE_MEM_RESULTS_REG = 
          processor_core_rtl1_tessent_mbist_bap_inst.INCLUDE_MEM_RESULTS_REG;
      Attribute tessent_design_instance = "PROG_MEM_2_interface_inst";
   }
   Instance processor_core_rtl1_tessent_mbist_bap_inst Of 
       processor_core_rtl1_tessent_mbist_bap {
      InputPort reset = processor_core_rtl1_tessent_sib_sti_inst.ijtag_to_reset;
      InputPort ijtag_select = 
          processor_core_rtl1_tessent_sib_mbist_inst.ijtag_to_sel;
      InputPort si = processor_core_rtl1_tessent_sib_sti_inst.ijtag_to_si;
      InputPort capture_en = 
          processor_core_rtl1_tessent_sib_sti_inst.ijtag_to_ce;
      InputPort shift_en = processor_core_rtl1_tessent_sib_sti_inst.ijtag_to_se;
      InputPort update_en = processor_core_rtl1_tessent_sib_sti_inst.ijtag_to_ue;

      InputPort tck = processor_core_rtl1_tessent_sib_sti_inst.ijtag_to_tck;
      InputPort memory_bypass_en = 
          processor_core_rtl1_tessent_sib_sti_inst.ltest_to_mem_bypass_en;
      InputPort mcp_bounding_en = 
          processor_core_rtl1_tessent_sib_sti_inst.ltest_to_mcp_bounding_en;
      InputPort ltest_en = processor_core_rtl1_tessent_sib_sti_inst.ltest_to_en;
      InputPort fromBist[0] = 
          processor_core_rtl1_tessent_mbist_c1_controller_inst.MBISTPG_SO;
      InputPort MBISTPG_GO[0] = 
          processor_core_rtl1_tessent_mbist_c1_controller_inst.MBISTPG_GO;
      InputPort MBISTPG_DONE[0] = 
          processor_core_rtl1_tessent_mbist_c1_controller_inst.MBISTPG_DONE;
      Attribute tessent_design_instance = 
          "processor_core_rtl1_tessent_mbist_bap_inst";
   }
   Instance processor_core_rtl1_tessent_mbist_c1_controller_inst Of 
       processor_core_rtl1_tessent_mbist_c1_controller {
      InputPort BIST_CLK = 
          processor_core_rtl2_tessent_occ_dco_clk_inst.clock_out;
      InputPort MBISTPG_EN = 
          processor_core_rtl1_tessent_mbist_bap_inst.bistEn[0];
      InputPort LV_TM = processor_core_rtl1_tessent_mbist_bap_inst.ltest_to_en;
      InputPort MEM_BYPASS_EN = 
          processor_core_rtl1_tessent_mbist_bap_inst.memory_bypass_to_en;
      InputPort MCP_BOUNDING_EN = 
          processor_core_rtl1_tessent_mbist_bap_inst.mcp_bounding_to_en;
      InputPort MBISTPG_ASYNC_RESETN = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_ASYNC_RESET;
      InputPort MBISTPG_DIAG_EN = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_DIAG_EN;
      InputPort BIST_SETUP2 = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_SETUP[2];
      InputPort BIST_SETUP[1] = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_SETUP[1];
      InputPort BIST_SETUP[0] = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_SETUP[0];
      InputPort MBISTPG_REDUCED_ADDR_CNT_EN = 
          processor_core_rtl1_tessent_mbist_bap_inst.REDUCED_ADDRESS_COUNT;
      InputPort MBISTPG_MEM_RST = 
          processor_core_rtl1_tessent_mbist_bap_inst.ENABLE_MEM_RESET;
      InputPort MBISTPG_TESTDATA_SELECT = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_SELECT_TEST_DATA;
      InputPort FL_CNT_MODE[1] = 
          processor_core_rtl1_tessent_mbist_bap_inst.FL_CNT_MODE1;
      InputPort FL_CNT_MODE[0] = 
          processor_core_rtl1_tessent_mbist_bap_inst.FL_CNT_MODE0;
      InputPort MBISTPG_ALGO_MODE[1] = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_ALGO_MODE1;
      InputPort MBISTPG_ALGO_MODE[0] = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_ALGO_MODE0;
      InputPort TCK = processor_core_rtl1_tessent_sib_sti_inst.ijtag_to_tck;
      InputPort BIST_SI = processor_core_rtl1_tessent_mbist_bap_inst.toBist[0];
      InputPort BIST_HOLD = processor_core_rtl1_tessent_mbist_bap_inst.BIST_HOLD;

      InputPort MEM0_BIST_COLLAR_SO = DATA_MEM_1_interface_inst.BIST_SO;
      InputPort MEM1_BIST_COLLAR_SO = PROG_MEM_2_interface_inst.BIST_SO;
      Attribute tessent_design_instance = 
          "processor_core_rtl1_tessent_mbist_c1_controller_inst";
   }
   Instance processor_core_rtl1_tessent_sib_mbist_inst Of 
       processor_core_rtl1_tessent_sib_2 {
      InputPort ijtag_reset = 
          processor_core_rtl1_tessent_sib_sti_inst.ijtag_to_reset;
      InputPort ijtag_sel = 
          processor_core_rtl1_tessent_sib_sti_inst.ijtag_to_sel;
      InputPort ijtag_si = processor_core_rtl1_tessent_sib_sti_inst.ijtag_to_si;
      InputPort ijtag_ce = processor_core_rtl1_tessent_sib_sti_inst.ijtag_to_ce;
      InputPort ijtag_se = processor_core_rtl1_tessent_sib_sti_inst.ijtag_to_se;
      InputPort ijtag_ue = processor_core_rtl1_tessent_sib_sti_inst.ijtag_to_ue;
      InputPort ijtag_tck = 
          processor_core_rtl1_tessent_sib_sti_inst.ijtag_to_tck;
      InputPort ijtag_from_so = processor_core_rtl1_tessent_mbist_bap_inst.so;
      Attribute tessent_design_instance = 
          "processor_core_rtl1_tessent_sib_mbist_inst";
   }
   Instance processor_core_rtl1_tessent_sib_sti_inst Of 
       processor_core_rtl1_tessent_sib_1 {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = ijtag_sel;
      InputPort ijtag_si = ijtag_si;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_from_so = 
          processor_core_rtl1_tessent_sib_mbist_inst.ijtag_so;
      InputPort ltest_en = 
          processor_core_rtl2_tessent_tdr_sri_ctrl_inst.ltest_en;
      InputPort ltest_occ_en = 
          processor_core_rtl2_tessent_tdr_sri_ctrl_inst.tck_occ_en;
      InputPort ltest_static_clock_control_mode = 'b0;
      InputPort ltest_clock_sequence[1] = 'b0;
      InputPort ltest_clock_sequence[0] = 'b0;
      InputPort ltest_mem_bypass_en = 
          processor_core_rtl2_tessent_tdr_sri_ctrl_inst.memory_bypass_en;
      InputPort ltest_mcp_bounding_en = 'b0;
      Attribute tessent_design_instance = 
          "processor_core_rtl1_tessent_sib_sti_inst";
   }
   Instance processor_core_rtl2_tessent_edt_c1_ext_inst Of 
       processor_core_rtl2_tessent_edt_c1_ext {
      InputPort edt_bypass = 
          processor_core_rtl2_tessent_edt_c1_ext_tdr_inst.edt_bypass;
      Attribute tessent_design_instance = 
          "processor_core_rtl2_tessent_edt_c1_ext_inst";
   }
   Instance processor_core_rtl2_tessent_edt_c1_ext_tdr_inst Of 
       processor_core_rtl2_tessent_edt_c1_ext_tdr {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = 
          processor_core_rtl2_tessent_sib_edt_inst.ijtag_to_sel;
      InputPort ijtag_si = 
          processor_core_rtl2_tessent_edt_c1_int_tdr_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      Attribute tessent_design_instance = 
          "processor_core_rtl2_tessent_edt_c1_ext_tdr_inst";
   }
   Instance processor_core_rtl2_tessent_edt_c1_int_inst Of 
       processor_core_rtl2_tessent_edt_c1_int {
      InputPort edt_bypass = 
          processor_core_rtl2_tessent_edt_c1_int_tdr_inst.edt_bypass;
      Attribute tessent_design_instance = 
          "processor_core_rtl2_tessent_edt_c1_int_inst";
   }
   Instance processor_core_rtl2_tessent_edt_c1_int_tdr_inst Of 
       processor_core_rtl2_tessent_edt_c1_int_tdr {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = 
          processor_core_rtl2_tessent_sib_edt_inst.ijtag_to_sel;
      InputPort ijtag_si = processor_core_rtl2_tessent_sib_occ_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      Attribute tessent_design_instance = 
          "processor_core_rtl2_tessent_edt_c1_int_tdr_inst";
   }
   Instance processor_core_rtl2_tessent_occ_dco_clk_inst Of 
       processor_core_rtl2_tessent_occ {
      InputPort fast_clock = dco_clk;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_si = processor_core_rtl2_tessent_sib_ssn_inst.ijtag_so;
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_sel = 
          processor_core_rtl2_tessent_sib_occ_inst.ijtag_to_sel;
      Attribute tessent_design_instance = 
          "processor_core_rtl2_tessent_occ_dco_clk_inst";
   }
   Instance processor_core_rtl2_tessent_sib_edt_inst Of 
       processor_core_rtl2_tessent_sib_2 {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = 
          processor_core_rtl2_tessent_sib_sri_inst.ijtag_to_sel;
      InputPort ijtag_si = processor_core_rtl2_tessent_sib_occ_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_from_so = 
          processor_core_rtl2_tessent_edt_c1_ext_tdr_inst.ijtag_so;
      Attribute tessent_design_instance = 
          "processor_core_rtl2_tessent_sib_edt_inst";
   }
   Instance processor_core_rtl2_tessent_sib_occ_inst Of 
       processor_core_rtl2_tessent_sib_2 {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = 
          processor_core_rtl2_tessent_sib_sri_inst.ijtag_to_sel;
      InputPort ijtag_si = processor_core_rtl2_tessent_sib_ssn_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_from_so = 
          processor_core_rtl2_tessent_occ_dco_clk_inst.ijtag_so;
      Attribute tessent_design_instance = 
          "processor_core_rtl2_tessent_sib_occ_inst";
   }
   Instance processor_core_rtl2_tessent_sib_sri_ctrl_inst Of 
       processor_core_rtl2_tessent_sib_2 {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = 
          processor_core_rtl2_tessent_sib_sri_inst.ijtag_to_sel;
      InputPort ijtag_si = processor_core_rtl2_tessent_sib_edt_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_from_so = 
          processor_core_rtl2_tessent_tdr_sri_ctrl_inst.ijtag_so;
      Attribute tessent_design_instance = 
          "processor_core_rtl2_tessent_sib_sri_ctrl_inst";
   }
   Instance processor_core_rtl2_tessent_sib_sri_inst Of 
       processor_core_rtl2_tessent_sib_1 {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = ijtag_sel;
      InputPort ijtag_si = processor_core_rtl1_tessent_sib_sti_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_from_so = 
          processor_core_rtl2_tessent_sib_sri_ctrl_inst.ijtag_so;
      Attribute tessent_design_instance = 
          "processor_core_rtl2_tessent_sib_sri_inst";
   }
   Instance processor_core_rtl2_tessent_sib_ssn_inst Of 
       processor_core_rtl2_tessent_sib_2 {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = 
          processor_core_rtl2_tessent_sib_sri_inst.ijtag_to_sel;
      InputPort ijtag_si = processor_core_rtl1_tessent_sib_sti_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_from_so = 
          processor_core_rtl2_tessent_ssn_scan_host_1_inst.ijtag_so;
      Attribute tessent_design_instance = 
          "processor_core_rtl2_tessent_sib_ssn_inst";
   }
   Instance processor_core_rtl2_tessent_ssn_pipe_1_inst Of 
       processor_core_rtl2_tessent_ssn_pipe_w2_1 {
      InputPort ijtag_reset = ijtag_reset;
      InputPort bus_clock = ssn_bus_clock;
      InputPort bus_data_in[1] = ssn_bus_data_in[1];
      InputPort bus_data_in[0] = ssn_bus_data_in[0];
      Attribute tessent_design_instance = 
          "processor_core_rtl2_tessent_ssn_pipe_1_inst";
   }
   Instance processor_core_rtl2_tessent_ssn_pipe_2_inst Of 
       processor_core_rtl2_tessent_ssn_pipe_w2_1 {
      InputPort ijtag_reset = ijtag_reset;
      InputPort bus_clock = ssn_bus_clock;
      InputPort bus_data_in[1] = 
          processor_core_rtl2_tessent_ssn_scan_host_1_inst.bus_data_out[1];
      InputPort bus_data_in[0] = 
          processor_core_rtl2_tessent_ssn_scan_host_1_inst.bus_data_out[0];
      Attribute tessent_design_instance = 
          "processor_core_rtl2_tessent_ssn_pipe_2_inst";
   }
   Instance processor_core_rtl2_tessent_ssn_scan_host_1_inst Of 
       processor_core_rtl2_tessent_ssn_scan_host_1 {
      InputPort bus_clock = ssn_bus_clock;
      InputPort bus_data_in[1] = 
          processor_core_rtl2_tessent_ssn_pipe_1_inst.bus_data_out[1];
      InputPort bus_data_in[0] = 
          processor_core_rtl2_tessent_ssn_pipe_1_inst.bus_data_out[0];
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_sel = 
          processor_core_rtl2_tessent_sib_ssn_inst.ijtag_to_sel;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_si = processor_core_rtl1_tessent_sib_sti_inst.ijtag_so;
      Attribute tessent_design_instance = 
          "processor_core_rtl2_tessent_ssn_scan_host_1_inst";
   }
   Instance processor_core_rtl2_tessent_tdr_sri_ctrl_inst Of 
       processor_core_rtl2_tessent_tdr_sri_ctrl {
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = 
          processor_core_rtl2_tessent_sib_sri_ctrl_inst.ijtag_to_sel;
      InputPort ijtag_si = processor_core_rtl2_tessent_sib_edt_inst.ijtag_so;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_tck = ijtag_tck;
      Attribute tessent_design_instance = 
          "processor_core_rtl2_tessent_tdr_sri_ctrl_inst";
   }
}

// Source: wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl2.dft_inserted_design/processor_core.icl
Module SYNC_1RW_8Kx16 {
   // ICL module read from source on or near line 233 of file '/home/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl1.dft_inserted_design/processor_core.icl'
   DataInPort A[12:0] {
      Attribute connection_rule_option = "allowed_no_source";
      Attribute tessent_memory_bist_function = "address";
   }
   ClockPort CLK;
   Attribute tessent_use_in_dft_specification = "false";
   Attribute keep_active_during_scan_test = "false";
   Attribute tessent_memory_module = "without_internal_scan_logic";
}

// Source: wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl2.dft_inserted_design/processor_core.icl
Module processor_core_rtl1_tessent_mbist_c1_interface_m1 {
   // ICL module read from source on or near line 246 of file '/home/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl1.dft_inserted_design/processor_core.icl'
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
   DataOutPort A[12:0] {
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_memory_bist_function = "address";
   }
   Attribute tessent_instrument_type = "mentor::memory_bist";
   Attribute tessent_instrument_subtype = "memory_interface";
   Attribute tessent_signature = "5ef3fe0ea73def8a9bdf1ed58027ebe2";
   Attribute tessent_ignore_during_icl_verification = "on";
   Attribute keep_active_during_scan_test = "false";
   Attribute tessent_use_in_dft_specification = "false";
   Attribute tessent_bist_input_select_persistent_cell_output_list = 
       "tessent_persistent_cell_BIST_INPUT_SELECT_INT/Y";
   Attribute tessent_async_bypass_persistent_cell_input_list = "";
   Attribute tessent_bist_clk_persistent_cell_output_list = 
       "tessent_persistent_cell_GATING_BIST_CLK/GCK";
   Attribute tessent_memory_output_is_tristate = "false";
   Attribute tessent_memory_control_inputs_list = 
       "CEN SELECT ACTIVELOW OEN OUTPUTENABLE ACTIVELOW";
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
   ScanRegister GO_ID_REG[15:0] {
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

// Source: wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl2.dft_inserted_design/processor_core.icl
Module processor_core_rtl1_tessent_mbist_c1_interface_m2 {
   // ICL module read from source on or near line 305 of file '/home/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl1.dft_inserted_design/processor_core.icl'
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
   DataOutPort A[12:0] {
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_memory_bist_function = "address";
   }
   Attribute tessent_instrument_type = "mentor::memory_bist";
   Attribute tessent_instrument_subtype = "memory_interface";
   Attribute tessent_signature = "2f4cb0aaf1e6645a238087c6c8c0aa08";
   Attribute tessent_ignore_during_icl_verification = "on";
   Attribute keep_active_during_scan_test = "false";
   Attribute tessent_use_in_dft_specification = "false";
   Attribute tessent_bist_input_select_persistent_cell_output_list = 
       "tessent_persistent_cell_BIST_INPUT_SELECT_INT/Y";
   Attribute tessent_async_bypass_persistent_cell_input_list = "";
   Attribute tessent_bist_clk_persistent_cell_output_list = 
       "tessent_persistent_cell_GATING_BIST_CLK/GCK";
   Attribute tessent_memory_output_is_tristate = "false";
   Attribute tessent_memory_control_inputs_list = 
       "CEN SELECT ACTIVELOW OEN OUTPUTENABLE ACTIVELOW";
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
   ScanRegister GO_ID_REG[15:0] {
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

// Source: wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl2.dft_inserted_design/processor_core.icl
Module processor_core_rtl1_tessent_mbist_bap {
   // ICL module read from source on or near line 364 of file '/home/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl1.dft_inserted_design/processor_core.icl'
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

// Source: wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl2.dft_inserted_design/processor_core.icl
Module processor_core_rtl1_tessent_mbist_c1_controller {
   // ICL module read from source on or near line 622 of file '/home/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl1.dft_inserted_design/processor_core.icl'
   ClockPort BIST_CLK;
   DataInPort MBISTPG_EN;
   DataInPort LV_TM {
      Attribute connection_rule_option = "allowed_tied_low";
   }
   DataInPort MEM_BYPASS_EN {
      Attribute connection_rule_option = "allowed_tied_low";
   }
   DataInPort MCP_BOUNDING_EN {
      Attribute connection_rule_option = "allowed_tied_low";
   }
   DataInPort MBISTPG_ASYNC_RESETN {
      RefEnum AsyncResetN;
   }
   DataInPort MBISTPG_DIAG_EN;
   DataInPort BIST_SETUP2;
   DataInPort BIST_SETUP[1:0];
   DataInPort MBISTPG_REDUCED_ADDR_CNT_EN {
      RefEnum OnOff;
   }
   DataInPort MBISTPG_MEM_RST {
      RefEnum OnOff;
   }
   DataInPort MBISTPG_TESTDATA_SELECT {
      RefEnum OnOff;
   }
   DataInPort FL_CNT_MODE[1:0];
   DataInPort MBISTPG_ALGO_MODE[1:0];
   DataOutPort MBISTPG_GO {
      RefEnum PassFail;
   }
   DataOutPort MBISTPG_DONE {
      RefEnum PassFail;
   }
   TCKPort TCK;
   ScanInPort BIST_SI;
   ScanOutPort MBISTPG_SO {
      Source BIST_SO_INT;
   }
   ShiftEnPort BIST_HOLD;
   ToShiftEnPort BIST_SHIFT_COLLAR {
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataOutPort BIST_COLLAR_EN0 {
      Attribute tessent_memory_alias = "m1";
   }
   ScanOutPort MEM0_BIST_COLLAR_SI {
      Source MEM0_BIST_COLLAR_SI_INT;
   }
   ScanInPort MEM0_BIST_COLLAR_SO;
   DataOutPort BIST_COLLAR_EN1 {
      Attribute tessent_memory_alias = "m2";
   }
   ScanOutPort MEM1_BIST_COLLAR_SI {
      Source MEM1_BIST_COLLAR_SI_INT;
   }
   ScanInPort MEM1_BIST_COLLAR_SO;
   ScanInterface Client {
      Port BIST_SI;
      Port MBISTPG_SO;
      Port BIST_HOLD;
   }
   ScanInterface MEM0_INTERFACE {
      Port BIST_SHIFT_COLLAR;
      Port MEM0_BIST_COLLAR_SI;
      Port MEM0_BIST_COLLAR_SO;
   }
   ScanInterface MEM1_INTERFACE {
      Port BIST_SHIFT_COLLAR;
      Port MEM1_BIST_COLLAR_SI;
      Port MEM1_BIST_COLLAR_SO;
   }
   Attribute tessent_instrument_container = "processor_core_rtl1_mbist";
   Attribute tessent_instrument_type = "mentor::memory_bist";
   Attribute tessent_instrument_subtype = "controller";
   Attribute tessent_signature = "8b013cc22799fc2c914ff92154b6556b";
   Attribute tessent_ignore_during_icl_verification = "on";
   Attribute keep_active_during_scan_test = "false";
   Attribute tessent_use_in_dft_specification = "false";
   Attribute tessent_bist_clk_persistent_cell_output_list = 
       "tessent_persistent_cell_GATING_BIST_CLK/GCK";
   Alias SETUP_MODE[2:0] = BIST_SETUP2, BIST_SETUP[1:0] {
      RefEnum SetupModes;
   }
   Alias RUN_MODE[2:0] = BIST_SETUP2, BIST_SETUP[1:0] {
      RefEnum RunModes;
   }
   Alias INST_POINTER_REG[5:0] = INST_POINTER_REG_HW[5:0] {
   }
   Alias A_ADD_REG_Y[3:0] = A_ADD_REG_Y_HW[3:0] {
   }
   Alias A_ADD_REG_X[8:0] = A_ADD_REG_X_HW[8:0] {
   }
   Alias B_ADD_REG_Y[3:0] = B_ADD_REG_Y_HW[3:0] {
   }
   Alias B_ADD_REG_X[8:0] = B_ADD_REG_X_HW[8:0] {
   }
   Alias JCNT[2:0] = JCNT_HW[2:0] {
   }
   Alias WDATA_REG[1:0] = WDATA_REG_HW[1:0] {
   }
   Alias EDATA_REG[1:0] = EDATA_REG_HW[1:0] {
   }
   Alias REPEATLOOP_A_CNTR_REG[1:0] = REPEATLOOP_A_CNTR_REG_HW[1:0] {
   }
   Alias REPEATLOOP_B_CNTR_REG[1:0] = REPEATLOOP_B_CNTR_REG_HW[1:0] {
   }
   Alias MEM1_BIST_COLLAR_SI_INT = REPEATLOOP_B_CNTR_REG_HW[1] {
   }
   Alias MEM0_BIST_COLLAR_SI_INT = MEM1_BIST_COLLAR_SO {
   }
   Alias STOP_ERROR_CNT_REG[11:0] = STOP_ERROR_CNT_REG_HW[11:0] {
   }
   Alias BIST_SO_INT = BIST_SO_Pipeline {
   }
   Enum PassFail {
      Pass = 1'b1;
      Fail = 1'b0;
      Ignore = 1'bx;
   }
   Enum AsyncResetN {
      On = 1'b0;
      Off = 1'b1;
   }
   Enum SetupModes {
      Short = 3'b000;
      Long = 3'b001;
   }
   Enum OnOff {
      On = 1'b1;
      Off = 1'b0;
   }
   Enum RunModes {
      HWDefault = 3'b010;
      RunTimeProg = 3'b011;
      Idle = 3'bx0x;
      Off = 3'bxxx;
   }
   ScanRegister BIST_SI_Pipeline {
      ScanInSource BIST_SI;
      ResetValue 1'b0;
   }
   ScanRegister DIAG_EN_REG[0:0] {
      ScanInSource BIST_SI_Pipeline;
   }
   ScanRegister CMP_EN_MASK_EN[0:0] {
      ScanInSource DIAG_EN_REG[0];
   }
   ScanRegister CMP_EN_PARITY[0:0] {
      ScanInSource CMP_EN_MASK_EN[0];
      CaptureSource 1'bx;
   }
   ScanRegister MEM_SELECT_REG0[0:0] {
      ScanInSource CMP_EN_PARITY[0];
      ResetValue 1'd1;
   }
   ScanRegister ALGO_SEL_CNT_REG[0:0] {
      ScanInSource MEM_SELECT_REG0[0];
      RefEnum OnOff;
   }
   ScanRegister SELECT_COMMON_OPSET_REG[0:0] {
      ScanInSource ALGO_SEL_CNT_REG[0];
      RefEnum OnOff;
   }
   ScanRegister SELECT_COMMON_DATA_PAT_REG[0:0] {
      ScanInSource SELECT_COMMON_OPSET_REG[0];
      RefEnum OnOff;
   }
   ScanRegister MICROCODE_EN_REG[0:0] {
      ScanInSource SELECT_COMMON_DATA_PAT_REG[0];
   }
   ScanRegister INST_POINTER_REG_HW[0:5] {
      ScanInSource MICROCODE_EN_REG[0];
   }
   ScanRegister A_ADD_REG_Y_HW[0:3] {
      ScanInSource INST_POINTER_REG_HW[5];
   }
   ScanRegister A_ADD_REG_X_HW[0:8] {
      ScanInSource A_ADD_REG_Y_HW[3];
   }
   ScanRegister B_ADD_REG_Y_HW[0:3] {
      ScanInSource A_ADD_REG_X_HW[8];
   }
   ScanRegister B_ADD_REG_X_HW[0:8] {
      ScanInSource B_ADD_REG_Y_HW[3];
   }
   ScanRegister JCNT_HW[0:2] {
      ScanInSource B_ADD_REG_X_HW[8];
   }
   ScanRegister OPSET_SELECT_REG[0:0] {
      ScanInSource JCNT_HW[2];
   }
   ScanRegister WDATA_REG_HW[0:1] {
      ScanInSource OPSET_SELECT_REG[0];
   }
   ScanRegister EDATA_REG_HW[0:1] {
      ScanInSource WDATA_REG_HW[1];
   }
   ScanRegister X_ADDR_BIT_SEL_REG[0:0] {
      ScanInSource EDATA_REG_HW[1];
   }
   ScanRegister Y_ADDR_BIT_SEL_REG[0:0] {
      ScanInSource X_ADDR_BIT_SEL_REG[0];
   }
   ScanRegister REPEATLOOP_A_CNTR_REG_HW[0:1] {
      ScanInSource Y_ADDR_BIT_SEL_REG[0];
   }
   ScanRegister REPEATLOOP_B_CNTR_REG_HW[0:1] {
      ScanInSource REPEATLOOP_A_CNTR_REG_HW[1];
   }
   ScanRegister STOP_ON_ERROR_REG[0:0] {
      ScanInSource MEM0_BIST_COLLAR_SO;
   }
   ScanRegister FREEZE_STOP_ERROR_REG[0:0] {
      ScanInSource STOP_ON_ERROR_REG[0];
   }
   ScanRegister STOP_ERROR_CNT_REG_HW[0:11] {
      ScanInSource FREEZE_STOP_ERROR_REG[0];
   }
   ScanRegister STEP_COUNTER_FREEZE_REG[0:0] {
      ScanInSource STOP_ERROR_CNT_REG_HW[11];
      RefEnum OnOff;
   }
   ScanRegister STEP_COUNTER_REG[0:0] {
      ScanInSource STEP_COUNTER_FREEZE_REG[0];
      ResetValue 1'd0;
   }
   ScanRegister BIST_SO_Pipeline {
      ScanInSource BIST_SO_OUT;
      ResetValue 1'b0;
   }
   ScanMux CONTROLLER_SETUP_CHAIN SelectedBy MBISTPG_EN, SETUP_MODE[2:0] {
      1'b1, 3'b00x : STEP_COUNTER_REG[0];
   }
   ScanMux BIST_SO_OUT SelectedBy LV_TM, MBISTPG_ASYNC_RESETN {
      2'b01 : CONTROLLER_SETUP_CHAIN;
   }
   LogicSignal LONG_SETUP {
      MBISTPG_EN, SETUP_MODE[2:0] == 1'b1, 3'b001;
   }
   LogicSignal SHORT_SETUP {
      MBISTPG_EN, SETUP_MODE[2:0] == 1'b1, 3'b000;
   }
}

// Source: wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl2.dft_inserted_design/processor_core.icl
Module processor_core_rtl1_tessent_sib_2 {
   // ICL module read from source on or near line 870 of file '/home/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl1.dft_inserted_design/processor_core.icl'
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
      Source sib;
   }
   ToSelectPort ijtag_to_sel {
      Attribute connection_rule_option = "allowed_no_destination";
   }
   ScanInPort ijtag_from_so {
      Attribute connection_rule_option = "allowed_no_source";
   }
   ScanInterface client {
      Port ijtag_si;
      Port ijtag_so;
      Port ijtag_sel;
      Port ijtag_tck;
      Port ijtag_reset;
      Port ijtag_se;
      Port ijtag_ce;
      Port ijtag_ue;
   }
   ScanInterface host {
      Port ijtag_from_so;
      Port ijtag_to_sel;
   }
   Attribute keep_active_during_scan_test = "false";
   Attribute tessent_use_in_dft_specification = "false";
   Attribute tessent_instrument_type = "mentor::ijtag_node";
   Attribute tessent_signature = "e7392ab1b889107c6d7b0577567e3350";
   Attribute tessent_instrument_container = "processor_core_rtl1_ijtag";
   Attribute tessent_instrument_subtype = "sib";
   ScanRegister sib {
      ScanInSource scan_in_mux;
      CaptureSource sib;
      ResetValue 1'b0;
   }
   ScanMux scan_in_mux SelectedBy sib {
      1'b0 : ijtag_si;
      1'b1 : ijtag_from_so;
   }
}

// Source: wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl2.dft_inserted_design/processor_core.icl
Module processor_core_rtl1_tessent_sib_1 {
   // ICL module read from source on or near line 922 of file '/home/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl1.dft_inserted_design/processor_core.icl'
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
      Source sib;
   }
   ToSelectPort ijtag_to_sel {
      Attribute connection_rule_option = "allowed_no_destination";
   }
   ScanInPort ijtag_from_so {
      Attribute connection_rule_option = "allowed_no_source";
   }
   ToResetPort ijtag_to_reset {
      ActivePolarity 0;
      Source ijtag_reset;
   }
   ScanOutPort ijtag_to_si {
      Source ijtag_si;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   ToCaptureEnPort ijtag_to_ce {
      Source ijtag_ce;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   ToShiftEnPort ijtag_to_se {
      Source ijtag_se;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   ToUpdateEnPort ijtag_to_ue {
      Source ijtag_ue;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   ToTCKPort ijtag_to_tck {
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataInPort ltest_en {
      Attribute explicit_iwrite_only = 1'b1;
      Attribute connection_rule_option = "allowed_tied_low";
   }
   DataInPort ltest_occ_en {
      Attribute explicit_iwrite_only = 1'b1;
      Attribute connection_rule_option = "allowed_tied_low";
   }
   DataInPort ltest_static_clock_control_mode {
      Attribute connection_rule_option = "allowed_tied_low";
   }
   DataInPort ltest_clock_sequence[1:0] {
      Attribute connection_rule_option = "allowed_no_source";
      Attribute tessent_use_in_dft_specification = "false";
      Attribute function_modifier = "tessent_clock_sequence";
   }
   DataOutPort ltest_to_en {
      Source ltest_en;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataInPort ltest_mem_bypass_en {
      Attribute connection_rule_option = "allowed_tied";
   }
   DataOutPort ltest_to_mem_bypass_en {
      Source ltest_to_mem_bypass_en_int;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   DataInPort ltest_mcp_bounding_en {
      Attribute explicit_iwrite_only = 1'b1;
      Attribute connection_rule_option = "allowed_tied";
      Attribute forced_low_dft_signal_list = "ltest_en";
   }
   DataOutPort ltest_to_mcp_bounding_en {
      Source ltest_to_mcp_bounding_en_int;
      Attribute connection_rule_option = "allowed_no_destination";
   }
   ScanInterface client {
      Port ijtag_si;
      Port ijtag_so;
      Port ijtag_sel;
      Port ijtag_tck;
      Port ijtag_reset;
      Port ijtag_se;
      Port ijtag_ce;
      Port ijtag_ue;
   }
   ScanInterface host {
      Port ijtag_from_so;
      Port ijtag_to_tck;
      Port ijtag_to_reset;
      Port ijtag_to_se;
      Port ijtag_to_ce;
      Port ijtag_to_ue;
      Port ijtag_to_sel;
      Port ijtag_to_si;
   }
   Attribute keep_active_during_scan_test = "true";
   Attribute tessent_dft_function = "scan_tested_instrument_host";
   Attribute tessent_use_in_dft_specification = "false";
   Attribute tessent_instrument_type = "mentor::ijtag_node";
   Attribute tessent_signature = "7ae0bd514ccf669a1bbc1f88cba987ee";
   Attribute tessent_instrument_container = "processor_core_rtl1_ijtag";
   Attribute tessent_instrument_subtype = "sib";
   ScanRegister sib {
      ScanInSource scan_in_mux;
      CaptureSource sib;
      ResetValue 1'b0;
   }
   ScanMux scan_in_mux SelectedBy sib, ltest_en {
      2'b10 : ijtag_from_so;
      2'b11 : 1'bx;
      2'bxx : ijtag_si;
   }
   LogicSignal ltest_to_mem_bypass_en_int {
      ltest_mem_bypass_en & ltest_en;
   }
   LogicSignal ltest_to_mcp_bounding_en_int {
      ltest_mcp_bounding_en & ltest_en;
   }
}

// Source: wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl2.dft_inserted_design/processor_core.icl
Module processor_core_rtl2_tessent_edt_c1_ext {
   // ICL module read from source on or near line 17 of file '/data/workspaces/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/processor_core/tsdb_outdir/instruments/processor_core_rtl2_edt.instrument/processor_core_rtl2_tessent_edt_c1_ext.icl'
   DataInPort edt_bypass {
      RefEnum OnOffTable;
      Attribute tessent_no_input_constraints = "on";
   }
   Attribute keep_active_during_scan_test = "true";
   Attribute tessent_instrument_type = "mentor::edt";
   Attribute tessent_signature = "4ecf32ab148a689237e64865ff3373a9";
   Enum OnOffTable {
      off = 1'b0;
      on = 1'b1;
   }
}

// Source: wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl2.dft_inserted_design/processor_core.icl
Module processor_core_rtl2_tessent_edt_c1_ext_tdr {
   // ICL module read from source on or near line 17 of file '/data/workspaces/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/processor_core/tsdb_outdir/instruments/processor_core_rtl2_edt.instrument/processor_core_rtl2_tessent_edt_c1_ext_tdr.icl'
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
   Attribute tessent_signature = "dd88f5439480895f026ce7ad98a2d1d9";
   Attribute tessent_instrument_container = "processor_core_rtl2_edt";
   ScanRegister tdr[0:0] {
      ScanInSource ijtag_si;
      CaptureSource tdr[0:0];
      DefaultLoadValue 1'b0;
      ResetValue 1'b0;
   }
}

// Source: wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl2.dft_inserted_design/processor_core.icl
Module processor_core_rtl2_tessent_edt_c1_int {
   // ICL module read from source on or near line 17 of file '/data/workspaces/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/processor_core/tsdb_outdir/instruments/processor_core_rtl2_edt.instrument/processor_core_rtl2_tessent_edt_c1_int.icl'
   DataInPort edt_bypass {
      RefEnum OnOffTable;
      Attribute tessent_no_input_constraints = "on";
   }
   Attribute keep_active_during_scan_test = "true";
   Attribute tessent_instrument_type = "mentor::edt";
   Attribute tessent_signature = "a37855f7b9bf328b6e80ceff04e3d7ff";
   Enum OnOffTable {
      off = 1'b0;
      on = 1'b1;
   }
}

// Source: wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl2.dft_inserted_design/processor_core.icl
Module processor_core_rtl2_tessent_edt_c1_int_tdr {
   // ICL module read from source on or near line 17 of file '/data/workspaces/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/processor_core/tsdb_outdir/instruments/processor_core_rtl2_edt.instrument/processor_core_rtl2_tessent_edt_c1_int_tdr.icl'
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
   Attribute tessent_signature = "9b2a4f8d8bb23babe0feeaa79b4422b4";
   Attribute tessent_instrument_container = "processor_core_rtl2_edt";
   ScanRegister tdr[0:0] {
      ScanInSource ijtag_si;
      CaptureSource tdr[0:0];
      DefaultLoadValue 1'b0;
      ResetValue 1'b0;
   }
}

// Source: wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl2.dft_inserted_design/processor_core.icl
Module processor_core_rtl2_tessent_occ {
   // ICL module read from source on or near line 17 of file '/data/workspaces/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/processor_core/tsdb_outdir/instruments/processor_core_rtl2_occ.instrument/processor_core_rtl2_tessent_occ.icl'
   ClockPort fast_clock;
   ToClockPort clock_out {
      Source tck_mux;
      Attribute exclude_from_sdc = "on";
   }
   TCKPort ijtag_tck;
   ScanInPort ijtag_si;
   ScanOutPort ijtag_so {
      Source tdr_sib.scan_out;
   }
   ResetPort ijtag_reset {
      ActivePolarity 0;
   }
   CaptureEnPort ijtag_ce;
   ShiftEnPort ijtag_se;
   UpdateEnPort ijtag_ue;
   SelectPort ijtag_sel;
   Attribute keep_active_during_scan_test = "true";
   Attribute tessent_use_in_dft_specification = "false";
   Attribute tessent_instrument_type = "mentor::occ";
   Attribute tessent_instrument_subtype = "standard";
   Attribute tessent_instrument_container = "processor_core_rtl2_occ.instrument";

   Attribute tessent_signature = "976685a3a026ac375789db9ca2aebfa5";
   Attribute tdr_parent_instance = "ijtag_control";
   Alias test_mode = tdr[0] {
   }
   Alias fast_capture_mode = tdr[1] {
   }
   Alias active_upstream_parent_occ = tdr[2] {
   }
   Alias capture_cycle_width[1:0] = tdr[4:3] {
   }
   Alias inject_tck = tdr[5] {
   }
   ScanRegister tdr[5:0] {
      ScanInSource ijtag_si;
      CaptureSource 6'b000000;
      ResetValue 6'b000000;
   }
   ClockMux tck_mux SelectedBy inject_tck {
      1'b1 : ijtag_tck;
      1'b0 : fast_clock;
   }
   Instance tdr_sib Of processor_core_rtl2_tessent_occ_sib_int {
      InputPort clock = ijtag_tck;
      InputPort reset = ijtag_reset;
      InputPort scan_in = ijtag_si;
      InputPort capture_en = ijtag_ce;
      InputPort shift_en = ijtag_se;
      InputPort update_en = ijtag_ue;
      InputPort enable = ijtag_sel;
      InputPort from_scan_out = tdr[0];
   }
}

// Source: wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl2.dft_inserted_design/processor_core.icl
Module processor_core_rtl2_tessent_occ_sib_int {
   // ICL module read from source on or near line 75 of file '/data/workspaces/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/processor_core/tsdb_outdir/instruments/processor_core_rtl2_occ.instrument/processor_core_rtl2_tessent_occ.icl'
   TCKPort clock;
   ResetPort reset {
      ActivePolarity 0;
   }
   ScanInPort scan_in;
   ScanOutPort scan_out {
      Source sib;
   }
   CaptureEnPort capture_en;
   ShiftEnPort shift_en;
   UpdateEnPort update_en;
   SelectPort enable;
   ToSelectPort to_scan_en {
      Source to_scan_en_and;
   }
   ScanInPort from_scan_out;
   ScanInterface client {
      Port scan_in;
      Port scan_out;
      Port enable;
   }
   ScanInterface host {
      Port from_scan_out;
      Port to_scan_en;
   }
   ScanRegister sib {
      ScanInSource scan_in_mux;
      CaptureSource 1'b0;
      ResetValue 1'b0;
   }
   ScanMux scan_in_mux SelectedBy sib {
      1'b0 : scan_in;
      1'b1 : from_scan_out;
   }
   LogicSignal to_scan_en_and {
      enable, sib == 2'b11;
   }
}

// Source: wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl2.dft_inserted_design/processor_core.icl
Module processor_core_rtl2_tessent_sib_2 {
   // ICL module read from source on or near line 17 of file '/data/workspaces/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/processor_core/tsdb_outdir/instruments/processor_core_rtl2_ijtag.instrument/processor_core_rtl2_tessent_sib_2.icl'
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
      Source sib;
   }
   ToSelectPort ijtag_to_sel {
      Attribute connection_rule_option = "allowed_no_destination";
   }
   ScanInPort ijtag_from_so {
      Attribute connection_rule_option = "allowed_no_source";
   }
   ScanInterface client {
      Port ijtag_si;
      Port ijtag_so;
      Port ijtag_sel;
      Port ijtag_tck;
      Port ijtag_reset;
      Port ijtag_se;
      Port ijtag_ce;
      Port ijtag_ue;
   }
   ScanInterface host {
      Port ijtag_from_so;
      Port ijtag_to_sel;
   }
   Attribute keep_active_during_scan_test = "true";
   Attribute tessent_use_in_dft_specification = "false";
   Attribute tessent_instrument_type = "mentor::ijtag_node";
   Attribute tessent_signature = "5f1dd8df2266c58662dbbac21304245f";
   Attribute tessent_instrument_container = "processor_core_rtl2_ijtag";
   Attribute tessent_instrument_subtype = "sib";
   ScanRegister sib {
      ScanInSource scan_in_mux;
      CaptureSource sib;
      ResetValue 1'b0;
   }
   ScanMux scan_in_mux SelectedBy sib {
      1'b0 : ijtag_si;
      1'b1 : ijtag_from_so;
   }
}

// Source: wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl2.dft_inserted_design/processor_core.icl
Module processor_core_rtl2_tessent_sib_1 {
   // ICL module read from source on or near line 17 of file '/data/workspaces/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/processor_core/tsdb_outdir/instruments/processor_core_rtl2_ijtag.instrument/processor_core_rtl2_tessent_sib_1.icl'
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
      Source sib;
   }
   ToSelectPort ijtag_to_sel {
      Attribute connection_rule_option = "allowed_no_destination";
   }
   ScanInPort ijtag_from_so {
      Attribute connection_rule_option = "allowed_no_source";
   }
   ScanInterface client {
      Port ijtag_si;
      Port ijtag_so;
      Port ijtag_sel;
      Port ijtag_tck;
      Port ijtag_reset;
      Port ijtag_se;
      Port ijtag_ce;
      Port ijtag_ue;
   }
   ScanInterface host {
      Port ijtag_from_so;
      Port ijtag_to_sel;
   }
   Attribute keep_active_during_scan_test = "true";
   Attribute tessent_dft_function = "scan_resource_instrument_host";
   Attribute tessent_use_in_dft_specification = "false";
   Attribute tessent_instrument_type = "mentor::ijtag_node";
   Attribute tessent_signature = "2ece8b7c008f609989adb8b0986fee1b";
   Attribute tessent_instrument_container = "processor_core_rtl2_ijtag";
   Attribute tessent_instrument_subtype = "sib";
   ScanRegister sib {
      ScanInSource scan_in_mux;
      CaptureSource sib;
      ResetValue 1'b0;
   }
   ScanMux scan_in_mux SelectedBy sib {
      1'b0 : ijtag_si;
      1'b1 : ijtag_from_so;
   }
}

// Source: wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl2.dft_inserted_design/processor_core.icl
Module processor_core_rtl2_tessent_ssn_pipe_w2_1 {
   // ICL module read from source on or near line 17 of file '/data/workspaces/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/processor_core/tsdb_outdir/instruments/processor_core_rtl2_ssn.instrument/processor_core_rtl2_tessent_ssn_pipe_w2_1.icl'
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

// Source: wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl2.dft_inserted_design/processor_core.icl
Module processor_core_rtl2_tessent_ssn_scan_host_1 {
   // ICL module read from source on or near line 17 of file '/data/workspaces/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/processor_core/tsdb_outdir/instruments/processor_core_rtl2_ssn.instrument/processor_core_rtl2_tessent_ssn_scan_host_1.icl'
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

// Source: wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl2.dft_inserted_design/processor_core.icl
Module processor_core_rtl2_tessent_ssn_scan_host_1_inner {
   // ICL module read from source on or near line 86 of file '/data/workspaces/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/processor_core/tsdb_outdir/instruments/processor_core_rtl2_ssn.instrument/processor_core_rtl2_tessent_ssn_scan_host_1.icl'
   ResetPort ijtag_reset {
      ActivePolarity 0;
   }
   TCKPort ijtag_clock;
   SelectPort ijtag_sel;
   CaptureEnPort ijtag_ce;
   ShiftEnPort ijtag_se;
   UpdateEnPort ijtag_ue;
   ScanInPort ijtag_si;
   ScanOutPort ijtag_so {
      Source ijtag_so_mux;
   }
   DataOutPort enable {
      Source enable_shift_reg;
   }
   DataOutPort initial_offset[30:0] {
      Source initial_offset_shift_reg;
   }
   DataOutPort relative_initial_offset_enable {
      Source relative_initial_offset_enable_shift_reg;
   }
   DataOutPort bus_width[1:0] {
      Source bus_width_shift_reg;
   }
   DataOutPort bits_per_packet[1:0] {
      Source bits_per_packet_shift_reg;
   }
   DataOutPort to_scan_in_bits[1:0] {
      Source to_scan_in_bits_shift_reg;
   }
   DataOutPort from_scan_out_bits[1:0] {
      Source from_scan_out_bits_shift_reg;
   }
   DataOutPort from_scan_out_le_strobe_en {
      Source from_scan_out_le_strobe_en_shift_reg;
   }
   DataOutPort total_shift_cnt_minus_one[27:0] {
      Source total_shift_cnt_minus_one_shift_reg;
   }
   DataOutPort extra_shift_packets[27:0] {
      Source extra_shift_packets_shift_reg;
   }
   DataOutPort packet_size[15:0] {
      Source packet_size_shift_reg;
   }
   DataOutPort timeslot_sharing_count_minus_one[2:0] {
      Source timeslot_sharing_count_minus_one_shift_reg;
   }
   DataOutPort timeslot_sharing_offset[2:0] {
      Source timeslot_sharing_offset_shift_reg;
   }
   DataOutPort force_suppress_capture {
      Source force_suppress_capture_shift_reg;
   }
   DataOutPort initial_bit0_position[1:0] {
      Source initial_bit0_position_shift_reg;
   }
   DataOutPort loop_back_en {
      Source loop_back_en_shift_reg;
   }
   DataOutPort capture_packets[2:0] {
      Source capture_packets_shift_reg;
   }
   DataOutPort init_scan_load_used {
      Source init_scan_load_used_shift_reg;
   }
   DataOutPort disable_first_scan_load_masking {
      Source disable_first_scan_load_masking_shift_reg;
   }
   DataOutPort packets_per_capture_clock_minus_one[3:0] {
      Source packets_per_capture_clock_minus_one_shift_reg;
   }
   DataOutPort scan_en_transition_packets[6:0] {
      Source scan_en_transition_packets_shift_reg;
   }
   DataOutPort scan_en_launch_packet[6:0] {
      Source scan_en_launch_packet_shift_reg;
   }
   DataOutPort edt_update_falling_transition_words[6:0] {
      Source edt_update_falling_transition_words_shift_reg;
   }
   DataOutPort edt_update_falling_launch_word[6:0] {
      Source edt_update_falling_launch_word_shift_reg;
   }
   DataOutPort min_shift_clock_low_width[5:0] {
      Source min_shift_clock_low_width_shift_reg;
   }
   DataOutPort min_capture_clock_low_width[5:0] {
      Source min_capture_clock_low_width_shift_reg;
   }
   DataOutPort packet_width_is_less_than_two_bus_clock {
      Source packet_width_is_less_than_two_bus_clock_shift_reg;
   }
   DataOutPort infinite_shift_mode {
      Source infinite_shift_mode_signal;
   }
   DataOutPort streaming_through_ijtag_en {
      Source streaming_through_ijtag_en_shift_reg;
   }
   Attribute tessent_instrument_type = "mentor::ssn";
   Attribute tessent_instrument_subtype = "scan_host_ijtag_reg";
   Attribute tessent_signature = "1078d6a4fa6840b3637ca2e8a9c4a2cf";
   ScanRegister config_sib_shift_reg {
      ScanInSource config_sib_si_mux;
      ResetValue 1'b0;
      Attribute tessent_ssn_function = "config_inclusion_bit";
      Attribute tessent_rtl_reg = "config_sib";
   }
   ScanRegister enable_shift_reg {
      ScanInSource streaming_through_ijtag_en_si_mux;
      CaptureSource enable_shift_reg;
      ResetValue 1'b0;
      Attribute tessent_ssn_function = "ssh_enable";
      Attribute tessent_rtl_reg = "enable_update_stage";
   }
   ScanRegister fake_data_bus_reg {
      ScanInSource ijtag_si;
      Attribute tessent_ignore_during_icl_verification = "on";
   }
   ScanRegister streaming_through_ijtag_en_shift_reg {
      ScanInSource enable_shift_reg;
      ResetValue 1'b0;
      Attribute tessent_ssn_function = "streaming_through_ijtag_enable";
      Attribute tessent_rtl_reg = "streaming_through_ijtag_en_update_stage";
      Attribute explicit_iwrite_only = 1'b1;
   }
   ScanRegister initial_offset_shift_reg[30:0] {
      ScanInSource streaming_through_ijtag_en_shift_reg;
      CaptureSource initial_offset_shift_reg[30:0];
      Attribute tessent_rtl_reg = "initial_offset_shift_reg[30:0]";
   }
   ScanRegister relative_initial_offset_enable_shift_reg {
      ScanInSource initial_offset_shift_reg[0];
      CaptureSource relative_initial_offset_enable_shift_reg;
      Attribute tessent_rtl_reg = "relative_initial_offset_enable_shift_reg";
   }
   ScanRegister bus_width_shift_reg[1:0] {
      ScanInSource relative_initial_offset_enable_shift_reg;
      CaptureSource bus_width_shift_reg[1:0];
      Attribute tessent_rtl_reg = "bus_width_shift_reg[1:0]";
   }
   ScanRegister bits_per_packet_shift_reg[1:0] {
      ScanInSource bus_width_shift_reg[0];
      CaptureSource bits_per_packet_shift_reg[1:0];
      Attribute tessent_rtl_reg = "bits_per_packet_shift_reg[1:0]";
   }
   ScanRegister to_scan_in_bits_shift_reg[1:0] {
      ScanInSource bits_per_packet_shift_reg[0];
      CaptureSource to_scan_in_bits_shift_reg[1:0];
      Attribute tessent_rtl_reg = "to_scan_in_bits_shift_reg[1:0]";
   }
   ScanRegister from_scan_out_bits_shift_reg[1:0] {
      ScanInSource to_scan_in_bits_shift_reg[0];
      CaptureSource from_scan_out_bits_shift_reg[1:0];
      Attribute tessent_rtl_reg = "from_scan_out_bits_shift_reg[1:0]";
   }
   ScanRegister from_scan_out_le_strobe_en_shift_reg {
      ScanInSource from_scan_out_bits_shift_reg[0];
      CaptureSource from_scan_out_le_strobe_en_shift_reg;
      Attribute tessent_rtl_reg = "from_scan_out_le_strobe_en_shift_reg";
   }
   ScanRegister total_shift_cnt_minus_one_shift_reg[27:0] {
      ScanInSource from_scan_out_le_strobe_en_shift_reg;
      CaptureSource total_shift_cnt_minus_one_shift_reg[27:0];
      Attribute tessent_rtl_reg = "total_shift_cnt_minus_one_shift_reg[27:0]";
   }
   ScanRegister extra_shift_packets_shift_reg[27:0] {
      ScanInSource total_shift_cnt_minus_one_shift_reg[0];
      CaptureSource extra_shift_packets_shift_reg[27:0];
      Attribute tessent_rtl_reg = "extra_shift_packets_shift_reg[27:0]";
   }
   ScanRegister packet_size_shift_reg[15:0] {
      ScanInSource extra_shift_packets_shift_reg[0];
      CaptureSource packet_size_shift_reg[15:0];
      Attribute tessent_rtl_reg = "packet_size_shift_reg[15:0]";
   }
   ScanRegister timeslot_sharing_count_minus_one_shift_reg[2:0] {
      ScanInSource packet_size_shift_reg[0];
      CaptureSource timeslot_sharing_count_minus_one_shift_reg[2:0];
      Attribute tessent_rtl_reg = 
          "timeslot_sharing_count_minus_one_shift_reg[2:0]";
   }
   ScanRegister timeslot_sharing_offset_shift_reg[2:0] {
      ScanInSource timeslot_sharing_count_minus_one_shift_reg[0];
      CaptureSource timeslot_sharing_offset_shift_reg[2:0];
      Attribute tessent_rtl_reg = "timeslot_sharing_offset_shift_reg[2:0]";
   }
   ScanRegister force_suppress_capture_shift_reg {
      ScanInSource timeslot_sharing_offset_shift_reg[0];
      CaptureSource force_suppress_capture_shift_reg;
      Attribute tessent_rtl_reg = "force_suppress_capture_shift_reg";
   }
   ScanRegister initial_bit0_position_shift_reg[1:0] {
      ScanInSource force_suppress_capture_shift_reg;
      CaptureSource initial_bit0_position_shift_reg[1:0];
      Attribute tessent_rtl_reg = "initial_bit0_position_shift_reg[1:0]";
   }
   ScanRegister loop_back_en_shift_reg {
      ScanInSource initial_bit0_position_shift_reg[0];
      CaptureSource loop_back_en_shift_reg;
      Attribute tessent_rtl_reg = "loop_back_en_shift_reg";
   }
   ScanRegister capture_packets_shift_reg[2:0] {
      ScanInSource loop_back_en_shift_reg;
      CaptureSource capture_packets_shift_reg[2:0];
      Attribute tessent_rtl_reg = "capture_packets_shift_reg[2:0]";
   }
   ScanRegister init_scan_load_used_shift_reg {
      ScanInSource capture_packets_shift_reg[0];
      CaptureSource init_scan_load_used_shift_reg;
      Attribute tessent_rtl_reg = "init_scan_load_used_shift_reg";
   }
   ScanRegister disable_first_scan_load_masking_shift_reg {
      ScanInSource init_scan_load_used_shift_reg;
      CaptureSource disable_first_scan_load_masking_shift_reg;
      Attribute tessent_rtl_reg = "disable_first_scan_load_masking_shift_reg";
   }
   ScanRegister packets_per_capture_clock_minus_one_shift_reg[3:0] {
      ScanInSource disable_first_scan_load_masking_shift_reg;
      CaptureSource packets_per_capture_clock_minus_one_shift_reg[3:0];
      Attribute tessent_rtl_reg = 
          "packets_per_capture_clock_minus_one_shift_reg[3:0]";
   }
   ScanRegister scan_en_transition_packets_shift_reg[6:0] {
      ScanInSource packets_per_capture_clock_minus_one_shift_reg[0];
      CaptureSource scan_en_transition_packets_shift_reg[6:0];
      Attribute tessent_rtl_reg = "scan_en_transition_packets_shift_reg[6:0]";
   }
   ScanRegister scan_en_launch_packet_shift_reg[6:0] {
      ScanInSource scan_en_transition_packets_shift_reg[0];
      CaptureSource scan_en_launch_packet_shift_reg[6:0];
      Attribute tessent_rtl_reg = "scan_en_launch_packet_shift_reg[6:0]";
   }
   ScanRegister edt_update_falling_transition_words_shift_reg[6:0] {
      ScanInSource scan_en_launch_packet_shift_reg[0];
      CaptureSource edt_update_falling_transition_words_shift_reg[6:0];
      Attribute tessent_rtl_reg = 
          "edt_update_falling_transition_words_shift_reg[6:0]";
   }
   ScanRegister edt_update_falling_launch_word_shift_reg[6:0] {
      ScanInSource edt_update_falling_transition_words_shift_reg[0];
      CaptureSource edt_update_falling_launch_word_shift_reg[6:0];
      Attribute tessent_rtl_reg = 
          "edt_update_falling_launch_word_shift_reg[6:0]";
   }
   ScanRegister min_shift_clock_low_width_shift_reg[5:0] {
      ScanInSource edt_update_falling_launch_word_shift_reg[0];
      CaptureSource min_shift_clock_low_width_shift_reg[5:0];
      Attribute tessent_rtl_reg = "min_shift_clock_low_width_shift_reg[5:0]";
   }
   ScanRegister min_capture_clock_low_width_shift_reg[5:0] {
      ScanInSource min_shift_clock_low_width_shift_reg[0];
      CaptureSource min_capture_clock_low_width_shift_reg[5:0];
      Attribute tessent_rtl_reg = "min_capture_clock_low_width_shift_reg[5:0]";
   }
   ScanRegister packet_width_is_less_than_two_bus_clock_shift_reg {
      ScanInSource min_capture_clock_low_width_shift_reg[0];
      CaptureSource packet_width_is_less_than_two_bus_clock_shift_reg;
      Attribute tessent_rtl_reg = 
          "packet_width_is_less_than_two_bus_clock_shift_reg";
   }
   ScanRegister bus_in_shift_reg[1:0] {
      ScanInSource ijtag_si;
   }
   ScanRegister bus_out_shift_reg[1:0] {
      ScanInSource bus_in_shift_reg[0];
   }
   ScanMux config_sib_si_mux SelectedBy config_sib_shift_reg, enable_shift_reg {
      2'b11 : bus_out_shift_reg[0];
      2'bxx : ijtag_si;
   }
   ScanMux streaming_through_ijtag_en_si_mux SelectedBy 
       streaming_through_ijtag_en_si_mux_select {
      1'b0 : config_sib_shift_reg;
      1'b1 : fake_data_bus_reg;
   }
   ScanMux ijtag_so_mux SelectedBy config_sib_shift_reg, enable_shift_reg {
      Attribute exclude_codes_from_icl_verify = "{2'b01}";
      2'b10 : packet_width_is_less_than_two_bus_clock_shift_reg;
      2'b00 : enable_shift_reg;
      2'b01 : enable_shift_reg;
      2'b11 : enable_shift_reg;
   }
   LogicSignal streaming_through_ijtag_en_si_mux_select {
      streaming_through_ijtag_en_shift_reg && enable_shift_reg;
   }
   LogicSignal infinite_shift_mode_signal {
      total_shift_cnt_minus_one_shift_reg == 28'b0;
   }
}

// Source: wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl2.dft_inserted_design/processor_core.icl
Module processor_core_rtl2_tessent_tdr_sri_ctrl {
   // ICL module read from source on or near line 17 of file '/data/workspaces/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/processor_core/tsdb_outdir/instruments/processor_core_rtl2_ijtag.instrument/processor_core_rtl2_tessent_tdr_sri_ctrl.icl'
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
   DataOutPort ext_edt_mode {
      Source tdr[9];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_dft_signal_name = "ext_edt_mode";
      Attribute tessent_dft_signal_usage = "scan_mode";
      Attribute tessent_dft_signal_scan_mode_type = "external";
      Attribute tessent_dft_signal_reset_value = 0;
      Attribute tessent_persistent_design_pin = 
          "{tessent_persistent_cell_ext_edt_mode/Y}";
   }
   DataOutPort int_edt_mode {
      Source tdr[8];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_dft_signal_name = "int_edt_mode";
      Attribute tessent_dft_signal_usage = "scan_mode";
      Attribute tessent_dft_signal_scan_mode_type = "internal";
      Attribute tessent_dft_signal_reset_value = 0;
      Attribute tessent_persistent_design_pin = 
          "{tessent_persistent_cell_int_edt_mode/Y}";
   }
   DataOutPort ext_mode {
      Source tdr[7];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_dft_signal_name = "ext_mode";
      Attribute tessent_dft_signal_usage = "scan_mode";
      Attribute tessent_dft_signal_scan_mode_type = "external";
      Attribute tessent_dft_signal_reset_value = 0;
      Attribute tessent_persistent_design_pin = 
          "{tessent_persistent_cell_ext_mode/Y}";
   }
   DataOutPort int_mode {
      Source tdr[6];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_dft_signal_name = "int_mode";
      Attribute tessent_dft_signal_usage = "scan_mode";
      Attribute tessent_dft_signal_scan_mode_type = "internal";
      Attribute tessent_dft_signal_reset_value = 0;
      Attribute tessent_persistent_design_pin = 
          "{tessent_persistent_cell_int_mode/Y}";
   }
   DataOutPort ext_ltest_en {
      Source tdr[5];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_dft_signal_name = "ext_ltest_en";
      Attribute tessent_dft_signal_usage = "logic_test_control";
      Attribute tessent_dft_signal_value_in_pre_scan_drc = "x";
      Attribute tessent_dft_signal_reset_value = 0;
      Attribute tessent_persistent_design_pin = 
          "{tessent_persistent_cell_ext_ltest_en/Y}";
   }
   DataOutPort int_ltest_en {
      Source tdr[4];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_dft_signal_name = "int_ltest_en";
      Attribute tessent_dft_signal_usage = "logic_test_control";
      Attribute tessent_dft_signal_value_in_pre_scan_drc = "x";
      Attribute tessent_dft_signal_reset_value = 0;
      Attribute tessent_persistent_design_pin = 
          "{tessent_persistent_cell_int_ltest_en/Y}";
   }
   DataOutPort async_set_reset_static_disable {
      Source tdr[3];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_dft_signal_name = "async_set_reset_static_disable";
      Attribute tessent_dft_signal_usage = "logic_test_control";
      Attribute tessent_dft_signal_value_in_pre_scan_drc = "0";
      Attribute tessent_dft_signal_reset_value = 0;
      Attribute tessent_persistent_design_pin = 
          "{tessent_persistent_cell_async_set_reset_static_disable/Y}";
   }
   DataOutPort tck_occ_en {
      Source tdr[2];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_dft_signal_name = "tck_occ_en";
      Attribute tessent_dft_signal_usage = "global_dft_control";
      Attribute tessent_dft_signal_default_value_in_all_test = 0;
      Attribute tessent_dft_signal_reset_value = 0;
      Attribute tessent_persistent_design_pin = 
          "{tessent_persistent_cell_tck_occ_en/Y}";
   }
   DataOutPort memory_bypass_en {
      Source tdr[1];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_dft_signal_name = "memory_bypass_en";
      Attribute tessent_dft_signal_usage = "logic_test_control";
      Attribute tessent_dft_signal_value_in_pre_scan_drc = "1";
      Attribute tessent_dft_signal_reset_value = 0;
      Attribute tessent_persistent_design_pin = 
          "{tessent_persistent_cell_memory_bypass_en/Y}";
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
   Attribute tessent_signature = "f10d092263b49ca09c7316b88bd768e4";
   Attribute tessent_instrument_container = "processor_core_rtl2_ijtag";
   ScanRegister tdr[9:0] {
      ScanInSource ijtag_si;
      CaptureSource 10'b0000000000;
      DefaultLoadValue 10'b0000000000;
      ResetValue 10'b0000000000;
   }
}

// Source: wrapped_cores/processor_core/tsdb_outdir/instruments/processor_core_rtl1_mbist.instrument/processor_core_rtl1_tessent_mbist_c1_controller_assembly.icl
Module processor_core_rtl1_tessent_mbist_c1_controller_assembly {
   // Created by ICL extraction
   DataOutPort BIST_DONE {
      Source processor_core_rtl1_tessent_mbist_c1_controller_inst.MBISTPG_DONE;
      Attribute tessent_use_in_dft_specification = "false";
   }
   DataOutPort BIST_GO {
      Source processor_core_rtl1_tessent_mbist_c1_controller_inst.MBISTPG_GO;
      Attribute tessent_use_in_dft_specification = "false";
   }
   DataInPort LV_TM {
      DefaultLoadValue 1'b0;
      Attribute tessent_timing = "scan_reconfiguration";
      Attribute connection_rule_option = "allowed_tied_low";
      Attribute tessent_use_in_dft_specification = "false";
   }
   CaptureEnPort capture_en;
   ClockPort clk_dco_clk {
      Attribute tessent_clock_domain_labels = "clk_dco_clk clk_dco_clk";
      Attribute tessent_clock_periods = "all 3.00ns";
   }
   SelectPort ijtag_select;
   ResetPort reset {
      ActivePolarity 0;
   }
   ShiftEnPort shift_en;
   ScanInPort si;
   ScanOutPort so {
      Source processor_core_rtl1_tessent_mbist_bap_inst.so;
   }
   TCKPort tck;
   UpdateEnPort update_en;
   ScanInterface C0 {
      Port si;
      Port so;
      Port ijtag_select;
      Port shift_en;
      Port capture_en;
      Port update_en;
      Port reset;
      Port tck;
   }
   Attribute tessent_design_format = "verilog_2001";
   Attribute test_setup_procfile = "";
   Attribute forced_low_input_port_list = 
       "{MCP_BOUNDING_EN} {MEM_BYPASS_EN} {m1_inst_OEN} {m2_inst_OEN}";
   Attribute icl_extraction_date = "Thu Aug 27 11:14:44 CST 2026";
   Attribute created_by_tessent_icl_extract = "true";
   Attribute tessent_design_id = "rtl";
   Instance m1_inst Of SYNC_1RW_8Kx16 {
      InputPort A[12] = m1_interface_instance.A[12];
      InputPort A[11] = m1_interface_instance.A[11];
      InputPort A[10] = m1_interface_instance.A[10];
      InputPort A[9] = m1_interface_instance.A[9];
      InputPort A[8] = m1_interface_instance.A[8];
      InputPort A[7] = m1_interface_instance.A[7];
      InputPort A[6] = m1_interface_instance.A[6];
      InputPort A[5] = m1_interface_instance.A[5];
      InputPort A[4] = m1_interface_instance.A[4];
      InputPort A[3] = m1_interface_instance.A[3];
      InputPort A[2] = m1_interface_instance.A[2];
      InputPort A[1] = m1_interface_instance.A[1];
      InputPort A[0] = m1_interface_instance.A[0];
      InputPort CLK = clk_dco_clk;
      Attribute tessent_design_instance = "m1_inst";
   }
   Instance m1_interface_instance Of 
       processor_core_rtl1_tessent_mbist_c1_interface_m1 {
      InputPort BIST_CLK = clk_dco_clk;
      InputPort BIST_COLLAR_EN = 
          processor_core_rtl1_tessent_mbist_c1_controller_inst.BIST_COLLAR_EN0;
      InputPort BIST_ASYNC_RESETN = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_ASYNC_RESET;
      InputPort BIST_SI = 
          processor_core_rtl1_tessent_mbist_c1_controller_inst.MEM0_BIST_COLLAR_SI;

      InputPort BIST_SHIFT_COLLAR = 
          processor_core_rtl1_tessent_mbist_c1_controller_inst.BIST_SHIFT_COLLAR;

      InputPort BIST_SETUP0 = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_SETUP[0];
      InputPort MEM_BYPASS_EN = 
          processor_core_rtl1_tessent_mbist_bap_inst.memory_bypass_to_en;
      InputPort MCP_BOUNDING_EN = 
          processor_core_rtl1_tessent_mbist_bap_inst.mcp_bounding_to_en;
      InputPort INCLUDE_MEM_RESULTS_REG = 
          processor_core_rtl1_tessent_mbist_bap_inst.INCLUDE_MEM_RESULTS_REG;
      Attribute tessent_design_instance = "m1_interface_instance";
   }
   Instance m2_inst Of SYNC_1RW_8Kx16 {
      InputPort A[12] = m2_interface_instance.A[12];
      InputPort A[11] = m2_interface_instance.A[11];
      InputPort A[10] = m2_interface_instance.A[10];
      InputPort A[9] = m2_interface_instance.A[9];
      InputPort A[8] = m2_interface_instance.A[8];
      InputPort A[7] = m2_interface_instance.A[7];
      InputPort A[6] = m2_interface_instance.A[6];
      InputPort A[5] = m2_interface_instance.A[5];
      InputPort A[4] = m2_interface_instance.A[4];
      InputPort A[3] = m2_interface_instance.A[3];
      InputPort A[2] = m2_interface_instance.A[2];
      InputPort A[1] = m2_interface_instance.A[1];
      InputPort A[0] = m2_interface_instance.A[0];
      InputPort CLK = clk_dco_clk;
      Attribute tessent_design_instance = "m2_inst";
   }
   Instance m2_interface_instance Of 
       processor_core_rtl1_tessent_mbist_c1_interface_m2 {
      InputPort BIST_CLK = clk_dco_clk;
      InputPort BIST_COLLAR_EN = 
          processor_core_rtl1_tessent_mbist_c1_controller_inst.BIST_COLLAR_EN1;
      InputPort BIST_ASYNC_RESETN = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_ASYNC_RESET;
      InputPort BIST_SI = 
          processor_core_rtl1_tessent_mbist_c1_controller_inst.MEM1_BIST_COLLAR_SI;

      InputPort BIST_SHIFT_COLLAR = 
          processor_core_rtl1_tessent_mbist_c1_controller_inst.BIST_SHIFT_COLLAR;

      InputPort BIST_SETUP0 = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_SETUP[0];
      InputPort MEM_BYPASS_EN = 
          processor_core_rtl1_tessent_mbist_bap_inst.memory_bypass_to_en;
      InputPort MCP_BOUNDING_EN = 
          processor_core_rtl1_tessent_mbist_bap_inst.mcp_bounding_to_en;
      InputPort INCLUDE_MEM_RESULTS_REG = 
          processor_core_rtl1_tessent_mbist_bap_inst.INCLUDE_MEM_RESULTS_REG;
      Attribute tessent_design_instance = "m2_interface_instance";
   }
   Instance processor_core_rtl1_tessent_mbist_bap_inst Of 
       processor_core_rtl1_tessent_mbist_bap {
      InputPort reset = reset;
      InputPort ijtag_select = ijtag_select;
      InputPort si = si;
      InputPort capture_en = capture_en;
      InputPort shift_en = shift_en;
      InputPort update_en = update_en;
      InputPort tck = tck;
      InputPort memory_bypass_en = 'b0;
      InputPort mcp_bounding_en = 'b0;
      InputPort ltest_en = LV_TM;
      InputPort fromBist[0] = 
          processor_core_rtl1_tessent_mbist_c1_controller_inst.MBISTPG_SO;
      InputPort MBISTPG_GO[0] = 
          processor_core_rtl1_tessent_mbist_c1_controller_inst.MBISTPG_GO;
      InputPort MBISTPG_DONE[0] = 
          processor_core_rtl1_tessent_mbist_c1_controller_inst.MBISTPG_DONE;
      Attribute tessent_design_instance = 
          "processor_core_rtl1_tessent_mbist_bap_inst";
   }
   Instance processor_core_rtl1_tessent_mbist_c1_controller_inst Of 
       processor_core_rtl1_tessent_mbist_c1_controller {
      InputPort BIST_CLK = clk_dco_clk;
      InputPort MBISTPG_EN = 
          processor_core_rtl1_tessent_mbist_bap_inst.bistEn[0];
      InputPort LV_TM = processor_core_rtl1_tessent_mbist_bap_inst.ltest_to_en;
      InputPort MEM_BYPASS_EN = 
          processor_core_rtl1_tessent_mbist_bap_inst.memory_bypass_to_en;
      InputPort MCP_BOUNDING_EN = 
          processor_core_rtl1_tessent_mbist_bap_inst.mcp_bounding_to_en;
      InputPort MBISTPG_ASYNC_RESETN = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_ASYNC_RESET;
      InputPort MBISTPG_DIAG_EN = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_DIAG_EN;
      InputPort BIST_SETUP2 = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_SETUP[2];
      InputPort BIST_SETUP[1] = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_SETUP[1];
      InputPort BIST_SETUP[0] = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_SETUP[0];
      InputPort MBISTPG_REDUCED_ADDR_CNT_EN = 
          processor_core_rtl1_tessent_mbist_bap_inst.REDUCED_ADDRESS_COUNT;
      InputPort MBISTPG_MEM_RST = 
          processor_core_rtl1_tessent_mbist_bap_inst.ENABLE_MEM_RESET;
      InputPort MBISTPG_TESTDATA_SELECT = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_SELECT_TEST_DATA;
      InputPort FL_CNT_MODE[1] = 
          processor_core_rtl1_tessent_mbist_bap_inst.FL_CNT_MODE1;
      InputPort FL_CNT_MODE[0] = 
          processor_core_rtl1_tessent_mbist_bap_inst.FL_CNT_MODE0;
      InputPort MBISTPG_ALGO_MODE[1] = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_ALGO_MODE1;
      InputPort MBISTPG_ALGO_MODE[0] = 
          processor_core_rtl1_tessent_mbist_bap_inst.BIST_ALGO_MODE0;
      InputPort TCK = tck;
      InputPort BIST_SI = processor_core_rtl1_tessent_mbist_bap_inst.toBist[0];
      InputPort BIST_HOLD = processor_core_rtl1_tessent_mbist_bap_inst.BIST_HOLD;

      InputPort MEM0_BIST_COLLAR_SO = m1_interface_instance.BIST_SO;
      InputPort MEM1_BIST_COLLAR_SO = m2_interface_instance.BIST_SO;
      Attribute tessent_design_instance = 
          "processor_core_rtl1_tessent_mbist_c1_controller_inst";
   }
}
