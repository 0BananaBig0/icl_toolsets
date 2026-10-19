// From "IEEE 1687 (IJTAG): ICL and PDL Explained" (ASSET InterTech) — a
// complete four-module network: the instrument, its wrapper, the network and
// the top level. Kept verbatim so the formatter is checked against a
// published example rather than against one of our own files.
Module BST_IO_LD_1Bit_Instr_1__LD_1Bit_Instr_1 {
  DataInPort xi_LD0;
  DataOutPort LD0;
}
Module LD_1Bit_Instr_1__Wrapper {
  ScanInPort TDI;
  ScanOutPort TDO { Source SIB_mux; }
  DataOutPort LD0 { Source LD_1Bit_Instr_1.LD0; }
  Instance LD_1Bit_Instr_1 Of BST_IO_LD_1Bit_Instr_1__LD_1Bit_Instr_1 {
    InputPort xi_LD0 = Write_TDR[0];
  }
  ScanRegister Write_TDR[0] {
    ResetValue 1'b0;
    ScanInSource SIB;
    CaptureSource LD_1Bit_Instr_1.LD0[0];
  }
  ScanRegister SIB {
    ResetValue 1'b0;
    ScanInSource TDI;
    CaptureSource SIB;
  }
  ScanMux SIB_Mux SelectedBy SIB {
    1'b0 : SIB;
    1'b1 : Write_TDR;
  }
}
Module IJTAG_Network {
  ScanInPort TDI;
  ScanOutPort TDO { Source SIB_Mux; }
  DataOutPort LD_1Bit_Instr_1___LD0 { Source LD_1Bit_Instr_1__W1.LD0; }
  Instance LD_1Bit_Instr_1__W1 Of LD_1Bit_Instr_1__Wrapper {
    InputPort TDI = SIB;
  }
  ScanRegister SIB {
    ResetValue 1'b0;
    ScanInSource TDI;
    CaptureSource SIB;
  }
  ScanMux SIB_Mux SelectedBy SIB {
    1'b0 : SIB;
    1'b1 : LD_1Bit_Instr_1__W1.TDO;
  }
}
Module IJTAG_Demo_1Bit_ET_0 {
  ScanInPort BSCAN_SPARTAN6_tdi;
  ScanOutPort BSCAN_SPARTAN6_tdo { Source IJTAG_Network_1.TDO; }
  DataOutPort K2 { Source IJTAG_Network_1.LD_1Bit_Instr_1___LD0; }
  Instance IJTAG_Network_1 Of IJTAG_Network {
    InputPort TDI = BSCAN_SPARTAN6_tdi;
  }
  ScanInterface BSCAN_SPARTAN6_Gateway {
    Port BSCAN_SPARTAN6_tdi;
    Port BSCAN_SPARTAN6_tdo;
  }
  AccessLink Tap_1149_dot_1 Of STD_1149_1_2001 {
    BSDLEntity XC6SLX9_FTG256;
    USER1 {
      ScanInterface {
        IJTAG_Demo_1Bit_ET_0.BSCAN_SPARTAN6_Gateway;
      }
    }
  }
}
