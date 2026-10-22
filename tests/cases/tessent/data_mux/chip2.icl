// Tessent Shell Reference v2023.4 DataMux example; NBSP normalized.
Module chip2 {
   ScanInPort  siA ;
   ScanInPort  siB ;
   ScanOutPort so      { Source scanM ; }
   ShiftEnPort   se1 ;
   CaptureEnPort ce1 ;
   UpdateEnPort  ue1 ;
   ShiftEnPort   se2 ;
   CaptureEnPort ce2 ;
   UpdateEnPort  ue2 ;
   TCKPort tck ;
   ClockPort clkA ;
   ClockPort clkB ;
   ToClockPort clk { Source clockM ; }
   DataInPort   dselA ;
   DataInPort   dselB[2:0] ;
   DataOutPort do[3:0] { Source dataM[3:0] ; }
   ScanInterface cA12 { 
      Port se1; Port ce1; Port ue1; 
      Chain c1 { Port siA; Port so ; }
   }
   ScanInterface cB12 { 
      Port se2; Port ce2; Port ue2; 
      Chain c1 { Port siB; Port so ; }
   }
   ScanRegister regA[7:0] {
      ScanInSource siA ;
      ResetValue   8'b0 ;
   }
   ScanRegister regB[7:0] {
      ScanInSource siB ;
      ResetValue   8'b0 ;
   }
   ScanMux scanM SelectedBy dselA {
      1'b0 : regA[0] ;
      1'b1 : regB[0] ;
   }
   ClockMux clockM SelectedBy dselA {
      1'b0 : clkA ;
      1'b1 : clkB ;
   }
   DataMux dataM[3:0] SelectedBy dselB[2:0] { 
      3'b000|3'b100  : regA[7:4] ;
      3'bx01         : regA[3:0] ;
      3'bx1x         : regB[5:2] ;
   }
}
