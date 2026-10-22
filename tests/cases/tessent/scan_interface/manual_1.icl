// Tessent v2023.4 ScanInterface example; TAP ellipsis supplied and fromTdr1/tdrEn1 typos corrected.
Module tdr4_1 {
   ScanInPort       si;
   ScanOutPort      so       { Source R[0]; }
   ShiftEnPort      se;
   SelectPort       en;
   CaptureEnPort    ce;
   UpdateEnPort     ue;
   TCKPort          tck;   
   ScanRegister R[7:0] { 
      ScanInSource     si;
      ResetValue       8'b0 ;
   }
}
Module tdr4_2 {
   ScanInPort       si;
   ScanOutPort      so       { Source R[0]; }
   ShiftEnPort      se;
   SelectPort       en;
   CaptureEnPort    ce;
   UpdateEnPort     ue;
   TCKPort          tck;
   ScanInterface interface1 { 
     Port en ;
     Port si; Port so; 
   }
   ScanRegister R[7:0] { 
      ScanInSource     si;
      ResetValue       8'b0 ;
   }
}
Module tdr4_3 {
   ScanInPort       si;
   ScanOutPort      so       { Source R[0]; }
   ShiftEnPort      se;
   SelectPort       en;
   CaptureEnPort    ce;
   UpdateEnPort     ue;
   TCKPort          tck;
   ScanInterface interface2 { 
     Port se ;
     Chain chain1 {
        Port si; Port so; 
     }
   }   
   ScanRegister R[7:0] { 
      ScanInSource     si;
      ResetValue       8'b0 ;
   }
}
