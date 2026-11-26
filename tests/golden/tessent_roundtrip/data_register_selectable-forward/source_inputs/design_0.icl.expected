// Completed IEEE 1687-2014 6.4.9 selectable-register example.
Module SelectableData {
  DataInPort data[3:0];
  DataInPort enable;
  DataRegister value[3:0] {
    WriteDataSource data;
    WriteEnSource enable;
    ResetValue 4'b0;
  }
  DataOutPort observed[3:0] { Source value; }
}
