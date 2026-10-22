// Generated; edit the PMM sources, not this file.

NameSpace ;
UseNameSpace ;
Module PATH_MONITOR_CONTROLLER_TEST_MODE_DECODER_PM_MODE {
    DataInPort i_test_mode_ijtag_di[5:0] {
        Attribute tessent_design_rtl_ports = "{i_test_mode[5]} {i_test_mode[4]} {i_test_mode[3]} {i_test_mode[2]} {i_test_mode[1]} {i_test_mode[0]}";
        Attribute tessent_design_gate_ports = "{i_test_mode[5]} {i_test_mode[4]} {i_test_mode[3]} {i_test_mode[2]} {i_test_mode[1]} {i_test_mode[0]}";
    }
    DataOutPort o_pmu_testmode_ijtag_do_1[1:0] {
        Attribute tessent_design_rtl_ports = "{o_pmu_testmode[1]} {o_pmu_testmode[0]}";
        Attribute tessent_design_gate_ports = "{o_pmu_testmode[1]} {o_pmu_testmode[0]}";
        Source i_test_mode_ijtag_di[1:0];
    }
    LogicSignal o_pmu_clock_mux_en_value {
        (i_test_mode_ijtag_di[1] && i_test_mode_ijtag_di[0]) || (!i_test_mode_ijtag_di[1] && !i_test_mode_ijtag_di[0]);
    }
    DataOutPort o_pmu_clock_mux_en_ijtag_do_2 {
        Attribute tessent_design_rtl_ports = "o_pmu_clock_mux_en";
        Attribute tessent_design_gate_ports = "o_pmu_clock_mux_en";
        Source o_pmu_clock_mux_en_value;
    }
    DataOutPort o_clkdiv_cg_test_ijtag_do_3 {
        Attribute tessent_design_rtl_ports = "o_clkdiv_cg_test";
        Attribute tessent_design_gate_ports = "o_clkdiv_cg_test";
        Source i_test_mode_ijtag_di[5];
    }
    DataOutPort o_pmc_test_en_ijtag_do_4 {
        Attribute tessent_design_rtl_ports = "o_pmc_test_en";
        Attribute tessent_design_gate_ports = "o_pmc_test_en";
        Source i_test_mode_ijtag_di[4];
    }
    DataOutPort o_pmc_mode_tdr_override_ijtag_do_5 {
        Attribute tessent_design_rtl_ports = "o_pmc_mode_tdr_override";
        Attribute tessent_design_gate_ports = "o_pmc_mode_tdr_override";
        Source i_test_mode_ijtag_di[2];
    }
    DataOutPort debug_en_tdr_ijtag_do_6 {
        Attribute tessent_design_rtl_ports = "debug_en_tdr";
        Attribute tessent_design_gate_ports = "debug_en_tdr";
        Source i_test_mode_ijtag_di[3];
    }
}
