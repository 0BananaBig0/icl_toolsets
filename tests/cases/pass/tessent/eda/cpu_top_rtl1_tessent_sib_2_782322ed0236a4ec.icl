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
//       Created on: Mon Oct 26 08:54:16 CST 2026
//--------------------------------------------------------------------------


NameSpace ;
UseNameSpace ;
Module cpu_top_rtl1_tessent_sib_2 {
 
   ResetPort     ijtag_reset            { ActivePolarity 0;      }
   SelectPort    ijtag_sel;
   ScanInPort    ijtag_si;
   CaptureEnPort ijtag_ce;
   ShiftEnPort   ijtag_se;
   UpdateEnPort  ijtag_ue;
   TCKPort       ijtag_tck;
   ScanOutPort   ijtag_so               { Source sib;            }
   ToSelectPort  ijtag_to_sel           {
     Attribute connection_rule_option = "allowed_no_destination";
   }
   ScanInPort    ijtag_from_so          {
     Attribute connection_rule_option = "allowed_no_source";
   }
 
   ScanInterface client {
     Port ijtag_si;
     Port ijtag_so;
     Port ijtag_sel;
     Port ijtag_tck;
     Port ijtag_reset;
     Port ijtag_se;
     Port ijtag_ce;
     Port ijtag_ue;
   }
   ScanInterface host   {
     Port ijtag_from_so;
     Port ijtag_to_sel;
   }
 
   Attribute keep_active_during_scan_test = "false";
 
   ScanRegister sib {
     ScanInSource    scan_in_mux;
     CaptureSource   sib;
     ResetValue      1'b0;
   }
 
   ScanMux scan_in_mux SelectedBy sib {
     1'b0 : ijtag_si;
     1'b1 : ijtag_from_so;
   }
 
   Attribute tessent_use_in_dft_specification = "false";
   Attribute tessent_instrument_type          = "mentor::ijtag_node";
   Attribute tessent_signature                = "e8b1aac2ef2306aac70c65dbc79836fc";
   Attribute tessent_instrument_container     = "cpu_top_rtl1_ijtag";
   Attribute tessent_instrument_subtype       = "sib";
}
