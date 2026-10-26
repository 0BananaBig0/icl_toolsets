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
//       Created on: Mon Oct 26 09:10:56 CST 2026
//--------------------------------------------------------------------------


NameSpace ;
UseNameSpace ;
Module chip_top_rtl2_tessent_occ {
   ClockPort fast_clock {
   }
   ToClockPort clock_out {
     Source tck_mux;
     Attribute exclude_from_sdc = "true";
   }
   TCKPort ijtag_tck;
   ScanInPort ijtag_si;
   ScanOutPort ijtag_so {
      Source tdr_sib.scan_out;
   }
   ResetPort ijtag_reset {
      ActivePolarity 0;
   }
   CaptureEnPort ijtag_ce;
   ShiftEnPort ijtag_se;
   UpdateEnPort ijtag_ue;
   SelectPort ijtag_sel;
  
   Instance tdr_sib Of chip_top_rtl2_tessent_occ_sib_int {
      InputPort clock         = ijtag_tck;
      InputPort reset         = ijtag_reset;
      InputPort scan_in       = ijtag_si;
      InputPort capture_en    = ijtag_ce;
      InputPort shift_en      = ijtag_se;
      InputPort update_en     = ijtag_ue;
      InputPort enable        = ijtag_sel;
      InputPort from_scan_out = tdr[0];
   }
  
   ScanRegister tdr[5:0] {
      ScanInSource  ijtag_si;
      CaptureSource 6'b000000;
      ResetValue    6'b000000;
   }
  
   ClockMux tck_mux SelectedBy inject_tck {
     1'b1 : ijtag_tck;
     1'b0 : fast_clock;
   }
 
   Alias test_mode = tdr[0];
   Alias fast_capture_mode = tdr[1];
   Alias active_upstream_parent_occ = tdr[2];
   Alias capture_cycle_width[1:0] = tdr[4:3];
   Alias inject_tck = tdr[5];
  
   Attribute keep_active_during_scan_test = "true";
   Attribute tessent_use_in_dft_specification = "false";
   Attribute tessent_instrument_type      = "mentor::occ";
   Attribute tessent_instrument_subtype   = "standard";
   Attribute tessent_instrument_container = "chip_top_rtl2_occ.instrument";
   Attribute tessent_signature            = "a3fa8ff9db9bbb8a66058b7a04c9bcdb";
   Attribute tdr_parent_instance          = "ijtag_control";
}
