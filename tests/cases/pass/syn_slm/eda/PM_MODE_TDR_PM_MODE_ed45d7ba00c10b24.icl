// Generated; edit the PMM sources, not this file.

NameSpace ;
UseNameSpace ;
Module PM_MODE_TDR_PM_MODE {
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
        Source Mode[0];
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
    ScanRegister ClockDividerTestEnable {
        ScanInSource SI_ijtag_si;
        CaptureSource ClockDividerTestEnable;
        ResetValue 1'b0;
        Attribute synopsys_access = "RW";
    }
    ScanRegister TestEnable {
        ScanInSource ClockDividerTestEnable;
        CaptureSource TestEnable;
        ResetValue 1'b0;
        Attribute synopsys_access = "RW";
    }
    ScanRegister DebugEnable {
        ScanInSource TestEnable;
        CaptureSource DebugEnable;
        ResetValue 1'b0;
        Attribute synopsys_access = "RW";
    }
    ScanRegister TAISelect {
        ScanInSource DebugEnable;
        CaptureSource TAISelect;
        ResetValue 1'b0;
        Attribute synopsys_access = "RW";
    }
    ScanRegister Mode[1:0] {
        ScanInSource TAISelect;
        CaptureSource Mode;
        ResetValue 2'b0;
        Attribute synopsys_access = "RW";
    }
    DataOutPort clock_divider_test_enable_ijtag_do_1 {
        Source ClockDividerTestEnable;
    }
    DataOutPort test_enable_ijtag_do_2 {
        Source TestEnable;
    }
    DataOutPort debug_enable_ijtag_do_3 {
        Source DebugEnable;
    }
    DataOutPort tai_select_ijtag_do_4 {
        Source TAISelect;
    }
    DataOutPort pmu_mode_ijtag_do_5[1:0] {
        Source Mode;
    }
}
