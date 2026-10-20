// Rejected, and the rejection is correct.
//
// `instance_def` names the module with `module_name`, and
//
//   module_name : SCALAR_ID ;
//
// — a plain identifier only. So `Instance i Of $MOD;` is not legal.
//
// The contrast that makes this easy to get wrong: iProc references use
// `ref_module_name : SCALAR_ID | parameter_ref`, so
// `WriteCallBack $M::$P <D>;` *is* legal and is accepted (see the iProc
// cases in tests/block_review.py). Two rules, one of which takes a
// parameter and one of which does not. Neither should be widened to match
// the other.
Module m { Instance i Of $MOD; }
