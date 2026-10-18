// The Tessent IJTAG User's Manual, "ICL Network Extraction of Parameterized
// Modules", gives these fragments separated by prose, with their surroundings
// elided and — because they are illustrations — with the instances written
// outside any module. Completed here into one file.
//
// The one change is structural: `Instance` is a module-level construct, so
// the two instances belong inside a module rather than at file scope. The
// manual's own text says as much when it shows the generated top-level ICL.
Module bus {
  DataInPort datain[$MSB:0];
  DataOutPort dataout { Source RegD[$MSB:0]; }
  Parameter MSB = 5;
  ScanRegister RegD[$MSB:0] { ScanInSource datain; }
}
Module top {
  DataInPort toplevel_datain[17:0];
  DataOutPort toplevel_dataout[7:0];
  // The manual's second fragment: an instance that keeps the default, so its
  // data-in port is 6 bits wide.
  Instance bus_inst1 Of bus { InputPort datain[5:0] = toplevel_datain[15:10]; }
  // And one that overrides the parameter — 8 bits wide instead of 6.
  Instance bus_inst2 Of bus {
    Parameter MSB = 7;
    InputPort datain[7:0] = toplevel_datain[17:10];
  }
}
