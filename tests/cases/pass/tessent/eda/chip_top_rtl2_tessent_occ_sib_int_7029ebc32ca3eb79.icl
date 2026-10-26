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
Module chip_top_rtl2_tessent_occ_sib_int {
   TCKPort clock;
   ResetPort reset {
      ActivePolarity 0;
   }
   ScanInPort scan_in;
   ScanOutPort scan_out { 
      Source sib; 
   }
   CaptureEnPort capture_en;
   ShiftEnPort shift_en;
   UpdateEnPort update_en;
   SelectPort enable;
   ToSelectPort to_scan_en {
      Source to_scan_en_and;
   }
   ScanInPort from_scan_out;
  
   ScanRegister sib {
      ScanInSource  scan_in_mux;
      CaptureSource 1'b0;
      ResetValue    1'b0;
   }
   ScanMux scan_in_mux SelectedBy sib {
       1'b0: scan_in;
       1'b1: from_scan_out;
   }
   LogicSignal to_scan_en_and  {
       (enable, sib) == 2'b11;
   }
   ScanInterface client {
      Port scan_in;
      Port scan_out;
      Port enable;
   }
   ScanInterface host {
      Port from_scan_out;
      Port to_scan_en;
   }
}
