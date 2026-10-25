// Self-constructed IEEE 1687-2014 6.4.9 address-domain probe.
Module AddressDomainLeaf {
  Parameter AW = 4;
  Parameter RV = 1;
  AddressPort a[$AW-1:0];
  DataInPort d[7:0];
  WriteEnPort we;
  ReadEnPort re;
  DataOutPort out[7:0] { Source group; }
  OneHotDataGroup group[7:0] {
    DataRegister r[7:0] { AddressValue $RV; }
  }
}
