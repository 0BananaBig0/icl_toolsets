// Scan-only model; hierarchy and port names match chip_top.v.


NameSpace ;
UseNameSpace ;
Module SIB2 {
    ResetPort ijtag_reset { ActivePolarity 0; }
    TCKPort ijtag_tck;
    CaptureEnPort ijtag_ce;
    ShiftEnPort ijtag_se;
    SelectPort ijtag_sel;
    UpdateEnPort ijtag_ue;
    ScanInPort ijtag_si;
    ScanInPort ijtag_from_so[3:0];
    ScanOutPort ijtag_so { Source Value[0]; }
    ScanOutPort ijtag_to_si { Source ijtag_si; }
    ToSelectPort ijtag_to_sel[3:0] { Source enable3, enable2, enable1, enable0; }
    LogicSignal enable0 { ijtag_sel && (Value == 2'd0); }
    LogicSignal enable1 { ijtag_sel && (Value == 2'd1); }
    LogicSignal enable2 { ijtag_sel && (Value == 2'd2); }
    LogicSignal enable3 { ijtag_sel && (Value == 2'd3); }
    ScanRegister Value[1:0] {
        ScanInSource mux;
        CaptureSource Value;
        ResetValue 0;
    }
    ScanMux mux SelectedBy Value {
        2'd0 : ijtag_from_so[0];
        2'd1 : ijtag_from_so[1];
        2'd2 : ijtag_from_so[2];
        2'd3 : ijtag_from_so[3];
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
        Port ijtag_from_so[0];
        Port ijtag_to_sel[0];
    }
    ScanInterface host1 {
        Port ijtag_to_si;
        Port ijtag_from_so[1];
        Port ijtag_to_sel[1];
    }
    ScanInterface host2 {
        Port ijtag_to_si;
        Port ijtag_from_so[2];
        Port ijtag_to_sel[2];
    }
    ScanInterface host3 {
        Port ijtag_to_si;
        Port ijtag_from_so[3];
        Port ijtag_to_sel[3];
    }
}
