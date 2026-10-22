// Generated; edit the PMM sources, not this file.

NameSpace ;
UseNameSpace ;
Module PMC {
    ShiftEnPort Shift_ijtag_se {
        Attribute tessent_design_rtl_ports = "Shift";
        Attribute tessent_design_gate_ports = "Shift";
    }
    CaptureEnPort Capture_ijtag_ce {
        Attribute tessent_design_rtl_ports = "Capture";
        Attribute tessent_design_gate_ports = "Capture";
    }
    UpdateEnPort Update_ijtag_ue {
        Attribute tessent_design_rtl_ports = "Update";
        Attribute tessent_design_gate_ports = "Update";
    }
    SelectPort Select_ijtag_sel {
        Attribute tessent_design_rtl_ports = "Select";
        Attribute tessent_design_gate_ports = "Select";
    }
    ResetPort TRSTN_ijtag_reset {
        Attribute tessent_design_rtl_ports = "TRSTN";
        Attribute tessent_design_gate_ports = "TRSTN";
        ActivePolarity 0;
    }
    TCKPort TCK_ijtag_tck {
        Attribute tessent_design_rtl_ports = "TCK";
        Attribute tessent_design_gate_ports = "TCK";
    }
    ScanInPort TDI_ijtag_si_1 {
        Attribute tessent_design_rtl_ports = "TDI";
        Attribute tessent_design_gate_ports = "TDI";
    }
    ScanOutPort TDO_ijtag_so {
        Attribute tessent_design_rtl_ports = "TDO";
        Attribute tessent_design_gate_ports = "TDO";
        Source path_monitor_controller_tai_inst.TDO_ijtag_so;
    }
    ScanInterface client {
        Port TDI_ijtag_si_1;
        Port TDO_ijtag_so;
        Port Shift_ijtag_se;
        Port Capture_ijtag_ce;
        Port Update_ijtag_ue;
        Port Select_ijtag_sel;
        Port TRSTN_ijtag_reset;
        Port TCK_ijtag_tck;
    }
    ScanInPort i_pmc2pmu_data_out_loop_ijtag_si_2[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmc2pmu_data_out_loop[1]}";
        Attribute tessent_design_gate_ports = "{i_pmc2pmu_data_out_loop[1]}";
    }
    ScanInPort i_pmc2pmu_capture_ff_tdo_ijtag_si_3[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmc2pmu_capture_ff_tdo[1]}";
        Attribute tessent_design_gate_ports = "{i_pmc2pmu_capture_ff_tdo[1]}";
    }
    ScanOutPort o_pmc2pmu_data_in_ijtag_to_si_1[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmc2pmu_data_in[1]}";
        Attribute tessent_design_gate_ports = "{o_pmc2pmu_data_in[1]}";
        Source path_monitor_controller_tai_inst.o_pmu_data_in_ijtag_to_si_1;
    }
    ScanOutPort o_pmc2pmu_capture_ff_tdi_ijtag_to_si_2[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmc2pmu_capture_ff_tdi[1]}";
        Attribute tessent_design_gate_ports = "{o_pmc2pmu_capture_ff_tdi[1]}";
        Source path_monitor_controller_tai_inst.o_pmu_capture_ff_tdi_ijtag_to_si_2;
    }
    ToSelectPort o_pmc2pmu_configure_path_sel_loop_ijtag_to_sel_1[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmc2pmu_configure_path_sel_loop[1]}";
        Attribute tessent_design_gate_ports = "{o_pmc2pmu_configure_path_sel_loop[1]}";
        Source path_monitor_controller_tai_inst.o_configure_path_sel_loop_ijtag_to_sel_1;
    }
    ToSelectPort o_pmc2pmu_configure_delay_sel_loop_ijtag_to_sel_2[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmc2pmu_configure_delay_sel_loop[1]}";
        Attribute tessent_design_gate_ports = "{o_pmc2pmu_configure_delay_sel_loop[1]}";
        Source path_monitor_controller_tai_inst.o_configure_delay_sel_loop_ijtag_to_sel_3;
    }
    ToSelectPort o_pmc2pmu_collect_status_loop_ijtag_to_sel_3[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmc2pmu_collect_status_loop[1]}";
        Attribute tessent_design_gate_ports = "{o_pmc2pmu_collect_status_loop[1]}";
        Source path_monitor_controller_tai_inst.o_pmu_collect_status_loop_ijtag_to_sel_2;
    }
    ToSelectPort o_pmc2pmu_capture_ff_shift_ijtag_to_sel_4[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmc2pmu_capture_ff_shift[1]}";
        Attribute tessent_design_gate_ports = "{o_pmc2pmu_capture_ff_shift[1]}";
        Source path_monitor_controller_tai_inst.o_pmu_capture_ff_shift_ijtag_to_sel_4;
    }
    Instance path_monitor_controller_tai_inst Of PATH_MONITOR_CONTROLLER_PATH_MONITOR_CONTROLLER_TAI {
        InputPort Shift_ijtag_se = Shift_ijtag_se;
        InputPort Capture_ijtag_ce = Capture_ijtag_ce;
        InputPort Update_ijtag_ue = Update_ijtag_ue;
        InputPort TRSTN_ijtag_reset = TRSTN_ijtag_reset;
        InputPort TCK_ijtag_tck = TCK_ijtag_tck;
        InputPort TDI_ijtag_si = TDI_ijtag_si_1;
        InputPort Select_ijtag_sel = Select_ijtag_sel;
        InputPort i_pmu_data_loop_ijtag_from_so_1 = i_pmc2pmu_data_out_loop_ijtag_si_2;
        InputPort i_pmu_capture_ff_tdo_ijtag_from_so_2 = i_pmc2pmu_capture_ff_tdo_ijtag_si_3;
    }
}
