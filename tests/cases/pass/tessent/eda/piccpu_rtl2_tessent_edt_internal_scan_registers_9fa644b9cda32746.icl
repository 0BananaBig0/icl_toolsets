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
//       Created on: Mon Oct 26 08:53:14 CST 2026
//--------------------------------------------------------------------------------



  // }}}
 

 // }}}
 

NameSpace ;
UseNameSpace ;
Module piccpu_rtl2_tessent_edt_internal_scan_registers {
   ScanInPort ijtag_si;
   ScanOutPort ijtag_so { Source blk1_sib_i.ijtag_so; }
   TCKPort ijtag_tck;
   ResetPort ijtag_reset { ActivePolarity 0; }
   SelectPort ijtag_sel;
   CaptureEnPort ijtag_ce;
   ShiftEnPort ijtag_se;
   UpdateEnPort ijtag_ue;
   DataInPort edt_single_bypass_chain {
      Attribute tessent_no_input_constraints = "on";
   }

   ScanMux edt_lbist_chain1_single_chain_mode_mux SelectedBy edt_single_bypass_chain {
      1'b1: ijtag_si;
   }
   ScanRegister edt_lbist_chain1[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain1_single_chain_mode_mux;
   }

   ScanMux edt_lbist_chain2_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain1[0];
   }
   ScanRegister edt_lbist_chain2[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain2_bypass_mux;
   }

   ScanMux edt_lbist_chain3_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain2[0];
   }
   ScanRegister edt_lbist_chain3[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain3_bypass_mux;
   }

   ScanMux edt_lbist_chain4_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain3[0];
   }
   ScanRegister edt_lbist_chain4[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain4_bypass_mux;
   }

   ScanMux edt_lbist_chain5_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain4[0];
   }
   ScanRegister edt_lbist_chain5[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain5_bypass_mux;
   }

   ScanMux edt_lbist_chain6_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain5[0];
   }
   ScanRegister edt_lbist_chain6[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain6_bypass_mux;
   }

   ScanMux edt_lbist_chain7_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain6[0];
   }
   ScanRegister edt_lbist_chain7[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain7_bypass_mux;
   }

   ScanMux edt_lbist_chain8_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain7[0];
   }
   ScanRegister edt_lbist_chain8[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain8_bypass_mux;
   }

   ScanMux edt_lbist_chain9_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain8[0];
   }
   ScanRegister edt_lbist_chain9[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain9_bypass_mux;
   }

   ScanMux edt_lbist_chain10_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain9[0];
   }
   ScanRegister edt_lbist_chain10[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain10_bypass_mux;
   }

   Instance blk1_sib_i Of piccpu_rtl2_tessent_single_chain_mode_logic_sib {
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_si = ijtag_si;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_sel = ijtag_sel;
      InputPort ijtag_from_so = edt_lbist_chain10[0];
   }

   Attribute keep_active_during_scan_test = "true";

   Attribute tessent_instrument_type = "mentor::edt";
   Attribute tessent_instrument_subtype = "edt_internal_scan_registers";
   Attribute tessent_signature = "c33d7ea4b4922788dd23fcc6a8210100";
}
