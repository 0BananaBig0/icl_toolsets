Module EdgeParameterForwardNumericWidth {
  Parameter VALUE = $WIDTH'H0_FF;
  Parameter WIDTH = $BITS;
  LocalParameter BITS = 8;
  DataInPort input[($WIDTH - 1):0] { DefaultLoadValue $VALUE; }
}
