Module EdgeDataRegisterTripleInversionComments {
  DataInPort enable;
  DataRegister value {
    WriteEnSource ~ /* outer */ ~ /* middle */ ~enable;
    WriteDataSource enable;
  }
}
