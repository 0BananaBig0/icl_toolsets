// Tessent v2023.4 AccessLink example; missing core module supplied.
Module mydesign {
  ClockPort CLK;
  AccessLink bsdl_tap Of STD_1149_1_2001 {
    BSDLEntity acme4523;
    BP0_LONG_SETUP {
      ScanInterface {core.ijtag0;}
      ActiveSignals {long_setup;}
    }
    BP0_SHORT_SETUP {
      ScanInterface {core.ijtag0;}
    }
    BP1_LONG_SETUP {
      ScanInterface {core.ijtag1;}
      ActiveSignals {long_setup;}
    }
    BP1_SHORT_SETUP {
      ScanInterface {core.ijtag1;}
    }
  } 
 
  Instance core Of core {
    InputPort selectWIR = bsdl_tap.long_setup;
    InputPort clk = CLK;
  } 
 
} 
 
Module core { ClockPort clk; DataInPort selectWIR; ScanInPort si; ScanOutPort so; SelectPort en; ScanInterface ijtag0 { Port si; Port so; Port en; } ScanInterface ijtag1 { Port si; Port so; Port en; } }
