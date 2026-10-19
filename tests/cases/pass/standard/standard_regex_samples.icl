// Every line here is a spelling taken from a published source rather than
// invented: IEEE 1687-2014 Appendix E, the Tessent IJTAG manual's
// parameterised-module example, and the Keysight application note. They are
// here so the scanner and the number rules are checked against the forms the
// standard actually writes, not against the forms we happened to think of.
Module bus {
  Parameter MSB = 5;
  // Appendix E: a space between the base and the digits.
  Attribute lic = 'h ef931832;
  // Appendix E: a parameter reference as a range bound.
  DataInPort datain[$MSB:0];
  DataOutPort dataout { Source RegD[$MSB:0]; }
  // Tessent manual: an unsized literal, no width at all.
  Instance block1_I1 Of block1 { InputPort din = 'b1; }
}
// Appendix E: namespaces and a namespace-qualified module reference.
NameSpace X;
UseNameSpace X;
Module ns_user {
  LocalParameter ABC = 4;
  Instance aa Of X::x;
  ScanRegister r {
    ResetValue 0235;
    DefaultLoadValue 1_2_3;
    CaptureSource 'HE;
  }
}
