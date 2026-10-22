Module ParameterWidthInvertedAliasDeferred {
  Parameter A = 8'd8; Parameter W = ~$A; Attribute a = $W'hFFFF;
}
