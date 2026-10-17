Module bus {
  DataInPort datain[ $MSB:0 ];
  Parameter MSB = 5;
}
Module top {
  Instance bus_inst1 Of bus {
    InputPort datain[5:0] = toplevel_datain[15:10];
  }
}
