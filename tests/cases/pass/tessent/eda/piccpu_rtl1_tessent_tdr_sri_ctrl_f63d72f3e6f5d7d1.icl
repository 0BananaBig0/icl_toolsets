//-------------------------------------------------
//  File created by: Tessent Shell
//          Version: 2026.2
//       Created on: Wed Aug 12 13:52:01 CST 2026
//-------------------------------------------------





// instanced as piccpu.piccpu_rtl1_tessent_mbist_bap_inst



// instanced as piccpu.piccpu_rtl1_tessent_sib_mbist_inst



// instanced as piccpu.piccpu_rtl1_tessent_sib_sri_inst



// instanced as piccpu.piccpu_rtl1_tessent_sib_sti_inst



// instanced as piccpu.piccpu_rtl1_tessent_tdr_sri_ctrl_inst



// instanced as piccpu.regs_piccpu_rtl1_tessent_mbist_c1_controller_inst



// instanced as piccpu.regs_picdram



// instanced as piccpu.regs_picdram_interface_inst

NameSpace ;
UseNameSpace ;
Module piccpu_rtl1_tessent_tdr_sri_ctrl {
   // ICL module read from source on or near line 17 of file '/data/workspaces/lianghuaxiao/tessent_2026.2_usage_examples/tessent_example_hybrid_tk_lbist_flow_v2026.2/tsdb_outdir/instruments/piccpu_rtl1_ijtag.instrument/piccpu_rtl1_tessent_tdr_sri_ctrl.icl'
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
      Source tdr[1];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_dft_signal_name = "async_set_reset_static_disable";
      Attribute tessent_dft_signal_usage = "logic_test_control";
      Attribute tessent_dft_signal_value_in_pre_scan_drc = "0";
      Attribute tessent_dft_signal_reset_value = 0;
      Attribute tessent_persistent_design_pin = 
          "{tessent_persistent_cell_async_set_reset_static_disable/Y}";
   }
   DataOutPort HTKLB_CCM_EN {
      Source tdr[0];
      Attribute connection_rule_option = "allowed_no_destination";
      Attribute tessent_dft_signal_name = "HTKLB_CCM_EN";
      Attribute tessent_dft_signal_usage = "logic_test_control";
      Attribute tessent_dft_signal_value_in_pre_scan_drc = "x";
      Attribute tessent_dft_signal_reset_value = 0;
      Attribute tessent_persistent_design_pin = 
          "{tessent_persistent_cell_HTKLB_CCM_EN/Y}";
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
   Attribute tessent_signature = "85c634ac39ae076f6e01e6f813dca1d0";
   Attribute tessent_instrument_container = "piccpu_rtl1_ijtag";
   ScanRegister tdr[1:0] {
      ScanInSource ijtag_si;
      CaptureSource 2'b00;
      DefaultLoadValue 2'b00;
      ResetValue 2'b00;
   }
}
