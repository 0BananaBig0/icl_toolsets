// Self-constructed 6.5.2(h) probe reusing the reservation network and BSDL.
Module EndStateAccessTop {
  Instance core Of ResourceInstrument;
  Alias idle = core.gate[0] { iApplyEndState 1'b0; }
  AccessLink tap Of STD_1149_1_2001 {
    BSDLEntity AccessProbe;
    IJTAG_ON { ScanInterface { core.client; } }
    IJTAG_OFF { ScanInterface { core.client; } }
  }
}
