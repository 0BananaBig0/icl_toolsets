// What the PDL and ICL lexers share, and the one place they part company in
// a way that costs us something.
//
// Shared, from pdl.g4 and icl.g4:
//   SCALAR_ID    : ('a'..'z'|'A'..'Z')('a'..'z'|'A'..'Z'|'0'..'9'|'_')* ;
//   ARGUMENT_REF : '$' SCALAR_ID ;
//   TSUFFIX      : 's' | 'ms' | 'us' | 'ns' | 'ps' | 'fs' | 'as' ;
// All three behave identically here — an identifier, a parameter reference,
// and a time unit.
//
// Different, and this file pins the difference:
//   PDL : TCL_HEX_NUMBER : '0x' (...) + ;   TCL_BIN_NUMBER : '0b' (...) + ;
//   ICL : sized numbers are 8'h10, 4'b0011 — there is no 0x form at all.
//
// So `0x10` below is not ICL. It is accepted anyway, and rendered as two
// values `0 x10`, because the scanner cuts it into NUMBER then IDENT and the
// space-separated value list takes both. That is the known cost of the
// permissive attribute rule (§5.32), not a PDL feature.
//
// It is left permissive on purpose: tightening `NUMBER IDENT` would also
// reject `Period 10 ns;` with the space in it, which real files write — see
// the case in this file.
Module m {
  Parameter W = 4;
  ScanRegister sr[$W-1:0] {
    ScanInSource si;
    ResetValue 8'h10;
  }
  ScanRegister sr2 {
    ScanInSource si;
    ResetValue 4'b0011;
  }
  ToClockPort c {
    Period 10ns;
  }
  ToClockPort c2 {
    // Space before the unit — accepted, and kept as written.
    Period 10 ns;
  }
  ScanRegister sr3 {
    ScanInSource si;
    // Not ICL: the 0x spelling from PDL. Accepted, split into `0 x10`.
    ResetValue 0x10;
  }
}
