// Generated; edit the PMM sources, not this file.

NameSpace ;
UseNameSpace ;
Module PATH_MONITOR_CONTROLLER_PATH_MONITOR_CONTROLLER_TAI {
    TCKPort TCK_ijtag_tck {
        Attribute tessent_design_rtl_ports = "TCK";
        Attribute tessent_design_gate_ports = "TCK";
    }
    ResetPort TRSTN_ijtag_reset {
        Attribute tessent_design_rtl_ports = "TRSTN";
        Attribute tessent_design_gate_ports = "TRSTN";
        ActivePolarity 0;
    }
    ShiftEnPort Shift_ijtag_se {
        Attribute tessent_design_rtl_ports = "Shift";
        Attribute tessent_design_gate_ports = "Shift";
    }
    UpdateEnPort Update_ijtag_ue {
        Attribute tessent_design_rtl_ports = "Update";
        Attribute tessent_design_gate_ports = "Update";
    }
    CaptureEnPort Capture_ijtag_ce {
        Attribute tessent_design_rtl_ports = "Capture";
        Attribute tessent_design_gate_ports = "Capture";
    }
    SelectPort Select_ijtag_sel {
        Attribute tessent_design_rtl_ports = "Select";
        Attribute tessent_design_gate_ports = "Select";
    }
    ScanInPort TDI_ijtag_si {
        Attribute tessent_design_rtl_ports = "TDI";
        Attribute tessent_design_gate_ports = "TDI";
    }
    ScanOutPort TDO_ijtag_so {
        Attribute tessent_design_rtl_ports = "TDO";
        Attribute tessent_design_gate_ports = "TDO";
        Source IEEE_1687_module_inst.o_scanout_ijtag_so;
    }
    ScanInPort i_pmu_data_loop_ijtag_from_so_1[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_data_loop[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_data_loop[1]}";
    }
    ScanOutPort o_pmu_data_in_ijtag_to_si_1[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_data_in[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_data_in[1]}";
        Source tai_output_interface_inst.o_pmu_data_in_ijtag_to_si_1;
    }
    DataOutPort o_wrck_clk_mux_en_loop_ijtag_do_1[1:1] {
        Attribute tessent_design_rtl_ports = "{o_wrck_clk_mux_en_loop[1]}";
        Attribute tessent_design_gate_ports = "{o_wrck_clk_mux_en_loop[1]}";
        Source clk_cntrl_inst.o_wrck_clk_mux_en_ijtag_do_1;
    }
    ToSelectPort o_configure_path_sel_loop_ijtag_to_sel_1[1:1] {
        Attribute tessent_design_rtl_ports = "{o_configure_path_sel_loop[1]}";
        Attribute tessent_design_gate_ports = "{o_configure_path_sel_loop[1]}";
        Source tai_output_interface_inst.o_configure_path_sel_loop_ijtag_to_sel_1;
    }
    ToSelectPort o_pmu_collect_status_loop_ijtag_to_sel_2[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_collect_status_loop[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_collect_status_loop[1]}";
        Source tai_output_interface_inst.o_pmu_collect_status_loop_ijtag_to_sel_2;
    }
    ToSelectPort o_configure_delay_sel_loop_ijtag_to_sel_3[1:1] {
        Attribute tessent_design_rtl_ports = "{o_configure_delay_sel_loop[1]}";
        Attribute tessent_design_gate_ports = "{o_configure_delay_sel_loop[1]}";
        Source tai_output_interface_inst.o_configure_delay_sel_loop_ijtag_to_sel_3;
    }
    DataOutPort o_pmu_func_clk_gate_en_loop_ijtag_do_2[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_func_clk_gate_en_loop[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_func_clk_gate_en_loop[1]}";
        Source tai_output_interface_inst.o_pmu_func_clk_gate_en_loop_ijtag_do;
    }
    DataOutPort o_pmu_testmode_ijtag_do_3[1:0] {
        Attribute tessent_design_rtl_ports = "{o_pmu_testmode[1]} {o_pmu_testmode[0]}";
        Attribute tessent_design_gate_ports = "{o_pmu_testmode[1]} {o_pmu_testmode[0]}";
        Source test_mode_decoder_inst.o_pmu_testmode_ijtag_do_1;
    }
    DataOutPort o_pmc_test_en_ijtag_do_4 {
        Attribute tessent_design_rtl_ports = "o_pmc_test_en";
        Attribute tessent_design_gate_ports = "o_pmc_test_en";
        Source test_mode_decoder_inst.o_pmc_test_en_ijtag_do_4;
    }
    DataOutPort o_clkdiv_cg_test_ijtag_do_5 {
        Attribute tessent_design_rtl_ports = "o_clkdiv_cg_test";
        Attribute tessent_design_gate_ports = "o_clkdiv_cg_test";
        Source test_mode_decoder_inst.o_clkdiv_cg_test_ijtag_do_3;
    }
    DataOutPort o_pmc_mode_tdr_override_ijtag_do_6 {
        Attribute tessent_design_rtl_ports = "o_pmc_mode_tdr_override";
        Attribute tessent_design_gate_ports = "o_pmc_mode_tdr_override";
        Source test_mode_decoder_inst.o_pmc_mode_tdr_override_ijtag_do_5;
    }
    ToSelectPort o_pmu_capture_ff_shift_ijtag_to_sel_4[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_shift[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_shift[1]}";
        Source tai_output_interface_inst.o_pmu_capture_ff_shift_ijtag_to_sel_4;
    }
    ScanOutPort o_pmu_capture_ff_tdi_ijtag_to_si_2[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_tdi[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_tdi[1]}";
        Source tai_output_interface_inst.o_pmu_capture_ff_tdi_ijtag_to_si_2;
    }
    ScanInPort i_pmu_capture_ff_tdo_ijtag_from_so_2[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_capture_ff_tdo[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_capture_ff_tdo[1]}";
    }
    Instance tai_output_interface_inst Of PATH_MONITOR_CONTROLLER_TAI_OUTPUT_INTERFACE_PATH_SELECT_DELAY_SELECT_CAPTURE_SHIFT_COLLECT_STATUS_FUNC_CLK_SEL {
        InputPort i_clk_ijtag_tck = TCK_ijtag_tck;
        InputPort i_rstn_ijtag_reset = TRSTN_ijtag_reset;
        InputPort i_configure_path_sel_ijtag_di_4 = IEEE_1687_module_inst.o_configure_path_sel_pmu_ijtag_to_sel_3;
        InputPort i_configure_delay_sel_ijtag_di_2 = IEEE_1687_module_inst.o_configure_delay_sel_pmu_ijtag_to_sel_2;
        InputPort i_pmu_collect_status_ijtag_di_3 = IEEE_1687_module_inst.o_pmu_collect_status_ijtag_to_sel_1;
        InputPort i_pmu_capture_ff_shift_ijtag_di_5 = IEEE_1687_module_inst.o_pmu_capture_ff_shift_ijtag_to_sel_4;
        InputPort i_pmu_func_clk_gate_en_ijtag_di_1 = IEEE_1687_module_inst.o_pmu_func_clk_gate_en_ijtag_do_2;
        InputPort i_pmu_capture_ff_tdi_ijtag_si_1 = IEEE_1687_module_inst.o_pmu_capture_ff_tdi_ijtag_to_si_4;
        InputPort pmu_status_in_ijtag_si_4 = IEEE_1687_module_inst.o_pmu_status_ijtag_to_si_3;
        InputPort i_path_select_ijtag_si_3 = IEEE_1687_module_inst.o_path_select_ijtag_to_si_2;
        InputPort i_delay_select_ijtag_si_2 = IEEE_1687_module_inst.o_delay_select_ijtag_to_si_1;
    }
    Instance IEEE_1687_module_inst Of PATH_MONITOR_CONTROLLER_IEEE_1687_module {
        InputPort i_scanin_ijtag_si = TDI_ijtag_si;
        InputPort i_resetn_ijtag_reset = TRSTN_ijtag_reset;
        InputPort i_tck_ijtag_tck_1 = TCK_ijtag_tck;
        InputPort i_captureEn_ijtag_ce = Capture_ijtag_ce;
        InputPort i_shiftEn_ijtag_se = Shift_ijtag_se;
        InputPort i_updateEn_ijtag_ue = Update_ijtag_ue;
        InputPort i_select_ijtag_sel = Select_ijtag_sel;
        InputPort i_wrck_gated_mutex_ijtag_tck_2 = TCK_ijtag_tck;
        InputPort i_pmu_data_out_ijtag_from_so_1 = i_pmu_data_loop_ijtag_from_so_1;
        InputPort i_pmu_capture_ff_data_out_ijtag_from_so_2 = i_pmu_capture_ff_tdo_ijtag_from_so_2;
    }
    Instance clk_cntrl_inst Of PATH_MONITOR_CONTROLLER_clk_cntrl {
        InputPort i_pmu_clock_mux_en_ijtag_di_1 = test_mode_decoder_inst.o_pmu_clock_mux_en_ijtag_do_2;
        InputPort i_pmu_mode_ijtag_di_2 = test_mode_decoder_inst.o_pmu_testmode_ijtag_do_1;
        InputPort i_resetn_ijtag_reset = TRSTN_ijtag_reset;
        InputPort i_testmode_sibSelect_ijtag_di_3 = IEEE_1687_module_inst.o_testmode_sibSelect_ijtag_do_3;
        InputPort i_sib_loop_bypass_tdr_sibSelect_ijtag_di_4 = IEEE_1687_module_inst.o_sib_loop_bypass_tdr_sibSelect_ijtag_do_4;
        InputPort i_pmu_path_select_tdr_sibSelect_ijtag_di_5 = IEEE_1687_module_inst.o_pmu_path_select_tdr_sibSelect_ijtag_do_5;
        InputPort i_pmu_path_select_tdr_sib_ijtag_di_6 = IEEE_1687_module_inst.o_pmu_path_select_tdr_sib_ijtag_do_6;
        InputPort i_pmu_delay_select_tdr_sibSelect_ijtag_di_7 = IEEE_1687_module_inst.o_pmu_delay_select_tdr_sibSelect_ijtag_do_7;
        InputPort i_pmu_delay_select_tdr_sib_ijtag_di_8 = IEEE_1687_module_inst.o_pmu_delay_select_tdr_sib_ijtag_do_8;
        InputPort i_pmu_collect_status_tdr_sibSelect_ijtag_di_9 = IEEE_1687_module_inst.o_pmu_collect_status_tdr_sibSelect_ijtag_do_9;
        InputPort i_pmu_collect_status_tdr_sib_ijtag_di_10 = IEEE_1687_module_inst.o_pmu_collect_status_tdr_sib_ijtag_do_11;
        InputPort i_pmu_func_clk_gate_en_tdr_sibSelect_ijtag_di_11 = IEEE_1687_module_inst.o_pmu_func_clk_gate_en_tdr_sibSelect_ijtag_do_12;
        InputPort i_sib2_bypass_tdr_sibSelect_ijtag_di_12 = IEEE_1687_module_inst.o_sib2_bypass_tdr_sibSelect_ijtag_do_10;
        InputPort i_pmu_capture_ff_tdr_sibSelect_ijtag_di_13 = IEEE_1687_module_inst.o_pmu_capture_ff_tdr_sibSelect_ijtag_do_13;
        InputPort i_pmu_capture_ff_tdr_sib_ijtag_di_14 = IEEE_1687_module_inst.o_pmu_capture_ff_tdr_sib_ijtag_do_14;
        InputPort i_wrck_gated_mutex_ijtag_tck = TCK_ijtag_tck;
    }
    Instance test_mode_decoder_inst Of PATH_MONITOR_CONTROLLER_TEST_MODE_DECODER_PM_MODE {
        InputPort i_test_mode_ijtag_di = IEEE_1687_module_inst.o_test_mode_1687_ijtag_do_1;
    }
    ScanInterface client {
        Port TDI_ijtag_si;
        Port TDO_ijtag_so;
        Port Select_ijtag_sel;
        Port Shift_ijtag_se;
        Port Capture_ijtag_ce;
        Port Update_ijtag_ue;
        Port TRSTN_ijtag_reset;
        Port TCK_ijtag_tck;
    }
    ScanInterface host_o_configure_path_sel_loop1 {
        Port o_pmu_data_in_ijtag_to_si_1[1];
        Port i_pmu_data_loop_ijtag_from_so_1[1];
        Port o_configure_path_sel_loop_ijtag_to_sel_1[1];
    }
    ScanInterface host_o_configure_delay_sel_loop1 {
        Port o_pmu_data_in_ijtag_to_si_1[1];
        Port i_pmu_data_loop_ijtag_from_so_1[1];
        Port o_configure_delay_sel_loop_ijtag_to_sel_3[1];
    }
    ScanInterface host_o_pmu_collect_status_loop1 {
        Port o_pmu_data_in_ijtag_to_si_1[1];
        Port i_pmu_data_loop_ijtag_from_so_1[1];
        Port o_pmu_collect_status_loop_ijtag_to_sel_2[1];
    }
    ScanInterface host_o_pmu_capture_ff_shift1 {
        Port o_pmu_capture_ff_tdi_ijtag_to_si_2[1];
        Port i_pmu_capture_ff_tdo_ijtag_from_so_2[1];
        Port o_pmu_capture_ff_shift_ijtag_to_sel_4[1];
    }
}
