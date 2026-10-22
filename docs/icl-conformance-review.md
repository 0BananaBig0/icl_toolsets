# ICL conformance and language-service review

Baseline: `44fc3db`. Reference: IEEE 1687-2014, normative Clause 6.
External comparison: Honza255/icl_parser `333f8bc7a54964138ac14a34fde9e942c496436d`.
ICL and PDL remain separate languages; this project has no PDL parser.

## Enum — syntax and local checks verified

Evidence: §6.5.3, printed 137–138 / PDF 151–152; dependencies §§6.3.7–8,
6.3.10.2, 6.3.12. Tessent Shell Reference v2023.4,
`tshell_ref/topics/Command_Enum_id3fd7e051.html`.

- Replaced the generic attribute body with nonempty `enum_item` productions.
- Enum symbols accept keywords, including Instance and AccessLink.
- Numeric values accept the specified concatenation, inversion and recursive
  unsigned integer arithmetic; strings, signal identifiers and boolean operators
  are rejected. Inter-token and standalone comments survive formatting.
- The lexer distinguishes decimal integers, based literals and time values;
  validates base-specific digits; removes unsupported signed/octal/z/? spellings;
  permits decimal separators, parameter widths and whitespace after a base.
- Detects duplicate member names, duplicate enum names within a module, and
  unequal statically known member widths, including sized concatenations.
  Widths depending on parameters or consuming objects require elaboration and
  are not claimed as validated. Widths beyond size_t are preserved without local
  width comparison. This is a syntax tool, not a complete numerical elaborator.
- `tests/cases/conformance.json` records provenance and acceptance independently
  of the directory name. The runner checks acceptance, content and idempotence.
- Original baseline failed 24/39 initial cases, then 5 additional comment/name
  cases and 2 fixed-width cases reproduced further defects. Corrected binary:
  **48/48 conformance cases pass**.
- Moved the fabricated nested-enum positive to the rejection corpus (§6.5.3);
  replaced the signed Verilog literal in the old positive block review. No merged
  Tessent fixture was modified. Tessent's Enum example contains overflowing
  constants; the derived test explicitly records corrections to those values.

## Open work

DataOutPort and subsequent blocks are not yet confirmed. Lexer changes must be
rechecked against every later context. Broader numeric evaluation, semantic
resolution, complete LSP services, coverage accounting, performance measurement,
whole-change review and final installation remain pending; no full-conformance
claim is made by this record.

## DataOutPort — syntax and local cardinality verified

IEEE §6.4.6.7 printed 88/PDF 102; Tessent Shell Reference v2023.4
`Command_Dataoutport_ide13afa16.html`. Honza `dataOutPort_def` and four item
alternatives agree with the normative grammar. Empty braces are accepted as
IEEE specifies even though Tessent prose recommends the semicolon form.

Added dedicated port productions, unsigned expression ranges, qualified signal
references, optional inversion, numeric sources, typed attribute concatenation,
and unique Source/Enable/RefEnum checks. Keywords work as names and references.
Dependencies reuse the same numeric and comment productions as Enum. String
scanning now supports literal/escaped newlines (§6.3.9), and nested block
comments are rejected (§6.3.7).

Initial result: 27/44 failures; then 3/3 lexical edge failures. Corrected result:
47/47 DataOutPort, 95/95 cumulative cases, 494 regression assertions and 730
legacy block cases pass. Corrected two derived positive fixtures and one inline
case claiming Enable lists were legal; the negative cases preserve that defect.
Cross-object signal widths/types and instance elaboration are not validated here.
