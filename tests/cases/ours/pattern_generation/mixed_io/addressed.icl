// Self-constructed parallel address/read controls in a serial-capable module.
Module MixedAddressed {
  ScanInPort si;
  ScanRegister scan { ScanInSource si; ResetValue 1'b0; }
  ScanOutPort so { Source scan; }
  AddressPort address;
  DataInPort data[3:0];
  WriteEnPort we;
  ReadEnPort re;
  OneHotDataGroup group[3:0] {
    DataRegister value[3:0] { AddressValue 0; ResetValue 4'b0; }
    DataRegister other[3:0] { AddressValue 1; ResetValue 4'b0; }
  }
  DataOutPort observed[3:0] { Source group; }
}
