// Generated; edit the PMM sources, not this file.

NameSpace ;
UseNameSpace ;
Module PATH_MONITOR_UNIT_CFG1_TOGGLE_DETECTOR_COLLECT_STATUS {
    ScanInPort status_out_ijtag_si {
        Attribute tessent_design_rtl_ports = "status_out";
        Attribute tessent_design_gate_ports = "status_out";
    }
    SelectPort collect_ijtag_sel {
        Attribute tessent_design_rtl_ports = "collect";
        Attribute tessent_design_gate_ports = "collect";
    }
    ScanOutPort toggle_out_ijtag_so {
        Attribute tessent_design_rtl_ports = "toggle_out";
        Attribute tessent_design_gate_ports = "toggle_out";
        Source in_toggle_out;
    }
    ScanInterface client_in_toggle_out {
        Port status_out_ijtag_si;
        Port toggle_out_ijtag_so;
        Port collect_ijtag_sel;
    }
    ScanRegister in_toggle_out {
        ScanInSource status_out_ijtag_si;
        ResetValue 1'b0;
        Attribute synopsys_access = "RO";
    }
}
