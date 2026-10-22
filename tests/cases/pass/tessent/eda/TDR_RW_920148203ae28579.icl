// Scan-only model; hierarchy and port names match chip_top.v.


NameSpace ;
UseNameSpace ;
Module TDR_RW {
    Parameter WIDTH = 1;
    ResetPort ijtag_reset { ActivePolarity 0; }
    TCKPort ijtag_tck;
    CaptureEnPort ijtag_ce;
    ShiftEnPort ijtag_se;
    SelectPort ijtag_sel;
    UpdateEnPort ijtag_ue;
    ScanInPort ijtag_si;
    ScanOutPort ijtag_so { Source Value[0]; }
    ScanRegister Value[$WIDTH-1:0] {
        ScanInSource ijtag_si;
        CaptureSource Value;
        ResetValue 0;
    }
    DataOutPort ijtag_do[$WIDTH-1:0] { Source Value; }
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
}
