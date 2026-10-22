// IEEE §6.4.9 printed 119/PDF 133: addressable example.
Module M0 {
  DataInPort byteIn1[7:0]; DataInPort byteIn0[7:0];
  AddressPort AddHigh[3:0]; AddressPort AddLow[7:0];
  ReadEnPort re; WriteEnPort we;
  DataOutPort byteOut0[7:0] { Source M1[7:0]; }
  DataOutPort byteOut1[7:0] { Source M1[15:8]; }
  OneHotDataGroup M1[15:0] {
    Instance I1 Of M2 { AddressValue 'h1000; }
    Instance I2 Of M2 { AddressValue 'h2000; }
    DataRegister R1 { AddressValue 'h3001; }
    DataRegister R2 { AddressValue 'h3002; }
  }
}
Module M2 {
  DataInPort dataIn[11:0]; AddressPort addIn[11:0];
  ReadEnPort re; WriteEnPort we;
  DataOutPort dataOut[11:0] { Source M1; }
  OneHotDataGroup M1[11:0] {
    DataRegister R1 { AddressValue 'h001; }
    DataRegister R2 { AddressValue 'h002; }
  }
}
