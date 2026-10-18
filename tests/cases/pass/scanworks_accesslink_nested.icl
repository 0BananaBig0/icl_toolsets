Module IJTAG_Demo_1Bit_ET_0 {
  ScanInPort BSCAN_SPARTAN6_tdi;
  ScanOutPort BSCAN_SPARTAN6_tdo {
    Source IJTAG_Network_1.TDO;
  }
  ScanInterface BSCAN_SPARTAN6_Gateway {
    Port BSCAN_SPARTAN6_tdi;
    Port BSCAN_SPARTAN6_tdo;
  }
  DataOutPort K2 {
    Source IJTAG_Network_1.LD_1Bit_Instr_1___LD0;
  }
  Instance IJTAG_Network_1 Of IJTAG_Network {
    InputPort TDI = BSCAN_SPARTAN6_tdi;
  }
  AccessLink XC6SLX9_FTG256 Of STD_1149_1 {
    BSDL_Entity XC6SLX9_FTG256;
    USER1 {
      ScanInterface {
        IJTAG_Demo_1Bit_ET_0.BSCAN_SPARTAN6_Gateway;
      }
    }
  }
}
