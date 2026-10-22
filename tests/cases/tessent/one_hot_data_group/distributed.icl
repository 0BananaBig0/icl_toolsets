// Tessent Shell Reference v2023.4 OneHotDataGroup example; NBSP normalized.
Module top {
   AddressPort   address[7:0] ;
   DataInPort    dataIn[15:0] ;
   WriteEnPort   write_enable ;
   ReadEnPort    read_enable ;
   DataOutPort   dataOut[15:0] { Source outMux[15:0] ; }
   OneHotDataGroup outMux[15:0] {
     Instance phy1 Of phy {
       AddressValue 2'b01, 6'b000000 ;
     }
     Instance phy2 Of phy {
       AddressValue 2'b10, 6'b000000 ;
     }
   }
}
Module phy {
   DataInPort    dataIn1[7:0] ;
   DataInPort    dataIn2[7:0] ;
   WriteEnPort   we ;
   ReadEnPort    re ;
   AddressPort   address1[3:0] ;
   AddressPort   address2[3:0] ;
   DataOutPort   dataOut[15:0] { Source outMux[15:0] ; }
   OneHotDataGroup outMux[15:0] {
     DataRegister R[15:0] {
       AddressValue 4'b0010,2'b00 ;
     }
     Instance bank1 Of bank {
       AddressValue 4'b0001,2'b00 ;
     }
   }
}
Module bank {
   DataInPort    dataIn[15:0];
   WriteEnPort   we ;
   ReadEnPort    re ;
   AddressPort   address[1:0];
   DataOutPort   dataOut[15:0] { Source outMux[15:0] ; }
   OneHotDataGroup outMux[15:0] {
     DataRegister D0[15:0] { AddressValue 2'b00 ; }
     DataRegister D1[15:0] { AddressValue 2'b01 ; }
     DataRegister D2[15:0] { AddressValue 2'b10 ; }
     DataRegister D3[15:0] { AddressValue 2'b11 ; }
   }
}
