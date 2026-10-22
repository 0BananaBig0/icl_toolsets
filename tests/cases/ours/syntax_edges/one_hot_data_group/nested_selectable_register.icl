Module EdgeNestedSelectableRegister {
  DataInPort enable;
  DataInPort input[7:0];
  OneHotDataGroup bus[7:0] {
    DataRegister value[7:0] {
      WriteEnSource enable;
      WriteDataSource input;
    }
  }
}
