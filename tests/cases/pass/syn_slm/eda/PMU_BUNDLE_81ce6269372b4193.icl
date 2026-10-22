// Generated; edit the PMM sources, not this file.

NameSpace ;
UseNameSpace ;
Module PMU_BUNDLE {
    SelectPort pmc2pmu_configure_path_sel_ijtag_sel_1[1:1] {
        Attribute tessent_design_rtl_ports = "{pmc2pmu_configure_path_sel[1]}";
        Attribute tessent_design_gate_ports = "{pmc2pmu_configure_path_sel[1]}";
    }
    SelectPort pmc2pmu_configure_delay_sel_ijtag_sel_2[1:1] {
        Attribute tessent_design_rtl_ports = "{pmc2pmu_configure_delay_sel[1]}";
        Attribute tessent_design_gate_ports = "{pmc2pmu_configure_delay_sel[1]}";
    }
    SelectPort pmc2pmu_collect_status_ijtag_sel_3[1:1] {
        Attribute tessent_design_rtl_ports = "{pmc2pmu_collect_status[1]}";
        Attribute tessent_design_gate_ports = "{pmc2pmu_collect_status[1]}";
    }
    SelectPort pmc2pmu_capture_ff_shift_ijtag_sel_4[1:1] {
        Attribute tessent_design_rtl_ports = "{pmc2pmu_capture_ff_shift[1]}";
        Attribute tessent_design_gate_ports = "{pmc2pmu_capture_ff_shift[1]}";
    }
    ScanInPort pmc2pmu_data_in_ijtag_si_1[1:1] {
        Attribute tessent_design_rtl_ports = "{pmc2pmu_data_in[1]}";
        Attribute tessent_design_gate_ports = "{pmc2pmu_data_in[1]}";
    }
    ScanOutPort pmu2pmc_data_out_ijtag_so_1[1:1] {
        Attribute tessent_design_rtl_ports = "{pmu2pmc_data_out[1]}";
        Attribute tessent_design_gate_ports = "{pmu2pmc_data_out[1]}";
        Source path_monitor_unit5_loop1.o_pmu_data_out_ijtag_so_1;
    }
    ScanInPort pmc2pmu_capture_ff_tdi_ijtag_si_2[1:1] {
        Attribute tessent_design_rtl_ports = "{pmc2pmu_capture_ff_tdi[1]}";
        Attribute tessent_design_gate_ports = "{pmc2pmu_capture_ff_tdi[1]}";
    }
    ScanOutPort pmu2pmc_capture_ff_tdo_ijtag_so_2[1:1] {
        Attribute tessent_design_rtl_ports = "{pmu2pmc_capture_ff_tdo[1]}";
        Attribute tessent_design_gate_ports = "{pmu2pmc_capture_ff_tdo[1]}";
        Source path_monitor_unit5_loop1.I_capture_tdo_ijtag_so_2;
    }
    ScanInterface client_pmc2pmu_configure_path_sel1 {
        Port pmc2pmu_data_in_ijtag_si_1[1];
        Port pmu2pmc_data_out_ijtag_so_1[1];
        Port pmc2pmu_configure_path_sel_ijtag_sel_1[1];
    }
    ScanInterface client_pmc2pmu_configure_delay_sel1 {
        Port pmc2pmu_data_in_ijtag_si_1[1];
        Port pmu2pmc_data_out_ijtag_so_1[1];
        Port pmc2pmu_configure_delay_sel_ijtag_sel_2[1];
    }
    ScanInterface client_pmc2pmu_collect_status1 {
        Port pmc2pmu_data_in_ijtag_si_1[1];
        Port pmu2pmc_data_out_ijtag_so_1[1];
        Port pmc2pmu_collect_status_ijtag_sel_3[1];
    }
    ScanInterface client_pmc2pmu_capture_ff_shift1 {
        Port pmc2pmu_capture_ff_tdi_ijtag_si_2[1];
        Port pmu2pmc_capture_ff_tdo_ijtag_so_2[1];
        Port pmc2pmu_capture_ff_shift_ijtag_sel_4[1];
    }
    Instance path_monitor_unit0_loop1 Of PMU {
        InputPort i_pmu_configure_path_sel_ijtag_sel_1 = pmc2pmu_configure_path_sel_ijtag_sel_1[1];
        InputPort i_pmu_configure_delay_sel_ijtag_sel_2 = pmc2pmu_configure_delay_sel_ijtag_sel_2[1];
        InputPort i_pmu_status_collect_ijtag_sel_3 = pmc2pmu_collect_status_ijtag_sel_3[1];
        InputPort I_capture_shift_ijtag_sel_4 = pmc2pmu_capture_ff_shift_ijtag_sel_4[1];
        InputPort i_pmu_data_in_ijtag_si_1 = pmc2pmu_data_in_ijtag_si_1[1];
        InputPort I_capture_tdi_ijtag_si_2 = pmc2pmu_capture_ff_tdi_ijtag_si_2[1];
    }
    Instance path_monitor_unit1_loop1 Of PMU {
        InputPort i_pmu_configure_path_sel_ijtag_sel_1 = pmc2pmu_configure_path_sel_ijtag_sel_1[1];
        InputPort i_pmu_configure_delay_sel_ijtag_sel_2 = pmc2pmu_configure_delay_sel_ijtag_sel_2[1];
        InputPort i_pmu_status_collect_ijtag_sel_3 = pmc2pmu_collect_status_ijtag_sel_3[1];
        InputPort I_capture_shift_ijtag_sel_4 = pmc2pmu_capture_ff_shift_ijtag_sel_4[1];
        InputPort i_pmu_data_in_ijtag_si_1 = path_monitor_unit0_loop1.o_pmu_data_out_ijtag_so_1;
        InputPort I_capture_tdi_ijtag_si_2 = path_monitor_unit0_loop1.I_capture_tdo_ijtag_so_2;
    }
    Instance path_monitor_unit2_loop1 Of PMU {
        InputPort i_pmu_configure_path_sel_ijtag_sel_1 = pmc2pmu_configure_path_sel_ijtag_sel_1[1];
        InputPort i_pmu_configure_delay_sel_ijtag_sel_2 = pmc2pmu_configure_delay_sel_ijtag_sel_2[1];
        InputPort i_pmu_status_collect_ijtag_sel_3 = pmc2pmu_collect_status_ijtag_sel_3[1];
        InputPort I_capture_shift_ijtag_sel_4 = pmc2pmu_capture_ff_shift_ijtag_sel_4[1];
        InputPort i_pmu_data_in_ijtag_si_1 = path_monitor_unit1_loop1.o_pmu_data_out_ijtag_so_1;
        InputPort I_capture_tdi_ijtag_si_2 = path_monitor_unit1_loop1.I_capture_tdo_ijtag_so_2;
    }
    Instance path_monitor_unit3_loop1 Of PMU {
        InputPort i_pmu_configure_path_sel_ijtag_sel_1 = pmc2pmu_configure_path_sel_ijtag_sel_1[1];
        InputPort i_pmu_configure_delay_sel_ijtag_sel_2 = pmc2pmu_configure_delay_sel_ijtag_sel_2[1];
        InputPort i_pmu_status_collect_ijtag_sel_3 = pmc2pmu_collect_status_ijtag_sel_3[1];
        InputPort I_capture_shift_ijtag_sel_4 = pmc2pmu_capture_ff_shift_ijtag_sel_4[1];
        InputPort i_pmu_data_in_ijtag_si_1 = path_monitor_unit2_loop1.o_pmu_data_out_ijtag_so_1;
        InputPort I_capture_tdi_ijtag_si_2 = path_monitor_unit2_loop1.I_capture_tdo_ijtag_so_2;
    }
    Instance path_monitor_unit4_loop1 Of PMU {
        InputPort i_pmu_configure_path_sel_ijtag_sel_1 = pmc2pmu_configure_path_sel_ijtag_sel_1[1];
        InputPort i_pmu_configure_delay_sel_ijtag_sel_2 = pmc2pmu_configure_delay_sel_ijtag_sel_2[1];
        InputPort i_pmu_status_collect_ijtag_sel_3 = pmc2pmu_collect_status_ijtag_sel_3[1];
        InputPort I_capture_shift_ijtag_sel_4 = pmc2pmu_capture_ff_shift_ijtag_sel_4[1];
        InputPort i_pmu_data_in_ijtag_si_1 = path_monitor_unit3_loop1.o_pmu_data_out_ijtag_so_1;
        InputPort I_capture_tdi_ijtag_si_2 = path_monitor_unit3_loop1.I_capture_tdo_ijtag_so_2;
    }
    Instance path_monitor_unit5_loop1 Of PMU {
        InputPort i_pmu_configure_path_sel_ijtag_sel_1 = pmc2pmu_configure_path_sel_ijtag_sel_1[1];
        InputPort i_pmu_configure_delay_sel_ijtag_sel_2 = pmc2pmu_configure_delay_sel_ijtag_sel_2[1];
        InputPort i_pmu_status_collect_ijtag_sel_3 = pmc2pmu_collect_status_ijtag_sel_3[1];
        InputPort I_capture_shift_ijtag_sel_4 = pmc2pmu_capture_ff_shift_ijtag_sel_4[1];
        InputPort i_pmu_data_in_ijtag_si_1 = path_monitor_unit4_loop1.o_pmu_data_out_ijtag_so_1;
        InputPort I_capture_tdi_ijtag_si_2 = path_monitor_unit4_loop1.I_capture_tdo_ijtag_so_2;
    }
}
