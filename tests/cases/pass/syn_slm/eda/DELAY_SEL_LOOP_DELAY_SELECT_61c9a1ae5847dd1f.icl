// Generated; edit the PMM sources, not this file.

NameSpace ;
UseNameSpace ;
Module DELAY_SEL_LOOP_DELAY_SELECT {
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
    ScanRegister Value[4:0] {
        ScanInSource SI_ijtag_si;
        ResetValue 5'b0;
        Attribute synopsys_access = "RW";
    }
    DataOutPort delay_sel_ijtag_do[4:0] {
        Source Value;
    }
}
