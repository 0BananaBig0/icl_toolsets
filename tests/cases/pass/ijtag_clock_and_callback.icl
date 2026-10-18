Module m {
  ToClockPort c {
    Period 10ns;
    FreqMultiplier 2;
    FreqDivider 4;
    DifferentialInvOf clk;
  }
  ClockPort cp {
    DifferentialInvOf clk2;
  }
  DataRegister dr {
    ReadCallBack m::p <D>;
    WriteCallBack p <R> 3 "x" $N;
    ReadDataSource a;
    AddressValue 3;
  }
  Instance i Of T {
    AllowBroadcastOnScanInterface si;
    AddressValue 3;
  }
}
