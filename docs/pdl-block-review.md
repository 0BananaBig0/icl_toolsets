# PDL block review — IEEE 1687-2014

This review concerns PDL clauses 7–8. Enum and DataOutPort belong to ICL clause 6;
they are not PDL grammar blocks. The two languages retain separate parsers,
keyword sets and trees. Passing the tests below establishes the listed static
checks, not complete runtime PDL conformance.

## Sources and reproducibility

Normative source: *IEEE Standard for Access and Control of Instrumentation
Embedded within a Semiconductor Device*, IEEE Std 1687-2014. Page references
below are printed page / PDF page. Normative command productions and rules take
precedence over the informative grammar summaries and third-party parsers.

The maintained [case manifest](../tests/cases/pdl_blocks.json) maps each input to
its block, source and clause. Cases live under `tests/cases/standard/pdl/`,
`ours/pdl/`, `tessent/pdl/` and `upstream/pdl/`, each in `block_review/`.
Standard examples are adapted/completed, rather than represented as original
tool output. They add stream declarations, module bindings, procedure bodies
and comments identifying assumed ICL objects where necessary.

```sh
make pdlcheck BISON=/usr/bin/bison
python3 tests/src/test_pdl_blocks.py --block lexical
python3 tests/src/test_pdl_blocks.py --block iScan
python3 tests/src/test_pdl_blocks.py \
  --formatter build/pdl-format-static --server build/pdl-lsp-static
```

Each block was checked before proceeding to the next: consult the production,
compare existing tests, add missing positive/negative inputs, observe failures
for identified gaps, repair the shared frontend, then rerun the block. The
runner checks executable exit status, refusal to format invalid input,
idempotence, real stdio LSP diagnostics and identical CLI/LSP formatting. It
reads subprocess bytes explicitly so Python newline translation cannot hide
CRLF source changes. Existing functional tests add navigation, completions,
correction after edits, non-execution and Tcl execution-equivalence checks on
harmless constructed programs.

## Sequential block findings

