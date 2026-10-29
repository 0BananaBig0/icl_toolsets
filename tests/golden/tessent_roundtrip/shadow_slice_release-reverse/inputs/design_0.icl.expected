// Self-constructed probe of resource aliases and indexed reservations.
Module SliceInstrument {
  ScanInPort si;
  SelectPort selected;
  CaptureEnPort capture;
  ShiftEnPort shift;
  UpdateEnPort update;
  TCKPort clock;
  ScanRegister gate[1:0] { ScanInSource si; ResetValue 2'b00; }
  Alias high = gate[1];
  Alias low = gate[0];
  Alias inverted_high = ~gate[1];
  ScanRegister target[3:0] { ScanInSource gate[0]; ResetValue 4'b0; }
  ScanMux route SelectedBy gate[0] { 1'b0: gate[0]; 1'b1: target[0]; }
  ScanOutPort so { Source route; }
  ScanInterface client {
    Port si; Port so; Port selected; Port capture; Port shift; Port update;
    Port clock;
  }
}
Module SliceTop {
  Instance core Of SliceInstrument;
  AccessLink tap Of STD_1149_1_2001 {
    BSDLEntity AccessProbe;
    IJTAG_ON { ScanInterface { core.client; } }
    IJTAG_OFF { ScanInterface { core.client; } }
  }
}
