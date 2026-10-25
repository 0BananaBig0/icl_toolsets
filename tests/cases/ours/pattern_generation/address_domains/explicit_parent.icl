// Self-constructed IEEE 1687-2014 6.4.9 address-domain probe.
Module ExplicitParentAddress {
  Parameter AW = 8;
  Parameter OFFSET = 4;
  Parameter RV = 1;
  AddressPort address[$AW-1:0];
  DataInPort data[7:0];
  WriteEnPort we;
  ReadEnPort re;
  DataOutPort out[7:0] { Source group; }
  OneHotDataGroup group[7:0] {
    Instance child Of AddressDomainLeaf {
      Parameter AW = $AW;
      Parameter RV = $RV;
      AddressValue $OFFSET;
      InputPort a = address;
    }
  }
}
