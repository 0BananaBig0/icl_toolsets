// Completed IEEE 1687-2014 6.5.2(h) primary-input end-state probe.
Module EndStatePins {
  DataInPort pin;
  DataOutPort observed { Source ~pin; }
  Alias idle = pin { iApplyEndState 1'b0; }
}
