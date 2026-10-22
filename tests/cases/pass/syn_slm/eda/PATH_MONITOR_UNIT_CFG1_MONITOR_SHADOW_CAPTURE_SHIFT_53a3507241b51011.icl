// Generated; edit the PMM sources, not this file.

NameSpace ;
UseNameSpace ;
Module PATH_MONITOR_UNIT_CFG1_MONITOR_SHADOW_CAPTURE_SHIFT {
    ScanInPort capture_tdo_ijtag_si {
        Attribute tessent_design_rtl_ports = "capture_tdo";
        Attribute tessent_design_gate_ports = "capture_tdo";
    }
    SelectPort capture_shift_ijtag_sel {
        Attribute tessent_design_rtl_ports = "capture_shift";
        Attribute tessent_design_gate_ports = "capture_shift";
    }
    ScanOutPort I_capture_tdo_ijtag_so {
        Attribute tessent_design_rtl_ports = "I_capture_tdo";
        Attribute tessent_design_gate_ports = "I_capture_tdo";
        Source shadow_out;
    }
    ScanInterface client_shadow_out {
        Port capture_tdo_ijtag_si;
        Port I_capture_tdo_ijtag_so;
        Port capture_shift_ijtag_sel;
    }
    ScanRegister shadow_out {
        ScanInSource capture_tdo_ijtag_si;
        ResetValue 1'b0;
        Attribute synopsys_access = "RO";
    }
    DataOutPort o_shadow_out_ijtag_do {
        Attribute tessent_design_rtl_ports = "o_shadow_out";
        Attribute tessent_design_gate_ports = "o_shadow_out";
        Source shadow_out;
    }
}
