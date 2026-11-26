// Self-constructed IEEE 1687-2014 6.4.9 scan-driven selectable register.
Module ScanDataInstrument {
  ScanInPort si;
  SelectPort select; CaptureEnPort capture; ShiftEnPort shift;
  UpdateEnPort update; TCKPort clock;
  ScanRegister bus[4:0] { ScanInSource si; ResetValue 5'b0; }
  DataRegister value[3:0] {
    WriteDataSource bus[3:0]; WriteEnSource bus[4]; ResetValue 4'b0;
  }
  ScanRegister observed[3:0] {
    ScanInSource bus[0]; CaptureSource value; ResetValue 4'b0;
  }
  ScanOutPort so { Source observed[0]; }
  ScanInterface client {
    Port si; Port so; Port select; Port capture; Port shift; Port update; Port clock;
  }
}
Module ScanData {
  Instance core Of ScanDataInstrument;
  AccessLink tap Of STD_1149_1_2001 {
    BSDLEntity AccessProbe;
    IJTAG_ON { ScanInterface { core.client; } }
    IJTAG_OFF { ScanInterface { core.client; } }
  }
}
