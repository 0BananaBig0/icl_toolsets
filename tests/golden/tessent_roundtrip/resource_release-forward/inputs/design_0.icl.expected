// Self-constructed reservation probe with the existing AccessProbe BSDL TAP.
Module ResourceLeaf {
  ScanInPort si;
  SelectPort select;
  CaptureEnPort capture;
  ShiftEnPort shift;
  UpdateEnPort update;
  TCKPort clock;
  ScanRegister value[7:0] { ScanInSource si; ResetValue 8'b0; }
  ScanOutPort so { Source value[0]; }
  ScanInterface client {
    Port si; Port so; Port select; Port capture; Port shift; Port update;
    Port clock;
  }
}
Module ResourceInstrument {
  ScanInPort si;
  SelectPort select;
  CaptureEnPort capture;
  ShiftEnPort shift;
  UpdateEnPort update;
  TCKPort clock;
  ScanRegister gate[1:0] { ScanInSource si; ResetValue 2'b00; }
  LogicSignal leaf_select { select & gate[0]; }
  Instance leaf Of ResourceLeaf {
    InputPort si = gate[0]; InputPort select = leaf_select;
    InputPort capture = capture; InputPort shift = shift;
    InputPort update = update; InputPort clock = clock;
  }
  ScanMux route SelectedBy gate[0] {
    1'b0: gate[0];
    1'b1: leaf.so;
  }
  ScanOutPort so { Source route; }
  ScanInterface client {
    Port si; Port so; Port select; Port capture; Port shift; Port update;
    Port clock;
  }
}
Module ResourceAccessTop {
  Instance core Of ResourceInstrument;
  AccessLink tap Of STD_1149_1_2001 {
    BSDLEntity AccessProbe;
    IJTAG_ON { ScanInterface { core.client; } }
    IJTAG_OFF { ScanInterface { core.client; } }
  }
}
