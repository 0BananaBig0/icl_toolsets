// Tessent v2023.4 ScanRegister Example 1: spaces normalized; sensor stray semicolon before port body removed.
Module tap {
   TCKPort         tck;
   ScanInPort      tdi;
   ScanOutPort     tdo       {  Source IRMux; }
   TMSPort         tms;
   TRSTPort        trst;
   ToSelectPort    tdrEn1    {  Source sel1;  }
   ScanInPort      fromTdr1;
   ToSelectPort    tapEn1    {  Source IR[2]; }
   ToCaptureEnPort tce;
   ToShiftEnPort   tse;
   ToUpdateEnPort  tue;
   ScanInterface clientTAP { 
     Port tdi; Port tdo; 
     Port tms; Port trst; 
   }
   ScanInterface hostTAP   { 
     Port fromTdr1; Port tdrEn1; 
     Port tce; Port tse; Port tue;
   }
   Instance fsm_i Of fsm  {
      InputPort tck  = tck;
      InputPort tms  = tms;
      InputPort trst = trst;
    }
   ScanRegister IR[2:0] {
      CaptureSource  3'b001;
      ResetValue     3'b000;
      ScanInSource   tdi;
   }
   ScanRegister bypass {
      CaptureSource  1'b0;
      ScanInSource   tdi;
   }
   ScanMux IRMux SelectedBy fsm_i.irSel {
      1'b0 : DRMux;
      1'b1 : IR[0];
   }
   ScanMux DRMux SelectedBy IR[2:0] {
      3'bx00 : bypass;
      3'bx01 : fromTdr1;
   }
   LogicSignal sel1 {  IR[2:0] == 3'bx01; }
}
Module fsm {
   TCKPort        tck;
   TMSPort        tms;
   TRSTPort       trst;
   ToIRSelectPort irSel;
   ToResetPort    tlr;
}
