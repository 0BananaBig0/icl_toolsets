// Generated; edit the PMM sources, not this file.

NameSpace ;
UseNameSpace ;
Module CAPTURE_LOOP_CAPTURE_SHIFT {
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
        Source Shadow;
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
    ScanRegister CaptureBit {
        ScanInSource SI_ijtag_si;
        ResetValue 1'b0;
        Attribute synopsys_access = "RO";
    }
    ScanRegister Shadow {
        ScanInSource CaptureBit;
        ResetValue 1'b0;
        Attribute synopsys_access = "RO";
    }
    DataOutPort capturebit_ijtag_do_1 {
        Source CaptureBit;
    }
    DataOutPort shadow_ijtag_do_2 {
        Source Shadow;
    }
}
