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
