// Rejected, and the rejection is correct.
//
//   scanInterface_name : SCALAR_ID ;
//
// — a ScanInterface name may not carry a subscript, even though the ports
// and registers declared around it may:
//
//   port_name     : SCALAR_ID | vector_id ;
//   register_name : SCALAR_ID | vector_id ;
//
// See §5.76 for the same asymmetry in instance_name and module_name. This
// one matters more than those two, because ScanInterface is where a chip
// file names its tap-facing ports: a subscript here would look natural to
// anyone writing ICL by hand, and widening the rule would cost nothing at
// the grammar level.
Module m { ScanInterface si[3:0] { Port p; } }
