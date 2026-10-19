// Two fragments from the Tessent IJTAG User's Manual, "ICL Extraction":
//
//   Module block1 { ... DataInPort din {
//     Attribute connection_rule_option = "allowed_tied_high"; } ... }
//   Module top { ... Instance block1_I1 Of block1 { InputPort din = 'b1; } ... }
//
// The manual writes them with "..." standing for the rest of each module.
// Filled in here. The point of the example is that tying a data-in port high
// would be a tracing violation in the netlist, and this attribute waives it —
// which is why the generated top level ties the input to 'b1.
Module block1 {
  ScanInPort si;
  ScanOutPort so { Source r; }
  ScanRegister r { ScanInSource si; }
  DataInPort din { Attribute connection_rule_option = "allowed_tied_high"; }
  DataOutPort dout { Source r; }
}
Module top {
  ScanInPort tdi;
  ScanOutPort tdo { Source block1_I1.so; }
  Instance block1_I1 Of block1 { InputPort din = 'b1; }
}
