// Tessent v2023.4 Instance example; stray 7 in WriteEnPort declaration removed.
Module chip_tdr { 
   ScanInPort       si;
   ScanOutPort      so       { Source tdr_inst.so; }
   ShiftEnPort      se;
   SelectPort       en;
   CaptureEnPort    ce;
   UpdateEnPort     ue;
   TCKPort          tck;
   Instance tdr_inst Of tdr { 
     InputPort si = si ;
   }   
}
Module tdr {
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
