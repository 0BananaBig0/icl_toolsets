// Tessent v2023.4 Chain example; missing child instances and module supplied.
Module chip2 {
   ScanInPort  siA1 ;
   ScanInPort  siA2 ;
   ScanOutPort soA1 { Source blockA.so1 ; }
   ScanOutPort soA2 { Source blockA.so2 ; }
   ScanInPort  siB1 ;
   ScanInPort  siB2 ;
   ScanOutPort soB1 { Source blockB.so1 ; }
   ScanOutPort soB2 { Source blockB.so2 ; }
   SelectPort   selA ;
   SelectPort   selB ;   ShiftEnPort   se ;
   CaptureEnPort ce ;
   UpdateEnPort  ue ;   TCKPort tck ;   ScanInterface cA12 { 
      Port selA; 
      Chain c1 { Port siA1; Port soA1 ; }
      Chain c2 { Port siA2; Port soA2 ; }
   }         ScanInterface cB12 { 
      Port selB; 
      Chain c1 { Port siB1; Port soB1 ; }
      Chain c2 { Port siB2; Port soB2 ; }
   }      
   Instance blockA Of Child; Instance blockB Of Child;
}
Module Child { ScanInPort si1; ScanInPort si2; ScanOutPort so1 { Source si1; } ScanOutPort so2 { Source si2; } }
