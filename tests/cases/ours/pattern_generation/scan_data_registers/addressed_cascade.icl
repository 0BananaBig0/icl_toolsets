// Self-constructed: ascending addresses must fall back when an earlier goal changes.
Module ScanData {
  ScanInPort si;
  ScanRegister bus[6:0] { ScanInSource si; ResetValue 7'b0; }
  Instance a Of Bank {
    InputPort address = bus[4]; InputPort we = bus[5]; InputPort data = bus[3:0];
  }
  Instance b Of Bank {
    InputPort address = bus[4]; InputPort we = bus[6]; InputPort data = a.out;
  }
  ScanOutPort so { Source bus[0]; }
}
Module Bank {
  AddressPort address; WriteEnPort we; DataInPort data[3:0];
  OneHotDataGroup group[3:0] {
    DataRegister r0[3:0] { AddressValue 0; ResetValue 4'b0; }
    DataRegister r1[3:0] { AddressValue 1; ResetValue 4'b0; }
  }
  DataOutPort out[3:0] { Source r0; }
}
