Module EdgeDataRegisterDoubleInversionComments {
  DataInPort enable;
  DataInPort input[3:0];
  DataRegister value[3:0] {
    WriteEnSource ~ /* outer */ ~ /* inner */ enable;
    WriteDataSource input[(1 + 2):2], ~input[1:0];
  }
}
