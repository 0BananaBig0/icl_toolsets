// IEEE 1687-2014 section 6.4.7 examples; missing modules, ports and wrapper supplied.
NameSpace NS1; Module MyMod1 {} NameSpace;
Module MyMod1 {}
Module clock_gen { Parameter MSB = 1; ClockPort REFCLK; DataInPort BYPASS; }
Module MyMod2 {
 ScanInPort SIa; ScanOutPort SOa; SelectPort Sela; ScanInterface A { Port SIa; }
 ScanInPort SIb; ScanOutPort SOb; SelectPort Selb; ScanInterface B { Port SIb; }
 ScanInPort SIc; ScanOutPort SOc; SelectPort Selc; ScanInterface C { Port SIc; }
}
Module Top {
 ClockPort CK_IN; DataInPort CK_BYP; ScanInPort SI[2:0]; DataInPort Sel[2:0];
 Instance Inst1 Of MyMod1;
 Instance Inst2 Of NS1::MyMod1;
 Instance Inst3 Of clock_gen { Parameter MSB = 4; InputPort REFCLK = CK_IN; InputPort BYPASS = CK_BYP; }
 Instance Inst4 Of MyMod2 {
  AllowBroadcastOnScanInterface A,B,C;
  InputPort SIa = SI[0]; InputPort SIb = SI[1]; InputPort SIc = SI[2];
  InputPort Sela = Sel[0]; InputPort Selb = Sel[1]; InputPort Selc = Sel[2];
 }
}
