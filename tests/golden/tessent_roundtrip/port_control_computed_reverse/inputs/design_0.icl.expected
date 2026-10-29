// Self-constructed IEEE 1687 data-port, ownership and merge qualification network.
Module PortControlSink {
  WriteEnPort we;
}
Module PortControlCore {
  ScanInPort si;
  ScanOutPort so { Source b[0]; }
  SelectPort selected;
  CaptureEnPort capture;
  ShiftEnPort shift;
  UpdateEnPort update;
  TCKPort clock;
  ScanRegister a[3:0] {
    ScanInSource si;
    ResetValue 4'b0;
  }
  ScanRegister b[3:0] {
    ScanInSource a[0];
    ResetValue 4'b0;
  }
  DataOutPort mirrored[3:0] { Source ~a; }
  Instance child Of PortControlSink { InputPort we = a[0]; }
  ScanInterface client {
    Port si; Port so; Port selected; Port capture; Port shift;
    Port update; Port clock;
  }
}
Module PortControlTop {
  Instance core Of PortControlCore;
  AccessLink tap Of STD_1149_1_2001 {
    BSDLEntity AccessProbe;
    IJTAG_ON { ScanInterface { core.client; } }
    IJTAG_OFF { ScanInterface { core.client; } }
  }
}
