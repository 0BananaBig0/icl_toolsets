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
//       Created on: Mon Oct 26 09:10:54 CST 2026
//--------------------------------------------------------------------------


NameSpace ;
UseNameSpace ;
Module chip_top_rtl2_tessent_ssn_mux_gps_byp {
  ClockPort       bus_clock {
    Attribute function_modifier = "tessent_ssn_clock";
    Attribute forced_high_dft_signal_list = "ssn_en";
  }
 
  DataInPort      bus_data_in[1:0] {
    Attribute function_modifier = "tessent_ssn_bus_data";
    Attribute forced_high_dft_signal_list = "ssn_en";
  }
  DataInPort      secondary_bus_data_in[1:0] {
    Attribute function_modifier = "tessent_ssn_bus_data";
    Attribute connection_rule_option = "allowed_no_source";
    Attribute forced_high_dft_signal_list = "ssn_en";
  }
  DataOutPort     bus_data_out[1:0] {
    Attribute function_modifier = "tessent_ssn_bus_data";
    Attribute forced_high_dft_signal_list = "ssn_en";
    Source    data_bus.bus_data_out;
  }
  DataOutPort     select_out {
    Attribute connection_rule_option = "allowed_no_destination";
    Attribute tessent_use_in_dft_specification = "false";
    Source    select_secondary_bus;
  }
  ResetPort       ijtag_reset {
    ActivePolarity 0;
  }
  TCKPort         ijtag_tck;
  SelectPort      ijtag_sel;
  CaptureEnPort   ijtag_ce;
  ShiftEnPort     ijtag_se;
  UpdateEnPort    ijtag_ue;
  ScanInPort      ijtag_si;
  ScanOutPort     ijtag_so {
    Source select_secondary_bus;
  }
 
  ScanRegister    select_secondary_bus {
    ScanInSource  ijtag_si;
    CaptureSource 1'b0;
    ResetValue 1'b0;
  }
 
  Instance data_bus Of chip_top_rtl2_tessent_ssn_mux_gps_byp_data_reg {
    InputPort bus_clock = bus_clock;
    InputPort select_secondary_bus = select_secondary_bus;
    InputPort bus_data_in = bus_data_in;
    InputPort secondary_bus_data_in = secondary_bus_data_in;
  }
  
  Attribute keep_active_during_scan_test = "true";
  Attribute tessent_instrument_type      = "mentor::ssn";
  Attribute tessent_instrument_subtype   = "multiplexer";
  Attribute tessent_instrument_container = "chip_top_rtl2_ssn.instrument";
  Attribute tessent_signature            = "932cee07908b74e64cabaa9593ef1416";
}
