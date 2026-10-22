// Generated; edit the PMM sources, not this file.

NameSpace ;
UseNameSpace ;
Module SIB1_SIB_1 {
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
    ScanInPort B0_ijtag_from_so_1;
    LogicSignal Enable0 {
        Select_ijtag_sel && (Value == 3'd0);
    }
    ToSelectPort Select0_ijtag_to_sel_1 {
        Source Enable0;
    }
    ScanInPort B1_ijtag_from_so_2;
    LogicSignal Enable1 {
        Select_ijtag_sel && (Value == 3'd1);
    }
    ToSelectPort Select1_ijtag_to_sel_2 {
        Source Enable1;
    }
    ScanInPort B2_ijtag_from_so_3;
    LogicSignal Enable2 {
        Select_ijtag_sel && (Value == 3'd2);
    }
    ToSelectPort Select2_ijtag_to_sel_3 {
        Source Enable2;
    }
    ScanRegister Value[2:0] {
        ScanInSource mux;
        CaptureSource Value;
        ResetValue 3'b0;
    }
    DataOutPort Selection_ijtag_do[2:0] {
        Source Value;
    }
    ScanMux mux SelectedBy Value {
        3'd0 : B0_ijtag_from_so_1;
        3'd1 : B1_ijtag_from_so_2;
        3'd2 : B2_ijtag_from_so_3;
        3'd3 : SI_ijtag_si;
        3'd4 : SI_ijtag_si;
        3'd5 : SI_ijtag_si;
        3'd6 : SI_ijtag_si;
        3'd7 : SI_ijtag_si;
    }
}
