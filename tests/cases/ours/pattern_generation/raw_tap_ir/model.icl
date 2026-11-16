// Self-constructed IEEE 1687-2014 6.4.17 network with a separate BSDL TAP.
Module RawIrInstrument {
  ScanInPort si;
  SelectPort select;
  ShiftEnPort shift;
  CaptureEnPort capture;
  UpdateEnPort update;
  TCKPort clock;
  DataInPort mode;
  ScanRegister r[7:0] { ScanInSource si; ResetValue 8'b0; }
  ScanMux mux SelectedBy mode { 1'b1: r[0]; 1'b0: si; }
  ScanOutPort so { Source mux; }
  ScanInterface client {
    Port si; Port so; Port select; Port shift; Port capture; Port update;
    Port clock;
  }
}
Module RawIrTop {
  ScanInPort TDI; ScanOutPort TDO; TMSPort TMS; TCKPort TCK; TRSTPort TRST;
  ScanInterface jtag {Port TDI; Port TDO; Port TMS; Port TCK; Port TRST;}
  Instance core Of RawIrInstrument { InputPort mode = tap.long_setup; }
  AccessLink tap Of STD_1149_1_2001 {
    BSDLEntity RawIrProbe;
    IJTAG_ON {
      ScanInterface { core.client; }
      ActiveSignals { long_setup; }
    }
    IJTAG_OFF { ScanInterface { core.client; } }
  }
}
