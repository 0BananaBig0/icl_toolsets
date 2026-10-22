// Scan-only model; hierarchy and port names match chip_top.v.


NameSpace ;
UseNameSpace ;
Module SIB1 {
    ResetPort ijtag_reset { ActivePolarity 0; }
    TCKPort ijtag_tck;
    CaptureEnPort ijtag_ce;
    ShiftEnPort ijtag_se;
    SelectPort ijtag_sel;
    UpdateEnPort ijtag_ue;
    ScanInPort ijtag_si;
    ScanInPort ijtag_from_so;
    ScanOutPort ijtag_so { Source Value[0]; }
    ScanOutPort ijtag_to_si { Source ijtag_si; }
    ToSelectPort ijtag_to_sel { Source enable; }
    LogicSignal enable { ijtag_sel && Value; }
    ScanRegister Value[0:0] {
        ScanInSource mux;
        CaptureSource Value;
        ResetValue 0;
    }
    ScanMux mux SelectedBy Value {
        1'b0 : ijtag_si;
        1'b1 : ijtag_from_so;
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
    ScanInterface host0 {
        Port ijtag_to_si;
        Port ijtag_from_so;
        Port ijtag_to_sel;
    }
}
