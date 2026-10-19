# origen/ — Origen-SDK/ijtag

21 files from `Origen-SDK/ijtag/examples/`, byte-identical to the upstream
repository (verified by content hash). That repository is the Origen
interface/driver for IEEE 1687, and is one of the "Repositories associated
with IJTAG" listed by Honza255/icl_parser.

17 of the 21 were rejected before this directory existed. Two constructs
accounted for all 17:

**`#include "file.icl"`** — 16 files. Not IEEE 1687: the standard's
`icl_source` has no such item. It is Origen's extension, spelled out in its
treetop grammar as `include_def : "#include" S path:STRING s ";"? s`, with the
semicolon optional. Kept verbatim, kept at top level (where that grammar puts
it), and the path is not opened — this tool formats one document and does not
follow references.

**`Instance p1 of SRegP1`** — 3 files, lowercase `of`. The standard and the
published ANTLR grammar both write `'Of'` only; Origen's grammar spells the
alternative as `('Of' / 'of')`. Two spellings in one group, `of_kw`, so the
`;` and `{ }` forms of an instance cannot fall out of step. The spelling is
preserved, not normalised.

One thing in Origen's grammar was deliberately **not** implemented:
`single_quoted_string`, `'...'`. No example file uses it, and it would collide
with the based-number literals `'b`, `'d` and `'h`, which are everywhere.
