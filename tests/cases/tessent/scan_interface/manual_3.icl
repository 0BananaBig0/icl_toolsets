// Tessent v2023.4 ScanInterface example; TAP ellipsis supplied and fromTdr1/tdrEn1 typos corrected.
Module sib {
   ScanInPort       si;
   ScanOutPort      so   { Source SIB; }
   ShiftEnPort      se;
   CaptureEnPort    ce;
   UpdateEnPort     ue;
   SelectPort       en;
   TCKPort          tck;
   ScanInPort       fso;
   ToSelectPort     ten  { Source And; }
   ScanInterface client {
     Port si; Port so; 
     Port se; Port ce; Port ue; Port en; 
   }
   ScanInterface host   {
     Port fso; Port ten; 
   }   
   ScanRegister SIB { 
      ResetValue     1'b0;
      ScanInSource   Mux;
   }   
   ScanMux Mux SelectedBy SIB {
      1'b0 : si;
      1'b1 : fso;
   }   
   LogicSignal And { en,SIB == 2'b11;  }
}
