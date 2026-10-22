// Generated; edit the PMM sources, not this file.

NameSpace ;
UseNameSpace ;
Module PATH_MONITOR_UNIT_CFG1_PATH_SELECT_REG_MODULE_PATH_SELECT {
    ScanInPort i_pmu_data_in_ijtag_si {
        Attribute tessent_design_rtl_ports = "i_pmu_data_in";
        Attribute tessent_design_gate_ports = "i_pmu_data_in";
    }
    SelectPort i_configure_path_sel_ijtag_sel {
        Attribute tessent_design_rtl_ports = "i_configure_path_sel";
        Attribute tessent_design_gate_ports = "i_configure_path_sel";
    }
    ScanOutPort o_path_sel_so_ijtag_so {
        Attribute tessent_design_rtl_ports = "o_path_sel_so";
        Attribute tessent_design_gate_ports = "o_path_sel_so";
        Source path_sel_reg[0];
    }
    ScanInterface client_path_sel_reg {
        Port i_pmu_data_in_ijtag_si;
        Port o_path_sel_so_ijtag_so;
        Port i_configure_path_sel_ijtag_sel;
    }
    ScanRegister path_sel_reg[2:0] {
        ScanInSource i_pmu_data_in_ijtag_si;
        ResetValue 3'b0;
        Attribute synopsys_access = "RW";
    }
    DataOutPort o_path_select_ijtag_do[2:0] {
        Attribute tessent_design_rtl_ports = "{o_path_select[2]} {o_path_select[1]} {o_path_select[0]}";
        Attribute tessent_design_gate_ports = "{o_path_select[2]} {o_path_select[1]} {o_path_select[0]}";
        Source path_sel_reg;
    }
}
