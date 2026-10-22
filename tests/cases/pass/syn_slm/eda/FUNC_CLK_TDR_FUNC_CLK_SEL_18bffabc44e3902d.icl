// Generated; edit the PMM sources, not this file.

NameSpace ;
UseNameSpace ;
Module FUNC_CLK_TDR_FUNC_CLK_SEL {
    ShiftEnPort Shift_ijtag_se;
    CaptureEnPort Capture_ijtag_ce;
    UpdateEnPort Update_ijtag_ue;
    SelectPort Select_ijtag_sel;
    ResetPort TRSTN_ijtag_reset {
        ActivePolarity 0;
    }
    TCKPort TCK_ijtag_tck;
    ScanInPort SI_ijtag_si;
    ScanOutPort SO_ijtag_so {
        Source Value[0];
    }
    ScanInterface client {
        Port SI_ijtag_si;
        Port SO_ijtag_so;
        Port Shift_ijtag_se;
        Port Capture_ijtag_ce;
        Port Update_ijtag_ue;
        Port Select_ijtag_sel;
        Port TRSTN_ijtag_reset;
        Port TCK_ijtag_tck;
    }
    ScanRegister Value[14:0] {
        ScanInSource SI_ijtag_si;
        CaptureSource Value;
        ResetValue 15'b0;
        Attribute synopsys_access = "RW";
    }
    DataOutPort func_clk_enable1_ijtag_do {
        Source Value[0];
    }
}
