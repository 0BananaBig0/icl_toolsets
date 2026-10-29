// Self-constructed: one address bus with independently enabled data lanes.
Module ScanData {
  ScanInPort si;
  ScanRegister bus[10:0] { ScanInSource si; ResetValue 11'b0; }
  Instance a Of Bank {
    InputPort address = bus[10]; InputPort we = bus[4]; InputPort data = bus[3:0];
  }
  Instance b Of Bank {
    InputPort address = bus[10]; InputPort we = bus[9]; InputPort data = bus[8:5];
  }
  ScanOutPort so { Source bus[0]; }
}
Module Bank {
  AddressPort address; WriteEnPort we; DataInPort data[3:0];
  OneHotDataGroup group[3:0] {
    DataRegister r0[3:0] { AddressValue 0; ResetValue 4'b0; }
    DataRegister r1[3:0] { AddressValue 1; ResetValue 4'b0; }
  }
}
