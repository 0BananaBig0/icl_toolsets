//--------------------------------------------------------------------------------
//
//  Unpublished work. Copyright 2026 Siemens
//
//  This material contains trade secrets or otherwise confidential 
//  information owned by Siemens Industry Software Inc. or its affiliates 
//  (collectively, SISW), or its licensors. Access to and use of this 
//  information is strictly limited as set forth in the Customer's 
//  applicable agreements with SISW.
//
//--------------------------------------------------------------------------------
//  File created by: Tessent Shell
//          Version: 2026.2
//       Created on: Wed Aug 12 13:58:39 CST 2026
//--------------------------------------------------------------------------------


NameSpace ;
UseNameSpace ;
Module gps_baseband_rtl1_tessent_edt_c1_int {
   DataInPort edt_bypass {
      RefEnum OnOffTable;
      Attribute tessent_no_input_constraints = "on";
   }

   Enum OnOffTable {
      off = 1'b0;
      on  = 1'b1;
   }

   Attribute keep_active_during_scan_test = "true";

   Attribute tessent_instrument_type = "mentor::edt";
   Attribute tessent_signature = "6b5c989c9d41ebd86a7c3b161c640cb8";
}
