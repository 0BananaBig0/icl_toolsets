Module bus {
  DataInPort datain[ $MSB:0 ];
  DataOutPort dataout {
    Source RegD[ $MSB:0 ];
  }
  Parameter MSB = 5;
}
Module top {
  Instance bus_inst1 Of bus {
    InputPort datain[5:0] = toplevel_datain[15:10];
  }
  Instance bus_inst2 Of bus {
    Parameter MSB = 7;
    InputPort datain[7:0] = toplevel_datain[17:10];
  }
}
