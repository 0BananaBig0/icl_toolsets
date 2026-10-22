// Tessent Shell Reference v2023.4 OneHotScanGroup example; NBSP normalized.
Module chip_onehotscangroup { 
   ScanInPort       si;
   ScanOutPort      so       { Source outScanMux; }
   ShiftEnPort      se;
   SelectPort       en[3:0];
   CaptureEnPort    ce;
   UpdateEnPort     ue;
   TCKPort          tck;
   OneHotScanGroup outScanMux {
     Port tdr_0.so ;
     Port tdr_1.so ;
     Port tdr_2.so ;
     Port tdr_3.so ;
   }
   Instance tdr_0 Of tdr { 
     InputPort si = si ;
     InputPort en = en[0];
   }   
   Instance tdr_1 Of tdr { 
     InputPort si = si ;
     InputPort en = en[1];
   }   
   Instance tdr_2 Of tdr { 
     InputPort si = si ;
     InputPort en = en[2];
   }   
   Instance tdr_3 Of tdr { 
     InputPort si = si ;
     InputPort en = en[3];
   }   
}
Module tdr {   ScanInPort       si;
   ScanOutPort      so       { Source R[0]; Enable en; }
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
