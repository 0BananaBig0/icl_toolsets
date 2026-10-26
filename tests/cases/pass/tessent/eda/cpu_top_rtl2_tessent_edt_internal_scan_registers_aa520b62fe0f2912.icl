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
//       Created on: Mon Oct 26 08:56:37 CST 2026
//--------------------------------------------------------------------------------



  // }}}
 

 // }}}
 

NameSpace ;
UseNameSpace ;
Module cpu_top_rtl2_tessent_edt_internal_scan_registers {
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

   ScanMux edt_lbist_chain11_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain10[0];
   }
   ScanRegister edt_lbist_chain11[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain11_bypass_mux;
   }

   ScanMux edt_lbist_chain12_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain11[0];
   }
   ScanRegister edt_lbist_chain12[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain12_bypass_mux;
   }

   ScanMux edt_lbist_chain13_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain12[0];
   }
   ScanRegister edt_lbist_chain13[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain13_bypass_mux;
   }

   ScanMux edt_lbist_chain14_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain13[0];
   }
   ScanRegister edt_lbist_chain14[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain14_bypass_mux;
   }

   ScanMux edt_lbist_chain15_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain14[0];
   }
   ScanRegister edt_lbist_chain15[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain15_bypass_mux;
   }

   ScanMux edt_lbist_chain16_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain15[0];
   }
   ScanRegister edt_lbist_chain16[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain16_bypass_mux;
   }

   ScanMux edt_lbist_chain17_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain16[0];
   }
   ScanRegister edt_lbist_chain17[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain17_bypass_mux;
   }

   ScanMux edt_lbist_chain18_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain17[0];
   }
   ScanRegister edt_lbist_chain18[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain18_bypass_mux;
   }

   ScanMux edt_lbist_chain19_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain18[0];
   }
   ScanRegister edt_lbist_chain19[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain19_bypass_mux;
   }

   ScanMux edt_lbist_chain20_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain19[0];
   }
   ScanRegister edt_lbist_chain20[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain20_bypass_mux;
   }

   ScanMux edt_lbist_chain21_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain20[0];
   }
   ScanRegister edt_lbist_chain21[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain21_bypass_mux;
   }

   ScanMux edt_lbist_chain22_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain21[0];
   }
   ScanRegister edt_lbist_chain22[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain22_bypass_mux;
   }

   ScanMux edt_lbist_chain23_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain22[0];
   }
   ScanRegister edt_lbist_chain23[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain23_bypass_mux;
   }

   ScanMux edt_lbist_chain24_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain23[0];
   }
   ScanRegister edt_lbist_chain24[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain24_bypass_mux;
   }

   ScanMux edt_lbist_chain25_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain24[0];
   }
   ScanRegister edt_lbist_chain25[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain25_bypass_mux;
   }

   ScanMux edt_lbist_chain26_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain25[0];
   }
   ScanRegister edt_lbist_chain26[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain26_bypass_mux;
   }

   ScanMux edt_lbist_chain27_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain26[0];
   }
   ScanRegister edt_lbist_chain27[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain27_bypass_mux;
   }

   ScanMux edt_lbist_chain28_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain27[0];
   }
   ScanRegister edt_lbist_chain28[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain28_bypass_mux;
   }

   ScanMux edt_lbist_chain29_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain28[0];
   }
   ScanRegister edt_lbist_chain29[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain29_bypass_mux;
   }

   ScanMux edt_lbist_chain30_single_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain29[0];
   }
   ScanRegister edt_lbist_chain30[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain30_single_bypass_mux;
   }

   ScanMux edt_lbist_chain31_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain30[0];
   }
   ScanRegister edt_lbist_chain31[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain31_bypass_mux;
   }

   ScanMux edt_lbist_chain32_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain31[0];
   }
   ScanRegister edt_lbist_chain32[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain32_bypass_mux;
   }

   ScanMux edt_lbist_chain33_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain32[0];
   }
   ScanRegister edt_lbist_chain33[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain33_bypass_mux;
   }

   ScanMux edt_lbist_chain34_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain33[0];
   }
   ScanRegister edt_lbist_chain34[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain34_bypass_mux;
   }

   ScanMux edt_lbist_chain35_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain34[0];
   }
   ScanRegister edt_lbist_chain35[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain35_bypass_mux;
   }

   ScanMux edt_lbist_chain36_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain35[0];
   }
   ScanRegister edt_lbist_chain36[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain36_bypass_mux;
   }

   ScanMux edt_lbist_chain37_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain36[0];
   }
   ScanRegister edt_lbist_chain37[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain37_bypass_mux;
   }

   ScanMux edt_lbist_chain38_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain37[0];
   }
   ScanRegister edt_lbist_chain38[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain38_bypass_mux;
   }

   ScanMux edt_lbist_chain39_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain38[0];
   }
   ScanRegister edt_lbist_chain39[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain39_bypass_mux;
   }

   ScanMux edt_lbist_chain40_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain39[0];
   }
   ScanRegister edt_lbist_chain40[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain40_bypass_mux;
   }

   ScanMux edt_lbist_chain41_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain40[0];
   }
   ScanRegister edt_lbist_chain41[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain41_bypass_mux;
   }

   ScanMux edt_lbist_chain42_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain41[0];
   }
   ScanRegister edt_lbist_chain42[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain42_bypass_mux;
   }

   ScanMux edt_lbist_chain43_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain42[0];
   }
   ScanRegister edt_lbist_chain43[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain43_bypass_mux;
   }

   ScanMux edt_lbist_chain44_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain43[0];
   }
   ScanRegister edt_lbist_chain44[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain44_bypass_mux;
   }

   ScanMux edt_lbist_chain45_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain44[0];
   }
   ScanRegister edt_lbist_chain45[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain45_bypass_mux;
   }

   ScanMux edt_lbist_chain46_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain45[0];
   }
   ScanRegister edt_lbist_chain46[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain46_bypass_mux;
   }

   ScanMux edt_lbist_chain47_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain46[0];
   }
   ScanRegister edt_lbist_chain47[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain47_bypass_mux;
   }

   ScanMux edt_lbist_chain48_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain47[0];
   }
   ScanRegister edt_lbist_chain48[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain48_bypass_mux;
   }

   ScanMux edt_lbist_chain49_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain48[0];
   }
   ScanRegister edt_lbist_chain49[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain49_bypass_mux;
   }

   ScanMux edt_lbist_chain50_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain49[0];
   }
   ScanRegister edt_lbist_chain50[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain50_bypass_mux;
   }

   ScanMux edt_lbist_chain51_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain50[0];
   }
   ScanRegister edt_lbist_chain51[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain51_bypass_mux;
   }

   ScanMux edt_lbist_chain52_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain51[0];
   }
   ScanRegister edt_lbist_chain52[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain52_bypass_mux;
   }

   ScanMux edt_lbist_chain53_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain52[0];
   }
   ScanRegister edt_lbist_chain53[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain53_bypass_mux;
   }

   ScanMux edt_lbist_chain54_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain53[0];
   }
   ScanRegister edt_lbist_chain54[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain54_bypass_mux;
   }

   ScanMux edt_lbist_chain55_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain54[0];
   }
   ScanRegister edt_lbist_chain55[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain55_bypass_mux;
   }

   ScanMux edt_lbist_chain56_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain55[0];
   }
   ScanRegister edt_lbist_chain56[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain56_bypass_mux;
   }

   ScanMux edt_lbist_chain57_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain56[0];
   }
   ScanRegister edt_lbist_chain57[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain57_bypass_mux;
   }

   ScanMux edt_lbist_chain58_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain57[0];
   }
   ScanRegister edt_lbist_chain58[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain58_bypass_mux;
   }

   ScanMux edt_lbist_chain59_single_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain58[0];
   }
   ScanRegister edt_lbist_chain59[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain59_single_bypass_mux;
   }

   ScanMux edt_lbist_chain60_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain59[0];
   }
   ScanRegister edt_lbist_chain60[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain60_bypass_mux;
   }

   ScanMux edt_lbist_chain61_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain60[0];
   }
   ScanRegister edt_lbist_chain61[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain61_bypass_mux;
   }

   ScanMux edt_lbist_chain62_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain61[0];
   }
   ScanRegister edt_lbist_chain62[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain62_bypass_mux;
   }

   ScanMux edt_lbist_chain63_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain62[0];
   }
   ScanRegister edt_lbist_chain63[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain63_bypass_mux;
   }

   ScanMux edt_lbist_chain64_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain63[0];
   }
   ScanRegister edt_lbist_chain64[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain64_bypass_mux;
   }

   ScanMux edt_lbist_chain65_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain64[0];
   }
   ScanRegister edt_lbist_chain65[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain65_bypass_mux;
   }

   ScanMux edt_lbist_chain66_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain65[0];
   }
   ScanRegister edt_lbist_chain66[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain66_bypass_mux;
   }

   ScanMux edt_lbist_chain67_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain66[0];
   }
   ScanRegister edt_lbist_chain67[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain67_bypass_mux;
   }

   ScanMux edt_lbist_chain68_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain67[0];
   }
   ScanRegister edt_lbist_chain68[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain68_bypass_mux;
   }

   ScanMux edt_lbist_chain69_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain68[0];
   }
   ScanRegister edt_lbist_chain69[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain69_bypass_mux;
   }

   ScanMux edt_lbist_chain70_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain69[0];
   }
   ScanRegister edt_lbist_chain70[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain70_bypass_mux;
   }

   ScanMux edt_lbist_chain71_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain70[0];
   }
   ScanRegister edt_lbist_chain71[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain71_bypass_mux;
   }

   ScanMux edt_lbist_chain72_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain71[0];
   }
   ScanRegister edt_lbist_chain72[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain72_bypass_mux;
   }

   ScanMux edt_lbist_chain73_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain72[0];
   }
   ScanRegister edt_lbist_chain73[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain73_bypass_mux;
   }

   ScanMux edt_lbist_chain74_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain73[0];
   }
   ScanRegister edt_lbist_chain74[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain74_bypass_mux;
   }

   ScanMux edt_lbist_chain75_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain74[0];
   }
   ScanRegister edt_lbist_chain75[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain75_bypass_mux;
   }

   ScanMux edt_lbist_chain76_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain75[0];
   }
   ScanRegister edt_lbist_chain76[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain76_bypass_mux;
   }

   ScanMux edt_lbist_chain77_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain76[0];
   }
   ScanRegister edt_lbist_chain77[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain77_bypass_mux;
   }

   ScanMux edt_lbist_chain78_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain77[0];
   }
   ScanRegister edt_lbist_chain78[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain78_bypass_mux;
   }

   ScanMux edt_lbist_chain79_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain78[0];
   }
   ScanRegister edt_lbist_chain79[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain79_bypass_mux;
   }

   ScanMux edt_lbist_chain80_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain79[0];
   }
   ScanRegister edt_lbist_chain80[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain80_bypass_mux;
   }

   ScanMux edt_lbist_chain81_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain80[0];
   }
   ScanRegister edt_lbist_chain81[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain81_bypass_mux;
   }

   ScanMux edt_lbist_chain82_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain81[0];
   }
   ScanRegister edt_lbist_chain82[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain82_bypass_mux;
   }

   ScanMux edt_lbist_chain83_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain82[0];
   }
   ScanRegister edt_lbist_chain83[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain83_bypass_mux;
   }

   ScanMux edt_lbist_chain84_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain83[0];
   }
   ScanRegister edt_lbist_chain84[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain84_bypass_mux;
   }

   ScanMux edt_lbist_chain85_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain84[0];
   }
   ScanRegister edt_lbist_chain85[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain85_bypass_mux;
   }

   ScanMux edt_lbist_chain86_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain85[0];
   }
   ScanRegister edt_lbist_chain86[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain86_bypass_mux;
   }

   ScanMux edt_lbist_chain87_bypass_mux SelectedBy edt_single_bypass_chain {
      1'b1: edt_lbist_chain86[0];
   }
   ScanRegister edt_lbist_chain87[1:0] {
      Attribute tessent_scan_register = "*";
      Attribute tessent_ignore_during_icl_verification = "on";
      ScanInSource edt_lbist_chain87_bypass_mux;
   }

   Instance blk1_sib_i Of cpu_top_rtl2_tessent_single_chain_mode_logic_sib {
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_si = ijtag_si;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_sel = ijtag_sel;
      InputPort ijtag_from_so = edt_lbist_chain87[0];
   }

   Attribute keep_active_during_scan_test = "true";

   Attribute tessent_instrument_type = "mentor::edt";
   Attribute tessent_instrument_subtype = "edt_internal_scan_registers";
   Attribute tessent_signature = "26806416a606252cca8dcbd2c2b93048";
}
