// Completed self-constructed probes of IEEE 1687-2014 6.4.15(b,c).
Module AddressedByte {
  AddressPort address[3:0];
  DataInPort data[7:0];
  ReadEnPort re;
  WriteEnPort we;
  DataOutPort value[7:0] { Source group; }
  OneHotDataGroup group[7:0] {
    DataRegister r[7:0] { AddressValue 1; }
  }
}
Module RegisterContribution {
  Alias target[7:0] = r;
  AddressPort address[3:0];
  DataInPort data[7:0];
  ReadEnPort re;
  WriteEnPort we;
  OneHotDataGroup wide[15:0] {
    DataRegister r[7:0] { AddressValue 1; }
  }
  DataOutPort observed[15:0] { Source wide; }
}
Module InstanceContribution {
  Alias target[7:0] = child.r;
  AddressPort address[7:0];
  DataInPort data[7:0];
  ReadEnPort re;
  WriteEnPort we;
  OneHotDataGroup wide[0:15] {
    Instance child Of AddressedByte { AddressValue 16; }
  }
  DataOutPort observed[0:15] { Source wide; }
}
Module MixedContribution {
  AddressPort address[7:0];
  DataInPort data[15:0];
  ReadEnPort re;
  WriteEnPort we;
  OneHotDataGroup wide[15:0] {
    DataRegister narrow[7:0] { AddressValue 1; }
    DataRegister full[15:0] { AddressValue 2; }
  }
  DataOutPort observed[15:0] { Source wide; }
}
Module SplitRegisterData {
  AddressPort address[7:0];
  DataInPort zfirst[7:0];
  DataInPort asecond[0:3];
  ReadEnPort re;
  WriteEnPort we;
  OneHotDataGroup group[7:0] {
    DataRegister r[7:0] { AddressValue 1; }
  }
  DataOutPort zupper[3:0] { Source group[7:4]; }
  DataOutPort alower[0:3] { Source group[3:0]; }
}
Module SplitContribution {
  AddressPort address[7:0];
  DataInPort data[15:0];
  ReadEnPort re;
  WriteEnPort we;
  OneHotDataGroup wide[15:0] {
    Instance child Of SplitRegisterData { AddressValue 16; }
    DataRegister full[15:0] { AddressValue 2; }
  }
  DataOutPort observed[15:0] { Source wide; }
}
