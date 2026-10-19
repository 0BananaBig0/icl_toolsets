Module SIB {
  ScanInPort si;
  ScanInPort fso;
  ScanOutPort so {
    Source sr;
    LaunchEdge Falling;
  }
  SelectPort en;
  ToSelectPort to_en;
  ScanRegister sr {
    ScanInSource mux1;
    CaptureSource 1'b0;
    ResetValue 1'b0;
  }
  ScanMux mux1 sr {
    1'b0 : si;
    1'b1 : fso;
  }
  PortGroup tap_side {si, so, en}
  PortGroup instrument_side {fso, to_en}
}
Module Chip_ICL {
  Instance SIB1 Of SIB {
    InputPort en = Tap1.en_TDR;
    InputPort fso = TDR1.so;
  }
  AccessLink Tap1 Of STD_1149_1 {
    BSDL_Entity Chip_ICL;
    READINSTR {
      ScanPath { SIB1; SIB2; }
      ActiveSignals { en_TDR; }
    }
  }
}
