# Block coverage audit — 2026-10-22

The subsequent [literal parameter-width fix](parameter-width-validation.md)
resolves the recorded alias-chain diagnostic gap; the results below are the
audit snapshot before that production change.

Baseline: `3612110`. This pass reuses the source-backed rulings in
[the conformance review](icl-conformance-review.md), checks the classified inputs
in order, and repairs weaknesses in the test evidence. It changes no production
parser, formatter or LSP behavior. ICL and PDL remain distinct; there is no PDL
parser or shared ICL/PDL token definition.

## Review method and result

Each block was run separately in manifest order, starting with Enum and then
DataOutPort. All 2,015 classified checks passed before the final cross-cutting
regression. These represent 2,013 unique files: the Tessent `chip2.icl` example
intentionally covers ScanMux, DataMux and ClockMux. It is stored once and tested
under all three headings. Trace collection runs each file once and attributes
its reductions to each relevant block.

IEEE Clause 6 is normative; Annex A and the Honza grammar are comparison aids.
The earlier per-block review records allowed items, required cardinalities,
recursive dependencies and source examples. Rechecked the Tessent v2023.4 Enum,
DataOutPort, ICL/PDL Limitations and Unsupported ICL Language Elements topics.
The Enum manual's overflowing values remain negative originals with separately
corrected examples. Tessent's unsupported namespaces/callbacks/AccessTogether
are not restrictions on IEEE syntax. DataOutPort's empty-body difference is
likewise kept explicit. Existing fixtures remain classified under
`tests/cases/{standard,tessent,upstream,ours,...}` with individual provenance in
`conformance.json`; no duplicate examples were added.

For each block the table below gives accepted/rejected classified checks and
unique Bison productions/Flex source-rule lines reached. A trace includes the
block's shared dependencies (Module headers, identifiers, comments, values), so
these columns overlap and must not be summed into a language-coverage percentage.
Rule execution proves reachability, not correct handling of every possible input.
The generated report stays ignored at `build/gramcov/blocks.json`.

| Block | IEEE clause / scope | Accept | Reject | Productions | Scanner rules |
|---|---|---:|---:|---:|---:|
| enum | 6.5.3 | 25 | 35 | 50 | 31 |
| data_out_port | 6.4.6.7 | 24 | 27 | 88 | 48 |
| data_in_port | 6.4.6.6 | 11 | 9 | 52 | 27 |
| scan_in_port | 6.4.6.1 | 9 | 14 | 59 | 32 |
| scan_out_port | 6.4.6.2 | 12 | 18 | 66 | 39 |
| shift_en_port | 6.4.6.3 | 9 | 14 | 59 | 32 |
| capture_en_port | 6.4.6.4 | 9 | 14 | 59 | 32 |
| update_en_port | 6.4.6.5 | 9 | 14 | 59 | 32 |
| to_shift_en_port | 6.4.6.8 | 10 | 14 | 67 | 35 |
| to_capture_en_port | 6.4.6.9 | 12 | 14 | 80 | 39 |
| to_update_en_port | 6.4.6.10 | 11 | 14 | 73 | 37 |
| select_port | 6.4.6.11 | 9 | 14 | 60 | 32 |
| to_select_port | 6.4.6.12 | 9 | 16 | 60 | 32 |
| reset_port | 6.4.6.13 | 12 | 18 | 60 | 34 |
| to_reset_port | 6.4.6.14 | 11 | 19 | 61 | 34 |
| tms_port | 6.4.6.15 | 9 | 14 | 66 | 38 |
| to_tms_port | 6.4.6.16 | 10 | 14 | 71 | 36 |
| tck_port | 6.4.6.17 | 9 | 14 | 60 | 32 |
| to_tck_port | 6.4.6.18 | 10 | 14 | 61 | 33 |
| clock_port | 6.4.6.19 | 12 | 16 | 62 | 35 |
| to_clock_port | 6.4.6.20 | 35 | 39 | 64 | 36 |
| trst_port | 6.4.6.21 | 9 | 14 | 60 | 32 |
| to_trst_port | 6.4.6.22 | 9 | 16 | 61 | 33 |
| to_ir_select_port | 6.4.6.23 | 9 | 14 | 61 | 33 |
| address_port | 6.4.6.24 | 9 | 14 | 60 | 32 |
| write_en_port | 6.4.6.25 | 9 | 14 | 60 | 32 |
| read_en_port | 6.4.6.26 | 9 | 14 | 60 | 32 |
| port_inventory | 6.4.6 | 3 | 3 | 33 | 21 |
| scan_register | 6.4.8 | 25 | 36 | 119 | 66 |
| data_register | 6.4.9 | 42 | 62 | 164 | 67 |
| logic_signal | 6.4.10 | 50 | 51 | 73 | 36 |
| scan_mux | 6.4.11 | 15 | 36 | 93 | 49 |
| data_mux | 6.4.12 | 14 | 37 | 93 | 49 |
| clock_mux | 6.4.13 | 13 | 37 | 93 | 49 |
| one_hot_scan_group | 6.4.14 | 10 | 25 | 79 | 47 |
| one_hot_data_group | 6.4.15 | 18 | 28 | 90 | 49 |
| scan_interface | 6.4.16 | 25 | 47 | 115 | 60 |
| instance | 6.4.7 | 29 | 42 | 117 | 61 |
| access_link | 6.4.17 | 33 | 48 | 184 | 109 |
| alias | 6.5.2 | 22 | 30 | 80 | 42 |
| parameter | 6.5.4 | 19 | 31 | 92 | 46 |
| attribute | 6.5.5 | 11 | 41 | 90 | 41 |
| module | 6.4.2–5 | 16 | 17 | 36 | 23 |
| fallback | Compatibility / invalid contexts | 7 | 35 | 127 | 55 |
| lexical | 6.3 | 82 | 82 | 108 | 58 |
| eda-corpus | Vendor integration | 121 | 0 | 150 | 76 |
| unclassified | Additional corpus (not block-classified) | 201 | 70 | 288 | 114 |

