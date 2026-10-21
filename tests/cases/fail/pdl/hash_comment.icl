// Rejected: `#` starts a comment in PDL. ICL uses `//`.
//
//   PDL : SL_COMMENT : '#' (~('\r'|'\n'))* ;
//   ICL : SL_COMMENT : '//' (~('\r'|'\n')*) '\r'? '\n' ;
//
// The comment above is ICL and is fine. The one below is PDL and is not.
Module m {
  ScanInPort si;
}
# This line is PDL, not ICL.
