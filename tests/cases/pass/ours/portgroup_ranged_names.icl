// A PortGroup name may carry a range.
//
// PortGroup is not in the published grammar — it is Tessent's — but a name in
// the list is a port name, and a port name may be a vector:
//
//   port_name : SCALAR_ID | vector_id ;
//   vector_id : SCALAR_ID '[' (index | rangex) ']' ;
//
// so `si[3:0]` here names the same port that `ScanInterface { Port si[3:0]; }`
// names — and that second form was already accepted. Only the bare identifier
// was, which made the two disagree about the same port.
//
// The real file (published/real.icl) writes only bare names:
//     PortGroup tap_side {si, so, en}
// so there was no failing input to point at; this was found by checking the
// list rule against port_name, not by a file that broke.
Module m {
  ScanInPort si[3:0];
  ScanOutPort so;
  SelectPort en[1];

  PortGroup with_range { si[3:0] }
  PortGroup with_index { en[1] }
  PortGroup mixed { si[3:0], so, en[1] }
  PortGroup with_comment {
    // The first two feed the TAP side.
    si[3:0], so
  }
  // Unchanged: the spelling a real file uses.
  PortGroup tap_side {si, so, en}
}
