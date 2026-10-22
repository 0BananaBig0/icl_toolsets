// Generated; edit the PMM sources, not this file.

NameSpace ;
UseNameSpace ;
Module PATH_MONITOR_CONTROLLER_clk_cntrl {
    // Sources describe settled controls; holding edges and clock routing are abstracted.
    DataInPort i_pmu_clock_mux_en_ijtag_di_1 {
        Attribute tessent_design_rtl_ports = "i_pmu_clock_mux_en";
        Attribute tessent_design_gate_ports = "i_pmu_clock_mux_en";
    }
    DataInPort i_pmu_mode_ijtag_di_2[1:0] {
        Attribute tessent_design_rtl_ports = "{i_pmu_mode[1]} {i_pmu_mode[0]}";
        Attribute tessent_design_gate_ports = "{i_pmu_mode[1]} {i_pmu_mode[0]}";
    }
    ResetPort i_resetn_ijtag_reset {
        Attribute tessent_design_rtl_ports = "i_resetn";
        Attribute tessent_design_gate_ports = "i_resetn";
        ActivePolarity 0;
    }
    DataInPort i_testmode_sibSelect_ijtag_di_3 {
        Attribute tessent_design_rtl_ports = "i_testmode_sibSelect";
        Attribute tessent_design_gate_ports = "i_testmode_sibSelect";
    }
    DataInPort i_sib_loop_bypass_tdr_sibSelect_ijtag_di_4 {
        Attribute tessent_design_rtl_ports = "i_sib_loop_bypass_tdr_sibSelect";
        Attribute tessent_design_gate_ports = "i_sib_loop_bypass_tdr_sibSelect";
    }
    DataInPort i_pmu_path_select_tdr_sibSelect_ijtag_di_5[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_path_select_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_path_select_tdr_sibSelect[1]}";
    }
    DataInPort i_pmu_path_select_tdr_sib_ijtag_di_6[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_path_select_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_path_select_tdr_sib[1]}";
    }
    DataInPort i_pmu_delay_select_tdr_sibSelect_ijtag_di_7[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_delay_select_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_delay_select_tdr_sibSelect[1]}";
    }
    DataInPort i_pmu_delay_select_tdr_sib_ijtag_di_8[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_delay_select_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_delay_select_tdr_sib[1]}";
    }
    DataInPort i_pmu_collect_status_tdr_sibSelect_ijtag_di_9[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_collect_status_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_collect_status_tdr_sibSelect[1]}";
    }
    DataInPort i_pmu_collect_status_tdr_sib_ijtag_di_10[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_collect_status_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_collect_status_tdr_sib[1]}";
    }
    DataInPort i_pmu_func_clk_gate_en_tdr_sibSelect_ijtag_di_11 {
        Attribute tessent_design_rtl_ports = "i_pmu_func_clk_gate_en_tdr_sibSelect";
        Attribute tessent_design_gate_ports = "i_pmu_func_clk_gate_en_tdr_sibSelect";
    }
    DataInPort i_sib2_bypass_tdr_sibSelect_ijtag_di_12[1:1] {
        Attribute tessent_design_rtl_ports = "{i_sib2_bypass_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{i_sib2_bypass_tdr_sibSelect[1]}";
    }
    DataInPort i_pmu_capture_ff_tdr_sibSelect_ijtag_di_13[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_capture_ff_tdr_sibSelect[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_capture_ff_tdr_sibSelect[1]}";
    }
    DataInPort i_pmu_capture_ff_tdr_sib_ijtag_di_14[1:1] {
        Attribute tessent_design_rtl_ports = "{i_pmu_capture_ff_tdr_sib[1]}";
        Attribute tessent_design_gate_ports = "{i_pmu_capture_ff_tdr_sib[1]}";
    }
    LogicSignal o_wrck_clk_mux_en_value {
        ((((i_pmu_mode_ijtag_di_2==2'b00)||(i_pmu_mode_ijtag_di_2==2'b11))&&(i_pmu_collect_status_tdr_sib_ijtag_di_10[1]||i_pmu_delay_select_tdr_sib_ijtag_di_8[1]||i_pmu_path_select_tdr_sib_ijtag_di_6[1]||i_pmu_capture_ff_tdr_sib_ijtag_di_14[1]))||(!((i_pmu_mode_ijtag_di_2==2'b00)||(i_pmu_mode_ijtag_di_2==2'b11))&&((i_pmu_collect_status_tdr_sib_ijtag_di_10[1]||i_pmu_capture_ff_tdr_sibSelect_ijtag_di_13[1]))))&&i_pmu_clock_mux_en_ijtag_di_1;
    }
    DataOutPort o_wrck_clk_mux_en_ijtag_do_1[1:1] {
        Attribute tessent_design_rtl_ports = "{o_wrck_clk_mux_en[1]}";
        Attribute tessent_design_gate_ports = "{o_wrck_clk_mux_en[1]}";
        Source o_wrck_clk_mux_en_value;
    }
    LogicSignal o_data_clamp_enable_value {
        (i_pmu_delay_select_tdr_sibSelect_ijtag_di_7[1])||(i_pmu_path_select_tdr_sibSelect_ijtag_di_5[1]);
    }
    DataOutPort o_data_clamp_enable_ijtag_do_2 {
        Attribute tessent_design_rtl_ports = "o_data_clamp_enable";
        Attribute tessent_design_gate_ports = "o_data_clamp_enable";
        Source o_data_clamp_enable_value;
    }
    LogicSignal o_mutex_clk_gate_en_value {
        i_sib_loop_bypass_tdr_sibSelect_ijtag_di_4||(i_pmu_path_select_tdr_sibSelect_ijtag_di_5[1])||i_testmode_sibSelect_ijtag_di_3||(i_pmu_delay_select_tdr_sibSelect_ijtag_di_7[1])||(i_pmu_collect_status_tdr_sibSelect_ijtag_di_9[1])||i_pmu_func_clk_gate_en_tdr_sibSelect_ijtag_di_11||(i_sib2_bypass_tdr_sibSelect_ijtag_di_12[1])||(i_pmu_capture_ff_tdr_sibSelect_ijtag_di_13[1]);
    }
    DataOutPort o_mutex_clk_gate_en_ijtag_do_3 {
        Attribute tessent_design_rtl_ports = "o_mutex_clk_gate_en";
        Attribute tessent_design_gate_ports = "o_mutex_clk_gate_en";
        Source o_mutex_clk_gate_en_value;
    }
    TCKPort i_wrck_gated_mutex_ijtag_tck {
        Attribute tessent_design_rtl_ports = "i_wrck_gated_mutex";
        Attribute tessent_design_gate_ports = "i_wrck_gated_mutex";
    }
}