| Block | IEEE clause; page | Review and maintained cases |
|---|---|---|
| Tcl compatibility, level-0, generic identifiers | 7.5.3, 7.6–7.7; 152–154 / 166–168 | Keywords as names, comments, separators, hierarchical identifiers, both vector delimiters, argument references, incomplete input, LF/CRLF continuations and NUL. Reject malformed literal identifiers and vectors. |
| Numbers | 7.8; 155 / 169 | Decimal, binary/hex prefixes including uppercase, unknown digits, ranges, argument references; reject empty/bad prefixes, signed and ICL-sized literals. Multiline scan data follows its separate production. |
| iPDLLevel | 7.9.3; 158 / 172 | Levels, version, level-0 restrictions and runtime level-1 values. Missing/abbreviated level declarations remain documented fragment compatibility. |
| iProcsForModule | 7.9.4; 159 / 173 | Qualified module and optional procedure namespace, invalid literal names, binding and runtime context. |
| iProc | 7.9.5; 160 / 174 | Valid/default arguments, duplicate/invalid names, required-after-default rejection, nested declarations in level-0, dynamic argument lists. Required-before-default is rule **a**. |
| iUseProcNameSpace, iPrefix | 7.9.6–7; 161 / 175 | Namespace/instance path validation, reset marker, known versus runtime contexts; preserve separate Tcl and PDL namespaces. |
| iCall | 7.9.8; 162 / 176 | Local, qualified and hierarchical procedure calls; reject malformed literal path/namespace components. |
| iReset | 7.9.9; 164 / 178 | Default/recognized forms, unknown option and excess arguments. |
| iRead | 7.9.10; 165 / 179 | Numeric, enum and argument values, missing target/value forms and malformed values. |
| iWrite | 7.9.11; 166 / 180 | Numeric/enum/argument values, invalid enum vectors and object suffixes, missing operands. |
| iScan | 7.9.12; 168 / 182 | Completed examples, named chain/length, input/output data, multiline binary/hex, omitted optional forms, duplicate options, malformed interface/chain, empty data and decimal whitespace. |
| iOverrideScanInterface | 7.9.13; 171 / 185 | Multiple interfaces; capture/update/broadcast on/off, missing interface/value and malformed paths. |
| iApply | 7.9.14; 173 / 187 | Default and end-state options; invalid option. Vendor end-in-pause remains separately documented. |
| iClock | 7.9.15; 174 / 188 | Clock object, malformed path and missing argument. Hardware clock sensitization remains a runtime check. |
| iClockOverride | 7.9.16; 174 / 188 | Source object and PDL-number frequency multiplier/divider, default form; invalid source, numbers and missing operands. |
| iRunLoop | 7.9.17; 175 / 189 | Counts, tck/sck and time forms, all seven units, fractional/exponent boundaries, missing port and malformed unit/clock. Integer-unit times are retained as observed vendor compatibility. |
| iMerge | 7.9.18; 176 / 190 | Level-0 begin/end balancing and permitted iCall/iNote contents; unmatched begin/end and illegal command rejection. Runtime flattening/scheduling is not simulated. |
| iTake | 7.9.19; 178 / 192 | Resource/default forms, invalid flags and documented vendor persistent form. |
| iRelease | 7.9.20; 179 / 193 | Resource and missing argument. |
| iNote | 7.9.21; 180 / 194 | Notes, optional/malformed parameters. The standard explicitly prohibits errors for malformed parameters: no command-parameter error is emitted. Tcl lexical errors remain errors. |
| iState | 7.9.22; 180 / 194 | Target/value and allowed modifiers, invalid data and multiple modifiers. |
| iGetReadData | 8.4.1; 183 / 197 | Register/interface forms, chain and bin/hex/dec, missing operands, malformed chain and mutually exclusive formats. |
| iGetMiscompares | 8.4.2; 186 / 200 | Readback formats/chains, incomplete arguments and conflicting formats. |
| iGetStatus | 8.4.3; 187 / 201 | Allowed status flag, malformed flag, forbidden level-0 use. |
| iSetFail | 8.4.4; 188 / 202 | Message/options, missing message and invalid option. |
| Level-1 Tcl recursion | 8.1–8.5; 181–189 / 195–203 | Nested control/scripts/expressions, known quoted bodies, dynamic bodies/lists and expanded arguments; deep scripts hit the 128-level bound while equally nested opaque data is preserved. Switch list vertical whitespace and escaped-newline cases exercise body-span handling. |

The clause 8.5 program is completed as a standalone stream. PDF typography
(smart quotes and en dashes) is converted to executable ASCII spelling,
`elseif` is joined to its `if` command, and the status expression uses
`[iGetStatus]`. These editorial repairs are not alternate PDL productions.

For iOverrideScanInterface the detailed production and examples use
`-capture`/`-update`; Table 6's `-captureEn`/`-updateEn` spelling differs. The
validator follows the detailed production. iClockOverride multiplier/divider
arguments follow normative `pdl_number`, not a decimal-only restriction.
Normative times have a decimal fraction and optional `e-` exponent. Integer
times such as `400ns`, `160000ps` and `1500ns` occur in the collected vendor
corpus; their acceptance is a compatibility extension.

Tessent references are the 2023.4 *Tessent Shell Reference Manual* topics
`iNote`, `iApply`, `iTake`, `iWrite`, and `iRunLoop`, available in the local
`tessent_2023_doc/htmldocs/tshell_ref/topics` tree. The corpus README separates
documented extensions from forms only observed in Synopsys SLM source. No
imported vendor PDL was evaluated.

## Honza255 comparison

