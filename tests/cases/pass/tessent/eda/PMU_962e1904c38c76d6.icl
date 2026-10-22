// Scan-only model; hierarchy and port names match chip_top.v.


NameSpace ;
UseNameSpace ;
Module PMU {
    ResetPort ijtag_reset { ActivePolarity 0; }
    TCKPort ijtag_tck;
    CaptureEnPort ijtag_ce;
    ShiftEnPort ijtag_se;
    ScanInPort counter_si;
    ScanInPort capture_si;
    SelectPort counter_sel;
    SelectPort capture_sel;
    DataInPort counters[15:0];
    DataInPort captures[2:0];
    ScanOutPort counter_so { Source Counter2.ijtag_so; }
    ScanOutPort capture_so { Source Capture2.ijtag_so; }
    Instance Counter1 Of TDR_RO {
        Parameter WIDTH = 8;
        InputPort ijtag_reset = ijtag_reset;
        InputPort ijtag_tck = ijtag_tck;
        InputPort ijtag_ce = ijtag_ce;
        InputPort ijtag_se = ijtag_se;
        InputPort ijtag_sel = counter_sel;
        InputPort ijtag_si = counter_si;
        InputPort ijtag_di = counters[7:0];
    }
    Instance Counter2 Of TDR_RO {
        Parameter WIDTH = 8;
        InputPort ijtag_reset = ijtag_reset;
        InputPort ijtag_tck = ijtag_tck;
        InputPort ijtag_ce = ijtag_ce;
        InputPort ijtag_se = ijtag_se;
        InputPort ijtag_sel = counter_sel;
        InputPort ijtag_si = Counter1.ijtag_so;
        InputPort ijtag_di = counters[15:8];
    }
    Instance Shadow Of TDR_RO {
        Parameter WIDTH = 1;
        InputPort ijtag_reset = ijtag_reset;
        InputPort ijtag_tck = ijtag_tck;
        InputPort ijtag_ce = ijtag_ce;
        InputPort ijtag_se = ijtag_se;
        InputPort ijtag_sel = capture_sel;
        InputPort ijtag_si = capture_si;
        InputPort ijtag_di = captures[0];
    }
    Instance Capture1 Of TDR_RO {
        Parameter WIDTH = 1;
        InputPort ijtag_reset = ijtag_reset;
        InputPort ijtag_tck = ijtag_tck;
        InputPort ijtag_ce = ijtag_ce;
        InputPort ijtag_se = ijtag_se;
        InputPort ijtag_sel = capture_sel;
        InputPort ijtag_si = Shadow.ijtag_so;
        InputPort ijtag_di = captures[1];
    }
    Instance Capture2 Of TDR_RO {
        Parameter WIDTH = 1;
        InputPort ijtag_reset = ijtag_reset;
        InputPort ijtag_tck = ijtag_tck;
        InputPort ijtag_ce = ijtag_ce;
        InputPort ijtag_se = ijtag_se;
        InputPort ijtag_sel = capture_sel;
        InputPort ijtag_si = Capture1.ijtag_so;
        InputPort ijtag_di = captures[2];
    }
    ScanInterface counter {
        Port counter_si;
        Port counter_so;
        Port counter_sel;
        Port ijtag_reset;
        Port ijtag_tck;
        Port ijtag_ce;
        Port ijtag_se;
    }
    ScanInterface capture {
        Port capture_si;
        Port capture_so;
        Port capture_sel;
        Port ijtag_reset;
        Port ijtag_tck;
        Port ijtag_ce;
        Port ijtag_se;
    }
}