## Recursive and lexical evidence

- Enum: wide decimal/hexadecimal values, partial-nibble truncation, concatenated
  widths, nested unsigned arithmetic, parameter references, duplicates and
  mismatched widths. DataOutPort then adds ranged/hierarchical/inverted sources,
  concatenations, singleton Source/Enable/RefEnum and nested Attribute values.
- Every other port reuses the checked lexical dependencies while testing its
  own permitted body items, empty-body rules, ranges, comments and invalid
  properties. Clock-factor/Period units and polarity are independently covered.
- Registers and OneHot groups cover nested addressable versus selectable forms,
  callback arguments, value/signal/Enum alternatives and disallowed nesting.
  ScanInterface covers Chain bodies, property placement and duplicate names.
- LogicSignal composes parentheses, reductions, unary and binary operators,
  comparisons and unknown numeric arguments. Muxes combine code lists, source
  concatenations and nested index expressions. Alias permits deep hierarchy;
  ordinary source references retain their narrower IEEE hierarchy rules.
- Instance, AccessLink, Parameter and Attribute cover namespaces, body contexts,
  forward references, cycles/type mismatches, balanced Generic payload nesting,
  comments/strings containing delimiters and malformed/unterminated forms.
- Lexical cases cover number bases, width tokens, underscores, longest-match
  boundaries, comments, strings/escapes, nonreserved keywords, CRLF, BOM and
  invalid bytes. Existing depth tests reach 200 nested parentheses, long
  expressions and unary chains; this is finite stress coverage, not an
  unbounded recursion guarantee.

The full suite also runs 29,177 acceptance-preserving metamorphic variants across
45 blocks (138 unsuitable seeds explicitly excluded), 5,568 keyword/context
cases, golden formatting, stdin/file parity, idempotence, protocol and live LSP
checks. These complement the generator trace; generated C++ coverage is not used
as a replacement for handwritten source evidence. The approach follows the
positive/negative corpus, round-trip, differential and protocol practices
recorded in [the Clang-methods report](clang-testing-and-icl-status.md).

## Harness and architecture findings

- The trace runner now rejects crashes, timeouts and disagreement with classified
  acceptance even after a complete-looking trace. Explicit empty selections and
  empty generated rule tables fail. `--no-fail` only relaxes unreached-rule
  failures. Per-block evidence includes multiply classified fixtures.
- The older regression runner's `check()` omitted its Boolean return, bypassing
  content and idempotence checks after successful parsing. Tests reproduced the
  false successes before the fix. All 1,416 assertions now execute and pass.
- Regression and legacy block checks now preserve whitespace inside strings and
  comments. Only layout and a common multiline-comment indentation shift are
  normalized. Leading BOM removal remains an intentional scanner behavior.
  All 714 legacy block inputs pass. Their existing inline inputs are materialized
  under ignored `tests/generated/ours/block_review/` when run.
- Removed stale comments that described concat control-signal inversion and
  ToCaptureEn/ToUpdateEn lists as unsupported. IEEE 6.3.12 and 6.4.6.9–10 permit
  them. This corrects test documentation without weakening parser validation.
