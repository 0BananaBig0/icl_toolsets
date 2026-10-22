// Tessent v2023.4 Instance example; stray 7 in WriteEnPort declaration removed.
Module top {
   AddressPort   address[7:0] ;
   DataInPort    dataIn[15:0] ;
   DataInPort    write_enable ;   
   DataInPort    read_enable ;
   DataOutPort   dataOut[15:0] { 
     Source outMux[15:0] ; 
     Enable read_enable ;
   }
   OneHotDataGroup outMux[15:0] {
     Instance phy1 Of phy { 
       AddressValue 2'b01, 6'b000000 ;
       InputPort we = write_enable ;
       InputPort re = read_enable ;
     }
     Instance phy2 Of phy { 
       AddressValue 2'b10, 6'b000000 ;
       InputPort we = write_enable ;
       InputPort re = read_enable ;
     }
   }
}
Module phy { 
   WriteEnPort we ;
   ReadEnPort    re ;
   AddressPort   address1[3:0] ;
   AddressPort   address2[3:0] ;
   DataInPort    dataIn1[7:0] ;
   DataInPort    dataIn2[7:0] ;
   DataOutPort   dataOut[15:0] { Source outMux[15:0] ;
 }
   OneHotDataGroup outMux[15:0] {
     Instance bank1 Of bank { 
       AddressValue 4'b0001,2'b00 ;
     }
     Instance bank2 Of bank { 
       AddressValue 4'b0010,2'b00 ;
     }
   }
}
Module bank {
   WriteEnPort   we ;
   ReadEnPort    re ;
   AddressPort   address[1:0];
   DataInPort    dataIn[15:0];
   DataOutPort   dataOut[15:0] { Source outMux[15:0] ;
 }   OneHotDataGroup outMux[15:0] {
//   AddressValue 2'bx,4'bx,2'b<num>
     DataRegister D0[15:0] { AddressValue 2'b00 ; }
     DataRegister D1[15:0] { AddressValue 2'b01 ; }
     DataRegister D2[15:0] { AddressValue 2'b10 ; }
     DataRegister D3[15:0] { AddressValue 2'b11 ; }
   }
}
