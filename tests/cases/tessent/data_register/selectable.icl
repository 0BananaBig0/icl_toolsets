// Tessent Shell Reference v2023.4 DataRegister example; normalized nonbreaking spaces.
Module chip_selectable_dataregister { 
   DataInPort  write_enable ;
   DataInPort  read_enable ;
   DataInPort  address[7:0] ;
   DataInPort  dataIn[15:0] ;
   DataOutPort dataOut[15:0] { Source outMux[15:0] ; }
   Parameter bank1Address = 4'b0011 ;
   Parameter bank2Address = 4'b1100 ;
   DataMux outMux[15:0] SelectedBy address[7:4] { 
     $bank1Address : bank1.dataOut[15:0] ;
     $bank2Address : bank2.dataOut[15:0] ;
   }
   Instance bank1 Of bank {
     InputPort we = write_enable ;
     InputPort re = read_enable ;
     InputPort dataIn[15:0] = dataIn[15:0] ;
     InputPort address[3:0] = address[3:0] ; 
   }
   Instance bank2 Of bank {
     InputPort we = write_enable ;
     InputPort re = read_enable ; 
     InputPort dataIn[15:0] = dataIn[15:0] ;
     InputPort address[3:0] = address[3:0] ; 
     Parameter D0_Address = 4'b1000 ;
     Parameter D1_Address = 4'b1100 ;
   }
}
Module bank {
   DataInPort    we ;  Alias weR = we { iApplyEndState 1'b0 ; } 
   DataInPort    re ;
   DataInPort    address[3:0];
   DataInPort    dataIn[15:0];
   DataOutPort   dataOut[15:0] { Source Dx_Mux[15:0] ;
 Enable re; }   Parameter D0_Address = 4'b0000 ;
   Parameter D1_Address = 4'b0001 ;
   DataMux Dx_Mux[15:0] SelectedBy we,address[3:0] {
     1'b0, $D0_Address : D0[15:0] ;
     1'b0, $D1_Address : D1[15:0] ;
   }
   LogicSignal D0_wen {  re,we,address[3:0] == 2'b01, $D0_Address ; }
   DataRegister D0[15:0] { 
     ResetValue       16'b0 ;
     WriteEnSource    D0_wen ;
     WriteDataSource  dataIn[15:0] ;
   }
   LogicSignal D1_wen {  re,we,address[3:0] == 2'b01, $D1_Address ; }
   DataRegister D1[15:0] { 
     ResetValue       16'b0 ;
     WriteEnSource    D1_wen ;
     WriteDataSource  dataIn[15:0] ;
   }
}
