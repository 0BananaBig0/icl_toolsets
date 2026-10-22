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
//       Created on: Wed Aug 12 13:51:47 CST 2026
//--------------------------------------------------------------------------


NameSpace ;
UseNameSpace ;
Module piccpu_rtl1_tessent_tdr_sri_ctrl {
 
  ResetPort     ijtag_reset             { ActivePolarity 0;      }
  SelectPort    ijtag_sel;
  ScanInPort    ijtag_si;
  CaptureEnPort ijtag_ce;
  ShiftEnPort   ijtag_se;
  UpdateEnPort  ijtag_ue;
  TCKPort       ijtag_tck;
  ScanOutPort   ijtag_so                { Source tdr[0];         }
  DataOutPort   async_set_reset_static_disable                   {
    Source tdr[1];
    Attribute connection_rule_option = "allowed_no_destination";
    Attribute tessent_dft_signal_name = "async_set_reset_static_disable";
    Attribute tessent_dft_signal_usage = "logic_test_control";
    Attribute tessent_dft_signal_value_in_pre_scan_drc = "0";
    Attribute tessent_dft_signal_reset_value = 0;
    Attribute tessent_persistent_design_pin = "tessent_persistent_cell_async_set_reset_static_disable/Y";
  }
  DataOutPort   HTKLB_CCM_EN            {
    Source tdr[0];
    Attribute connection_rule_option = "allowed_no_destination";
    Attribute tessent_dft_signal_name = "HTKLB_CCM_EN";
    Attribute tessent_dft_signal_usage = "logic_test_control";
    Attribute tessent_dft_signal_value_in_pre_scan_drc = "x";
    Attribute tessent_dft_signal_reset_value = 0;
    Attribute tessent_persistent_design_pin = "tessent_persistent_cell_HTKLB_CCM_EN/Y";
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
 
  ScanRegister tdr[1:0] {
    ScanInSource     ijtag_si;
    CaptureSource    2'b00;
    ResetValue       2'b00;
    DefaultLoadValue 2'b00;
  }
 
 
  Attribute tessent_use_in_dft_specification = "false";
  Attribute tessent_instrument_type          = "mentor::ijtag_node";
  Attribute tessent_instrument_subtype       = "tessent_tdr";
  Attribute tessent_signature                = "85c634ac39ae076f6e01e6f813dca1d0";
  Attribute tessent_instrument_container     = "piccpu_rtl1_ijtag";
}
