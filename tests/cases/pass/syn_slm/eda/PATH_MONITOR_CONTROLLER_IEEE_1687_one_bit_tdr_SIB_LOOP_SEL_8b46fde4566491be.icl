// Generated; edit the PMM sources, not this file.

NameSpace ;
UseNameSpace ;
Module PATH_MONITOR_CONTROLLER_IEEE_1687_one_bit_tdr_SIB_LOOP_SEL {
    TCKPort i_tck_ijtag_tck {
        Attribute tessent_design_rtl_ports = "i_tck";
        Attribute tessent_design_gate_ports = "i_tck";
    }
    ResetPort i_resetn_ijtag_reset {
        Attribute tessent_design_rtl_ports = "i_resetn";
        Attribute tessent_design_gate_ports = "i_resetn";
        ActivePolarity 0;
    }
    ShiftEnPort i_tdr_sibShiftEn_ijtag_se {
        Attribute tessent_design_rtl_ports = "i_tdr_sibShiftEn";
        Attribute tessent_design_gate_ports = "i_tdr_sibShiftEn";
    }
    CaptureEnPort i_tdr_sibCaptureEn_ijtag_ce {
        Attribute tessent_design_rtl_ports = "i_tdr_sibCaptureEn";
        Attribute tessent_design_gate_ports = "i_tdr_sibCaptureEn";
    }
    UpdateEnPort i_tdr_sibUpdateEn_ijtag_ue {
        Attribute tessent_design_rtl_ports = "i_tdr_sibUpdateEn";
        Attribute tessent_design_gate_ports = "i_tdr_sibUpdateEn";
    }
    ScanInPort i_tdr_scanin_ijtag_si {
        Attribute tessent_design_rtl_ports = "i_tdr_scanin";
        Attribute tessent_design_gate_ports = "i_tdr_scanin";
    }
    ScanOutPort o_tdr_scanout_ijtag_so {
        Attribute tessent_design_rtl_ports = "o_tdr_scanout";
        Attribute tessent_design_gate_ports = "o_tdr_scanout";
        Source tdr_shift_cap;
    }
    ScanRegister tdr_shift_cap {
        ScanInSource i_tdr_scanin_ijtag_si;
        CaptureSource tdr_shift_cap;
        ResetValue 1'b1;
    }
    DataOutPort o_tdr_update_ijtag_do {
        Attribute tessent_design_rtl_ports = "o_tdr_update";
        Attribute tessent_design_gate_ports = "o_tdr_update";
        Source tdr_shift_cap;
    }
}
