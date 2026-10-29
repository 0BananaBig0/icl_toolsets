// Self-constructed TAP wrappers for mixed parallel/serial qualification.
Module MixedPrimaryDataInstrument {
  ScanInPort si;
  DataInPort data[3:0];
  SelectPort select;
  CaptureEnPort capture;
  ShiftEnPort shift;
  UpdateEnPort update;
  TCKPort clock;
  ScanRegister enable { ScanInSource si; ResetValue 1'b0; }
  DataRegister value[3:0] {
    WriteDataSource data; WriteEnSource enable; ResetValue 4'b0;
  }
  ScanOutPort so { Source enable; }
  DataOutPort observed[3:0] { Source value; }
  ScanInterface client {
    Port si; Port so; Port select; Port capture; Port shift; Port update; Port clock;
  }
}
Module MixedScanDataInstrument {
  ScanInPort si;
  DataInPort enable;
  SelectPort select;
  CaptureEnPort capture;
  ShiftEnPort shift;
  UpdateEnPort update;
  TCKPort clock;
  ScanRegister data[3:0] { ScanInSource si; ResetValue 4'b0; }
  DataRegister value[3:0] {
    WriteDataSource data; WriteEnSource enable; ResetValue 4'b0;
  }
  ScanOutPort so { Source data[0]; }
  DataOutPort observed[3:0] { Source value; }
  ScanInterface client {
    Port si; Port so; Port select; Port capture; Port shift; Port update; Port clock;
  }
}
Module MixedPrimaryDataProbe {
  DataInPort data[3:0];
  Instance core Of MixedPrimaryDataInstrument { InputPort data = data; }
  DataOutPort observed[3:0] { Source core.observed; }
  AccessLink tap Of STD_1149_1_2001 {
    BSDLEntity AccessProbe;
    IJTAG_ON { ScanInterface { core.client; } }
    IJTAG_OFF { ScanInterface { core.client; } }
  }
}
Module MixedScanDataProbe {
  DataInPort enable;
  Instance core Of MixedScanDataInstrument { InputPort enable = enable; }
  DataOutPort observed[3:0] { Source core.observed; }
  AccessLink tap Of STD_1149_1_2001 {
    BSDLEntity AccessProbe;
    IJTAG_ON { ScanInterface { core.client; } }
    IJTAG_OFF { ScanInterface { core.client; } }
  }
}
