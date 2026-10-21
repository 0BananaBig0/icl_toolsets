// Rejected, and the rejection is correct.
//
//   namespace_name : SCALAR_ID ;
//
// — a namespace name is a single identifier. There is no dotted form, even
// though nearly everything else in ICL that names something may be
// hierarchical (`hier_port`, `hier_data_signal`, `iProc_namespace`).
NameSpace a.b;
