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
Module gps_baseband_rtl1_tessent_ssn_scan_host_1 {
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
    Source bus_out_reg;
    Attribute forced_high_dft_signal_list = "ssn_en";
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
    Source inner.ijtag_so;
  }
 
 Instance inner Of gps_baseband_rtl1_tessent_ssn_scan_host_1_inner {
   InputPort ijtag_reset = ijtag_reset;
   InputPort ijtag_clock   = ijtag_tck;
   InputPort ijtag_sel   = ijtag_sel;
   InputPort ijtag_ce    = ijtag_ce;
   InputPort ijtag_se    = ijtag_se;
   InputPort ijtag_ue    = ijtag_ue;
   InputPort ijtag_si    = ijtag_si;
   Attribute tessent_design_instance = "ijtag_registers";
 }
  DataRegister    bus_in_reg[1:0] {
    WriteDataSource bus_data_in;
    WriteEnSource   1'b1;
    ResetValue      2'b0;
    Attribute tessent_matched_scan_register = "inner.bus_in_shift_reg";
  }
 
  DataRegister    bus_out_reg[1:0] {
    WriteDataSource bus_in_reg;
    WriteEnSource   1'b1;
    ResetValue      2'b0;
    Attribute tessent_matched_scan_register = "inner.bus_out_shift_reg";
  }
 
 
  Attribute keep_active_during_scan_test      = "true";
  Attribute tessent_instrument_type           = "mentor::ssn";
  Attribute tessent_instrument_container      = "gps_baseband_rtl1_ssn.instrument";
  Attribute tessent_instrument_subtype        = "scan_host";
  Attribute tessent_signature                 = "e5c83755f9028d12f7259576d0fe1713";
  Attribute tessent_persistent_clock_pins     =
    "{tessent_persistent_cell_bus_clock_buf/A}";
  Attribute tessent_persistent_bus_in_pins    =
    "{tessent_persistent_cell_bus_data_in_buf_0/A} ",
    "{tessent_persistent_cell_bus_data_in_buf_1/A}";
  Attribute tessent_persistent_bus_out_pins   =
    "{tessent_persistent_cell_bus_data_out_buf_0/Y} ",
    "{tessent_persistent_cell_bus_data_out_buf_1/Y}";
}
