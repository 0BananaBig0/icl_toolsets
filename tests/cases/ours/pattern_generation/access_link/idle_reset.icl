// Self-constructed IEEE 1687-2014 6.4.17 network with a separate BSDL TAP.
Module AccessInstrument {
  ScanInPort si;
  SelectPort select;
  ShiftEnPort shift;
  CaptureEnPort capture;
  UpdateEnPort update;
  TCKPort clock;
  DataInPort mode;
  ResetPort rst;
  ScanRegister r[7:0] {
    ScanInSource si;
    ResetValue 8'b00000000;
  }
  ScanMux mux SelectedBy mode {
    1'b1 : r[0];
    1'b0 : si;
  }
  ScanOutPort so {
    Source mux;
  }
  ScanInterface client {
    Port si;
    Port so;
    Port select;
    Port shift;
    Port capture;
    Port update;
    Port clock;
  }
}
// Self-constructed IEEE 1687-2014 6.4.17 network with a separate BSDL TAP.
Module Trigger {
  ScanInPort si;
  SelectPort select;
  ShiftEnPort shift;
  CaptureEnPort capture;
  UpdateEnPort update;
  TCKPort clock;
  DataInPort mode;
  ScanRegister r[7:0] {
    ScanInSource si;
    CaptureSource 8'b00000001;
  }
  ScanMux mux SelectedBy mode {
    1'b0 : r[0];
    1'b1 : si;
  }
  ScanOutPort so {
    Source mux;
  }
  ScanInterface client {
    Port si;
    Port so;
    Port select;
    Port shift;
    Port capture;
    Port update;
    Port clock;
  }
}
// Self-constructed IEEE 1687-2014 6.4.17 network with a separate BSDL TAP.
Module Gate {
  ScanInPort si;
  SelectPort select;
  ShiftEnPort shift;
  CaptureEnPort capture;
  UpdateEnPort update;
  TCKPort clock;
  DataInPort mode;
  ResetPort rst;
  ScanRegister r[7:0] {
    ScanInSource si;
    ResetValue 8'b00000000;
    CaptureSource 8'b00000001;
  }
  ScanMux mux SelectedBy mode {
    1'b0 : r[0];
    1'b1 : si;
  }
  ScanOutPort so {
    Source mux;
  }
  ScanInterface client {
    Port si;
    Port so;
    Port select;
    Port shift;
    Port capture;
    Port update;
    Port clock;
  }
}
Module AccessTop {
  Instance trigger Of Trigger {
    InputPort mode = tap.long_setup;
    InputPort select = trigger.r[1];
    InputPort capture = trigger.r[1];
    InputPort shift = 1'b0;
    InputPort update = trigger.r[1];
  }
  Instance gate Of Gate {
    InputPort mode = tap.long_setup;
    InputPort select = trigger.r[1];
    InputPort capture = trigger.r[1];
    InputPort shift = 1'b0;
    InputPort update = trigger.r[1];
    InputPort rst = trigger.r[0];
  }
  Instance core Of AccessInstrument {
    InputPort rst = gate.r[0];
    InputPort mode = tap.long_setup;
  }
  AccessLink tap Of STD_1149_1_2001 {
    BSDLEntity AccessProbe;
    IJTAG_ON {
      ScanInterface  {
        trigger.client;
        gate.client;
        core.client;
      }
      ActiveSignals  {
        long_setup;
      }
    }
    IJTAG_OFF {
      ScanInterface  {
        trigger.client;
        gate.client;
        core.client;
      }
    }
  }
}