- Both frontends still use `ParsedDocument`, the same driver/parser/AST and
  `FormatParsedDocument`/format-tree renderer. LSP caches that parsed document
  and publishes its driver diagnostics. No second grammar, redundant AST or
  new runtime dependency was introduced.

The strengthened harness has 10 grammar-coverage, 3 block-oracle and 5 regression
unit tests. Failure injection checks that corrupted output, a crash or a timeout
cannot be reported as success. Tests live under `tests/tests/`, mirroring their
`tests/` source scripts.

## Measurements and limitations

On this Linux x86-64 host (GCC 16.2.1), the 689,451-byte Advanced_FlexScan fixture
produced 142,925,761 bytes of Bison/Flex trace in 44.88 seconds, using 35,304 KiB
maximum RSS (`/usr/bin/time -v`, stderr redirected to a file). A 30-second limit
was too short. The runner now uses temporary-file spooling and a 180-second
per-input bound, then scans trace lines without retaining the entire trace in
Python memory. The first complete spooled run took 112.43 seconds and 35,396 KiB
maximum RSS over 2,284 files. These are local measurements, not throughput
promises; slower CI machines exceeding that bound need a measured adjustment.

All 334 Bison productions and 119 Flex rules are reached. Ordinary gcov reports
561/561 parser lines and 211/213 scanner lines. The scanner misses map to entry
and string-clear lines of `IclScanGeneric`; its body executes in Generic payload
fixtures. Across handwritten sources: 2,681/2,729 lines (98.2%) and 3,865/6,506 raw
branches (59.4%). `make coverage` **still exits nonzero** against its unchanged
100% line / 90% branch thresholds. The denominator has not been filtered to hide
misses; C++ cleanup/exception paths and compiler mappings remain in raw reports.

Native GCC condition coverage (`-O0 -g --coverage -fcondition-coverage`,
`gcov -j -g -b`) measures parser 105/106 and scanner 24/24 outcomes. The remaining
parser mapping is aggregate construction at `signal_path`, not an explicit
handwritten condition. It remains included. Native condition coverage and
production/rule reachability answer different questions from raw branch coverage.

A known LSP diagnostic gap remains: a based literal with a parameter-derived
width may overflow after parameter resolution without a diagnostic. The live
suite records this as one explicit expected failure, paired with an ordinary
transport test. Full numerical elaboration, connectivity/type DRC and vendor
retargeting are not implemented. No result here proves all possible IEEE ICL
programs correct or removes these documented limits.

## Reproduction and delivery

```sh
PYTHONPATH=build/reference/python ICL_REF_DIR=/tmp/icl-reference-honza-20261021 \
  make check-dynamic BISON=/usr/bin/bison -j4
make formatcheck namingcheck BISON=/usr/bin/bison
make gramcov BISON=/usr/bin/bison
make coverage BISON=/usr/bin/bison -j4  # expected nonzero strict threshold result
```

The pinned upstream comparison is documented in
[honza-icl-comparison.md](honza-icl-comparison.md): 268 production mappings,
75 keyword patterns, eight byte-matched input files and six passing real adapter
tests. The refreshed 2,284-input comparison has 1,866 agreements, 270 local-only
acceptances and 148 reference-only acceptances; all reference-only rejections
have source-backed adjudications and none is a classified positive.

Independent review found no critical or important defect. It independently ran
the 10+3+5 harness tests, 1,416 regression assertions and 714 block inputs, and
verified manifest/differential counts. Its shared-fixture attribution concern is
resolved in the final trace report (ScanMux15 and DataMux14 accepted inputs).
The final focused harness/Makefile tests and refreshed `make gramcov` pass after
that attribution fix; the full dynamic suite and style/naming checks also pass.

Installed with `make install-dynamic prefix=/home/banana/.local
BISON=/usr/bin/bison`. Both executable files byte-match the tested build and
link dynamically. Installed checks pass: 2,015 conformance inputs, 42 semantic
LSP tests, 62 CLI/LSP interoperability assertions and seven diagnostics tests
with the one documented expected failure. The real Vim9/coc test passes
startup, error publication/clearing, formatting, definition, hover, symbols,
completion and preservation of invalid input. Its isolated configuration names
`/home/banana/.local/bin/icl-lsp` explicitly; no user editor settings were changed.

The coc JSON alternatives are maintained in [editor/coc-lsp.json](editor/coc-lsp.json)
and [editor/coc-cli.json](editor/coc-cli.json). Prefer LSP formatting to inherit
Vim/coc indentation options; the standalone CLI defaults to two spaces and does
not read vimrc. Build traces, gcov data, generated cases and binaries remain
ignored; no generated implementation is staged.
