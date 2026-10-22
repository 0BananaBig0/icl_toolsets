Module m {
  ToClockPort c {
    Period 10ns;
  }
  ToClockPort scaled {
    Source clk;
    FreqMultiplier 2;
    FreqDivider 4;
  }
  ToClockPort differential {
    DifferentialInvOf clk;
  }
  ClockPort cp {
    DifferentialInvOf clk2;
  }
  DataRegister dr_read {
    ReadCallBack m p <D>;
    ReadDataSource a;
  }
  DataRegister dr_write { WriteCallBack m p <R> 3 "x" $N; }
  OneHotDataGroup addressed { DataRegister dr { AddressValue 3; } }
  Instance i Of T {
    AllowBroadcastOnScanInterface si;
    AddressValue 3;
  }
}
