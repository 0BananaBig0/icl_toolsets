// Generated; edit the PMM sources, not this file.

NameSpace ;
UseNameSpace ;
Module PATH_MONITOR_UNIT_CFG1_PMU_STATUS_DETECTOR_COLLECT_STATUS {
    ScanInPort i_pmu_data_ijtag_si {
        Attribute tessent_design_rtl_ports = "i_pmu_data";
        Attribute tessent_design_gate_ports = "i_pmu_data";
    }
    SelectPort collect_ijtag_sel {
        Attribute tessent_design_rtl_ports = "collect";
        Attribute tessent_design_gate_ports = "collect";
    }
    ScanOutPort o_pmu_status_ijtag_so {
        Attribute tessent_design_rtl_ports = "o_pmu_status";
        Attribute tessent_design_gate_ports = "o_pmu_status";
        Source pmu_status_reg;
    }
    ScanInterface client_pmu_status_reg {
        Port i_pmu_data_ijtag_si;
        Port o_pmu_status_ijtag_so;
        Port collect_ijtag_sel;
    }
    ScanRegister pmu_status_reg {
        ScanInSource i_pmu_data_ijtag_si;
        ResetValue 1'b0;
        Attribute synopsys_access = "RO";
    }
}
