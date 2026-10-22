// Generated; edit the PMM sources, not this file.

NameSpace ;
UseNameSpace ;
Module PATH_MONITOR_CONTROLLER_IEEE_1687_pmu_tdr_PATH_SELECT_DELAY_SELECT_CAPTURE_SHIFT_COLLECT_STATUS {
    // Stable shift topology; capture/update holding and phase gating are abstracted.
    TCKPort i_tck_ijtag_tck {
        Attribute tessent_design_rtl_ports = "i_tck";
        Attribute tessent_design_gate_ports = "i_tck";
    }
    ResetPort i_resetn_ijtag_reset {
        Attribute tessent_design_rtl_ports = "i_resetn";
        Attribute tessent_design_gate_ports = "i_resetn";
        ActivePolarity 0;
    }
    ShiftEnPort i_pmu_tdr_sibShiftEn_ijtag_se {
        Attribute tessent_design_rtl_ports = "i_pmu_tdr_sibShiftEn";
        Attribute tessent_design_gate_ports = "i_pmu_tdr_sibShiftEn";
    }
    DataInPort i_pmu_tdr_sib_ijtag_di {
        Attribute tessent_design_rtl_ports = "i_pmu_tdr_sib";
        Attribute tessent_design_gate_ports = "i_pmu_tdr_sib";
    }
    CaptureEnPort i_pmu_tdr_sibCaptureEn_ijtag_ce {
        Attribute tessent_design_rtl_ports = "i_pmu_tdr_sibCaptureEn";
        Attribute tessent_design_gate_ports = "i_pmu_tdr_sibCaptureEn";
    }
    UpdateEnPort i_pmu_tdr_sibUpdateEn_ijtag_ue {
        Attribute tessent_design_rtl_ports = "i_pmu_tdr_sibUpdateEn";
        Attribute tessent_design_gate_ports = "i_pmu_tdr_sibUpdateEn";
    }
    ScanInPort i_pmu_tdr_scanin_ijtag_si {
        Attribute tessent_design_rtl_ports = "i_pmu_tdr_scanin";
        Attribute tessent_design_gate_ports = "i_pmu_tdr_scanin";
    }
    ScanOutPort o_pmu_tdr_scanout_ijtag_so {
        Attribute tessent_design_rtl_ports = "o_pmu_tdr_scanout";
        Attribute tessent_design_gate_ports = "o_pmu_tdr_scanout";
        Source i_pmu_tdr_data_out_ijtag_from_so;
    }
    ScanOutPort o_pmu_tdr_data_in_ijtag_to_si {
        Attribute tessent_design_rtl_ports = "o_pmu_tdr_data_in";
        Attribute tessent_design_gate_ports = "o_pmu_tdr_data_in";
        Source i_pmu_tdr_scanin_ijtag_si;
    }
    ToSelectPort o_pmu_shift_ijtag_to_sel {
        Attribute tessent_design_rtl_ports = "o_pmu_shift";
        Attribute tessent_design_gate_ports = "o_pmu_shift";
        Source i_pmu_tdr_sib_ijtag_di;
    }
    ScanInPort i_pmu_tdr_data_out_ijtag_from_so {
        Attribute tessent_design_rtl_ports = "i_pmu_tdr_data_out";
        Attribute tessent_design_gate_ports = "i_pmu_tdr_data_out";
    }
    ScanInterface client {
        Port i_pmu_tdr_scanin_ijtag_si;
        Port o_pmu_tdr_scanout_ijtag_so;
        Port i_pmu_tdr_sibShiftEn_ijtag_se;
        Port i_pmu_tdr_sibCaptureEn_ijtag_ce;
        Port i_pmu_tdr_sibUpdateEn_ijtag_ue;
        Port i_tck_ijtag_tck;
        Port i_resetn_ijtag_reset;
    }
    ScanInterface host {
        Port i_pmu_tdr_data_out_ijtag_from_so;
        Port o_pmu_shift_ijtag_to_sel;
    }
}
