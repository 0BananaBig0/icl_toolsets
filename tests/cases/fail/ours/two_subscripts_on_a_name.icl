// A vector name carries exactly one subscript in the published grammar:
//
//   vector_id : SCALAR_ID '[' (index | rangex) ']' ;
//
// so `sr[0][1]` — two subscripts on one name — is not legal, and must stay
// rejected. It is rejected here.
//
// This file guards against the opposite mistake too: an index may *contain*
// a name that itself has subscripts (`sr[a[0][1][2]]`, in
// tests/block_review.py), because index : integer_expr. That is accepted.
// The two must not be confused: one is one subscript too many on a name,
// the other is a nested expression inside the one subscript.
Module m { ScanRegister sr[0][1]; }
