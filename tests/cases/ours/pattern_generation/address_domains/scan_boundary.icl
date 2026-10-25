// The scan register drives an unaddressed bank; offsets start inside the bank.
Module AddressDomainBank {
  AddressPort address[7:0];
  DataInPort data[7:0];
  WriteEnPort we;
  ReadEnPort re;
  DataOutPort out[7:0] { Source group; }
  OneHotDataGroup group[7:0] {
    Instance child Of AddressDomainLeaf { AddressValue 4; }
  }
}
Module ScanAddressBoundary {
  ScanInPort si;
  SelectPort select;
  ScanRegister control[17:0] { ScanInSource si; ResetValue 0; }
  ScanOutPort so { Source control[0]; }
  ScanInterface client { Port si; Port so; Port select; }
  Instance bank Of AddressDomainBank {
    InputPort address = control[7:0];
    InputPort we = control[8];
    InputPort re = control[9];
    InputPort data = control[17:10];
  }
  Alias target[7:0] = bank.child.r;
  DataOutPort out[7:0] { Source bank.out; }
}
