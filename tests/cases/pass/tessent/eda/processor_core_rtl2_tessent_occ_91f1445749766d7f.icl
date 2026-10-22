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
Module processor_core_rtl2_tessent_occ {
   // ICL module read from source on or near line 1396 of file '/home/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl2.dft_inserted_design/processor_core.icl'
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
