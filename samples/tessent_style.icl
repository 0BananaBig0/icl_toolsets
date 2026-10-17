/*
 * Tessent-style ICL sample.
 *
 * Exercises the constructs that Tessent IJTAG emits / consumes (see
 * Tessent IJTAG User's Manual, "ICL and PDL Modeling"): SIB switching,
 * explicit ScanInterfaces, iProcs, AccessLink over 1149.1, and a ScanMux
 * selection table. Used by tests/run_tests.py.
 */

Module SIB_mux {
  ScanInPort si;
  ScanInPort fso;
  ScanOutPort so { Source sr; LaunchEdge Falling; }
  SelectPort sel;
  ToSelectPort to_sel;
  ResetPort rst;
  ScanRegister sr {
    ScanInSource mux1;
    CaptureSource 1'b0;
    ResetValue 1'b0;
  }
  ScanMux mux1 SelectedBy sr {
    1'b0 : si;
    1'b1 : fso;
  }
  PortGroup tap_side {si, so, sel}
  PortGroup instrument_side {fso, to_sel}
}

Module TDR_8 {
  Parameter Width = 8;
  Attribute name = "TDR_8";
  ScanInPort si;
  ScanOutPort so { Source tdr; }
  ShiftEnPort se;
  CaptureEnPort ce;
  UpdateEnPort ue;
  SelectPort sel;
  ResetPort rst;
  TCKPort tck;
  DataInPort di[7:0];
  DataOutPort do[7:0] { Source tdr[7:0]; }
  ScanRegister tdr[7:0] {
    ScanInSource si;
    CaptureSource di[7:0];
    ResetValue 8'b0000_0000;
  }
  ScanInterface client_si { Port si; Source si; }
}

Module my_ip {
  Attribute name = "my_ip";
  ScanInPort si;
  ScanOutPort so { Source TDR; }
  SelectPort sel;
  ToSelectPort to_sel;
  DataInPort di[3:0];
  ScanRegister TDR[3:0] { ScanInSource si; CaptureSource di[3:0]; }
  Alias run_test = TDR[0] { iProcsForModule my_ip; }
  Enum mode { FAST = 2'b00; SLOW = 2'b01; }
}

Module Chip_ICL {
  Attribute name = "Chip_ICL";
  ScanInPort TDI;
  ScanOutPort TDO { Source SIB1; }
  ShiftEnPort SE;
  CaptureEnPort CE;
  UpdateEnPort UE;
  SelectPort SEL;
  ResetPort TRST;
  TCKPort TCK;

  // The scan chain runs through a SIB per instrument.
  Instance SIB1 Of SIB_mux {
    InputPort si = TDI;
    InputPort sel = Tap1.en_TDR;
    InputPort fso = inst_1.so;
  }

  Instance inst_1 Of my_ip {
    InputPort si = SIB1.so;
    InputPort sel = SIB1.to_sel;
  }

  LogicSignal tck_gated { Source TCK; }

  Connection {
    TDI -> SIB1.si;
    SIB1.so -> inst_1.si;
    inst_1.so -> TDO;
  }

  AccessLink Tap1 Of STD_1149_1 {
    BSDL_Entity Chip_ICL;
    READINSTR {
      ScanPath { SIB1; inst_1; }
      ActiveSignals { en_TDR; }
    }
  }
}
