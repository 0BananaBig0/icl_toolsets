// Module mux_remote3 {
//   ScanInPort SI; CaptureEnPort CE; ShiftEnPort SE;
//   UpdateEnPort UE; SelectPort SEL; ResetPort RST;
//   TCKPort TCK; ScanOutPort SO { Source mux3; } DataInPort RS[2:0];
// 
//   Instance WI1 Of WrappedInstr { InputPort SI = SI; }
//   Instance WI2 Of WrappedInstr { InputPort SI = mux1; }
//   Instance WI3 Of WrappedInstr { InputPort SI = mux2; }
// 
//   ScanMux mux1 SelectedBy RS[2] {
//     1'b0 : SI;
//     1'b1 : WI1.SO;
//   }
//   ScanMux mux2 SelectedBy RS[1] {
//     1'b0 : mux1;
//     1'b1 : WI2.SO;
//   }
//   ScanMux mux3 SelectedBy RS[0] {
//     1'b0 : mux2;
//     1'b1 : WI3.SO;
//   }
// }
// 
// Module remote_control {
//   ScanInPort SI; CaptureEnPort CE; ShiftEnPort SE;
//   UpdateEnPort UE; SelectPort SEL; ResetPort RST;
//   TCKPort TCK; ScanOutPort SO { Source reg3.SO; }
//   DataOutPort RS[2:0] { Source reg3.DO[2:0]; }
// 
//   Instance reg3 Of SReg { InputPort SI = SI; Parameter Size = 3;}
// }

Module Context {
  ScanInPort SI;
  ScanOutPort SO { Source reg2.SO; }
  ShiftEnPort SE;
  CaptureEnPort CE;
  UpdateEnPort UE;
  SelectPort SEL;
  ResetPort RST;
  TCKPort TCK;

  Instance SIB1 Of SIB_mux_pre {
    InputPort SI = SI;
    InputPort fromSO = reg1.SO;
  }
  Instance I1 Of Instrument { InputPort DI = reg1.DO; }
  Instance reg1 Of SReg {
    InputPort SI = SIB1.toSI;
    InputPort DI = I1.DO;
  }
  Instance I2 Of Instrument { InputPort DI = reg2.DO; }
  Instance reg2 Of SReg {
    InputPort SI = SIB1.SO;
    InputPort DI = I2.DO;
  }
}

Module Instrument {
  DataInPort DI[(2+3)+2:0];
  DataOutPort DO[7:0];

  Alias enable = DI[7] { RefEnum YesNo; }
  Alias mode[3:0] = DI[6:5],DI[3:2] { RefEnum Modes; }
  Alias data[2:0] = DI[4],DI[1:0];
  Alias okay = DO[0]{ RefEnum PassFail; }
  Alias done = DO[1] { RefEnum YesNo; }
  Alias count[5:0] = DO[7:2];

  Enum PassFail { Pass = 1'b1;
                  Fail = 1'b0; }
  Enum YesNo    { Yes  = 1'b1;
                  No   = 1'b0; }
  Enum Modes    { red   = 4'b0011;
                  blue  = 4'b1000;
                  green = 4'b0100; }
}

Module SRegP0 {    
  ScanInPort      SI;
  ScanOutPort     SO { Source SR[0];}
  ShiftEnPort     SE;
  CaptureEnPort   CE;
  UpdateEnPort    UE;
  SelectPort      SEL;
  ResetPort       RST;
  TCKPort         TCK;
  DataInPort      DI[7:0];
  DataOutPort     DO[7:0] {Source SR; }
  ScanInterface   scan_client { Port SI; Port SO; Port SEL; }

  ScanRegister SR[7:0] { ScanInSource SI;
                         CaptureSource DI;
                         ResetValue 8'b00000000; }                   
}

Module SRegP1 {
  Parameter MSB = 7;

  ScanInPort      SI;
  ScanOutPort     SO { Source SR[0];}
  ShiftEnPort     SE;
  CaptureEnPort   CE;
  UpdateEnPort    UE;
  SelectPort      SEL;
  ResetPort       RST;
  TCKPort         TCK;
  DataInPort      DI[$MSB:0];
  DataOutPort     DO[$MSB:0] {Source SR; }
  ScanInterface   scan_client { Port SI; Port SO; Port SEL; }

  ScanRegister SR[$MSB:0] { ScanInSource SI;
                            CaptureSource DI;
                            ResetValue 'b00000000; 
  }                   
}

Module SRegP2 {
  Instance p1 of SRegP1 { Parameter MSB = 2; }
}

Module SReg {    
  Parameter Size = 8;

  ScanInPort      SI;
  ScanOutPort     SO { Source SR[0];}
  ShiftEnPort     SE;
  CaptureEnPort   CE;
  UpdateEnPort    UE;
  SelectPort      SEL;
  ResetPort       RST;
  TCKPort         TCK;
  DataInPort      DI[$Size-1:0];
  DataOutPort     DO[$Size-1:0] {Source SR; }
  ScanInterface   scan_client { Port SI; Port SO; Port SEL; }

  ScanRegister SR[$Size-1:0] { ScanInSource SI;
                               CaptureSource DI;
                               ResetValue $Size'b00000000; }                   
}

Module SIB_mux_pre {
  ScanInPort    SI;
  CaptureEnPort CE;
  ShiftEnPort   SE;
  UpdateEnPort  UE;
  SelectPort    SEL;
  ResetPort     RST;
  TCKPort       TCK;
  ScanOutPort   SO { Source SR; }
  ScanInterface client {
    Port SI; Port CE; Port SE; Port UE;
    Port SEL; Port RST; Port TCK; Port SO;
  }
  ScanInPort      fromSO;
  ToCaptureEnPort toCE;
  ToShiftEnPort   toSE;
  ToUpdateEnPort  toUE;
  ToSelectPort    toSEL;
  ToResetPort     toRST;
  ToTCKPort       toTCK;
  ScanOutPort     toSI { Source SI; }
  ScanInterface host {
    Port fromSO; Port toCE; Port toSE; Port toUE;
    Port toSEL; Port toRST; Port toTCK; Port toSI;
  }
  ScanRegister SR {
    ScanInSource SIBmux; CaptureSource SR; ResetValue 1'b0;
  }
  ScanMux SIBmux SelectedBy SR {
    1'b0 : SI;
    1'b1 : fromSO;
  }
}