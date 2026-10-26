// Self-constructed indexed ActiveSignal probe; load simple.icl first.
Module AccessTop {
  Instance core Of AccessInstrument { InputPort mode = tap.long_setup[2]; }
  AccessLink tap Of STD_1149_1_2001 {
    BSDLEntity AccessProbe;
    IJTAG_ON {
      ScanInterface { core.client; }
      ActiveSignals { long_setup[3:1]; }
    }
    IJTAG_OFF { ScanInterface { core.client; } }
  }
}
