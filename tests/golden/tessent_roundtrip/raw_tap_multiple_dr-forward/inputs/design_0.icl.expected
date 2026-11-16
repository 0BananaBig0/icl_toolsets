// Self-constructed IEEE 1687-2014 6.4.17 network with a separate BSDL TAP.
Module MultipleDrInstrument {
  ScanInPort si;
  SelectPort select;
  ShiftEnPort shift;
  CaptureEnPort capture;
  UpdateEnPort update;
  TCKPort clock;
  DataInPort mode;
  ScanRegister a[2:0] {ScanInSource si; CaptureSource a; ResetValue 3'b0;}
  ScanRegister b[4:0] {ScanInSource a[0]; CaptureSource b; ResetValue 5'b0;}
  ScanMux mux SelectedBy mode { 1'b1: b[0]; 1'b0: si; }
  ScanOutPort so { Source mux; }
  ScanInterface client {
    Port si; Port so; Port select; Port shift; Port capture; Port update;
    Port clock;
  }
}
Module MultipleDrTop {
  ScanInPort TDI; ScanOutPort TDO; TMSPort TMS; TCKPort TCK; TRSTPort TRST;
  ScanInterface jtag {Port TDI; Port TDO; Port TMS; Port TCK; Port TRST;}
  Instance core Of MultipleDrInstrument { InputPort mode = tap.long_setup; }
  AccessLink tap Of STD_1149_1_2001 {
    BSDLEntity MultipleDrProbe;
    IJTAG_ON {
      ScanInterface { core.client; }
      ActiveSignals { long_setup; }
    }
    IJTAG_OFF { ScanInterface { core.client; } }
  }
}
