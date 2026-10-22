Module top {
   ScanInPort       si;
   ScanOutPort      so      {  Source inst2.so;   }
   ShiftEnPort      se;
   SelectPort       en;
   CaptureEnPort    ce;
   UpdateEnPort     ue;
   TCKPort          tck;
   
   Instance inst1 Of inst { 
      InputPort si = si ;
      Parameter Width = 16 ;
      Parameter userValue = 16'hffff ;
   }
   Instance inst2 Of inst { 
      InputPort si = inst1.so ;
      Parameter Width = 4 ;
      Parameter userValue = 4'ha ;
   }
}
