// Generated; edit the PMM sources, not this file.

NameSpace ;
UseNameSpace ;
Module COLLECT_LOOP_COLLECT_STATUS {
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
        Source Toggle;
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
    ScanRegister Status {
        ScanInSource SI_ijtag_si;
        ResetValue 1'b0;
        Attribute synopsys_access = "RO";
    }
    ScanRegister Toggle {
        ScanInSource Status;
        ResetValue 1'b0;
        Attribute synopsys_access = "RO";
    }
    DataOutPort status_ijtag_do_1 {
        Source Status;
    }
    DataOutPort toggle_ijtag_do_2 {
        Source Toggle;
    }
}
