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
//       Created on: Mon Oct 26 09:10:55 CST 2026
//--------------------------------------------------------------------------


NameSpace ;
UseNameSpace ;
Module chip_top_rtl2_tessent_ssn_receiver_1x_pipe_w2_1 {
 
  ResetPort       ijtag_reset {
    ActivePolarity 0;
  }
  ClockPort       bus_clock {
    Attribute function_modifier = "tessent_ssn_clock";
    Attribute forced_high_dft_signal_list = "ssn_en";
  }
  DataInPort      bus_data_in[1:0] {
    Attribute function_modifier = "tessent_ssn_bus_data";
    Attribute forced_high_dft_signal_list = "ssn_en";
  }
  DataOutPort     bus_data_out[1:0] {  
    Attribute function_modifier = "tessent_ssn_bus_data";
    Attribute forced_high_dft_signal_list = "ssn_en";
    Source    bus_out_reg;
  }
 
  DataRegister    bus_in_reg[1:0] {
    Attribute       tessent_active_clock_edge = "falling";
    WriteDataSource bus_data_in[1:0];
    WriteEnSource   1'b1;
    ResetValue      2'b0;
  }
 
  DataRegister    bus_out_reg[1:0] {
    WriteDataSource bus_in_reg[1:0];
    WriteEnSource   1'b1;
    ResetValue      2'b0;
  }
 
 
  Attribute keep_active_during_scan_test      = "true";
  Attribute tessent_instrument_type           = "mentor::ssn";
  Attribute tessent_instrument_subtype        = "receiver_pipeline";
  Attribute tessent_instrument_container      = "chip_top_rtl2_ssn.instrument";
  Attribute tessent_signature                 = "f76b4c08f8164781c281f01ffe25d855";
}
