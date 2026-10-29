// Self-constructed primary observation of held scan storage.
Module MixedScanObservation {
  ScanInPort si;
  ScanRegister data[3:0] { ScanInSource si; ResetValue 4'b0; }
  ScanOutPort so { Source data[0]; }
  DataOutPort observed[3:0] { Source data; }
}
