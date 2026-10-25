// Completed width/default-connection probe based on Tessent Shell Reference
// v2023.4, Instance, Example 2. Address and data buses intentionally differ.
Module NarrowBus {
  AddressPort address[1:0];
  DataInPort data[7:0];
  WriteEnPort we;
  ReadEnPort re;
  DataOutPort out[7:0] { Source registers; }
  OneHotDataGroup registers[7:0] {
    DataRegister r[7:0] { AddressValue 1; }
  }
}
Module AddressDefaultsTop {
  AddressPort address[7:0];
  DataInPort data[15:0];
  WriteEnPort we;
  ReadEnPort re;
  DataOutPort out[7:0] { Source group; }
  OneHotDataGroup group[7:0] {
    Instance child Of NarrowBus { AddressValue 4; }
  }
}
