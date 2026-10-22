// Generated; edit the PMM sources, not this file.

NameSpace ;
UseNameSpace ;
Module PATH_MONITOR_CONTROLLER_PMM_1687_SIB_REG_3_SIB_2_SIB_1 {
    TCKPort clk_ijtag_tck {
        Attribute tessent_design_rtl_ports = "clk";
        Attribute tessent_design_gate_ports = "clk";
    }
    ResetPort TRSTN_ijtag_reset {
        Attribute tessent_design_rtl_ports = "TRSTN";
        Attribute tessent_design_gate_ports = "TRSTN";
        ActivePolarity 0;
    }
    SelectPort Select_ijtag_sel {
        Attribute tessent_design_rtl_ports = "Select";
        Attribute tessent_design_gate_ports = "Select";
    }
    ShiftEnPort ShiftEn_ijtag_se {
        Attribute tessent_design_rtl_ports = "ShiftEn";
        Attribute tessent_design_gate_ports = "ShiftEn";
    }
    CaptureEnPort CaptureEn_ijtag_ce {
        Attribute tessent_design_rtl_ports = "CaptureEn";
        Attribute tessent_design_gate_ports = "CaptureEn";
    }
    UpdateEnPort UpdateEn_ijtag_ue {
        Attribute tessent_design_rtl_ports = "UpdateEn";
        Attribute tessent_design_gate_ports = "UpdateEn";
    }
    ScanInPort TDI_ijtag_si {
        Attribute tessent_design_rtl_ports = "TDI";
        Attribute tessent_design_gate_ports = "TDI";
    }
    ScanOutPort TDO_ijtag_so {
        Attribute tessent_design_rtl_ports = "TDO";
        Attribute tessent_design_gate_ports = "TDO";
        Source TDR_shift[0];
    }
    ScanRegister TDR_shift[2:0] {
        ScanInSource TDI_ijtag_si;
        CaptureSource TDR_shift;
        ResetValue 3'b0;
    }
    DataOutPort TDR_ijtag_do_1[2:0] {
        Attribute tessent_design_rtl_ports = "{TDR[2]} {TDR[1]} {TDR[0]}";
        Attribute tessent_design_gate_ports = "{TDR[2]} {TDR[1]} {TDR[0]}";
        Source TDR_shift;
    }
    DataOutPort sibSelect_reg_ijtag_do_2[2:0] {
        Attribute tessent_design_rtl_ports = "{sibSelect_reg[2]} {sibSelect_reg[1]} {sibSelect_reg[0]}";
        Attribute tessent_design_gate_ports = "{sibSelect_reg[2]} {sibSelect_reg[1]} {sibSelect_reg[0]}";
        Source TDR_shift;
    }
}
