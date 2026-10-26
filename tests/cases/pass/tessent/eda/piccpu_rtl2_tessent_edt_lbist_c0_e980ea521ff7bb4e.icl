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
//       Created on: Mon Oct 26 08:53:16 CST 2026
//--------------------------------------------------------------------------------


NameSpace ;
UseNameSpace ;
Module piccpu_rtl2_tessent_edt_lbist_c0 {
   DataInPort lbist_en;
   ScanInPort ijtag_si;
   ScanOutPort ijtag_so {
      Source piccpu_rtl2_tessent_edt_c0_sib_misr_i.ijtag_so;
   }
   DataInPort edt_bypass {
      RefEnum OnOffTable;
      Attribute tessent_no_input_constraints = "on";
   }
   DataInPort edt_single_bypass_chain {
      RefEnum OnOffTable;
      Attribute tessent_no_input_constraints = "on";
   }
   DataInPort edt_low_power_shift_en {
      RefEnum OnOffTable;
      Attribute tessent_no_input_constraints = "on";
   }
   DataInPort ccm_en {
      Attribute connection_rule_option = "allowed_tied_low";
   }
   TCKPort ijtag_tck;
   ResetPort ijtag_reset {
      ActivePolarity 0;
   }
   SelectPort ijtag_sel;
   CaptureEnPort ijtag_ce;
   ShiftEnPort ijtag_se;
   UpdateEnPort ijtag_ue;

   ScanMux lbist_scan_in_mux SelectedBy ccm_en {
      1'b0: ijtag_si;
   }
   ScanMux lfsm_vec_scan_in_mux SelectedBy lbist_en {
      1'b1: lbist_scan_in_mux;
   }

   ScanRegister lfsm_vec[30:0] {
      ScanInSource lfsm_vec_scan_in_mux;
   }
   Instance piccpu_rtl2_tessent_edt_c0_sib_decompressor_i Of piccpu_rtl2_tessent_edt_c0_sib {
      InputPort ijtag_si = lbist_scan_in_mux;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = ijtag_sel;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_from_so = lfsm_vec[0];
   }

   ScanRegister lbist_lp_hold_reg[3:0] {
      ScanInSource piccpu_rtl2_tessent_edt_c0_sib_decompressor_i.ijtag_so;
   }
   ScanRegister lbist_lp_toggle_reg[3:0] {
      ScanInSource lbist_lp_hold_reg[0];
   }
   ScanRegister lbist_lp_switching_reg[3:0] {
      ScanInSource lbist_lp_toggle_reg[0];
   }
   Instance piccpu_rtl2_tessent_edt_c0_sib_lbist_lp_static_control_i Of piccpu_rtl2_tessent_edt_c0_sib {
      InputPort ijtag_si = piccpu_rtl2_tessent_edt_c0_sib_decompressor_i.ijtag_so;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = ijtag_sel;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_from_so = lbist_lp_switching_reg[0];
   }

   ScanRegister lbist_lp_mask_shift_reg[30:0] {
      ScanInSource piccpu_rtl2_tessent_edt_c0_sib_lbist_lp_static_control_i.ijtag_so;
   }
   Instance piccpu_rtl2_tessent_edt_c0_sib_lbist_lp_mask_shift_reg_i Of piccpu_rtl2_tessent_edt_c0_sib {
      InputPort ijtag_si = piccpu_rtl2_tessent_edt_c0_sib_lbist_lp_static_control_i.ijtag_so;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = ijtag_sel;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_from_so = lbist_lp_mask_shift_reg[0];
   }

   ScanRegister bist_chain_mask[9:0] {
      ScanInSource piccpu_rtl2_tessent_edt_c0_sib_lbist_lp_mask_shift_reg_i.ijtag_so;
   }

   ScanRegister bist_chain_mask_load_en {
      ScanInSource bist_chain_mask[0];
   }

   Instance piccpu_rtl2_tessent_edt_c0_sib_chain_mask_i Of piccpu_rtl2_tessent_edt_c0_sib {
      InputPort ijtag_si = piccpu_rtl2_tessent_edt_c0_sib_lbist_lp_mask_shift_reg_i.ijtag_so;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = ijtag_sel;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_from_so = bist_chain_mask_load_en;
   }

   ScanRegister misr[23:0] {
      ScanInSource piccpu_rtl2_tessent_edt_c0_sib_chain_mask_i.ijtag_so;
   }

   Instance piccpu_rtl2_tessent_edt_c0_sib_misr_i Of piccpu_rtl2_tessent_edt_c0_sib {
      InputPort ijtag_si = piccpu_rtl2_tessent_edt_c0_sib_chain_mask_i.ijtag_so;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = ijtag_sel;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_from_so = misr[0];
   }

   Enum OnOffTable {
      off = 1'b0;
      on  = 1'b1;
   }

   Attribute keep_active_during_scan_test = "true";

   Attribute tessent_instrument_container = "piccpu_rtl2_edt_lbist.instrument";
   Attribute tessent_instrument_type = "mentor::edt";
   Attribute tessent_signature = "ce7816e9b3c0d9d982fb43efc581d474";
}
