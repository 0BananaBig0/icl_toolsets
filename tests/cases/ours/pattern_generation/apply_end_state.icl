// Self-constructed IEEE 1687-2014 6.5.2(h) end-state restoration case.
Module EndState {
  ScanInPort si;
  ScanRegister trigger { ScanInSource si; ResetValue 1'b0; }
  ScanOutPort so { Source trigger; }
  Alias idle_trigger = trigger { iApplyEndState 1'b0; }
}
