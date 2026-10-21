// Rejected, and the rejection is correct — and the asymmetry is the point.
//
//   alias_def    : 'Alias' alias_name '=' concat_hier_data_signal ... ;
//   alias_name   : reg_port_signal_id ;           SCALAR_ID | vector_id
//   hier_data_signal : (instance_name '.')* reg_port_signal_id ;
//
// So the *value* may be hierarchical (`Alias a = i1.i2.b;` — legal, and in
// the corpus), while the *name* may not. The two sit on either side of one
// `=`, and widening alias_name to match would cost nothing at the grammar
// level, which is exactly why this case exists.
Module m {
  Alias i.a = b;
}
