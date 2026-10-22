// Two rules that no real file happens to reach, pinned here so they are
// covered rather than exempted.

// `include_def : "#include" S path:STRING s ";"? s` — the semicolon is
// optional in Origen's grammar. All 16 files that use #include omit it, so
// the form with one would otherwise be untested.
#include "e4.icl";

Module m {
  ScanInPort SI;
  ScanOutPort SO { Source SR; }

  // Lowercase `of` as an attribute name. No file does this — `of` only ever
  // follows an instance name. But every *other* keyword may be an attribute
  // name, and exempting this one would add a third entry to the exception
  // list in the generative keyword test, which is the drift detector. So it
  // is treated uniformly, and this pins the uniform treatment.
  Instance i of M {
    Attribute of = 1'b0;
  }
}
