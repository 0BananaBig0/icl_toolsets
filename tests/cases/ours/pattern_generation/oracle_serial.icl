// Self-constructed single-register vendor/generator comparison.
Module SerialOracle {
  ScanInPort si;
  ScanRegister r { ScanInSource si; }
  ScanOutPort so { Source r; }
}
