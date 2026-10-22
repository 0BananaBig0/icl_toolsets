Module DiagnosticParameterWidthOverflow {
  Parameter VALUE = $WIDTH'H1_FF;
  Parameter WIDTH = $BITS;
  LocalParameter BITS = 8;
  DataInPort input[($WIDTH - 1):0] { DefaultLoadValue $VALUE; }
}
