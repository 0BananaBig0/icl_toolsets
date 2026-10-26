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
//       Created on: Mon Oct 26 08:53:08 CST 2026
//--------------------------------------------------------------------------


NameSpace ;
UseNameSpace ;
Module piccpu_rtl2_tessent_occ {
   ClockPort fast_clock {
     Attribute icl_extraction_port_trigger_list = "clock_out";
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
   DataInPort static_clock_control_mode { Attribute connection_rule_option = "allowed_tied"; }
   DataInPort clock_sequence[2:0] {
     Attribute connection_rule_option = "allowed_no_source";
     Attribute function_modifier = "tessent_clock_sequence";
   }
  
   Instance tdr_sib Of piccpu_rtl2_tessent_occ_sib_int {
      InputPort clock         = ijtag_tck;
      InputPort reset         = ijtag_reset;
      InputPort scan_in       = ijtag_si;
      InputPort capture_en    = ijtag_ce;
      InputPort shift_en      = ijtag_se;
      InputPort update_en     = ijtag_ue;
      InputPort enable        = ijtag_sel;
      InputPort from_scan_out = tdr[0];
   }
  
   ScanRegister tdr[9:0] {
      ScanInSource  ijtag_si;
      CaptureSource 10'b0000000000;
      ResetValue    10'b0000000000;
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
   Alias ijtag_static_clock_control_mode = tdr[6];
   Alias ijtag_clock_sequence[2:0] = tdr[9:7];
  
   Attribute keep_active_during_scan_test = "true";
   Attribute tessent_use_in_dft_specification = "false";
   Attribute tessent_instrument_type      = "mentor::occ";
   Attribute tessent_instrument_subtype   = "standard";
   Attribute tessent_instrument_container = "piccpu_rtl2_occ.instrument";
   Attribute tessent_signature            = "3352e3b25ca0babc5451969537f9888f";
   Attribute tdr_parent_instance          = "ijtag_control";
}
