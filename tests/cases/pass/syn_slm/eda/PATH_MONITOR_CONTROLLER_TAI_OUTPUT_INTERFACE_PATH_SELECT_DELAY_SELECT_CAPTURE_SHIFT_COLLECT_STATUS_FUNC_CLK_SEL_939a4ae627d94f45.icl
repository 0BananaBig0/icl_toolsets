// Generated; edit the PMM sources, not this file.

NameSpace ;
UseNameSpace ;
Module PATH_MONITOR_CONTROLLER_TAI_OUTPUT_INTERFACE_PATH_SELECT_DELAY_SELECT_CAPTURE_SHIFT_COLLECT_STATUS_FUNC_CLK_SEL {
    // Falling-edge holding copies: Sources describe settled values, not cycle timing.
    ScanMux capture_serial_1 SelectedBy i_pmu_capture_ff_shift_ijtag_di_5[1] {
        1'b0 : 1'b0;
        1'b1 : i_pmu_capture_ff_tdi_ijtag_si_1[1];
    }
    ScanMux i_delay_select_serial_1 SelectedBy i_configure_delay_sel_ijtag_di_2[1] {
        1'b0 : 1'b0;
        1'b1 : i_delay_select_ijtag_si_2[1];
    }
    ScanMux i_path_select_serial_1 SelectedBy i_configure_path_sel_ijtag_di_4[1] {
        1'b0 : i_delay_select_serial_1;
        1'b1 : i_path_select_ijtag_si_3[1];
    }
    ScanMux pmu_status_in_serial_1 SelectedBy i_pmu_collect_status_ijtag_di_3[1] {
        1'b0 : i_path_select_serial_1;
        1'b1 : pmu_status_in_ijtag_si_4[1];
    }
    TCKPort i_clk_ijtag_tck {
        Attribute tessent_design_rtl_ports = "i_clk";
        Attribute tessent_design_gate_ports = "i_clk";
    }
    ResetPort i_rstn_ijtag_reset {
        Attribute tessent_design_rtl_ports = "i_rstn";
        Attribute tessent_design_gate_ports = "i_rstn";
        ActivePolarity 0;
    }
    DataInPort i_pmu_func_clk_gate_en_ijtag_di_1[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_func_clk_gate_en[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_func_clk_gate_en[1]}";
    }
    DataInPort i_configure_delay_sel_ijtag_di_2[1:1] {
        Attribute tessent_design_rtl_ports = "{i_configure_delay_sel[1]}";
        Attribute tessent_design_gate_ports = "{i_configure_delay_sel[1]}";
    }
    DataInPort i_pmu_collect_status_ijtag_di_3[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_collect_status[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_collect_status[1]}";
    }
    ScanInPort i_pmu_capture_ff_tdi_ijtag_si_1[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_capture_ff_tdi[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_capture_ff_tdi[1]}";
    }
    DataInPort i_configure_path_sel_ijtag_di_4[1:1] {
        Attribute tessent_design_rtl_ports = "{i_configure_path_sel[1]}";
        Attribute tessent_design_gate_ports = "{i_configure_path_sel[1]}";
    }
    ScanInPort i_delay_select_ijtag_si_2[1:1] {
        Attribute tessent_design_rtl_ports = "{i_delay_select[1]}";
        Attribute tessent_design_gate_ports = "{i_delay_select[1]}";
    }
    ScanInPort i_path_select_ijtag_si_3[1:1] {
        Attribute tessent_design_rtl_ports = "{i_path_select[1]}";
        Attribute tessent_design_gate_ports = "{i_path_select[1]}";
    }
    ScanInPort pmu_status_in_ijtag_si_4[1:1] {
        Attribute tessent_design_rtl_ports = "{pmu_status_in[1]}";
        Attribute tessent_design_gate_ports = "{pmu_status_in[1]}";
    }
    DataInPort i_pmu_capture_ff_shift_ijtag_di_5[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_capture_ff_shift[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_capture_ff_shift[1]}";
    }
    ToSelectPort o_configure_path_sel_loop_ijtag_to_sel_1[1:1] {
        Attribute tessent_design_rtl_ports = "{o_configure_path_sel_loop[1]}";
        Attribute tessent_design_gate_ports = "{o_configure_path_sel_loop[1]}";
        Source i_configure_path_sel_ijtag_di_4;
    }
    ToSelectPort o_pmu_collect_status_loop_ijtag_to_sel_2[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_collect_status_loop[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_collect_status_loop[1]}";
        Source i_pmu_collect_status_ijtag_di_3;
    }
    ToSelectPort o_configure_delay_sel_loop_ijtag_to_sel_3[1:1] {
        Attribute tessent_design_rtl_ports = "{o_configure_delay_sel_loop[1]}";
        Attribute tessent_design_gate_ports = "{o_configure_delay_sel_loop[1]}";
        Source i_configure_delay_sel_ijtag_di_2;
    }
    DataOutPort o_pmu_func_clk_gate_en_loop_ijtag_do[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_func_clk_gate_en_loop[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_func_clk_gate_en_loop[1]}";
        Source i_pmu_func_clk_gate_en_ijtag_di_1;
    }
    ScanOutPort o_pmu_data_in_ijtag_to_si_1[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_data_in[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_data_in[1]}";
        Source pmu_status_in_serial_1;
    }
    ToSelectPort o_pmu_capture_ff_shift_ijtag_to_sel_4[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_shift[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_shift[1]}";
        Source i_pmu_capture_ff_shift_ijtag_di_5;
    }
    ScanOutPort o_pmu_capture_ff_tdi_ijtag_to_si_2[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_tdi[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_tdi[1]}";
        Source capture_serial_1;
    }
    ScanInterface host_capture1 {
        Port o_pmu_capture_ff_tdi_ijtag_to_si_2[1];
        Port o_pmu_capture_ff_shift_ijtag_to_sel_4[1];
    }
    ScanInterface host_o_configure_path_sel_loop1 {
        Port o_pmu_data_in_ijtag_to_si_1[1];
        Port o_configure_path_sel_loop_ijtag_to_sel_1[1];
    }
    ScanInterface host_o_configure_delay_sel_loop1 {
        Port o_pmu_data_in_ijtag_to_si_1[1];
        Port o_configure_delay_sel_loop_ijtag_to_sel_3[1];
    }
    ScanInterface host_o_pmu_collect_status_loop1 {
        Port o_pmu_data_in_ijtag_to_si_1[1];
        Port o_pmu_collect_status_loop_ijtag_to_sel_2[1];
    }
}
