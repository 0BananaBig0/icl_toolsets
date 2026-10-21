// Rejected: `0b101` is pdl.g4's TCL_BIN_NUMBER, not ICL.
//
//   PDL : TCL_BIN_NUMBER : '0b' ('0'..'1'|'x'|'X')+ ;
//   ICL : sized_number   : 4'b0011  — the base comes after an apostrophe.
Module m {
  ScanRegister sr {
    ResetValue 0b101;
  }
}
