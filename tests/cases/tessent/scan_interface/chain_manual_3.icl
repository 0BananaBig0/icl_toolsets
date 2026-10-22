// Tessent v2023.4 Chain example; missing child instances and module supplied.
Module chip3 {
   ScanInPort  si[3:0] ;
   ScanOutPort so[3:0] { Source blockA.so1, blockA.so2,
                                blockB.so1, blockB.so2 ; 
                       }   ShiftEnPort   se[1:0] ;
   CaptureEnPort ce[1:0] ;
   UpdateEnPort  ue[1:0] ;   TCKPort tck ;   ScanInterface cA12 { 
      Port se[0]; Port ce[0]; Port ue[0]; 
      Chain c1 { Port si[0]; Port so[0] ; }
      Chain c2 { Port si[1]; Port so[1] ; }
   }         ScanInterface cB12 { 
      Port se[1] ; Port ce[1]; Port ue[1]; 
      Chain c1 { Port si[2]; Port so[2] ; }
      Chain c2 { Port si[3]; Port so[3] ; }
   }
   Instance blockA Of Child; Instance blockB Of Child;
}
Module Child { ScanInPort si1; ScanInPort si2; ScanOutPort so1 { Source si1; } ScanOutPort so2 { Source si2; } }
