Module wrapped_pmm_ip {
  ScanInPort     TDI;
  ScanOutPort    TDO { Source SIB1[0]; }
  ShiftEnPort    Shift;
  CaptureEnPort  Capture;
  UpdateEnPort   Update;
  SelectPort     Select;
  ResetPort      TRSTN { ActivePolarity 0; }
  TCKPort        TCK;
  ScanInterface client { 
    Port TDI; Port Capture; Port Shift; Port Update;
    Port Select; Port TRSTN; Port TCK; Port TDO;
  }
  ScanRegister SIB1[2:0] {
     ScanInSource SIB1mux; CaptureSource SIB1 ; ResetValue 3'b000;
     Attribute synopsys_access = "RW";
  }
  ScanRegister SIB_LOOP_SEL[3:0] {
     ScanInSource SIB_LOOP_SELmux; CaptureSource SIB_LOOP_SEL; ResetValue 4'b0000;
     Attribute synopsys_access = "RW";
  }
  ScanRegister SIB2_LOOP1[2:0] {
     ScanInSource SIB2_LOOP1mux; CaptureSource SIB2_LOOP1; ResetValue 3'b000;
     Attribute synopsys_access = "RW";
  }
  ScanRegister PM_MODE_TDR[5:0] { ScanInSource TDI; ResetValue 6'b0; Attribute synopsys_access = "RW";}
  ScanRegister FUNC_CLK_TDR[14:0] { ScanInSource TDI; ResetValue 15'b0; Attribute synopsys_access = "RW";}
  ScanRegister LOOP_BYPASS_TDR[0:0] { ScanInSource TDI; ResetValue 1'b1; Attribute synopsys_access = "RW";}
  ScanRegister SIB2_LOOP1_BYPASS_TDR[0:0] { ScanInSource TDI; ResetValue 1'b1; Attribute synopsys_access = "RW";}
  ScanRegister PATH_SEL_LOOP1[17:0] { ScanInSource TDI; ResetValue 18'b0; Attribute synopsys_access = "RW";}
  ScanRegister DELAY_SEL_LOOP1[29:0] { ScanInSource TDI; ResetValue 30'b0; Attribute synopsys_access = "RW";}
  ScanRegister COLLECT_LOOP1[11:0] { ScanInSource TDI; ResetValue 12'b0; Attribute synopsys_access = "RO";}
  ScanRegister CAPTURE_LOOP1[11:0] { ScanInSource TDI; ResetValue 12'b0; Attribute synopsys_access = "RO";}
  ScanMux SIB1mux SelectedBy SIB1 {
    3'b000 :  SIB_LOOP_SEL[0];
    3'b001 :  PM_MODE_TDR[0];
    3'b010 :  FUNC_CLK_TDR[0];
    3'b011 :  TDI;
    3'b100 :  TDI;
    3'b101 :  TDI;
    3'b110 :  TDI;
    3'b111 :  TDI;
 }
  ScanMux SIB_LOOP_SELmux SelectedBy SIB_LOOP_SEL {
    4'b0000 :  LOOP_BYPASS_TDR[0];
    4'b0001 :  SIB2_LOOP1[0];
    4'b0010 :  TDI;
    4'b0011 :  TDI;
    4'b0100 :  TDI;
    4'b0101 :  TDI;
    4'b0110 :  TDI;
    4'b0111 :  TDI;
    4'b1000 :  TDI;
    4'b1001 :  TDI;
    4'b1010 :  TDI;
    4'b1011 :  TDI;
    4'b1100 :  TDI;
    4'b1101 :  TDI;
    4'b1110 :  TDI;
    4'b1111 :  TDI;
 }
  ScanMux SIB2_LOOP1mux SelectedBy SIB2_LOOP1 {
    3'b000 :  SIB2_LOOP1_BYPASS_TDR[0] ;
    3'b001 :  PATH_SEL_LOOP1[0];
    3'b010 :  DELAY_SEL_LOOP1[0];
    3'b011 :  COLLECT_LOOP1[0];
    3'b100 :  CAPTURE_LOOP1[0];
    3'b101 :  TDI;
    3'b110 :  TDI;
    3'b111 :  TDI;
 }
}
