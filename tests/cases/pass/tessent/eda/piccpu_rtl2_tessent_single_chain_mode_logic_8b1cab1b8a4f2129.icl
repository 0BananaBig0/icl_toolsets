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
Module piccpu_rtl2_tessent_single_chain_mode_logic { // {{{
    ScanInPort          ijtag_si;
    ScanOutPort         ijtag_so            { Source single_chain_sib_i.ijtag_so; }
    TCKPort             ijtag_tck;
    ResetPort           ijtag_reset         { ActivePolarity 0; }
    SelectPort          ijtag_sel;
    CaptureEnPort       ijtag_ce;
    ShiftEnPort         ijtag_se;
    UpdateEnPort        ijtag_ue;
    DataInPort          edt_single_bypass_chain_in { Attribute tessent_no_input_constraints = "on"; Attribute connection_rule_option = "allowed_tied_low"; }
    DataOutPort         edt_single_bypass_chain_out { Source edt_single_bypass_chain_ctrl; }
 
    LogicSignal edt_single_bypass_chain_ctrl {
        edt_single_bypass_chain_in || tdr_single_bypass;
    }
 
    ScanRegister tdr_single_bypass {
        ScanInSource    ijtag_si;
        ResetValue      1'b0;
    }
 
    Instance tdr_sib_i Of piccpu_rtl2_tessent_single_chain_mode_logic_sib {
        InputPort ijtag_tck     = ijtag_tck;
        InputPort ijtag_si      = ijtag_si;
        InputPort ijtag_reset   = ijtag_reset;
        InputPort ijtag_sel     = ijtag_sel;
        InputPort ijtag_ce      = ijtag_ce;
        InputPort ijtag_se      = ijtag_se;
        InputPort ijtag_ue      = ijtag_ue;
        InputPort ijtag_from_so = tdr_single_bypass;
    }
 
    Instance piccpu_rtl2_tessent_edt_internal_scan_registers_i Of piccpu_rtl2_tessent_edt_internal_scan_registers {
        InputPort ijtag_si      = tdr_sib_i.ijtag_so;
        InputPort ijtag_tck     = ijtag_tck;
        InputPort ijtag_reset   = ijtag_reset;
        InputPort ijtag_sel     = single_chain_sib_i.ijtag_to_sel;
        InputPort ijtag_ce      = ijtag_ce;
        InputPort ijtag_se      = ijtag_se;
        InputPort ijtag_ue      = ijtag_ue;
        InputPort edt_single_bypass_chain = tdr_single_bypass;
    }
 
    Instance single_chain_sib_i Of piccpu_rtl2_tessent_single_chain_mode_logic_sib {
        InputPort ijtag_tck     = ijtag_tck;
        InputPort ijtag_si      = tdr_sib_i.ijtag_so;
        InputPort ijtag_reset   = ijtag_reset;
        InputPort ijtag_sel     = ijtag_sel;
        InputPort ijtag_ce      = ijtag_ce;
        InputPort ijtag_se      = ijtag_se;
        InputPort ijtag_ue      = ijtag_ue;
        InputPort ijtag_from_so = piccpu_rtl2_tessent_edt_internal_scan_registers_i.ijtag_so;
    }
 
    Attribute keep_active_during_scan_test = "true";
    Attribute tessent_instrument_container = "piccpu_rtl2_lbist.instrument";
    Attribute tessent_instrument_type    = "mentor::logic_bist";
    Attribute tessent_instrument_subtype = "single_chain_mode_logic";
    Attribute tessent_use_in_dft_specification = "false";
    Attribute tessent_signature          = "17efc9227cfa3834389b87ad3dc4210a";
}
