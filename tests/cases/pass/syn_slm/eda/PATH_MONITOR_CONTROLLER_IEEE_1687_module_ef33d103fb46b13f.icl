// Generated; edit the PMM sources, not this file.

NameSpace ;
UseNameSpace ;
Module PATH_MONITOR_CONTROLLER_IEEE_1687_module {
    ScanInPort i_scanin_ijtag_si {
        Attribute tessent_design_rtl_ports = "i_scanin";
        Attribute tessent_design_gate_ports = "i_scanin";
    }
    ResetPort i_resetn_ijtag_reset {
        Attribute tessent_design_rtl_ports = "i_resetn";
        Attribute tessent_design_gate_ports = "i_resetn";
        ActivePolarity 0;
    }
    TCKPort i_tck_ijtag_tck_1 {
        Attribute tessent_design_rtl_ports = "i_tck";
        Attribute tessent_design_gate_ports = "i_tck";
    }
    CaptureEnPort i_captureEn_ijtag_ce {
        Attribute tessent_design_rtl_ports = "i_captureEn";
        Attribute tessent_design_gate_ports = "i_captureEn";
    }
    ShiftEnPort i_shiftEn_ijtag_se {
        Attribute tessent_design_rtl_ports = "i_shiftEn";
        Attribute tessent_design_gate_ports = "i_shiftEn";
    }
    UpdateEnPort i_updateEn_ijtag_ue {
        Attribute tessent_design_rtl_ports = "i_updateEn";
        Attribute tessent_design_gate_ports = "i_updateEn";
    }
    SelectPort i_select_ijtag_sel {
        Attribute tessent_design_rtl_ports = "i_select";
        Attribute tessent_design_gate_ports = "i_select";
    }
    ScanOutPort o_scanout_ijtag_so {
        Attribute tessent_design_rtl_ports = "o_scanout";
        Attribute tessent_design_gate_ports = "o_scanout";
        Source pmm_tai_sib_inst.o_scanout_ijtag_so;
    }
    DataOutPort o_test_mode_1687_ijtag_do_1[5:0] {
        Attribute tessent_design_rtl_ports = "{o_test_mode_1687[5]} {o_test_mode_1687[4]} {o_test_mode_1687[3]} {o_test_mode_1687[2]} {o_test_mode_1687[1]} {o_test_mode_1687[0]}";
        Attribute tessent_design_gate_ports = "{o_test_mode_1687[5]} {o_test_mode_1687[4]} {o_test_mode_1687[3]} {o_test_mode_1687[2]} {o_test_mode_1687[1]} {o_test_mode_1687[0]}";
        Source IEEE_1687_dr_inst.o_test_mode_ijtag_do_1;
    }
    ScanOutPort o_delay_select_ijtag_to_si_1[1:1] {
        Attribute tessent_design_rtl_ports = "{o_delay_select[1]}";
        Attribute tessent_design_gate_ports = "{o_delay_select[1]}";
        Source IEEE_1687_dr_inst.o_delay_select_ijtag_to_si_2;
    }
    ToSelectPort o_pmu_collect_status_ijtag_to_sel_1[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_collect_status[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_collect_status[1]}";
        Source IEEE_1687_dr_inst.o_pmu_collect_status_ijtag_to_sel_3;
    }
    ToSelectPort o_configure_delay_sel_pmu_ijtag_to_sel_2[1:1] {
        Attribute tessent_design_rtl_ports = "{o_configure_delay_sel_pmu[1]}";
        Attribute tessent_design_gate_ports = "{o_configure_delay_sel_pmu[1]}";
        Source IEEE_1687_dr_inst.o_configure_delay_select_pmu_ijtag_to_sel_2;
    }
    ToSelectPort o_configure_path_sel_pmu_ijtag_to_sel_3[1:1] {
        Attribute tessent_design_rtl_ports = "{o_configure_path_sel_pmu[1]}";
        Attribute tessent_design_gate_ports = "{o_configure_path_sel_pmu[1]}";
        Source IEEE_1687_dr_inst.o_configure_path_sel_pmu_ijtag_to_sel_1;
    }
    ScanOutPort o_path_select_ijtag_to_si_2[1:1] {
        Attribute tessent_design_rtl_ports = "{o_path_select[1]}";
        Attribute tessent_design_gate_ports = "{o_path_select[1]}";
        Source IEEE_1687_dr_inst.o_path_select_ijtag_to_si_1;
    }
    DataOutPort o_pmu_func_clk_gate_en_ijtag_do_2[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_func_clk_gate_en[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_func_clk_gate_en[1]}";
        Source IEEE_1687_dr_inst.o_pmu_func_clk_gate_en_ijtag_do_2;
    }
    DataOutPort o_testmode_sibSelect_ijtag_do_3 {
        Attribute tessent_design_rtl_ports = "o_testmode_sibSelect";
        Attribute tessent_design_gate_ports = "o_testmode_sibSelect";
        Source pmm_tai_sib_inst.o_testmode_sibSelect_ijtag_do_1;
    }
    DataOutPort o_sib_loop_bypass_tdr_sibSelect_ijtag_do_4 {
        Attribute tessent_design_rtl_ports = "o_sib_loop_bypass_tdr_sibSelect";
        Attribute tessent_design_gate_ports = "o_sib_loop_bypass_tdr_sibSelect";
        Source pmm_tai_sib_inst.o_sib_loop_bypass_tdr_sibSelect_ijtag_do_12;
    }
    DataOutPort o_pmu_path_select_tdr_sibSelect_ijtag_do_5[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_path_select_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_path_select_tdr_sibSelect[1]}";
        Source pmm_tai_sib_inst.o_pmu_path_select_tdr_sibSelect_ijtag_do_3;
    }
    DataOutPort o_pmu_path_select_tdr_sib_ijtag_do_6[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_path_select_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_path_select_tdr_sib[1]}";
        Source pmm_tai_sib_inst.o_pmu_path_select_tdr_sib_ijtag_do_4;
    }
    DataOutPort o_pmu_delay_select_tdr_sibSelect_ijtag_do_7[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_delay_select_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_delay_select_tdr_sibSelect[1]}";
        Source pmm_tai_sib_inst.o_pmu_delay_select_tdr_sibSelect_ijtag_do_5;
    }
    DataOutPort o_pmu_delay_select_tdr_sib_ijtag_do_8[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_delay_select_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_delay_select_tdr_sib[1]}";
        Source pmm_tai_sib_inst.o_pmu_delay_select_tdr_sib_ijtag_do_6;
    }
    DataOutPort o_pmu_collect_status_tdr_sibSelect_ijtag_do_9[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_collect_status_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_collect_status_tdr_sibSelect[1]}";
        Source pmm_tai_sib_inst.o_pmu_collect_status_tdr_sibSelect_ijtag_do_7;
    }
    DataOutPort o_sib2_bypass_tdr_sibSelect_ijtag_do_10[1:1] {
        Attribute tessent_design_rtl_ports = "{o_sib2_bypass_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{o_sib2_bypass_tdr_sibSelect[1]}";
        Source pmm_tai_sib_inst.o_sib2_bypass_tdr_sibSelect_ijtag_do_9;
    }
    DataOutPort o_pmu_collect_status_tdr_sib_ijtag_do_11[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_collect_status_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_collect_status_tdr_sib[1]}";
        Source pmm_tai_sib_inst.o_pmu_collect_status_tdr_sib_ijtag_do_8;
    }
    DataOutPort o_pmu_func_clk_gate_en_tdr_sibSelect_ijtag_do_12 {
        Attribute tessent_design_rtl_ports = "o_pmu_func_clk_gate_en_tdr_sibSelect";
        Attribute tessent_design_gate_ports = "o_pmu_func_clk_gate_en_tdr_sibSelect";
        Source pmm_tai_sib_inst.o_pmu_func_clk_gate_en_tdr_sibSelect_ijtag_do_2;
    }
    ScanInPort i_pmu_data_out_ijtag_from_so_1[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_data_out[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_data_out[1]}";
    }
    ScanOutPort o_pmu_status_ijtag_to_si_3[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_status[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_status[1]}";
        Source IEEE_1687_dr_inst.o_pmu_status_ijtag_to_si_3;
    }
    TCKPort i_wrck_gated_mutex_ijtag_tck_2 {
        Attribute tessent_design_rtl_ports = "i_wrck_gated_mutex";
        Attribute tessent_design_gate_ports = "i_wrck_gated_mutex";
    }
    DataOutPort o_pmu_capture_ff_tdr_sibSelect_ijtag_do_13[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_tdr_sibSelect[1]}";
        Source pmm_tai_sib_inst.o_pmu_capture_ff_tdr_sibSelect_ijtag_do_10;
    }
    ToSelectPort o_pmu_capture_ff_shift_ijtag_to_sel_4[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_shift[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_shift[1]}";
        Source IEEE_1687_dr_inst.o_pmu_capture_ff_shift_ijtag_to_sel_4;
    }
    DataOutPort o_pmu_capture_ff_tdr_sib_ijtag_do_14[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_tdr_sib[1]}";
        Source pmm_tai_sib_inst.o_pmu_capture_ff_tdr_sib_ijtag_do_11;
    }
    ScanOutPort o_pmu_capture_ff_tdi_ijtag_to_si_4[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_tdi[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_tdi[1]}";
        Source IEEE_1687_dr_inst.o_pmu_capture_ff_tdr_data_in_ijtag_to_si_4;
    }
    ScanInPort i_pmu_capture_ff_data_out_ijtag_from_so_2[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_capture_ff_data_out[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_capture_ff_data_out[1]}";
    }
    Instance pmm_tai_sib_inst Of PATH_MONITOR_CONTROLLER_pmm_tai_sib_block {
        InputPort i_scanin_ijtag_si = i_scanin_ijtag_si;
        InputPort i_resetn_ijtag_reset = i_resetn_ijtag_reset;
        InputPort i_tck_ijtag_tck = i_tck_ijtag_tck_1;
        InputPort i_captureEn_ijtag_ce = i_captureEn_ijtag_ce;
        InputPort i_shiftEn_ijtag_se = i_shiftEn_ijtag_se;
        InputPort i_updateEn_ijtag_ue = i_updateEn_ijtag_ue;
        InputPort i_select_ijtag_sel = i_select_ijtag_sel;
        InputPort i_testmode_tdr_scanout_ijtag_from_so_1 = IEEE_1687_dr_inst.o_tm_tdr_scanout_ijtag_so_1;
        InputPort i_pmu_func_clk_gate_en_tdr_scanout_ijtag_from_so_2 = IEEE_1687_dr_inst.o_pmu_func_clk_gate_en_tdr_scanout_ijtag_so_2;
        InputPort i_pmu_path_select_tdr_scanout_ijtag_from_so_3 = IEEE_1687_dr_inst.o_pmu_path_select_tdr_scanout_ijtag_so_5;
        InputPort i_pmu_delay_select_tdr_scanout_ijtag_from_so_4 = IEEE_1687_dr_inst.o_pmu_delay_select_tdr_scanout_ijtag_so_6;
        InputPort i_pmu_collect_status_tdr_scanout_ijtag_from_so_5 = IEEE_1687_dr_inst.o_pmu_collect_status_tdr_scanout_ijtag_so_7;
        InputPort i_sib2_bypass_tdr_scanout_ijtag_from_so_6 = IEEE_1687_dr_inst.o_sib2_bypass_tdr_scanout_ijtag_so_4;
        InputPort i_pmu_capture_ff_tdr_scanout_ijtag_from_so_7 = IEEE_1687_dr_inst.o_pmu_capture_tdr_scanout_ijtag_so_8;
        InputPort i_sib_loop_bypass_tdr_scanout_ijtag_from_so_8 = IEEE_1687_dr_inst.o_sib_loop_bypass_tdr_scanout_ijtag_so_3;
    }
    Instance IEEE_1687_dr_inst Of PATH_MONITOR_CONTROLLER_IEEE_1687_dr {
        InputPort i_tck_ijtag_tck = i_wrck_gated_mutex_ijtag_tck_2;
        InputPort i_Resetn_ijtag_reset = i_resetn_ijtag_reset;
        InputPort i_tm_sibShiftEn_ijtag_se_1 = pmm_tai_sib_inst.o_testmode_sibShiftEn_ijtag_to_se_1;
        InputPort i_tm_sibCaptureEn_ijtag_ce_1 = pmm_tai_sib_inst.o_testmode_sibCaptureEn_ijtag_to_ce_1;
        InputPort i_tm_sibUpdateEn_ijtag_ue_1 = pmm_tai_sib_inst.o_testmode_sibUpdateEn_ijtag_to_ue_1;
        InputPort i_tm_tdr_scanin_ijtag_si_1 = pmm_tai_sib_inst.o_testmode_tdr_scanin_ijtag_to_si_1;
        InputPort i_pmu_func_clk_gate_en_tdr_sibShiftEn_ijtag_se_2 = pmm_tai_sib_inst.o_pmu_func_clk_gate_en_tdr_sibShiftEn_ijtag_to_se_2;
        InputPort i_pmu_func_clk_gate_en_tdr_sibCaptureEn_ijtag_ce_2 = pmm_tai_sib_inst.o_pmu_func_clk_gate_en_tdr_sibCaptureEn_ijtag_to_ce_2;
        InputPort i_pmu_func_clk_gate_en_tdr_sibUpdateEn_ijtag_ue_2 = pmm_tai_sib_inst.o_pmu_func_clk_gate_en_tdr_sibUpdateEn_ijtag_to_ue_2;
        InputPort i_pmu_func_clk_gate_en_tdr_scanin_ijtag_si_2 = pmm_tai_sib_inst.o_pmu_func_clk_gate_en_tdr_scanin_ijtag_to_si_2;
        InputPort i_sib_loop_bypass_tdr_sibShiftEn_ijtag_se_3 = pmm_tai_sib_inst.o_sib_loop_bypass_tdr_sibShiftEn_ijtag_to_se_8;
        InputPort i_sib_loop_bypass_tdr_sibCaptureEn_ijtag_ce_3 = pmm_tai_sib_inst.o_sib_loop_bypass_tdr_sibCaptureEn_ijtag_to_ce_8;
        InputPort i_sib_loop_bypass_tdr_sibUpdateEn_ijtag_ue_3 = pmm_tai_sib_inst.o_sib_loop_bypass_tdr_sibUpdateEn_ijtag_to_ue_8;
        InputPort i_sib_loop_bypass_tdr_scanin_ijtag_si_3 = pmm_tai_sib_inst.o_sib_loop_bypass_tdr_scanin_ijtag_to_si_8;
        InputPort i_sib2_bypass_tdr_sibShiftEn_ijtag_se_4 = pmm_tai_sib_inst.o_sib2_bypass_tdr_sibShiftEn_ijtag_to_se_6;
        InputPort i_sib2_bypass_tdr_sibCaptureEn_ijtag_ce_4 = pmm_tai_sib_inst.o_sib2_bypass_tdr_sibCaptureEn_ijtag_to_ce_6;
        InputPort i_sib2_bypass_tdr_sibUpdateEn_ijtag_ue_4 = pmm_tai_sib_inst.o_sib2_bypass_tdr_sibUpdateEn_ijtag_to_ue_6;
        InputPort i_sib2_bypass_tdr_scanin_ijtag_si_4 = pmm_tai_sib_inst.o_sib2_bypass_tdr_scanin_ijtag_to_si_6;
        InputPort i_pmu_path_select_tdr_sibShiftEn_ijtag_se_5 = pmm_tai_sib_inst.o_pmu_path_select_tdr_sibShiftEn_ijtag_to_se_3;
        InputPort i_pmu_path_select_tdr_sib_ijtag_di_1 = pmm_tai_sib_inst.o_pmu_path_select_tdr_sibSelect_ijtag_do_3;
        InputPort i_pmu_path_select_tdr_sibCaptureEn_ijtag_ce_5 = pmm_tai_sib_inst.o_pmu_path_select_tdr_sibCaptureEn_ijtag_to_ce_3;
        InputPort i_pmu_path_select_tdr_sibUpdateEn_ijtag_ue_5 = pmm_tai_sib_inst.o_pmu_path_select_tdr_sibUpdateEn_ijtag_to_ue_3;
        InputPort i_pmu_path_select_tdr_scanin_ijtag_si_5 = pmm_tai_sib_inst.o_pmu_path_select_tdr_scanin_ijtag_to_si_3;
        InputPort i_pmu_data_out_ijtag_from_so_1 = i_pmu_data_out_ijtag_from_so_1;
        InputPort i_pmu_delay_select_tdr_sibShiftEn_ijtag_se_6 = pmm_tai_sib_inst.o_pmu_delay_select_tdr_sibShiftEn_ijtag_to_se_4;
        InputPort i_pmu_delay_select_tdr_sib_ijtag_di_2 = pmm_tai_sib_inst.o_pmu_delay_select_tdr_sibSelect_ijtag_do_5;
        InputPort i_pmu_delay_select_tdr_sibCaptureEn_ijtag_ce_6 = pmm_tai_sib_inst.o_pmu_delay_select_tdr_sibCaptureEn_ijtag_to_ce_4;
        InputPort i_pmu_delay_select_tdr_sibUpdateEn_ijtag_ue_6 = pmm_tai_sib_inst.o_pmu_delay_select_tdr_sibUpdateEn_ijtag_to_ue_4;
        InputPort i_pmu_delay_select_tdr_scanin_ijtag_si_6 = pmm_tai_sib_inst.o_pmu_delay_select_tdr_scanin_ijtag_to_si_4;
        InputPort i_pmu_collect_status_tdr_sibShiftEn_ijtag_se_7 = pmm_tai_sib_inst.o_pmu_collect_status_tdr_sibShiftEn_ijtag_to_se_5;
        InputPort i_pmu_collect_status_tdr_sib_ijtag_di_3 = pmm_tai_sib_inst.o_pmu_collect_status_tdr_sibSelect_ijtag_do_7;
        InputPort i_pmu_collect_status_tdr_sibCaptureEn_ijtag_ce_7 = pmm_tai_sib_inst.o_pmu_collect_status_tdr_sibCaptureEn_ijtag_to_ce_5;
        InputPort i_pmu_collect_status_tdr_sibUpdateEn_ijtag_ue_7 = pmm_tai_sib_inst.o_pmu_collect_status_tdr_sibUpdateEn_ijtag_to_ue_5;
        InputPort i_pmu_collect_status_tdr_scanin_ijtag_si_7 = pmm_tai_sib_inst.o_pmu_collect_status_tdr_scanin_ijtag_to_si_5;
        InputPort i_pmu_capture_ff_tdr_sibShiftEn_ijtag_se_8 = pmm_tai_sib_inst.o_pmu_capture_ff_tdr_sibShiftEn_ijtag_to_se_7;
        InputPort i_pmu_capture_ff_tdr_sib_ijtag_di_4 = pmm_tai_sib_inst.o_pmu_capture_ff_tdr_sibSelect_ijtag_do_10;
        InputPort i_pmu_capture_ff_tdr_sibCaptureEn_ijtag_ce_8 = pmm_tai_sib_inst.o_pmu_capture_ff_tdr_sibCaptureEn_ijtag_to_ce_7;
        InputPort i_pmu_capture_ff_tdr_sibUpdateEn_ijtag_ue_8 = pmm_tai_sib_inst.o_pmu_capture_ff_tdr_sibUpdateEn_ijtag_to_ue_7;
        InputPort i_pmu_capture_ff_tdr_scanin_ijtag_si_8 = pmm_tai_sib_inst.o_pmu_capture_ff_tdr_scanin_ijtag_to_si_7;
        InputPort i_pmu_capture_ff_data_out_ijtag_from_so_2 = i_pmu_capture_ff_data_out_ijtag_from_so_2;
    }
    ScanInterface client {
        Port i_scanin_ijtag_si;
        Port o_scanout_ijtag_so;
        Port i_select_ijtag_sel;
        Port i_shiftEn_ijtag_se;
        Port i_captureEn_ijtag_ce;
        Port i_updateEn_ijtag_ue;
        Port i_resetn_ijtag_reset;
        Port i_tck_ijtag_tck_1;
    }
    ScanInterface host_o_configure_path_sel_pmu1 {
        Port o_path_select_ijtag_to_si_2[1];
        Port i_pmu_data_out_ijtag_from_so_1[1];
        Port o_configure_path_sel_pmu_ijtag_to_sel_3[1];
    }
    ScanInterface host_o_configure_delay_sel_pmu1 {
        Port o_delay_select_ijtag_to_si_1[1];
        Port i_pmu_data_out_ijtag_from_so_1[1];
        Port o_configure_delay_sel_pmu_ijtag_to_sel_2[1];
    }
    ScanInterface host_o_pmu_collect_status1 {
        Port o_pmu_status_ijtag_to_si_3[1];
        Port i_pmu_data_out_ijtag_from_so_1[1];
        Port o_pmu_collect_status_ijtag_to_sel_1[1];
    }
    ScanInterface host_o_pmu_capture_ff_shift1 {
        Port o_pmu_capture_ff_tdi_ijtag_to_si_4[1];
        Port i_pmu_capture_ff_data_out_ijtag_from_so_2[1];
        Port o_pmu_capture_ff_shift_ijtag_to_sel_4[1];
    }
}
