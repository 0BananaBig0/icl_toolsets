// Generated; edit the PMM sources, not this file.

NameSpace ;
UseNameSpace ;
Module PATH_MONITOR_CONTROLLER_IEEE_1687_dr {
    TCKPort i_tck_ijtag_tck {
        Attribute tessent_design_rtl_ports = "i_tck";
        Attribute tessent_design_gate_ports = "i_tck";
    }
    ResetPort i_Resetn_ijtag_reset {
        Attribute tessent_design_rtl_ports = "i_Resetn";
        Attribute tessent_design_gate_ports = "i_Resetn";
        ActivePolarity 0;
    }
    CaptureEnPort i_tm_sibCaptureEn_ijtag_ce_1 {
        Attribute tessent_design_rtl_ports = "i_tm_sibCaptureEn";
        Attribute tessent_design_gate_ports = "i_tm_sibCaptureEn";
    }
    UpdateEnPort i_tm_sibUpdateEn_ijtag_ue_1 {
        Attribute tessent_design_rtl_ports = "i_tm_sibUpdateEn";
        Attribute tessent_design_gate_ports = "i_tm_sibUpdateEn";
    }
    ShiftEnPort i_tm_sibShiftEn_ijtag_se_1 {
        Attribute tessent_design_rtl_ports = "i_tm_sibShiftEn";
        Attribute tessent_design_gate_ports = "i_tm_sibShiftEn";
    }
    ScanInPort i_tm_tdr_scanin_ijtag_si_1 {
        Attribute tessent_design_rtl_ports = "i_tm_tdr_scanin";
        Attribute tessent_design_gate_ports = "i_tm_tdr_scanin";
    }
    ScanOutPort o_tm_tdr_scanout_ijtag_so_1 {
        Attribute tessent_design_rtl_ports = "o_tm_tdr_scanout";
        Attribute tessent_design_gate_ports = "o_tm_tdr_scanout";
        Source IEEE_1687_tm_tdr_inst.o_tdr_scanout_ijtag_so;
    }
    DataOutPort o_test_mode_ijtag_do_1[5:0] {
        Attribute tessent_design_rtl_ports = "{o_test_mode[5]} {o_test_mode[4]} {o_test_mode[3]} {o_test_mode[2]} {o_test_mode[1]} {o_test_mode[0]}";
        Attribute tessent_design_gate_ports = "{o_test_mode[5]} {o_test_mode[4]} {o_test_mode[3]} {o_test_mode[2]} {o_test_mode[1]} {o_test_mode[0]}";
        Source IEEE_1687_tm_tdr_inst.o_tdr_update_ijtag_do;
    }
    ShiftEnPort i_pmu_func_clk_gate_en_tdr_sibShiftEn_ijtag_se_2 {
        Attribute tessent_design_rtl_ports = "i_pmu_func_clk_gate_en_tdr_sibShiftEn";
        Attribute tessent_design_gate_ports = "i_pmu_func_clk_gate_en_tdr_sibShiftEn";
    }
    CaptureEnPort i_pmu_func_clk_gate_en_tdr_sibCaptureEn_ijtag_ce_2 {
        Attribute tessent_design_rtl_ports = "i_pmu_func_clk_gate_en_tdr_sibCaptureEn";
        Attribute tessent_design_gate_ports = "i_pmu_func_clk_gate_en_tdr_sibCaptureEn";
    }
    UpdateEnPort i_pmu_func_clk_gate_en_tdr_sibUpdateEn_ijtag_ue_2 {
        Attribute tessent_design_rtl_ports = "i_pmu_func_clk_gate_en_tdr_sibUpdateEn";
        Attribute tessent_design_gate_ports = "i_pmu_func_clk_gate_en_tdr_sibUpdateEn";
    }
    ScanInPort i_pmu_func_clk_gate_en_tdr_scanin_ijtag_si_2 {
        Attribute tessent_design_rtl_ports = "i_pmu_func_clk_gate_en_tdr_scanin";
        Attribute tessent_design_gate_ports = "i_pmu_func_clk_gate_en_tdr_scanin";
    }
    ScanOutPort o_pmu_func_clk_gate_en_tdr_scanout_ijtag_so_2 {
        Attribute tessent_design_rtl_ports = "o_pmu_func_clk_gate_en_tdr_scanout";
        Attribute tessent_design_gate_ports = "o_pmu_func_clk_gate_en_tdr_scanout";
        Source IEEE_1687_pmu_func_clk_gate_en_tdr_inst.o_tdr_scanout_ijtag_so;
    }
    DataOutPort o_pmu_func_clk_gate_en_ijtag_do_2[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_func_clk_gate_en[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_func_clk_gate_en[1]}";
        Source IEEE_1687_pmu_func_clk_gate_en_tdr_inst.o_tdr_update_ijtag_do[0:0];
    }
    ShiftEnPort i_sib_loop_bypass_tdr_sibShiftEn_ijtag_se_3 {
        Attribute tessent_design_rtl_ports = "i_sib_loop_bypass_tdr_sibShiftEn";
        Attribute tessent_design_gate_ports = "i_sib_loop_bypass_tdr_sibShiftEn";
    }
    CaptureEnPort i_sib_loop_bypass_tdr_sibCaptureEn_ijtag_ce_3 {
        Attribute tessent_design_rtl_ports = "i_sib_loop_bypass_tdr_sibCaptureEn";
        Attribute tessent_design_gate_ports = "i_sib_loop_bypass_tdr_sibCaptureEn";
    }
    UpdateEnPort i_sib_loop_bypass_tdr_sibUpdateEn_ijtag_ue_3 {
        Attribute tessent_design_rtl_ports = "i_sib_loop_bypass_tdr_sibUpdateEn";
        Attribute tessent_design_gate_ports = "i_sib_loop_bypass_tdr_sibUpdateEn";
    }
    ScanInPort i_sib_loop_bypass_tdr_scanin_ijtag_si_3 {
        Attribute tessent_design_rtl_ports = "i_sib_loop_bypass_tdr_scanin";
        Attribute tessent_design_gate_ports = "i_sib_loop_bypass_tdr_scanin";
    }
    ScanOutPort o_sib_loop_bypass_tdr_scanout_ijtag_so_3 {
        Attribute tessent_design_rtl_ports = "o_sib_loop_bypass_tdr_scanout";
        Attribute tessent_design_gate_ports = "o_sib_loop_bypass_tdr_scanout";
        Source IEEE_1687_pmu_sib_loop_bypass_tdr_inst.o_tdr_scanout_ijtag_so;
    }
    ShiftEnPort i_sib2_bypass_tdr_sibShiftEn_ijtag_se_4[1:1] {
        Attribute tessent_design_rtl_ports = "{i_sib2_bypass_tdr_sibShiftEn[1]}";
        Attribute tessent_design_gate_ports = "{i_sib2_bypass_tdr_sibShiftEn[1]}";
    }
    CaptureEnPort i_sib2_bypass_tdr_sibCaptureEn_ijtag_ce_4[1:1] {
        Attribute tessent_design_rtl_ports = "{i_sib2_bypass_tdr_sibCaptureEn[1]}";
        Attribute tessent_design_gate_ports = "{i_sib2_bypass_tdr_sibCaptureEn[1]}";
    }
    UpdateEnPort i_sib2_bypass_tdr_sibUpdateEn_ijtag_ue_4[1:1] {
        Attribute tessent_design_rtl_ports = "{i_sib2_bypass_tdr_sibUpdateEn[1]}";
        Attribute tessent_design_gate_ports = "{i_sib2_bypass_tdr_sibUpdateEn[1]}";
    }
    ScanInPort i_sib2_bypass_tdr_scanin_ijtag_si_4[1:1] {
        Attribute tessent_design_rtl_ports = "{i_sib2_bypass_tdr_scanin[1]}";
        Attribute tessent_design_gate_ports = "{i_sib2_bypass_tdr_scanin[1]}";
    }
    ScanOutPort o_sib2_bypass_tdr_scanout_ijtag_so_4[1:1] {
        Attribute tessent_design_rtl_ports = "{o_sib2_bypass_tdr_scanout[1]}";
        Attribute tessent_design_gate_ports = "{o_sib2_bypass_tdr_scanout[1]}";
        Source IEEE_1687_sib2_bypass_tdr_inst_1.o_tdr_scanout_ijtag_so;
    }
    ShiftEnPort i_pmu_path_select_tdr_sibShiftEn_ijtag_se_5[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_path_select_tdr_sibShiftEn[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_path_select_tdr_sibShiftEn[1]}";
    }
    DataInPort i_pmu_path_select_tdr_sib_ijtag_di_1[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_path_select_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_path_select_tdr_sib[1]}";
    }
    CaptureEnPort i_pmu_path_select_tdr_sibCaptureEn_ijtag_ce_5[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_path_select_tdr_sibCaptureEn[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_path_select_tdr_sibCaptureEn[1]}";
    }
    UpdateEnPort i_pmu_path_select_tdr_sibUpdateEn_ijtag_ue_5[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_path_select_tdr_sibUpdateEn[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_path_select_tdr_sibUpdateEn[1]}";
    }
    ScanInPort i_pmu_path_select_tdr_scanin_ijtag_si_5[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_path_select_tdr_scanin[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_path_select_tdr_scanin[1]}";
    }
    ScanOutPort o_pmu_path_select_tdr_scanout_ijtag_so_5[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_path_select_tdr_scanout[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_path_select_tdr_scanout[1]}";
        Source IEEE_1687_pmu_path_select_tdr_inst_1.o_pmu_tdr_scanout_ijtag_so;
    }
    ToSelectPort o_configure_path_sel_pmu_ijtag_to_sel_1[1:1] {
        Attribute tessent_design_rtl_ports = "{o_configure_path_sel_pmu[1]}";
        Attribute tessent_design_gate_ports = "{o_configure_path_sel_pmu[1]}";
        Source IEEE_1687_pmu_path_select_tdr_inst_1.o_pmu_shift_ijtag_to_sel;
    }
    ScanOutPort o_path_select_ijtag_to_si_1[1:1] {
        Attribute tessent_design_rtl_ports = "{o_path_select[1]}";
        Attribute tessent_design_gate_ports = "{o_path_select[1]}";
        Source IEEE_1687_pmu_path_select_tdr_inst_1.o_pmu_tdr_data_in_ijtag_to_si;
    }
    ShiftEnPort i_pmu_delay_select_tdr_sibShiftEn_ijtag_se_6[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_delay_select_tdr_sibShiftEn[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_delay_select_tdr_sibShiftEn[1]}";
    }
    DataInPort i_pmu_delay_select_tdr_sib_ijtag_di_2[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_delay_select_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_delay_select_tdr_sib[1]}";
    }
    CaptureEnPort i_pmu_delay_select_tdr_sibCaptureEn_ijtag_ce_6[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_delay_select_tdr_sibCaptureEn[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_delay_select_tdr_sibCaptureEn[1]}";
    }
    UpdateEnPort i_pmu_delay_select_tdr_sibUpdateEn_ijtag_ue_6[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_delay_select_tdr_sibUpdateEn[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_delay_select_tdr_sibUpdateEn[1]}";
    }
    ScanInPort i_pmu_delay_select_tdr_scanin_ijtag_si_6[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_delay_select_tdr_scanin[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_delay_select_tdr_scanin[1]}";
    }
    ScanOutPort o_pmu_delay_select_tdr_scanout_ijtag_so_6[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_delay_select_tdr_scanout[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_delay_select_tdr_scanout[1]}";
        Source IEEE_1687_pmu_delay_select_tdr_inst_1.o_pmu_tdr_scanout_ijtag_so;
    }
    ScanOutPort o_delay_select_ijtag_to_si_2[1:1] {
        Attribute tessent_design_rtl_ports = "{o_delay_select[1]}";
        Attribute tessent_design_gate_ports = "{o_delay_select[1]}";
        Source IEEE_1687_pmu_delay_select_tdr_inst_1.o_pmu_tdr_data_in_ijtag_to_si;
    }
    ToSelectPort o_configure_delay_select_pmu_ijtag_to_sel_2[1:1] {
        Attribute tessent_design_rtl_ports = "{o_configure_delay_select_pmu[1]}";
        Attribute tessent_design_gate_ports = "{o_configure_delay_select_pmu[1]}";
        Source IEEE_1687_pmu_delay_select_tdr_inst_1.o_pmu_shift_ijtag_to_sel;
    }
    ShiftEnPort i_pmu_collect_status_tdr_sibShiftEn_ijtag_se_7[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_collect_status_tdr_sibShiftEn[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_collect_status_tdr_sibShiftEn[1]}";
    }
    DataInPort i_pmu_collect_status_tdr_sib_ijtag_di_3[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_collect_status_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_collect_status_tdr_sib[1]}";
    }
    CaptureEnPort i_pmu_collect_status_tdr_sibCaptureEn_ijtag_ce_7[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_collect_status_tdr_sibCaptureEn[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_collect_status_tdr_sibCaptureEn[1]}";
    }
    UpdateEnPort i_pmu_collect_status_tdr_sibUpdateEn_ijtag_ue_7[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_collect_status_tdr_sibUpdateEn[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_collect_status_tdr_sibUpdateEn[1]}";
    }
    ScanInPort i_pmu_collect_status_tdr_scanin_ijtag_si_7[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_collect_status_tdr_scanin[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_collect_status_tdr_scanin[1]}";
    }
    ScanOutPort o_pmu_collect_status_tdr_scanout_ijtag_so_7[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_collect_status_tdr_scanout[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_collect_status_tdr_scanout[1]}";
        Source IEEE_1687_pmu_collect_status_tdr_inst_1.o_pmu_tdr_scanout_ijtag_so;
    }
    ToSelectPort o_pmu_collect_status_ijtag_to_sel_3[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_collect_status[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_collect_status[1]}";
        Source IEEE_1687_pmu_collect_status_tdr_inst_1.o_pmu_shift_ijtag_to_sel;
    }
    ScanInPort i_pmu_data_out_ijtag_from_so_1[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_data_out[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_data_out[1]}";
    }
    ScanOutPort o_pmu_status_ijtag_to_si_3[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_status[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_status[1]}";
        Source IEEE_1687_pmu_collect_status_tdr_inst_1.o_pmu_tdr_data_in_ijtag_to_si;
    }
    DataInPort i_pmu_capture_ff_tdr_sib_ijtag_di_4[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_capture_ff_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_capture_ff_tdr_sib[1]}";
    }
    ShiftEnPort i_pmu_capture_ff_tdr_sibShiftEn_ijtag_se_8[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_capture_ff_tdr_sibShiftEn[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_capture_ff_tdr_sibShiftEn[1]}";
    }
    CaptureEnPort i_pmu_capture_ff_tdr_sibCaptureEn_ijtag_ce_8[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_capture_ff_tdr_sibCaptureEn[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_capture_ff_tdr_sibCaptureEn[1]}";
    }
    UpdateEnPort i_pmu_capture_ff_tdr_sibUpdateEn_ijtag_ue_8[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_capture_ff_tdr_sibUpdateEn[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_capture_ff_tdr_sibUpdateEn[1]}";
    }
    ScanInPort i_pmu_capture_ff_tdr_scanin_ijtag_si_8[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_capture_ff_tdr_scanin[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_capture_ff_tdr_scanin[1]}";
    }
    ScanOutPort o_pmu_capture_tdr_scanout_ijtag_so_8[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_tdr_scanout[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_tdr_scanout[1]}";
        Source IEEE_1687_pmu_capture_ff_tdr_inst_1.o_pmu_tdr_scanout_ijtag_so;
    }
    ScanOutPort o_pmu_capture_ff_tdr_data_in_ijtag_to_si_4[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_tdr_data_in[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_tdr_data_in[1]}";
        Source IEEE_1687_pmu_capture_ff_tdr_inst_1.o_pmu_tdr_data_in_ijtag_to_si;
    }
    ScanInPort i_pmu_capture_ff_data_out_ijtag_from_so_2[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_capture_ff_data_out[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_capture_ff_data_out[1]}";
    }
    ToSelectPort o_pmu_capture_ff_shift_ijtag_to_sel_4[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_shift[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_shift[1]}";
        Source IEEE_1687_pmu_capture_ff_tdr_inst_1.o_pmu_shift_ijtag_to_sel;
    }
    Instance IEEE_1687_tm_tdr_inst Of PATH_MONITOR_CONTROLLER_IEEE_1687_tdr_SIX_WIDTH_PM_MODE {
        InputPort i_tck_ijtag_tck = i_tck_ijtag_tck;
        InputPort i_resetn_ijtag_reset = i_Resetn_ijtag_reset;
        InputPort i_tdr_sibShiftEn_ijtag_se = i_tm_sibShiftEn_ijtag_se_1;
        InputPort i_tdr_sibCaptureEn_ijtag_ce = i_tm_sibCaptureEn_ijtag_ce_1;
        InputPort i_tdr_sibUpdateEn_ijtag_ue = i_tm_sibUpdateEn_ijtag_ue_1;
        InputPort i_tdr_scanin_ijtag_si = i_tm_tdr_scanin_ijtag_si_1;
    }
    Instance IEEE_1687_pmu_func_clk_gate_en_tdr_inst Of PATH_MONITOR_CONTROLLER_IEEE_1687_tdr_FIFTEEN_WIDTH_FUNC_CLK_SEL {
        InputPort i_tck_ijtag_tck = i_tck_ijtag_tck;
        InputPort i_resetn_ijtag_reset = i_Resetn_ijtag_reset;
        InputPort i_tdr_sibShiftEn_ijtag_se = i_pmu_func_clk_gate_en_tdr_sibShiftEn_ijtag_se_2;
        InputPort i_tdr_sibCaptureEn_ijtag_ce = i_pmu_func_clk_gate_en_tdr_sibCaptureEn_ijtag_ce_2;
        InputPort i_tdr_sibUpdateEn_ijtag_ue = i_pmu_func_clk_gate_en_tdr_sibUpdateEn_ijtag_ue_2;
        InputPort i_tdr_scanin_ijtag_si = i_pmu_func_clk_gate_en_tdr_scanin_ijtag_si_2;
    }
    Instance IEEE_1687_pmu_sib_loop_bypass_tdr_inst Of PATH_MONITOR_CONTROLLER_IEEE_1687_one_bit_tdr_SIB_LOOP_SEL {
        InputPort i_tck_ijtag_tck = i_tck_ijtag_tck;
        InputPort i_resetn_ijtag_reset = i_Resetn_ijtag_reset;
        InputPort i_tdr_sibShiftEn_ijtag_se = i_sib_loop_bypass_tdr_sibShiftEn_ijtag_se_3;
        InputPort i_tdr_sibCaptureEn_ijtag_ce = i_sib_loop_bypass_tdr_sibCaptureEn_ijtag_ce_3;
        InputPort i_tdr_sibUpdateEn_ijtag_ue = i_sib_loop_bypass_tdr_sibUpdateEn_ijtag_ue_3;
        InputPort i_tdr_scanin_ijtag_si = i_sib_loop_bypass_tdr_scanin_ijtag_si_3;
    }
    Instance IEEE_1687_sib2_bypass_tdr_inst_1 Of PATH_MONITOR_CONTROLLER_IEEE_1687_one_bit_tdr_SIB_2 {
        InputPort i_tck_ijtag_tck = i_tck_ijtag_tck;
        InputPort i_resetn_ijtag_reset = i_Resetn_ijtag_reset;
        InputPort i_tdr_sibShiftEn_ijtag_se = i_sib2_bypass_tdr_sibShiftEn_ijtag_se_4[1];
        InputPort i_tdr_sibCaptureEn_ijtag_ce = i_sib2_bypass_tdr_sibCaptureEn_ijtag_ce_4[1];
        InputPort i_tdr_sibUpdateEn_ijtag_ue = i_sib2_bypass_tdr_sibUpdateEn_ijtag_ue_4[1];
        InputPort i_tdr_scanin_ijtag_si = i_sib2_bypass_tdr_scanin_ijtag_si_4[1];
    }
    Instance IEEE_1687_pmu_path_select_tdr_inst_1 Of PATH_MONITOR_CONTROLLER_IEEE_1687_pmu_tdr_PATH_SELECT {
        InputPort i_tck_ijtag_tck = i_tck_ijtag_tck;
        InputPort i_resetn_ijtag_reset = i_Resetn_ijtag_reset;
        InputPort i_pmu_tdr_sibShiftEn_ijtag_se = i_pmu_path_select_tdr_sibShiftEn_ijtag_se_5[1];
        InputPort i_pmu_tdr_sib_ijtag_di = i_pmu_path_select_tdr_sib_ijtag_di_1[1];
        InputPort i_pmu_tdr_sibCaptureEn_ijtag_ce = i_pmu_path_select_tdr_sibCaptureEn_ijtag_ce_5[1];
        InputPort i_pmu_tdr_sibUpdateEn_ijtag_ue = i_pmu_path_select_tdr_sibUpdateEn_ijtag_ue_5[1];
        InputPort i_pmu_tdr_scanin_ijtag_si = i_pmu_path_select_tdr_scanin_ijtag_si_5[1];
        InputPort i_pmu_tdr_data_out_ijtag_from_so = i_pmu_data_out_ijtag_from_so_1[1];
    }
    Instance IEEE_1687_pmu_delay_select_tdr_inst_1 Of PATH_MONITOR_CONTROLLER_IEEE_1687_pmu_tdr_DELAY_SELECT {
        InputPort i_tck_ijtag_tck = i_tck_ijtag_tck;
        InputPort i_resetn_ijtag_reset = i_Resetn_ijtag_reset;
        InputPort i_pmu_tdr_sibShiftEn_ijtag_se = i_pmu_delay_select_tdr_sibShiftEn_ijtag_se_6[1];
        InputPort i_pmu_tdr_sib_ijtag_di = i_pmu_delay_select_tdr_sib_ijtag_di_2[1];
        InputPort i_pmu_tdr_sibCaptureEn_ijtag_ce = i_pmu_delay_select_tdr_sibCaptureEn_ijtag_ce_6[1];
        InputPort i_pmu_tdr_sibUpdateEn_ijtag_ue = i_pmu_delay_select_tdr_sibUpdateEn_ijtag_ue_6[1];
        InputPort i_pmu_tdr_scanin_ijtag_si = i_pmu_delay_select_tdr_scanin_ijtag_si_6[1];
        InputPort i_pmu_tdr_data_out_ijtag_from_so = i_pmu_data_out_ijtag_from_so_1[1];
    }
    Instance IEEE_1687_pmu_collect_status_tdr_inst_1 Of PATH_MONITOR_CONTROLLER_IEEE_1687_pmu_tdr_COLLECT_STATUS {
        InputPort i_tck_ijtag_tck = i_tck_ijtag_tck;
        InputPort i_resetn_ijtag_reset = i_Resetn_ijtag_reset;
        InputPort i_pmu_tdr_sibShiftEn_ijtag_se = i_pmu_collect_status_tdr_sibShiftEn_ijtag_se_7[1];
        InputPort i_pmu_tdr_sib_ijtag_di = i_pmu_collect_status_tdr_sib_ijtag_di_3[1];
        InputPort i_pmu_tdr_sibCaptureEn_ijtag_ce = i_pmu_collect_status_tdr_sibCaptureEn_ijtag_ce_7[1];
        InputPort i_pmu_tdr_sibUpdateEn_ijtag_ue = i_pmu_collect_status_tdr_sibUpdateEn_ijtag_ue_7[1];
        InputPort i_pmu_tdr_scanin_ijtag_si = i_pmu_collect_status_tdr_scanin_ijtag_si_7[1];
        InputPort i_pmu_tdr_data_out_ijtag_from_so = i_pmu_data_out_ijtag_from_so_1[1];
    }
    Instance IEEE_1687_pmu_capture_ff_tdr_inst_1 Of PATH_MONITOR_CONTROLLER_IEEE_1687_pmu_tdr_CAPTURE_SHIFT {
        InputPort i_tck_ijtag_tck = i_tck_ijtag_tck;
        InputPort i_resetn_ijtag_reset = i_Resetn_ijtag_reset;
        InputPort i_pmu_tdr_sibShiftEn_ijtag_se = i_pmu_capture_ff_tdr_sibShiftEn_ijtag_se_8[1];
        InputPort i_pmu_tdr_sib_ijtag_di = i_pmu_capture_ff_tdr_sib_ijtag_di_4[1];
        InputPort i_pmu_tdr_sibCaptureEn_ijtag_ce = i_pmu_capture_ff_tdr_sibCaptureEn_ijtag_ce_8[1];
        InputPort i_pmu_tdr_sibUpdateEn_ijtag_ue = i_pmu_capture_ff_tdr_sibUpdateEn_ijtag_ue_8[1];
        InputPort i_pmu_tdr_scanin_ijtag_si = i_pmu_capture_ff_tdr_scanin_ijtag_si_8[1];
        InputPort i_pmu_tdr_data_out_ijtag_from_so = i_pmu_capture_ff_data_out_ijtag_from_so_2[1];
    }
    ScanInterface client_IEEE_1687_tm_tdr_inst {
        Port i_tm_tdr_scanin_ijtag_si_1;
        Port o_tm_tdr_scanout_ijtag_so_1;
        Port i_tm_sibShiftEn_ijtag_se_1;
        Port i_tm_sibCaptureEn_ijtag_ce_1;
        Port i_tm_sibUpdateEn_ijtag_ue_1;
        Port i_tck_ijtag_tck;
        Port i_Resetn_ijtag_reset;
    }
    ScanInterface client_IEEE_1687_pmu_func_clk_gate_en_tdr_inst {
        Port i_pmu_func_clk_gate_en_tdr_scanin_ijtag_si_2;
        Port o_pmu_func_clk_gate_en_tdr_scanout_ijtag_so_2;
        Port i_pmu_func_clk_gate_en_tdr_sibShiftEn_ijtag_se_2;
        Port i_pmu_func_clk_gate_en_tdr_sibCaptureEn_ijtag_ce_2;
        Port i_pmu_func_clk_gate_en_tdr_sibUpdateEn_ijtag_ue_2;
        Port i_tck_ijtag_tck;
        Port i_Resetn_ijtag_reset;
    }
    ScanInterface client_IEEE_1687_pmu_sib_loop_bypass_tdr_inst {
        Port i_sib_loop_bypass_tdr_scanin_ijtag_si_3;
        Port o_sib_loop_bypass_tdr_scanout_ijtag_so_3;
        Port i_sib_loop_bypass_tdr_sibShiftEn_ijtag_se_3;
        Port i_sib_loop_bypass_tdr_sibCaptureEn_ijtag_ce_3;
        Port i_sib_loop_bypass_tdr_sibUpdateEn_ijtag_ue_3;
        Port i_tck_ijtag_tck;
        Port i_Resetn_ijtag_reset;
    }
    ScanInterface client_IEEE_1687_sib2_bypass_tdr_inst_1 {
        Port i_sib2_bypass_tdr_scanin_ijtag_si_4[1];
        Port o_sib2_bypass_tdr_scanout_ijtag_so_4[1];
        Port i_sib2_bypass_tdr_sibShiftEn_ijtag_se_4[1];
        Port i_sib2_bypass_tdr_sibCaptureEn_ijtag_ce_4[1];
        Port i_sib2_bypass_tdr_sibUpdateEn_ijtag_ue_4[1];
        Port i_tck_ijtag_tck;
        Port i_Resetn_ijtag_reset;
    }
    ScanInterface client_IEEE_1687_pmu_path_select_tdr_inst_1 {
        Port i_pmu_path_select_tdr_scanin_ijtag_si_5[1];
        Port o_pmu_path_select_tdr_scanout_ijtag_so_5[1];
        Port i_pmu_path_select_tdr_sibShiftEn_ijtag_se_5[1];
        Port i_pmu_path_select_tdr_sibCaptureEn_ijtag_ce_5[1];
        Port i_pmu_path_select_tdr_sibUpdateEn_ijtag_ue_5[1];
        Port i_tck_ijtag_tck;
        Port i_Resetn_ijtag_reset;
    }
    ScanInterface host_IEEE_1687_pmu_path_select_tdr_inst_1 {
        Port i_pmu_data_out_ijtag_from_so_1[1];
        Port o_configure_path_sel_pmu_ijtag_to_sel_1[1];
    }
    ScanInterface client_IEEE_1687_pmu_delay_select_tdr_inst_1 {
        Port i_pmu_delay_select_tdr_scanin_ijtag_si_6[1];
        Port o_pmu_delay_select_tdr_scanout_ijtag_so_6[1];
        Port i_pmu_delay_select_tdr_sibShiftEn_ijtag_se_6[1];
        Port i_pmu_delay_select_tdr_sibCaptureEn_ijtag_ce_6[1];
        Port i_pmu_delay_select_tdr_sibUpdateEn_ijtag_ue_6[1];
        Port i_tck_ijtag_tck;
        Port i_Resetn_ijtag_reset;
    }
    ScanInterface host_IEEE_1687_pmu_delay_select_tdr_inst_1 {
        Port i_pmu_data_out_ijtag_from_so_1[1];
        Port o_configure_delay_select_pmu_ijtag_to_sel_2[1];
    }
    ScanInterface client_IEEE_1687_pmu_collect_status_tdr_inst_1 {
        Port i_pmu_collect_status_tdr_scanin_ijtag_si_7[1];
        Port o_pmu_collect_status_tdr_scanout_ijtag_so_7[1];
        Port i_pmu_collect_status_tdr_sibShiftEn_ijtag_se_7[1];
        Port i_pmu_collect_status_tdr_sibCaptureEn_ijtag_ce_7[1];
        Port i_pmu_collect_status_tdr_sibUpdateEn_ijtag_ue_7[1];
        Port i_tck_ijtag_tck;
        Port i_Resetn_ijtag_reset;
    }
    ScanInterface host_IEEE_1687_pmu_collect_status_tdr_inst_1 {
        Port i_pmu_data_out_ijtag_from_so_1[1];
        Port o_pmu_collect_status_ijtag_to_sel_3[1];
    }
    ScanInterface client_IEEE_1687_pmu_capture_ff_tdr_inst_1 {
        Port i_pmu_capture_ff_tdr_scanin_ijtag_si_8[1];
        Port o_pmu_capture_tdr_scanout_ijtag_so_8[1];
        Port i_pmu_capture_ff_tdr_sibShiftEn_ijtag_se_8[1];
        Port i_pmu_capture_ff_tdr_sibCaptureEn_ijtag_ce_8[1];
        Port i_pmu_capture_ff_tdr_sibUpdateEn_ijtag_ue_8[1];
        Port i_tck_ijtag_tck;
        Port i_Resetn_ijtag_reset;
    }
    ScanInterface host_IEEE_1687_pmu_capture_ff_tdr_inst_1 {
        Port i_pmu_capture_ff_data_out_ijtag_from_so_2[1];
        Port o_pmu_capture_ff_shift_ijtag_to_sel_4[1];
    }
}
