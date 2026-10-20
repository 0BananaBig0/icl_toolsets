// `#` is not a comment in ICL. It is in PDL.
//
// The reference repository ships two grammars: icl.g4 (440 lines) and
// pdl.g4 (210 lines). PDL is the other half of IEEE 1687 — the language the
// iProc callbacks are written in — and it is a separate file type, so this
// formatter has no business matching it. But their lexers sit side by side
// and share a lot, which makes the differences worth pinning down:
//
//   PDL : SL_COMMENT : '#' (~('\r'|'\n'))* ;
//   ICL : // and /* */ only
//
// Accepting `#` here would mean PDL had leaked into the ICL scanner. It has
// not — this file is rejected, which is the correct answer.
//
// PDL's own grammar is not fully realised upstream either: issue #4,
// "Compile the bundled PDL grammar into a real Python parser", is still open.
Module m {
  # a PDL comment, not an ICL one
  ScanInPort si;
}
