// Generated; edit the PMM sources, not this file.

NameSpace ;
UseNameSpace ;
Module PMU {
    ScanInPort i_pmu_data_in_ijtag_si_1 {
        Attribute tessent_design_rtl_ports = "i_pmu_data_in";
        Attribute tessent_design_gate_ports = "i_pmu_data_in";
    }
    ScanInPort I_capture_tdi_ijtag_si_2 {
        Attribute tessent_design_rtl_ports = "I_capture_tdi";
        Attribute tessent_design_gate_ports = "I_capture_tdi";
    }
    SelectPort i_pmu_configure_path_sel_ijtag_sel_1 {
        Attribute tessent_design_rtl_ports = "i_pmu_configure_path_sel";
        Attribute tessent_design_gate_ports = "i_pmu_configure_path_sel";
    }
    SelectPort i_pmu_configure_delay_sel_ijtag_sel_2 {
        Attribute tessent_design_rtl_ports = "i_pmu_configure_delay_sel";
        Attribute tessent_design_gate_ports = "i_pmu_configure_delay_sel";
    }
    SelectPort i_pmu_status_collect_ijtag_sel_3 {
        Attribute tessent_design_rtl_ports = "i_pmu_status_collect";
        Attribute tessent_design_gate_ports = "i_pmu_status_collect";
    }
    SelectPort I_capture_shift_ijtag_sel_4 {
        Attribute tessent_design_rtl_ports = "I_capture_shift";
        Attribute tessent_design_gate_ports = "I_capture_shift";
    }
    ScanOutPort o_pmu_data_out_ijtag_so_1 {
        Attribute tessent_design_rtl_ports = "o_pmu_data_out";
        Attribute tessent_design_gate_ports = "o_pmu_data_out";
        Source data_return;
    }
    ScanOutPort I_capture_tdo_ijtag_so_2 {
        Attribute tessent_design_rtl_ports = "I_capture_tdo";
        Attribute tessent_design_gate_ports = "I_capture_tdo";
        Source monitor_shadow_inst.I_capture_tdo_ijtag_so;
    }
    LogicSignal configure {
        i_pmu_configure_path_sel_ijtag_sel_1 || i_pmu_configure_delay_sel_ijtag_sel_2;
    }
    ScanMux configure_return SelectedBy i_pmu_configure_delay_sel_ijtag_sel_2 {
        1'b0 : path_sel_reg_inst.o_path_sel_so_ijtag_so;
        1'b1 : delay_selector_capture_ff.delay_select_ijtag_so_1;
    }
    ScanMux data_return SelectedBy configure {
        1'b0 : toggle_detector_inst.toggle_out_ijtag_so;
        1'b1 : configure_return;
    }
    ScanInterface PATH {
        Port i_pmu_data_in_ijtag_si_1;
        Port o_pmu_data_out_ijtag_so_1;
        Port i_pmu_configure_path_sel_ijtag_sel_1;
    }
    ScanInterface DELAY {
        Port i_pmu_data_in_ijtag_si_1;
        Port o_pmu_data_out_ijtag_so_1;
        Port i_pmu_configure_delay_sel_ijtag_sel_2;
    }
    ScanInterface COLLECT {
        Port i_pmu_data_in_ijtag_si_1;
        Port o_pmu_data_out_ijtag_so_1;
        Port i_pmu_status_collect_ijtag_sel_3;
    }
    ScanInterface CAPTURE {
        Port I_capture_tdi_ijtag_si_2;
        Port I_capture_tdo_ijtag_so_2;
        Port I_capture_shift_ijtag_sel_4;
    }
    Instance path_sel_reg_inst Of PATH_MONITOR_UNIT_CFG1_PATH_SELECT_REG_MODULE_PATH_SELECT {
        InputPort i_pmu_data_in_ijtag_si = i_pmu_data_in_ijtag_si_1;
        InputPort i_configure_path_sel_ijtag_sel = i_pmu_configure_path_sel_ijtag_sel_1;
    }
    Instance pmu_status_detector_inst Of PATH_MONITOR_UNIT_CFG1_PMU_STATUS_DETECTOR_COLLECT_STATUS {
        InputPort i_pmu_data_ijtag_si = i_pmu_data_in_ijtag_si_1;
        InputPort collect_ijtag_sel = i_pmu_status_collect_ijtag_sel_3;
    }
    Instance toggle_detector_inst Of PATH_MONITOR_UNIT_CFG1_TOGGLE_DETECTOR_COLLECT_STATUS {
        InputPort status_out_ijtag_si = pmu_status_detector_inst.o_pmu_status_ijtag_so;
        InputPort collect_ijtag_sel = i_pmu_status_collect_ijtag_sel_3;
    }
    Instance monitor_shadow_inst Of PATH_MONITOR_UNIT_CFG1_MONITOR_SHADOW_CAPTURE_SHIFT {
        InputPort capture_tdo_ijtag_si = delay_selector_capture_ff.o_capture_tdo_ijtag_so_2;
        InputPort capture_shift_ijtag_sel = I_capture_shift_ijtag_sel_4;
    }
    Instance delay_selector_capture_ff Of PATH_MONITOR_UNIT_CFG1_DELAY_SEL_CAPTURE_FF_DELAY_SELECT_CAPTURE_SHIFT {
        InputPort i_pmu_data_in_ijtag_si_1 = i_pmu_data_in_ijtag_si_1;
        InputPort i_configure_delay_sel_ijtag_sel_1 = i_pmu_configure_delay_sel_ijtag_sel_2;
        InputPort i_capture_data_ijtag_si_2 = I_capture_tdi_ijtag_si_2;
        InputPort i_capture_shift_ijtag_sel_2 = I_capture_shift_ijtag_sel_4;
    }
}
