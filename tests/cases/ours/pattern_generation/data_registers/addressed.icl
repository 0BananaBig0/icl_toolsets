// Self-constructed shared-address-bus scheduling probe (IEEE 6.4.9).
Module AddressedData {
  AddressPort address[1:0];
  DataInPort data[3:0];
  WriteEnPort we;
  ReadEnPort re;
  OneHotDataGroup group[3:0] {
    DataRegister a[3:0] { AddressValue 1; ResetValue 4'b0; }
    DataRegister b[3:0] { AddressValue 2; ResetValue 4'b0; }
  }
  DataOutPort observed[3:0] { Source group; }
}
