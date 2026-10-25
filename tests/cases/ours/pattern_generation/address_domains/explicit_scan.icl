// Self-constructed IEEE 1687-2014 6.4.9 address-domain probe.
Module ExplicitScanAddress {
  ScanInPort si;
  SelectPort select;
  ScanRegister control[13:0] { ScanInSource si; }
  ScanOutPort so { Source control[0]; }
  ScanInterface client { Port si; Port so; Port select; }
  DataOutPort out[7:0] { Source group; }
  OneHotDataGroup group[7:0] {
    Instance child Of AddressDomainLeaf {
      AddressValue 4;
      InputPort a = control[3:0];
      InputPort we = control[4];
      InputPort re = control[5];
      InputPort d = control[13:6];
    }
  }
}
