//--------------------------------------------------------------------------
//
//  Unpublished work. Copyright 2026 Siemens
//
//  This material contains trade secrets or otherwise confidential 
//  information owned by Siemens Industry Software Inc. or its affiliates 
//  (collectively, SISW), or its licensors. Access to and use of this 
//  information is strictly limited as set forth in the Customer's 
//  applicable agreements with SISW.
//
//--------------------------------------------------------------------------
//  File created by: Tessent Shell
//          Version: 2026.2
//       Created on: Wed Aug 12 13:58:31 CST 2026
//--------------------------------------------------------------------------


NameSpace ;
UseNameSpace ;
Module gps_baseband_rtl1_tessent_tdr_sri_ctrl {
 
  ResetPort     ijtag_reset             { ActivePolarity 0;      }
  SelectPort    ijtag_sel;
  ScanInPort    ijtag_si;
  CaptureEnPort ijtag_ce;
  ShiftEnPort   ijtag_se;
  UpdateEnPort  ijtag_ue;
  TCKPort       ijtag_tck;
  ScanOutPort   ijtag_so                { Source tdr[0];         }
  DataOutPort   async_set_reset_static_disable                   {
    Source tdr[5];
    Attribute connection_rule_option = "allowed_no_destination";
    Attribute tessent_dft_signal_name = "async_set_reset_static_disable";
    Attribute tessent_dft_signal_usage = "logic_test_control";
    Attribute tessent_dft_signal_value_in_pre_scan_drc = "0";
    Attribute tessent_dft_signal_reset_value = 0;
    Attribute tessent_persistent_design_pin = "tessent_persistent_cell_async_set_reset_static_disable/Y";
  }
  DataOutPort   ext_edt_mode            {
    Source tdr[4];
    Attribute connection_rule_option = "allowed_no_destination";
    Attribute tessent_dft_signal_name = "ext_edt_mode";
    Attribute tessent_dft_signal_usage = "scan_mode";
    Attribute tessent_dft_signal_scan_mode_type = "external";
    Attribute tessent_dft_signal_reset_value = 0;
    Attribute tessent_persistent_design_pin = "tessent_persistent_cell_ext_edt_mode/Y";
  }
  DataOutPort   int_edt_mode            {
    Source tdr[3];
    Attribute connection_rule_option = "allowed_no_destination";
    Attribute tessent_dft_signal_name = "int_edt_mode";
    Attribute tessent_dft_signal_usage = "scan_mode";
    Attribute tessent_dft_signal_scan_mode_type = "internal";
    Attribute tessent_dft_signal_reset_value = 0;
    Attribute tessent_persistent_design_pin = "tessent_persistent_cell_int_edt_mode/Y";
  }
  DataOutPort   ext_ltest_en            {
    Source tdr[2];
    Attribute connection_rule_option = "allowed_no_destination";
    Attribute tessent_dft_signal_name = "ext_ltest_en";
    Attribute tessent_dft_signal_usage = "logic_test_control";
    Attribute tessent_dft_signal_value_in_pre_scan_drc = "x";
    Attribute tessent_dft_signal_reset_value = 0;
    Attribute tessent_persistent_design_pin = "tessent_persistent_cell_ext_ltest_en/Y";
  }
  DataOutPort   int_ltest_en            {
    Source tdr[1];
    Attribute connection_rule_option = "allowed_no_destination";
    Attribute tessent_dft_signal_name = "int_ltest_en";
    Attribute tessent_dft_signal_usage = "logic_test_control";
    Attribute tessent_dft_signal_value_in_pre_scan_drc = "x";
    Attribute tessent_dft_signal_reset_value = 0;
    Attribute tessent_persistent_design_pin = "tessent_persistent_cell_int_ltest_en/Y";
  }
  DataOutPort   ltest_en                {
    Source tdr[0];
    Attribute connection_rule_option = "allowed_no_destination";
    Attribute tessent_dft_signal_name = "ltest_en";
    Attribute tessent_dft_signal_usage = "logic_test_control";
    Attribute tessent_dft_signal_value_in_pre_scan_drc = "1";
    Attribute tessent_dft_signal_reset_value = 0;
    Attribute tessent_persistent_design_pin = "tessent_persistent_cell_ltest_en/Y";
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
 
  ScanRegister tdr[5:0] {
    ScanInSource     ijtag_si;
    CaptureSource    6'b000000;
    ResetValue       6'b000000;
    DefaultLoadValue 6'b000000;
  }
 
 
  Attribute tessent_use_in_dft_specification = "false";
  Attribute tessent_instrument_type          = "mentor::ijtag_node";
  Attribute tessent_instrument_subtype       = "tessent_tdr";
  Attribute tessent_signature                = "a114a1a0ca115237a1431f79975aacec";
  Attribute tessent_instrument_container     = "gps_baseband_rtl1_ijtag";
}
