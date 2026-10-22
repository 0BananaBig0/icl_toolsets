// Generated; edit the PMM sources, not this file.

NameSpace ;
UseNameSpace ;
Module PATH_MONITOR_CONTROLLER_pmm_tai_sib_block {
    ScanMux sib2_muxout_1_ SelectedBy sib2_inst_1.sibSelect_reg_ijtag_do_2[2:0] {
        3'd0 : i_sib2_bypass_tdr_scanout_ijtag_from_so_6[1];
        3'd1 : i_pmu_path_select_tdr_scanout_ijtag_from_so_3[1];
        3'd2 : i_pmu_delay_select_tdr_scanout_ijtag_from_so_4[1];
        3'd3 : i_pmu_collect_status_tdr_scanout_ijtag_from_so_5[1];
        3'd4 : i_pmu_capture_ff_tdr_scanout_ijtag_from_so_7[1];
        3'd5 : i_scanin_ijtag_si;
        3'd6 : i_scanin_ijtag_si;
        3'd7 : i_scanin_ijtag_si;
    }
    ScanMux sib1_muxout SelectedBy sib1_inst.sibSelect_reg_ijtag_do_2[2:0] {
        3'd0 : loop_sel_inst.TDO_ijtag_so;
        3'd1 : i_testmode_tdr_scanout_ijtag_from_so_1;
        3'd2 : i_pmu_func_clk_gate_en_tdr_scanout_ijtag_from_so_2;
        3'd3 : i_scanin_ijtag_si;
        3'd4 : i_scanin_ijtag_si;
        3'd5 : i_scanin_ijtag_si;
        3'd6 : i_scanin_ijtag_si;
        3'd7 : i_scanin_ijtag_si;
    }
    ScanMux loop_return SelectedBy loop_sel_inst.sibSelect_reg_ijtag_do_2 {
        4'd0 : i_sib_loop_bypass_tdr_scanout_ijtag_from_so_8;
        4'd1 : sib2_inst_1.TDO_ijtag_so;
        4'd2 : i_scanin_ijtag_si;
        4'd3 : i_scanin_ijtag_si;
        4'd4 : i_scanin_ijtag_si;
        4'd5 : i_scanin_ijtag_si;
        4'd6 : i_scanin_ijtag_si;
        4'd7 : i_scanin_ijtag_si;
        4'd8 : i_scanin_ijtag_si;
        4'd9 : i_scanin_ijtag_si;
        4'd10 : i_scanin_ijtag_si;
        4'd11 : i_scanin_ijtag_si;
        4'd12 : i_scanin_ijtag_si;
        4'd13 : i_scanin_ijtag_si;
        4'd14 : i_scanin_ijtag_si;
        4'd15 : i_scanin_ijtag_si;
    }
    ScanInPort i_scanin_ijtag_si {
        Attribute tessent_design_rtl_ports = "i_scanin";
        Attribute tessent_design_gate_ports = "i_scanin";
    }
    ResetPort i_resetn_ijtag_reset {
        Attribute tessent_design_rtl_ports = "i_resetn";
        Attribute tessent_design_gate_ports = "i_resetn";
        ActivePolarity 0;
    }
    TCKPort i_tck_ijtag_tck {
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
    ScanOutPort o_testmode_tdr_scanin_ijtag_to_si_1 {
        Attribute tessent_design_rtl_ports = "o_testmode_tdr_scanin";
        Attribute tessent_design_gate_ports = "o_testmode_tdr_scanin";
        Source i_scanin_ijtag_si;
    }
    ScanInPort i_testmode_tdr_scanout_ijtag_from_so_1 {
        Attribute tessent_design_rtl_ports = "i_testmode_tdr_scanout";
        Attribute tessent_design_gate_ports = "i_testmode_tdr_scanout";
    }
    ToCaptureEnPort o_testmode_sibCaptureEn_ijtag_to_ce_1 {
        Attribute tessent_design_rtl_ports = "o_testmode_sibCaptureEn";
        Attribute tessent_design_gate_ports = "o_testmode_sibCaptureEn";
    }
    ToShiftEnPort o_testmode_sibShiftEn_ijtag_to_se_1 {
        Attribute tessent_design_rtl_ports = "o_testmode_sibShiftEn";
        Attribute tessent_design_gate_ports = "o_testmode_sibShiftEn";
    }
    ToUpdateEnPort o_testmode_sibUpdateEn_ijtag_to_ue_1 {
        Attribute tessent_design_rtl_ports = "o_testmode_sibUpdateEn";
        Attribute tessent_design_gate_ports = "o_testmode_sibUpdateEn";
    }
    LogicSignal o_testmode_sibSelect_value {
        (sib1_inst.sibSelect_reg_ijtag_do_2 == 3'b001);
    }
    DataOutPort o_testmode_sibSelect_ijtag_do_1 {
        Attribute tessent_design_rtl_ports = "o_testmode_sibSelect";
        Attribute tessent_design_gate_ports = "o_testmode_sibSelect";
        Source o_testmode_sibSelect_value;
    }
    ScanOutPort o_scanout_ijtag_so {
        Attribute tessent_design_rtl_ports = "o_scanout";
        Attribute tessent_design_gate_ports = "o_scanout";
        Source sib1_inst.TDO_ijtag_so;
    }
    ScanOutPort o_pmu_func_clk_gate_en_tdr_scanin_ijtag_to_si_2 {
        Attribute tessent_design_rtl_ports = "o_pmu_func_clk_gate_en_tdr_scanin";
        Attribute tessent_design_gate_ports = "o_pmu_func_clk_gate_en_tdr_scanin";
        Source i_scanin_ijtag_si;
    }
    ScanInPort i_pmu_func_clk_gate_en_tdr_scanout_ijtag_from_so_2 {
        Attribute tessent_design_rtl_ports = "i_pmu_func_clk_gate_en_tdr_scanout";
        Attribute tessent_design_gate_ports = "i_pmu_func_clk_gate_en_tdr_scanout";
    }
    ToCaptureEnPort o_pmu_func_clk_gate_en_tdr_sibCaptureEn_ijtag_to_ce_2 {
        Attribute tessent_design_rtl_ports = "o_pmu_func_clk_gate_en_tdr_sibCaptureEn";
        Attribute tessent_design_gate_ports = "o_pmu_func_clk_gate_en_tdr_sibCaptureEn";
    }
    ToShiftEnPort o_pmu_func_clk_gate_en_tdr_sibShiftEn_ijtag_to_se_2 {
        Attribute tessent_design_rtl_ports = "o_pmu_func_clk_gate_en_tdr_sibShiftEn";
        Attribute tessent_design_gate_ports = "o_pmu_func_clk_gate_en_tdr_sibShiftEn";
    }
    ToUpdateEnPort o_pmu_func_clk_gate_en_tdr_sibUpdateEn_ijtag_to_ue_2 {
        Attribute tessent_design_rtl_ports = "o_pmu_func_clk_gate_en_tdr_sibUpdateEn";
        Attribute tessent_design_gate_ports = "o_pmu_func_clk_gate_en_tdr_sibUpdateEn";
    }
    LogicSignal o_pmu_func_clk_gate_en_tdr_sibSelect_value {
        (sib1_inst.sibSelect_reg_ijtag_do_2 == 3'b010);
    }
    DataOutPort o_pmu_func_clk_gate_en_tdr_sibSelect_ijtag_do_2 {
        Attribute tessent_design_rtl_ports = "o_pmu_func_clk_gate_en_tdr_sibSelect";
        Attribute tessent_design_gate_ports = "o_pmu_func_clk_gate_en_tdr_sibSelect";
        Source o_pmu_func_clk_gate_en_tdr_sibSelect_value;
    }
    ScanOutPort o_pmu_path_select_tdr_scanin_ijtag_to_si_3[1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_path_select_tdr_scanin[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_path_select_tdr_scanin[1]}";
        Source i_scanin_ijtag_si;
    }
    ScanInPort i_pmu_path_select_tdr_scanout_ijtag_from_so_3[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_path_select_tdr_scanout[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_path_select_tdr_scanout[1]}";
    }
    ToCaptureEnPort o_pmu_path_select_tdr_sibCaptureEn_ijtag_to_ce_3[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_path_select_tdr_sibCaptureEn[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_path_select_tdr_sibCaptureEn[1]}";
    }
    ToShiftEnPort o_pmu_path_select_tdr_sibShiftEn_ijtag_to_se_3[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_path_select_tdr_sibShiftEn[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_path_select_tdr_sibShiftEn[1]}";
    }
    ToUpdateEnPort o_pmu_path_select_tdr_sibUpdateEn_ijtag_to_ue_3[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_path_select_tdr_sibUpdateEn[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_path_select_tdr_sibUpdateEn[1]}";
    }
    LogicSignal o_pmu_path_select_tdr_sibSelect_1__value {
        (sib2_inst_1.sibSelect_reg_ijtag_do_2 == 3'b001);
    }
    DataOutPort o_pmu_path_select_tdr_sibSelect_ijtag_do_3[1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_path_select_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_path_select_tdr_sibSelect[1]}";
        Source o_pmu_path_select_tdr_sibSelect_1__value;
    }
    LogicSignal o_pmu_path_select_tdr_sib_1__value {
        (sib2_inst_1.TDR_ijtag_do_1 == 3'b001);
    }
    DataOutPort o_pmu_path_select_tdr_sib_ijtag_do_4[1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_path_select_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_path_select_tdr_sib[1]}";
        Source o_pmu_path_select_tdr_sib_1__value;
    }
    ScanOutPort o_pmu_delay_select_tdr_scanin_ijtag_to_si_4[1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_delay_select_tdr_scanin[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_delay_select_tdr_scanin[1]}";
        Source i_scanin_ijtag_si;
    }
    ScanInPort i_pmu_delay_select_tdr_scanout_ijtag_from_so_4[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_delay_select_tdr_scanout[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_delay_select_tdr_scanout[1]}";
    }
    ToCaptureEnPort o_pmu_delay_select_tdr_sibCaptureEn_ijtag_to_ce_4[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_delay_select_tdr_sibCaptureEn[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_delay_select_tdr_sibCaptureEn[1]}";
    }
    ToShiftEnPort o_pmu_delay_select_tdr_sibShiftEn_ijtag_to_se_4[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_delay_select_tdr_sibShiftEn[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_delay_select_tdr_sibShiftEn[1]}";
    }
    ToUpdateEnPort o_pmu_delay_select_tdr_sibUpdateEn_ijtag_to_ue_4[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_delay_select_tdr_sibUpdateEn[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_delay_select_tdr_sibUpdateEn[1]}";
    }
    LogicSignal o_pmu_delay_select_tdr_sibSelect_1__value {
        (sib2_inst_1.sibSelect_reg_ijtag_do_2 == 3'b010);
    }
    DataOutPort o_pmu_delay_select_tdr_sibSelect_ijtag_do_5[1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_delay_select_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_delay_select_tdr_sibSelect[1]}";
        Source o_pmu_delay_select_tdr_sibSelect_1__value;
    }
    LogicSignal o_pmu_delay_select_tdr_sib_1__value {
        (sib2_inst_1.TDR_ijtag_do_1 == 3'b010);
    }
    DataOutPort o_pmu_delay_select_tdr_sib_ijtag_do_6[1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_delay_select_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_delay_select_tdr_sib[1]}";
        Source o_pmu_delay_select_tdr_sib_1__value;
    }
    ScanOutPort o_pmu_collect_status_tdr_scanin_ijtag_to_si_5[1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_collect_status_tdr_scanin[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_collect_status_tdr_scanin[1]}";
        Source i_scanin_ijtag_si;
    }
    ScanInPort i_pmu_collect_status_tdr_scanout_ijtag_from_so_5[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_collect_status_tdr_scanout[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_collect_status_tdr_scanout[1]}";
    }
    ToCaptureEnPort o_pmu_collect_status_tdr_sibCaptureEn_ijtag_to_ce_5[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_collect_status_tdr_sibCaptureEn[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_collect_status_tdr_sibCaptureEn[1]}";
    }
    ToShiftEnPort o_pmu_collect_status_tdr_sibShiftEn_ijtag_to_se_5[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_collect_status_tdr_sibShiftEn[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_collect_status_tdr_sibShiftEn[1]}";
    }
    ToUpdateEnPort o_pmu_collect_status_tdr_sibUpdateEn_ijtag_to_ue_5[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_collect_status_tdr_sibUpdateEn[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_collect_status_tdr_sibUpdateEn[1]}";
    }
    LogicSignal o_pmu_collect_status_tdr_sibSelect_1__value {
        (sib2_inst_1.sibSelect_reg_ijtag_do_2 == 3'b011);
    }
    DataOutPort o_pmu_collect_status_tdr_sibSelect_ijtag_do_7[1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_collect_status_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_collect_status_tdr_sibSelect[1]}";
        Source o_pmu_collect_status_tdr_sibSelect_1__value;
    }
    LogicSignal o_pmu_collect_status_tdr_sib_1__value {
        (sib2_inst_1.TDR_ijtag_do_1 == 3'b011);
    }
    DataOutPort o_pmu_collect_status_tdr_sib_ijtag_do_8[1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_collect_status_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_collect_status_tdr_sib[1]}";
        Source o_pmu_collect_status_tdr_sib_1__value;
    }
    ScanOutPort o_sib2_bypass_tdr_scanin_ijtag_to_si_6[1] {
        Attribute tessent_design_rtl_ports = "{o_sib2_bypass_tdr_scanin[1]}";
        Attribute tessent_design_gate_ports = "{o_sib2_bypass_tdr_scanin[1]}";
        Source i_scanin_ijtag_si;
    }
    ScanInPort i_sib2_bypass_tdr_scanout_ijtag_from_so_6[1:1] {
        Attribute tessent_design_rtl_ports = "{i_sib2_bypass_tdr_scanout[1]}";
        Attribute tessent_design_gate_ports = "{i_sib2_bypass_tdr_scanout[1]}";
    }
    ToCaptureEnPort o_sib2_bypass_tdr_sibCaptureEn_ijtag_to_ce_6[1:1] {
        Attribute tessent_design_rtl_ports = "{o_sib2_bypass_tdr_sibCaptureEn[1]}";
        Attribute tessent_design_gate_ports = "{o_sib2_bypass_tdr_sibCaptureEn[1]}";
    }
    ToShiftEnPort o_sib2_bypass_tdr_sibShiftEn_ijtag_to_se_6[1:1] {
        Attribute tessent_design_rtl_ports = "{o_sib2_bypass_tdr_sibShiftEn[1]}";
        Attribute tessent_design_gate_ports = "{o_sib2_bypass_tdr_sibShiftEn[1]}";
    }
    ToUpdateEnPort o_sib2_bypass_tdr_sibUpdateEn_ijtag_to_ue_6[1:1] {
        Attribute tessent_design_rtl_ports = "{o_sib2_bypass_tdr_sibUpdateEn[1]}";
        Attribute tessent_design_gate_ports = "{o_sib2_bypass_tdr_sibUpdateEn[1]}";
    }
    LogicSignal o_sib2_bypass_tdr_sibSelect_1__value {
        (sib2_inst_1.sibSelect_reg_ijtag_do_2 == 3'b000) && (loop_sel_inst.sibSelect_reg_ijtag_do_2 == 4'd1);
    }
    DataOutPort o_sib2_bypass_tdr_sibSelect_ijtag_do_9[1] {
        Attribute tessent_design_rtl_ports = "{o_sib2_bypass_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{o_sib2_bypass_tdr_sibSelect[1]}";
        Source o_sib2_bypass_tdr_sibSelect_1__value;
    }
    ScanInPort i_pmu_capture_ff_tdr_scanout_ijtag_from_so_7[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_capture_ff_tdr_scanout[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_capture_ff_tdr_scanout[1]}";
    }
    ScanOutPort o_pmu_capture_ff_tdr_scanin_ijtag_to_si_7[1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_tdr_scanin[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_tdr_scanin[1]}";
        Source i_scanin_ijtag_si;
    }
    ToCaptureEnPort o_pmu_capture_ff_tdr_sibCaptureEn_ijtag_to_ce_7[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_tdr_sibCaptureEn[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_tdr_sibCaptureEn[1]}";
    }
    ToShiftEnPort o_pmu_capture_ff_tdr_sibShiftEn_ijtag_to_se_7[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_tdr_sibShiftEn[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_tdr_sibShiftEn[1]}";
    }
    ToUpdateEnPort o_pmu_capture_ff_tdr_sibUpdateEn_ijtag_to_ue_7[1:1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_tdr_sibUpdateEn[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_tdr_sibUpdateEn[1]}";
    }
    LogicSignal o_pmu_capture_ff_tdr_sibSelect_1__value {
        (sib2_inst_1.sibSelect_reg_ijtag_do_2 == 3'b100);
    }
    DataOutPort o_pmu_capture_ff_tdr_sibSelect_ijtag_do_10[1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_tdr_sibSelect[1]}";
        Source o_pmu_capture_ff_tdr_sibSelect_1__value;
    }
    LogicSignal o_pmu_capture_ff_tdr_sib_1__value {
        (sib2_inst_1.TDR_ijtag_do_1 == 3'b100);
    }
    DataOutPort o_pmu_capture_ff_tdr_sib_ijtag_do_11[1] {
        Attribute tessent_design_rtl_ports = "{o_pmu_capture_ff_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{o_pmu_capture_ff_tdr_sib[1]}";
        Source o_pmu_capture_ff_tdr_sib_1__value;
    }
    ScanOutPort o_sib_loop_bypass_tdr_scanin_ijtag_to_si_8 {
        Attribute tessent_design_rtl_ports = "o_sib_loop_bypass_tdr_scanin";
        Attribute tessent_design_gate_ports = "o_sib_loop_bypass_tdr_scanin";
        Source i_scanin_ijtag_si;
    }
    ScanInPort i_sib_loop_bypass_tdr_scanout_ijtag_from_so_8 {
        Attribute tessent_design_rtl_ports = "i_sib_loop_bypass_tdr_scanout";
        Attribute tessent_design_gate_ports = "i_sib_loop_bypass_tdr_scanout";
    }
    ToCaptureEnPort o_sib_loop_bypass_tdr_sibCaptureEn_ijtag_to_ce_8 {
        Attribute tessent_design_rtl_ports = "o_sib_loop_bypass_tdr_sibCaptureEn";
        Attribute tessent_design_gate_ports = "o_sib_loop_bypass_tdr_sibCaptureEn";
    }
    ToShiftEnPort o_sib_loop_bypass_tdr_sibShiftEn_ijtag_to_se_8 {
        Attribute tessent_design_rtl_ports = "o_sib_loop_bypass_tdr_sibShiftEn";
        Attribute tessent_design_gate_ports = "o_sib_loop_bypass_tdr_sibShiftEn";
    }
    ToUpdateEnPort o_sib_loop_bypass_tdr_sibUpdateEn_ijtag_to_ue_8 {
        Attribute tessent_design_rtl_ports = "o_sib_loop_bypass_tdr_sibUpdateEn";
        Attribute tessent_design_gate_ports = "o_sib_loop_bypass_tdr_sibUpdateEn";
    }
    LogicSignal o_sib_loop_bypass_tdr_sibSelect_value {
        (loop_sel_inst.sibSelect_reg_ijtag_do_2 == 4'b0000) && (sib1_inst.sibSelect_reg_ijtag_do_2 == 3'b000);
    }
    DataOutPort o_sib_loop_bypass_tdr_sibSelect_ijtag_do_12 {
        Attribute tessent_design_rtl_ports = "o_sib_loop_bypass_tdr_sibSelect";
        Attribute tessent_design_gate_ports = "o_sib_loop_bypass_tdr_sibSelect";
        Source o_sib_loop_bypass_tdr_sibSelect_value;
    }
    LogicSignal sib2_inst_1_Select_value {
        i_select_ijtag_sel&&(loop_sel_inst.sibSelect_reg_ijtag_do_2== 4'd1);
    }
    Instance sib2_inst_1 Of PATH_MONITOR_CONTROLLER_PMM_1687_SIB_REG_3_SIB_2 {
        InputPort TDI_ijtag_si = sib2_muxout_1_;
        InputPort TRSTN_ijtag_reset = i_resetn_ijtag_reset;
        InputPort clk_ijtag_tck = i_tck_ijtag_tck;
        InputPort Select_ijtag_sel = sib2_inst_1_Select_value;
        InputPort ShiftEn_ijtag_se = i_shiftEn_ijtag_se;
        InputPort CaptureEn_ijtag_ce = i_captureEn_ijtag_ce;
        InputPort UpdateEn_ijtag_ue = i_updateEn_ijtag_ue;
    }
    LogicSignal loop_sel_inst_Select_value {
        i_select_ijtag_sel&&(sib1_inst.sibSelect_reg_ijtag_do_2==3'b000);
    }
    Instance loop_sel_inst Of PATH_MONITOR_CONTROLLER_PMM_1687_SIB_REG_4_SIB_LOOP_SEL {
        InputPort TDI_ijtag_si = loop_return;
        InputPort TRSTN_ijtag_reset = i_resetn_ijtag_reset;
        InputPort clk_ijtag_tck = i_tck_ijtag_tck;
        InputPort Select_ijtag_sel = loop_sel_inst_Select_value;
        InputPort ShiftEn_ijtag_se = i_shiftEn_ijtag_se;
        InputPort CaptureEn_ijtag_ce = i_captureEn_ijtag_ce;
        InputPort UpdateEn_ijtag_ue = i_updateEn_ijtag_ue;
    }
    Instance sib1_inst Of PATH_MONITOR_CONTROLLER_PMM_1687_SIB_REG_3_SIB_1 {
        InputPort TDI_ijtag_si = sib1_muxout;
        InputPort TRSTN_ijtag_reset = i_resetn_ijtag_reset;
        InputPort clk_ijtag_tck = i_tck_ijtag_tck;
        InputPort Select_ijtag_sel = i_select_ijtag_sel;
        InputPort ShiftEn_ijtag_se = i_shiftEn_ijtag_se;
        InputPort CaptureEn_ijtag_ce = i_captureEn_ijtag_ce;
        InputPort UpdateEn_ijtag_ue = i_updateEn_ijtag_ue;
    }
    ScanInterface client {
        Port i_scanin_ijtag_si;
        Port o_scanout_ijtag_so;
        Port i_select_ijtag_sel;
        Port i_shiftEn_ijtag_se;
        Port i_captureEn_ijtag_ce;
        Port i_updateEn_ijtag_ue;
        Port i_resetn_ijtag_reset;
        Port i_tck_ijtag_tck;
    }
    ScanInterface host_o_testmode_sibShiftEn {
        Port o_testmode_tdr_scanin_ijtag_to_si_1;
        Port i_testmode_tdr_scanout_ijtag_from_so_1;
        Port o_testmode_sibShiftEn_ijtag_to_se_1;
        Port o_testmode_sibCaptureEn_ijtag_to_ce_1;
        Port o_testmode_sibUpdateEn_ijtag_to_ue_1;
    }
    ScanInterface host_o_pmu_func_clk_gate_en_tdr_sibShiftEn {
        Port o_pmu_func_clk_gate_en_tdr_scanin_ijtag_to_si_2;
        Port i_pmu_func_clk_gate_en_tdr_scanout_ijtag_from_so_2;
        Port o_pmu_func_clk_gate_en_tdr_sibShiftEn_ijtag_to_se_2;
        Port o_pmu_func_clk_gate_en_tdr_sibCaptureEn_ijtag_to_ce_2;
        Port o_pmu_func_clk_gate_en_tdr_sibUpdateEn_ijtag_to_ue_2;
    }
    ScanInterface host_o_pmu_path_select_tdr_sibShiftEn1 {
        Port i_pmu_path_select_tdr_scanout_ijtag_from_so_3[1];
        Port o_pmu_path_select_tdr_sibShiftEn_ijtag_to_se_3[1];
        Port o_pmu_path_select_tdr_sibCaptureEn_ijtag_to_ce_3[1];
        Port o_pmu_path_select_tdr_sibUpdateEn_ijtag_to_ue_3[1];
    }
    ScanInterface host_o_pmu_delay_select_tdr_sibShiftEn1 {
        Port i_pmu_delay_select_tdr_scanout_ijtag_from_so_4[1];
        Port o_pmu_delay_select_tdr_sibShiftEn_ijtag_to_se_4[1];
        Port o_pmu_delay_select_tdr_sibCaptureEn_ijtag_to_ce_4[1];
        Port o_pmu_delay_select_tdr_sibUpdateEn_ijtag_to_ue_4[1];
    }
    ScanInterface host_o_pmu_collect_status_tdr_sibShiftEn1 {
        Port i_pmu_collect_status_tdr_scanout_ijtag_from_so_5[1];
        Port o_pmu_collect_status_tdr_sibShiftEn_ijtag_to_se_5[1];
        Port o_pmu_collect_status_tdr_sibCaptureEn_ijtag_to_ce_5[1];
        Port o_pmu_collect_status_tdr_sibUpdateEn_ijtag_to_ue_5[1];
    }
    ScanInterface host_o_sib2_bypass_tdr_sibShiftEn1 {
        Port o_sib2_bypass_tdr_scanin_ijtag_to_si_6[1];
        Port i_sib2_bypass_tdr_scanout_ijtag_from_so_6[1];
        Port o_sib2_bypass_tdr_sibShiftEn_ijtag_to_se_6[1];
        Port o_sib2_bypass_tdr_sibCaptureEn_ijtag_to_ce_6[1];
        Port o_sib2_bypass_tdr_sibUpdateEn_ijtag_to_ue_6[1];
    }
    ScanInterface host_o_pmu_capture_ff_tdr_sibShiftEn1 {
        Port i_pmu_capture_ff_tdr_scanout_ijtag_from_so_7[1];
        Port o_pmu_capture_ff_tdr_sibShiftEn_ijtag_to_se_7[1];
        Port o_pmu_capture_ff_tdr_sibCaptureEn_ijtag_to_ce_7[1];
        Port o_pmu_capture_ff_tdr_sibUpdateEn_ijtag_to_ue_7[1];
    }
    ScanInterface host_o_sib_loop_bypass_tdr_sibShiftEn {
        Port o_sib_loop_bypass_tdr_scanin_ijtag_to_si_8;
        Port i_sib_loop_bypass_tdr_scanout_ijtag_from_so_8;
        Port o_sib_loop_bypass_tdr_sibShiftEn_ijtag_to_se_8;
        Port o_sib_loop_bypass_tdr_sibCaptureEn_ijtag_to_ce_8;
        Port o_sib_loop_bypass_tdr_sibUpdateEn_ijtag_to_ue_8;
    }
}
