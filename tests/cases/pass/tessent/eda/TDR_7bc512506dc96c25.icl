NameSpace ;
UseNameSpace ;
Module TDR {
   ResetPort reset {
      ActivePolarity 0;
      Attribute connection_rule_option = "allowed_no_source";
   }
   SelectPort en;
   TCKPort tck;
   ScanInPort si;
   ScanOutPort so {
      Source EN_reg;
   }
   ShiftEnPort se;
   UpdateEnPort ue;
   CaptureEnPort ce;
   DataInPort Temp[5:0];
   DataOutPort En {
      Source EN_reg;
   }
   ScanRegister SR[5:0] {
      ScanInSource si;
      CaptureSource Temp;
   }
   ScanRegister EN_reg {
      ScanInSource SR[0];
   }
}
