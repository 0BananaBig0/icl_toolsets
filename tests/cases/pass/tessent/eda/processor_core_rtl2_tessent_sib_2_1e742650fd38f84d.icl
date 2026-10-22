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
Module processor_core_rtl2_tessent_sib_2 {
   // ICL module read from source on or near line 1500 of file '/home/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hierarchical_flow_with_ssn_v2026.2/wrapped_cores/processor_core/tsdb_outdir/dft_inserted_designs/processor_core_rtl2.dft_inserted_design/processor_core.icl'
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
