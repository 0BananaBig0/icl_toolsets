Module Instrument {
  DataInPort DI[7:0];
  DataOutPort DO[7:0];
  ClockPort CLK;
  Alias enable = DI[7] {RefEnum YesNo;}
  Alias go = DO[0] {RefEnum PassFail;}
  Alias mode[1:0] = DI[6:5] {RefEnum Modes;}
  Enum YesNo {Yes = 1'b1; No = 1'b0;}
  Enum Modes { default = 2'b11; debug = 2'b10; }
}