Reviewed [Honza255/icl_parser](https://github.com/Honza255/icl_parser) at commit
`333f8bc7a54964138ac14a34fde9e942c496436d`, specifically the separate
`src/pdl_parser/pdl.g4`. Its **24 PDL command names are all present** in our
independent PDL command table; `on` and `off` are value words. The upstream
keyword-name fixture is constructed from that list, not copied from an upstream
test. No runnable PDL test suite or PDL fixture files were found at that revision;
its ICL tests do not establish PDL command coverage.

The matching comparison covers identifiers, vector spellings, number prefixes,
scan strings, times, command forms, procedure binding and level-1 names.
Our added cases address literal paths, scan options/data, clock ratios, time
forms, merge balance and readback modifiers. Upstream is useful as a checklist,
but its grammar cannot serve as an independent PDL acceptance oracle as written:

- `scanInterface_name` lacks a terminating semicolon; `text_message` references
  an undefined `string` rule.
- Level-1 command productions are not connected to the source/procedure rules.
- One `-chain` spelling contains a typographic en dash.
- Source-interface argument references and uppercase binary/hex prefixes are
  missing relative to IEEE productions.
- Scan length and clock ratios are restricted to `POS_INT`, where IEEE permits
  `pdl_number`.

These upstream restrictions were not copied into this implementation. No
claim of a successful upstream PDL parser run is made.

## Structure and code quality

Both PDL tools use the same `pdl::Document` definition, command/word AST,
parsing, semantic analysis, declarations/references and formatting. The
executables diverge only into CLI file handling and editor protocol handling.
Formatting requests from the LSP call the same `FormatParsedDocument` used by
the CLI's `FormatDocument`. CLI/protocol plumbing is shared with the ICL tools
through language adapters; that does not merge the language grammars or tokens.

There are no PDL Flex/Bison files. Tcl's non-evaluating parsing APIs handle its
lexical/nesting rules; authored C++ checks handle PDL command semantics. A second
PDL lexer would duplicate Tcl parsing and create disagreement on substitutions,
quotes, comments and scripts. Shared `AnalyzeWords` avoids separate traversal
logic for literal versus runtime arguments. Script bodies are distinguished
from opaque data; known quoted bodies are checked without changing their words.
Expanded argument lists cannot supply reliable static positions, so affected
validation/context is conservative.

Independent review found runtime-option false positives in iRunLoop and
iProcsForModule, and missing literal level-0 iCall argument validation. The
same optional-position ambiguity was reproduced in iScan. Nine regressions
cover these defects and valid controls: unknown level-1 options defer dependent
checks and namespace resolution, while independent literal checks remain.
Literal level-0 call arguments must be numbers, enum names or register/port
identifiers; level-1 calls may carry arbitrary Tcl data.

The CRLF parser shadow is allocated only when needed and preserves byte count:
`backslash CR LF` becomes `backslash LF space` for Tcl's parser, while output
and diagnostic offsets refer to original bytes. Semantic literal extraction
collapses line continuations; output retains original word spelling. This fixes
the mismatch without modifying the original input buffer.

Parsing/formatting walks command/word trees, with bounded script recursion.
Unknown Tcl evaluation/dataflow remains opaque. Navigation may scan declarations;
workspace-query performance is not covered by the CLI timing measurement.
No unrelated architecture rewrite or new dependency was needed for this review.

## Testing practices adopted

- [Flex tests/Makefile.am](https://github.com/westes/flex/blob/master/tests/Makefile.am)
  builds scanner/parser fixtures and runs separate input-driven wrappers. Here
  maintained input fixtures exercise authored PDL behavior through real tools.
- [clang FormatTest.cpp](https://github.com/llvm/llvm-project/blob/main/clang/unittests/Format/FormatTest.cpp)
  compares canonical output and whitespace/nesting variants. Our cases check
  idempotence, preserved words, CLI/LSP parity and harmless Tcl equivalence.
- [clangd DiagnosticsTests.cpp](https://github.com/llvm/llvm-project/blob/main/clang-tools-extra/clangd/unittests/DiagnosticsTests.cpp)
  checks diagnostic messages and ranges. Our functional/corpus suites check
  correction, expected messages/lines and actual stdio diagnostics, in addition
  to positive/negative block acceptance.
- Existing [parser testing practices](icl-testing-practices.md) describe GNU
  Bison harnesses, authored action coverage and the separate ICL rule-reach gate.

External pages above were retrieved on 2026-10-23 as methodological references;
their source is not added to this repository.

## Verification and remaining limits

The final manifest contains **133 cases: 50 accepted and 83 rejected**, including
23 adapted/completed IEEE cases, 108 constructed cases, one vendor time case and
one upstream-keyword case. These supplement the 26 functional test methods and
79 byte-unique vendor streams (73 accepted, six expected argument-order errors).

Verification on 2026-10-23, x86-64 WSL2 Linux 6.18.33.2, glibc 2.43, Tcl 9.0.2,
C++17 optimized builds:

| Check | Result |
|---|---|
| `make check-dynamic formatcheck namingcheck BISON=/usr/bin/bison -j4` | Passed, exit 0. Five optional reference-ANTLR integration tests skipped because `ICL_REF_DIR` was unset. |
| Dynamic PDL tools | 26 functional methods, 133 block cases and all 79 corpus files passed. |
| Rebuilt static PDL tools | Same functional/block/corpus checks passed. |
| Installed dynamic and static PDL tools | Same checks passed separately at each prefix. |
| Installed binaries | All eight hashes match their build outputs; every `--version` terminates successfully. All four static binaries have no ELF `INTERP` or `NEEDED` entries. |
| `make coverage BISON=/usr/bin/bison -j4` | **Failed unchanged strict gate:** 3,715/3,840 lines (96.7%) and 6,176/10,349 branches (59.7%). A final focused PDL functional rerun and complete gcov regeneration give the same totals. |
| PDL document implementation | 930/1,004 lines (92.6%), 2,091/3,443 branches (60.7%). |
| ICL authored actions | Bison actions 563/563 lines; Flex actions 211/213 lines. These are separate from generated-code and rule-reach coverage. |
| Existing ICL rule-reach check | 334/334 grammar productions and 119/119 scanner rules reached across 2,324 files. |

The strict 100% line / 90% branch gate remains unchanged.
Generated Flex/Bison internals and third-party Tcl are excluded; authored PDL
C++ and shared headers are included. Rule/keyword presence is not branch coverage.
Raw branch totals include compiler exception/cleanup edges, but remaining gaps
also include untested authored error/navigation paths. They are not all explained
away as compiler artifacts. A checkpoint commit does not imply that the strict
coverage acceptance gate passed. The block runner's negative inputs assert diagnostic
presence; representative exact messages/locations are covered by the functional
and corpus suites, rather than every block-manifest entry.

The native CLI formatted the same 868,139-byte positive SLM file used for the
baseline, with output discarded, in 0.0250/0.0218/0.0227 seconds (median 0.0227 s),
maximum child peak RSS 18,404 KiB from `getrusage`. Baseline median was 0.0220 s;
these process timings do not establish a speedup. Reproduce with three native
`build/pdl-format tests/cases/pdl_eda/syn_slm/wrapped_pmm_ip_characterization_debug_869fdacdb5f1cd49.pdl`
runs, measuring `time.monotonic` and `RUSAGE_CHILDREN`.

Installation commands executed successfully:

```sh
make install-dynamic prefix=/home/banana/.local BISON=/usr/bin/bison
make install-static \
  prefix=/home/banana/cpp_workspace/icl-format/build/install-static \
  MANIFEST=build/install-static-manifest.txt BISON=/usr/bin/bison
```

The first prefix supplies ordinary `pdl-format`/`pdl-lsp` commands; the second
keeps the static variants separately. The installer prints the maintained
[coc.nvim configuration](editor/coc-lsp.json). The PDL server provides the same
formatting engine as the standalone formatter; no second language-server entry
is required for `pdl-format`. Personal editor configuration was not changed.

Static acceptance does not prove ICL object widths, enum membership, hardware
connectivity, retargeting, scheduling, arbitrary Tcl dataflow or runtime results.
Missing-level fragments, abbreviated declarations and documented/observed vendor
extensions remain accepted. Only the first diagnostic is reported. Tcl 8.6,
target-host static portability and an actual PDL coc.nvim session require
separate verification.
