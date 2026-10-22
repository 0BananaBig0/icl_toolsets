// Generated; edit the PMM sources, not this file.

NameSpace ;
UseNameSpace ;
Module PATH_MONITOR_UNIT_CFG1_DELAY_SEL_CAPTURE_FF_DELAY_SELECT_CAPTURE_SHIFT {
    ScanInPort i_pmu_data_in_ijtag_si_1 {
        Attribute tessent_design_rtl_ports = "i_pmu_data_in";
        Attribute tessent_design_gate_ports = "i_pmu_data_in";
    }
    SelectPort i_configure_delay_sel_ijtag_sel_1 {
        Attribute tessent_design_rtl_ports = "i_configure_delay_sel";
        Attribute tessent_design_gate_ports = "i_configure_delay_sel";
    }
    ScanOutPort delay_select_ijtag_so_1 {
        Attribute tessent_design_rtl_ports = "delay_select";
        Attribute tessent_design_gate_ports = "delay_select";
        Source o_delay_select[0];
    }
    ScanInterface client_o_delay_select {
        Port i_pmu_data_in_ijtag_si_1;
        Port delay_select_ijtag_so_1;
        Port i_configure_delay_sel_ijtag_sel_1;
    }
    ScanRegister o_delay_select[4:0] {
        ScanInSource i_pmu_data_in_ijtag_si_1;
        ResetValue 5'b0;
        Attribute synopsys_access = "RW";
    }
    ScanInPort i_capture_data_ijtag_si_2 {
        Attribute tessent_design_rtl_ports = "i_capture_data";
        Attribute tessent_design_gate_ports = "i_capture_data";
    }
    SelectPort i_capture_shift_ijtag_sel_2 {
        Attribute tessent_design_rtl_ports = "i_capture_shift";
        Attribute tessent_design_gate_ports = "i_capture_shift";
    }
    ScanOutPort o_capture_tdo_ijtag_so_2 {
        Attribute tessent_design_rtl_ports = "o_capture_tdo";
        Attribute tessent_design_gate_ports = "o_capture_tdo";
        Source capture_data_reg;
    }
    ScanInterface client_capture_data_reg {
        Port i_capture_data_ijtag_si_2;
        Port o_capture_tdo_ijtag_so_2;
        Port i_capture_shift_ijtag_sel_2;
    }
    ScanRegister capture_data_reg {
        ScanInSource i_capture_data_ijtag_si_2;
        ResetValue 1'b0;
        Attribute synopsys_access = "RO";
    }
    DataOutPort delay_select_mux_ijtag_do_1[4:0] {
        Attribute tessent_design_rtl_ports = "{delay_select_mux[4]} {delay_select_mux[3]} {delay_select_mux[2]} {delay_select_mux[1]} {delay_select_mux[0]}";
        Attribute tessent_design_gate_ports = "{delay_select_mux[4]} {delay_select_mux[3]} {delay_select_mux[2]} {delay_select_mux[1]} {delay_select_mux[0]}";
        Source o_delay_select;
    }
    DataOutPort o_capture_data_ijtag_do_2 {
        Attribute tessent_design_rtl_ports = "o_capture_data";
        Attribute tessent_design_gate_ports = "o_capture_data";
        Source capture_data_reg;
    }
}
