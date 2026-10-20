// Rejected, and the rejection is correct.
//
//   instance_name : SCALAR_ID ;
//
// — an Instance name may not carry a subscript. Ports and registers may:
//
//   port_name     : SCALAR_ID | vector_id ;
//   register_name : SCALAR_ID | vector_id ;
//
// So `ScanRegister sr[3:0]` is legal while `Instance i[3:0]` is not, even
// though the two appear in the same module and look like the same kind of
// declaration. Widening instance_name would cost nothing at the grammar
// level, which is exactly why it should not be done silently.
Module m { Instance i[3:0] Of M; }
