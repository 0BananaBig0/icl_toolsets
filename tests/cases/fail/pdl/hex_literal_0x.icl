// Rejected: `0x10` is pdl.g4's TCL_HEX_NUMBER, not ICL.
//
//   PDL : TCL_HEX_NUMBER : '0x' ('0'..'9'|'A'..'F'|'a'..'f'|'x'|'X')+ ;
//   ICL : sized_number   : 8'h10  — the base comes after an apostrophe.
//
// ICL has no 0x form at all. Accepted as NUMBER("0") + IDENT("x10") and
// re-rendered as `0 x10`, which changed the file's text, before the lexer
// rule named it.
Module m {
  ScanRegister sr {
    ResetValue 0x10;
  }
}
