// Generated; edit the PMM sources, not this file.

NameSpace ;
UseNameSpace ;
Module SIB_LOOP_SEL {
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
        Select_ijtag_sel && (Value == 4'd0);
    }
    ToSelectPort Select0_ijtag_to_sel_1 {
        Source Enable0;
    }
    ScanInPort B1_ijtag_from_so_2;
    LogicSignal Enable1 {
        Select_ijtag_sel && (Value == 4'd1);
    }
    ToSelectPort Select1_ijtag_to_sel_2 {
        Source Enable1;
    }
    ScanRegister Value[3:0] {
        ScanInSource mux;
        CaptureSource Value;
        ResetValue 4'b0;
    }
    DataOutPort Selection_ijtag_do[3:0] {
        Source Value;
    }
    ScanMux mux SelectedBy Value {
        4'd0 : B0_ijtag_from_so_1;
        4'd1 : B1_ijtag_from_so_2;
        4'd2 : SI_ijtag_si;
        4'd3 : SI_ijtag_si;
        4'd4 : SI_ijtag_si;
        4'd5 : SI_ijtag_si;
        4'd6 : SI_ijtag_si;
        4'd7 : SI_ijtag_si;
        4'd8 : SI_ijtag_si;
        4'd9 : SI_ijtag_si;
        4'd10 : SI_ijtag_si;
        4'd11 : SI_ijtag_si;
        4'd12 : SI_ijtag_si;
        4'd13 : SI_ijtag_si;
        4'd14 : SI_ijtag_si;
        4'd15 : SI_ijtag_si;
    }
}
