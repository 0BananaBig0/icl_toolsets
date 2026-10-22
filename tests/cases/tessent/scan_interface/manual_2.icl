// Tessent v2023.4 ScanInterface example; TAP ellipsis supplied and fromTdr1/tdrEn1 typos corrected.
Module tap {
   TCKPort         tck;
   ScanInPort      tdi;
   ScanOutPort     tdo       {  Source IRMux; }
   TMSPort         tms;
   TRSTPort        trst;
   ToSelectPort    tdrEn     {  Source sel;  }
   ScanInPort      fromTdr;
   ToSelectPort    tapEn     {  Source IR[2]; }
   ToCaptureEnPort tce;
   ToShiftEnPort   tse;
   ToUpdateEnPort  tue;
   ScanInterface clientTAP { 
     Port tdi; Port tdo; 
     Port tms; Port trst; 
   }
   ScanInterface hostTAP   { 
     Port fromTdr; Port tdrEn; 
     Port tce; Port tse; Port tue;
   }
   DataInPort sel; ScanInPort IRMux; ScanRegister IR[2:0] { ScanInSource tdi; }
}
