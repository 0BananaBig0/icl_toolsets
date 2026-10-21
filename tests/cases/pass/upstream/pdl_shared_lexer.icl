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
// So `0x10` is not ICL, and is now rejected outright by a scanner rule —
// see fail/pdl/hex_literal_0x.icl. It used to be accepted and re-rendered
// as `0 x10`, because the scanner cut it into NUMBER then IDENT and the
// space-separated value list took both; that rewrote the file's text while
// every invariant still held, since character conservation cannot see a
// space that was not there before.
//
// The rule matches only when the base letter follows the digits with no
// space between, so `Period 10 ns;` — a real spelling, with the unit
// separated — is untouched. That is the case this file exists to hold
// alongside the ICL literals.
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
}
