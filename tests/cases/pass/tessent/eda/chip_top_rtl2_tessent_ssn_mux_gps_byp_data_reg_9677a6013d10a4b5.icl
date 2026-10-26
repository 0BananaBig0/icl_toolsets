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
Module chip_top_rtl2_tessent_ssn_mux_gps_byp_data_reg {
  ClockPort       bus_clock {
    Attribute function_modifier = "tessent_ssn_clock";    
  }
  DataInPort      bus_data_in[1:0] {
    Attribute function_modifier = "tessent_ssn_bus_data";
    Attribute forced_high_dft_signal_list = "ssn_en";
  }
  DataInPort      secondary_bus_data_in[1:0] {
    Attribute function_modifier = "tessent_ssn_bus_data";
    Attribute forced_high_dft_signal_list = "ssn_en";
  }
  DataOutPort     bus_data_out[1:0] {
    Attribute function_modifier = "tessent_ssn_bus_data";
    Attribute forced_high_dft_signal_list = "ssn_en";
    Source    bus_out_reg;
  }
  DataInPort      select_secondary_bus;
 
  DataRegister    bus_out_reg[1:0] {
    WriteDataSource BusMux;
    WriteEnSource   1'b1;
    ResetValue      2'b0;
  }
 
  DataMux         BusMux[1:0] SelectedBy select_secondary_bus {
    1'b0 : bus_data_in[1:0];
    1'b1 : secondary_bus_data_in[1:0];
  }
}
