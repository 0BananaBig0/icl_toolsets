Module EdgeAddressableLeaf {
  DataInPort input[7:0];
  DataOutPort output[7:0] { Source input; }
}
Module EdgeAddressableGroup {
  AddressPort address[3:0];
  ReadEnPort read;
  WriteEnPort write;
  DataInPort input[7:0];
  OneHotDataGroup bus[7:0] {
    DataRegister value[7:0] { AddressValue 4'h1; ResetValue 8'h00; }
    Instance child Of EdgeAddressableLeaf {
      AddressValue 4'h2;
      InputPort input = input;
    }
  }
  DataOutPort output[7:0] { Source bus; }
}
