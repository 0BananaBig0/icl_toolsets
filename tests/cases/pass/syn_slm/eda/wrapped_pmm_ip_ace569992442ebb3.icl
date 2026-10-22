// Generated; edit the PMM sources, not this file.

NameSpace ;
UseNameSpace ;
Module wrapped_pmm_ip {
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
    ScanInPort TDI_ijtag_si {
        Attribute tessent_design_rtl_ports = "TDI";
        Attribute tessent_design_gate_ports = "TDI";
    }
    ScanOutPort TDO_ijtag_so {
        Attribute tessent_design_rtl_ports = "TDO";
        Attribute tessent_design_gate_ports = "TDO";
        Source path_monitor_controller_inst.TDO_ijtag_so;
    }
    ScanInterface client {
        Port TDI_ijtag_si;
        Port TDO_ijtag_so;
        Port Shift_ijtag_se;
        Port Capture_ijtag_ce;
        Port Update_ijtag_ue;
        Port Select_ijtag_sel;
        Port TRSTN_ijtag_reset;
        Port TCK_ijtag_tck;
    }
    Instance path_monitor_controller_inst Of PMC {
        InputPort Shift_ijtag_se = Shift_ijtag_se;
        InputPort Capture_ijtag_ce = Capture_ijtag_ce;
        InputPort Update_ijtag_ue = Update_ijtag_ue;
        InputPort TRSTN_ijtag_reset = TRSTN_ijtag_reset;
        InputPort TCK_ijtag_tck = TCK_ijtag_tck;
        InputPort TDI_ijtag_si_1 = TDI_ijtag_si;
        InputPort Select_ijtag_sel = Select_ijtag_sel;
        InputPort i_pmc2pmu_data_out_loop_ijtag_si_2 = pmu_bundle_inst.pmu2pmc_data_out_ijtag_so_1;
        InputPort i_pmc2pmu_capture_ff_tdo_ijtag_si_3 = pmu_bundle_inst.pmu2pmc_capture_ff_tdo_ijtag_so_2;
    }
    Instance pmu_bundle_inst Of PMU_BUNDLE {
        InputPort pmc2pmu_data_in_ijtag_si_1 = path_monitor_controller_inst.o_pmc2pmu_data_in_ijtag_to_si_1;
        InputPort pmc2pmu_capture_ff_tdi_ijtag_si_2 = path_monitor_controller_inst.o_pmc2pmu_capture_ff_tdi_ijtag_to_si_2;
        InputPort pmc2pmu_configure_path_sel_ijtag_sel_1 = path_monitor_controller_inst.o_pmc2pmu_configure_path_sel_loop_ijtag_to_sel_1;
        InputPort pmc2pmu_configure_delay_sel_ijtag_sel_2 = path_monitor_controller_inst.o_pmc2pmu_configure_delay_sel_loop_ijtag_to_sel_2;
        InputPort pmc2pmu_collect_status_ijtag_sel_3 = path_monitor_controller_inst.o_pmc2pmu_collect_status_loop_ijtag_to_sel_3;
        InputPort pmc2pmu_capture_ff_shift_ijtag_sel_4 = path_monitor_controller_inst.o_pmc2pmu_capture_ff_shift_ijtag_to_sel_4;
    }
}
