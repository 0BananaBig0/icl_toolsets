Module ParameterWidthNestedOverride {
  Parameter W = 4; Instance child Of Leaf { Parameter W = 16; Parameter V = $W'h10; }
}
