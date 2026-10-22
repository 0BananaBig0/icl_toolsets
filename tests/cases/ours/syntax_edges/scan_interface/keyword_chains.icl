Module EdgeInterfaceKeywordChains {
  ScanInPort si0;
  ScanInPort si1;
  ScanRegister a[3:0] { ScanInSource si0; }
  ScanRegister b[3:0] { ScanInSource si1; }
  ScanOutPort so0 { Source a[0]; }
  ScanOutPort so1 { Source b[0]; }
  SelectPort sel;
  ScanInterface scan {
    Port sel;
    Chain /* keyword name */ Chain {
      Port si0;
      Port so0;
      DefaultLoadValue 2'b01, /* low */ 2'b10;
    }
    Chain Port {
      Port si1;
      Port so1;
      DefaultLoadValue 4'H0;
    }
  }
}
