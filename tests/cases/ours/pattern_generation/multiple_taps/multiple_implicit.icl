// Self-constructed two-device-interface probe; one logical TCK.
Module TapCell {
  ScanInPort si; ScanOutPort so { Source value[0]; }
  SelectPort select; CaptureEnPort capture; ShiftEnPort shift;
  UpdateEnPort update; TCKPort clock;
  ResetPort reset { ActivePolarity 0; }
  ScanRegister value[3:0] { ScanInSource si; ResetValue 4'b0000; }
  ScanInterface client {
    Port si; Port so; Port select; Port capture; Port shift; Port update;
    Port clock;
  }
}
Module MultiTapTop {
  Instance a Of TapCell;
  AccessLink tapA Of STD_1149_1_2001 {
    BSDLEntity ProbeA;
    IJTAG_ON { ScanInterface { a.client; } }
    IJTAG_OFF { ScanInterface { a.client; } }
  }
  Instance b Of TapCell;
  AccessLink tapB Of STD_1149_1_2001 {
    BSDLEntity ProbeB;
    IJTAG_ON { ScanInterface { b.client; } }
    IJTAG_OFF { ScanInterface { b.client; } }
  }
}
