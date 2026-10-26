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
//       Created on: Mon Oct 26 08:56:39 CST 2026
//--------------------------------------------------------------------------------


NameSpace ;
UseNameSpace ;
Module cpu_top_rtl2_tessent_edt_lbist_c1 {
   DataInPort lbist_en;
   ScanInPort ijtag_si;
   ScanOutPort ijtag_so {
      Source cpu_top_rtl2_tessent_edt_c1_sib_misr_i.ijtag_so;
   }
   DataInPort edt_bypass {
      RefEnum OnOffTable;
      Attribute tessent_no_input_constraints = "on";
   }
   DataInPort edt_single_bypass_chain {
      RefEnum OnOffTable;
      Attribute tessent_no_input_constraints = "on";
   }
   TCKPort ijtag_tck;
   ResetPort ijtag_reset {
      ActivePolarity 0;
   }
   SelectPort ijtag_sel;
   CaptureEnPort ijtag_ce;
   ShiftEnPort ijtag_se;
   UpdateEnPort ijtag_ue;

   ScanMux lfsm_vec_scan_in_mux SelectedBy lbist_en {
      1'b1: ijtag_si;
   }

   ScanRegister lfsm_vec[30:0] {
      ScanInSource lfsm_vec_scan_in_mux;
   }
   Instance cpu_top_rtl2_tessent_edt_c1_sib_decompressor_i Of cpu_top_rtl2_tessent_edt_c1_sib {
      InputPort ijtag_si = ijtag_si;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = ijtag_sel;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_from_so = lfsm_vec[0];
   }

   ScanRegister bist_chain_mask[86:0] {
      ScanInSource cpu_top_rtl2_tessent_edt_c1_sib_decompressor_i.ijtag_so;
   }

   ScanRegister bist_chain_mask_load_en {
      ScanInSource bist_chain_mask[0];
   }

   Instance cpu_top_rtl2_tessent_edt_c1_sib_chain_mask_i Of cpu_top_rtl2_tessent_edt_c1_sib {
      InputPort ijtag_si = cpu_top_rtl2_tessent_edt_c1_sib_decompressor_i.ijtag_so;
      InputPort ijtag_tck = ijtag_tck;
      InputPort ijtag_reset = ijtag_reset;
      InputPort ijtag_sel = ijtag_sel;
      InputPort ijtag_ce = ijtag_ce;
      InputPort ijtag_se = ijtag_se;
      InputPort ijtag_ue = ijtag_ue;
      InputPort ijtag_from_so = bist_chain_mask_load_en;
   }

   ScanRegister misr[23:0] {
      ScanInSource cpu_top_rtl2_tessent_edt_c1_sib_chain_mask_i.ijtag_so;
   }

   Instance cpu_top_rtl2_tessent_edt_c1_sib_misr_i Of cpu_top_rtl2_tessent_edt_c1_sib {
      InputPort ijtag_si = cpu_top_rtl2_tessent_edt_c1_sib_chain_mask_i.ijtag_so;
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

   Attribute tessent_instrument_container = "cpu_top_rtl2_edt_lbist.instrument";
   Attribute tessent_instrument_type = "mentor::edt";
   Attribute tessent_signature = "4a36434b549f38e5e368a50b7cde7764";
}
