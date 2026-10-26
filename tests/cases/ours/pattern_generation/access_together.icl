// Self-constructed IEEE 1687-2014 6.5.2(i) alias access regression.
// a/b require one scan; c is on an unrelated, mutually exclusive route.
Module Network {
  ScanInPort si;
  ScanRegister gate { ScanInSource si; }
  ScanRegister a { ScanInSource gate; }
  ScanRegister b { ScanInSource a; }
  ScanRegister c { ScanInSource gate; }
  ScanMux route SelectedBy gate { 1'b0: c; 1'b1: b; }
  ScanOutPort so { Source route; }
  Alias pair[1:0] = a, ~b { AccessTogether; }
  Alias nested = pair[0];
}
