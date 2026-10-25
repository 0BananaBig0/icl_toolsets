// Self-constructed explicit/implicit binding probe, same widths as the
// Tessent-derived address_defaults.icl fixture.
Module PartialDataTop {
  AddressPort address[7:0];
  DataInPort data[15:0];
  DataInPort override_data[3:0];
  WriteEnPort we;
  ReadEnPort re;
  DataOutPort out[7:0] { Source group; }
  OneHotDataGroup group[7:0] {
    Instance child Of NarrowBus {
      AddressValue 4;
      InputPort data[7:4] = override_data;
    }
  }
}
Module SplitBus {
  AddressPort address[1:0];
  DataInPort upper[3:0];
  DataInPort lower[0:3];
  WriteEnPort we;
  ReadEnPort re;
  DataOutPort out[7:0] { Source registers; }
  OneHotDataGroup registers[7:0] {
    DataRegister r[7:0] { AddressValue 1; }
  }
}
Module ExplicitDataTop {
  AddressPort address[7:0];
  DataInPort data[15:0];
  DataInPort override_data[3:0];
  WriteEnPort we;
  ReadEnPort re;
  DataOutPort out[7:0] { Source group; }
  OneHotDataGroup group[7:0] {
    Instance child Of SplitBus {
      AddressValue 4;
      InputPort upper = override_data;
    }
  }
}
Module SplitDefaultTop {
  AddressPort address[7:0];
  DataInPort zfirst[1:0];
  DataInPort asecond[0:13];
  WriteEnPort we;
  ReadEnPort re;
  DataOutPort out[7:0] { Source group; }
  OneHotDataGroup group[7:0] {
    Instance child Of SplitBus { AddressValue 4; }
  }
}
Module ShortDataTop {
  AddressPort address[7:0];
  DataInPort data[3:0];
  WriteEnPort we;
  ReadEnPort re;
  DataOutPort out[7:0] { Source group; }
  OneHotDataGroup group[7:0] {
    Instance child Of NarrowBus { AddressValue 4; }
  }
}
