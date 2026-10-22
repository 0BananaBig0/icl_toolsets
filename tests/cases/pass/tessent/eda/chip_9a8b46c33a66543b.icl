// Scan-only model; hierarchy and port names match chip_top.v.


NameSpace ;
UseNameSpace ;
Module chip {
    ResetPort reset_n { ActivePolarity 0; }
    TCKPort TCK;
    ScanInPort TDI;
    SelectPort sel;
    ShiftEnPort shift_en;
    UpdateEnPort update_en;
    CaptureEnPort capture_en;
    DataInPort pmu0_counters[15:0];
    DataInPort pmu1_counters[15:0];
    DataInPort pmu0_captures[2:0];
    DataInPort pmu1_captures[2:0];
    ScanOutPort TDO { Source PMC.ijtag_so; }
    DataOutPort mode_select { Source PMC.mode_select; }
    DataOutPort path_select[2:0] { Source PMC.path_select; }
    Instance PMC Of PMC {
        InputPort ijtag_reset = reset_n;
        InputPort ijtag_tck = TCK;
        InputPort ijtag_ce = capture_en;
        InputPort ijtag_se = shift_en;
        InputPort ijtag_sel = sel;
        InputPort ijtag_ue = update_en;
        InputPort ijtag_si = TDI;
        InputPort counter_from_so = PMU1.counter_so;
        InputPort capture_from_so = PMU1.capture_so;
    }
    Instance PMU0 Of PMU {
        InputPort ijtag_reset = reset_n;
        InputPort ijtag_tck = TCK;
        InputPort ijtag_ce = capture_en;
        InputPort ijtag_se = shift_en;
        InputPort counter_si = PMC.counter_to_si;
        InputPort capture_si = PMC.capture_to_si;
        InputPort counter_sel = PMC.counter_to_sel;
        InputPort capture_sel = PMC.capture_to_sel;
        InputPort counters = pmu0_counters;
        InputPort captures = pmu0_captures;
    }
    Instance PMU1 Of PMU {
        InputPort ijtag_reset = reset_n;
        InputPort ijtag_tck = TCK;
        InputPort ijtag_ce = capture_en;
        InputPort ijtag_se = shift_en;
        InputPort counter_si = PMU0.counter_so;
        InputPort capture_si = PMU0.capture_so;
        InputPort counter_sel = PMC.counter_to_sel;
        InputPort capture_sel = PMC.capture_to_sel;
        InputPort counters = pmu1_counters;
        InputPort captures = pmu1_captures;
    }
    ScanInterface client {
        Port TDI;
        Port TDO;
        Port reset_n;
        Port TCK;
        Port sel;
        Port shift_en;
        Port update_en;
        Port capture_en;
    }
}
