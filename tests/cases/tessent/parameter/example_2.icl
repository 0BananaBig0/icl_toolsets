Module inst {
   ScanInPort       si;
   ScanOutPort      so      {  Source R[0];   }
   TCKPort          tck;
   Parameter  Width  = 8 ;
   Parameter  Regmax = $Width - 1 ;
   Parameter  userValue = 8'hff ;
   Parameter  Val_A  = 4'b1001, $userValue, 4'b1111 ;
   Parameter  RegName = "R" ;
   LocalParameter  Regmin = 0 ;
   
   ScanRegister R[$Regmax:$Regmin] { 
      Attribute     Register = "$RegName($RegMax:$RegMin)" ;
      ScanInSource  si;
      ResetValue    Reset ;
      RefEnum       SomeValues ;
   }   Enum SomeValues {
       Reset = $Width'b0 ;
       User1 = $userValue ;
   }
}
