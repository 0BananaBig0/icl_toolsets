// Scan-only model; hierarchy and port names match chip_top.v.


NameSpace ;
UseNameSpace ;
Module PMC {
    ResetPort ijtag_reset { ActivePolarity 0; }
    TCKPort ijtag_tck;
    CaptureEnPort ijtag_ce;
    ShiftEnPort ijtag_se;
    SelectPort ijtag_sel;
    UpdateEnPort ijtag_ue;
    ScanInPort ijtag_si;
    ScanInPort counter_from_so;
    ScanInPort capture_from_so;
    ScanOutPort ijtag_so { Source SIB1.ijtag_so; }
    ScanOutPort counter_to_si { Source SIB2.ijtag_to_si; }
    ScanOutPort capture_to_si { Source SIB2.ijtag_to_si; }
    ToSelectPort counter_to_sel { Source SIB2.ijtag_to_sel[2]; }
    ToSelectPort capture_to_sel { Source SIB2.ijtag_to_sel[3]; }
    DataOutPort mode_select { Source MODE_SELECT.ijtag_do; }
    DataOutPort path_select[2:0] { Source PATH_SELECT.ijtag_do; }
    Instance SIB1 Of SIB1 {
        InputPort ijtag_reset = ijtag_reset;
        InputPort ijtag_tck = ijtag_tck;
        InputPort ijtag_ce = ijtag_ce;
        InputPort ijtag_se = ijtag_se;
        InputPort ijtag_ue = ijtag_ue;
        InputPort ijtag_sel = ijtag_sel;
        InputPort ijtag_si = ijtag_si;
        InputPort ijtag_from_so = SIB2.ijtag_so;
    }
    Instance SIB2 Of SIB2 {
        InputPort ijtag_reset = ijtag_reset;
        InputPort ijtag_tck = ijtag_tck;
        InputPort ijtag_ce = ijtag_ce;
        InputPort ijtag_se = ijtag_se;
        InputPort ijtag_ue = ijtag_ue;
        InputPort ijtag_sel = SIB1.ijtag_to_sel;
        InputPort ijtag_si = SIB1.ijtag_to_si;
        InputPort ijtag_from_so = capture_from_so, counter_from_so, PATH_SELECT.ijtag_so, MODE_SELECT.ijtag_so;
    }
    Instance MODE_SELECT Of TDR_RW {
        Parameter WIDTH = 1;
        InputPort ijtag_reset = ijtag_reset;
        InputPort ijtag_tck = ijtag_tck;
        InputPort ijtag_ce = ijtag_ce;
        InputPort ijtag_se = ijtag_se;
        InputPort ijtag_ue = ijtag_ue;
        InputPort ijtag_sel = SIB2.ijtag_to_sel[0];
        InputPort ijtag_si = SIB2.ijtag_to_si;
    }
    Instance PATH_SELECT Of TDR_RW {
        Parameter WIDTH = 3;
        InputPort ijtag_reset = ijtag_reset;
        InputPort ijtag_tck = ijtag_tck;
        InputPort ijtag_ce = ijtag_ce;
        InputPort ijtag_se = ijtag_se;
        InputPort ijtag_ue = ijtag_ue;
        InputPort ijtag_sel = SIB2.ijtag_to_sel[1];
        InputPort ijtag_si = SIB2.ijtag_to_si;
    }
    ScanInterface client {
        Port ijtag_si;
        Port ijtag_so;
        Port ijtag_reset;
        Port ijtag_tck;
        Port ijtag_ce;
        Port ijtag_se;
        Port ijtag_sel;
        Port ijtag_ue;
    }
    ScanInterface counters {
        Port counter_to_si;
        Port counter_from_so;
        Port counter_to_sel;
    }
    ScanInterface captures {
        Port capture_to_si;
        Port capture_from_so;
        Port capture_to_sel;
    }
}
