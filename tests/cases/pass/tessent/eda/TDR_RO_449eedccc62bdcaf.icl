// Scan-only model; hierarchy and port names match chip_top.v.


NameSpace ;
UseNameSpace ;
Module TDR_RO {
    Parameter WIDTH = 1;
    ResetPort ijtag_reset { ActivePolarity 0; }
    TCKPort ijtag_tck;
    CaptureEnPort ijtag_ce;
    ShiftEnPort ijtag_se;
    SelectPort ijtag_sel;
    ScanInPort ijtag_si;
    DataInPort ijtag_di[$WIDTH-1:0];
    ScanOutPort ijtag_so { Source Value[0]; }
    ScanRegister Value[$WIDTH-1:0] {
        ScanInSource ijtag_si;
        CaptureSource ijtag_di;
        ResetValue 0;
    }
    ScanInterface client {
        Port ijtag_si;
        Port ijtag_so;
        Port ijtag_reset;
        Port ijtag_tck;
        Port ijtag_ce;
        Port ijtag_se;
        Port ijtag_sel;
    }
}
