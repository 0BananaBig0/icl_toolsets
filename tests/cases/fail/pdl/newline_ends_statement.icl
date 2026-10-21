// Rejected: a missing semicolon.
//
//   PDL : eoc : SEMICOLON | NL ;   — a newline can end a statement.
//   ICL : every *_def ends in ';'  — a newline cannot.
//
// So this is PDL punctuation, not a typo to be tolerated.
Module m {
  ScanInPort si
  ScanOutPort so
}
