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
//       Created on: Thu Aug 27 11:22:36 CST 2026
//--------------------------------------------------------------------------


NameSpace ;
UseNameSpace ;
Module processor_core_rtl2_tessent_edt_c1_int_tdr {
 
  ResetPort     ijtag_reset             { ActivePolarity 0;      }
  SelectPort    ijtag_sel;
  ScanInPort    ijtag_si;
  CaptureEnPort ijtag_ce;
  ShiftEnPort   ijtag_se;
  UpdateEnPort  ijtag_ue;
  TCKPort       ijtag_tck;
  ScanOutPort   ijtag_so                { Source tdr[0];         }
  DataOutPort   edt_bypass              {
    Source tdr[0];
    Attribute connection_rule_option = "allowed_no_destination";
    Attribute tessent_persistent_design_pin = "tessent_persistent_cell_edt_bypass/Y";
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
  ScanRegister tdr[0:0] {
    ScanInSource     ijtag_si;
    CaptureSource    tdr[0:0];
    ResetValue       1'b0;
    DefaultLoadValue 1'b0;
  }
 
 
  Attribute tessent_use_in_dft_specification = "false";
  Attribute tessent_instrument_type          = "mentor::ijtag_node";
  Attribute tessent_instrument_subtype       = "tessent_tdr";
  Attribute tessent_signature                = "9b2a4f8d8bb23babe0feeaa79b4422b4";
  Attribute tessent_instrument_container     = "processor_core_rtl2_edt";
}
