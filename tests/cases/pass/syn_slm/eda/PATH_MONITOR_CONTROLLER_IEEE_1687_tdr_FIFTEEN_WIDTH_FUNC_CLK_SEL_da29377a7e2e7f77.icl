// Generated; edit the PMM sources, not this file.

NameSpace ;
UseNameSpace ;
Module PATH_MONITOR_CONTROLLER_IEEE_1687_tdr_FIFTEEN_WIDTH_FUNC_CLK_SEL {
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
        Source tdr_shift_cap[0];
    }
    ScanRegister tdr_shift_cap[14:0] {
        ScanInSource i_tdr_scanin_ijtag_si;
        CaptureSource tdr_shift_cap;
        ResetValue 15'b0;
    }
    DataOutPort o_tdr_update_ijtag_do[14:0] {
        Attribute tessent_design_rtl_ports = "{o_tdr_update[14]} {o_tdr_update[13]} {o_tdr_update[12]} {o_tdr_update[11]} {o_tdr_update[10]} {o_tdr_update[9]} {o_tdr_update[8]} {o_tdr_update[7]} {o_tdr_update[6]} {o_tdr_update[5]} {o_tdr_update[4]} {o_tdr_update[3]} {o_tdr_update[2]} {o_tdr_update[1]} {o_tdr_update[0]}";
        Attribute tessent_design_gate_ports = "{o_tdr_update[14]} {o_tdr_update[13]} {o_tdr_update[12]} {o_tdr_update[11]} {o_tdr_update[10]} {o_tdr_update[9]} {o_tdr_update[8]} {o_tdr_update[7]} {o_tdr_update[6]} {o_tdr_update[5]} {o_tdr_update[4]} {o_tdr_update[3]} {o_tdr_update[2]} {o_tdr_update[1]} {o_tdr_update[0]}";
        Source tdr_shift_cap;
    }
}
