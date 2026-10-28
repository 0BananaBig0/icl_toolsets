// Self-constructed shared bus: eight addressed DataRegisters.
Module ScanData {
  ScanInPort si;
  ScanRegister bus[7:0] { ScanInSource si; ResetValue 8'b0; }
  Instance bank Of Bank {
    InputPort address = bus[6:4]; InputPort we = bus[7]; InputPort data = bus[3:0];
  }
  ScanOutPort so { Source bus[0]; }
}
Module Bank {
  AddressPort address[2:0]; WriteEnPort we; DataInPort data[3:0];
  OneHotDataGroup group[3:0] {
    DataRegister r0[3:0] { AddressValue 0; ResetValue 4'b0; }
    DataRegister r1[3:0] { AddressValue 1; ResetValue 4'b0; }
    DataRegister r2[3:0] { AddressValue 2; ResetValue 4'b0; }
    DataRegister r3[3:0] { AddressValue 3; ResetValue 4'b0; }
    DataRegister r4[3:0] { AddressValue 4; ResetValue 4'b0; }
    DataRegister r5[3:0] { AddressValue 5; ResetValue 4'b0; }
    DataRegister r6[3:0] { AddressValue 6; ResetValue 4'b0; }
    DataRegister r7[3:0] { AddressValue 7; ResetValue 4'b0; }
  }
}
