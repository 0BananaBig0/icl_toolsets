# Combined ICL, PDL and Vim audit — 2026-10-24

Baseline: `8424ac1`. Scope: `icl-format`, `icl-lsp`, `pdl-format`, `pdl-lsp`,
and `vim-icl-pdl-modern`. This is a revalidation of the maintained block reviews
with an additional cross-check of editor highlighting. It does not supersede
documented runtime/elaboration limits or prove acceptance of every possible input.

## Evidence and block order

The normative source is the supplied IEEE Std 1687-2014 PDF, 283 pages, SHA256
`2a5e19825c1ca731e105ea82c68fd9b96fd0e1ccd50778de4909cf3bbe010ac6`.
Physical PDF page numbers equal printed Arabic page numbers plus 14. A fresh
extraction lives under ignored `build/combined-audit/`; the old
`/tmp/ieee1687-standard.txt` was
an HTML challenge response and was discarded as evidence.

ICL uses normative clause 6 and PDL clauses 7–8. Annex grammar summaries and
third-party grammars are comparison aids. The existing
[ICL review](icl-conformance-review.md), [block inventory](block-coverage-audit.md),
and [PDL review](pdl-block-review.md) provide per-block productions, recursive
dependencies, examples, source attribution and compatibility decisions. Those
decisions were reused rather than replacing them with reference-parser acceptance.

The ICL conformance runner was invoked separately for each block in manifest
order, beginning with **Enum (60 cases)** then **DataOutPort (51 cases)**. Each
invocation completed successfully before the next began. All **46 categories /
2,055 checks** passed, including ports, registers, LogicSignal expressions,
muxes/groups/interfaces, hierarchy, aliases, parameters, attributes and lexical
boundaries. Categories also include compatibility and collected-corpus checks.

PDL was checked separately in its own manifest order: lexical forms and
procedures, then command blocks from iReset through iSetFail, recursive Tcl
scripts and the upstream-derived keyword fixture. All **22 categories / 192
checks** passed. These tests verify acceptance/rejection, invalid-input safety,
formatting stability, preserved spellings, LSP diagnostics and CLI/LSP agreement.
Enum and DataOutPort are not PDL grammar blocks.

The Tessent 2023.4 Enum and DataOutPort reference topics were reread. Their
documented differences remain explicit: overflowing example constants are
negative originals with separately corrected derivatives, and Tessent's empty
DataOutPort-body restriction does not override the IEEE production. Existing
completed IEEE/Tessent examples remain in their source-classified directories;
no copies of the proprietary manuals or duplicate fixtures were added.

## Upstream comparison

Rechecked Honza255/icl_parser at
`333f8bc7a54964138ac14a34fde9e942c496436d`:

- All **75** ICL keyword/fixed-word spellings have explicit scanner rules.
- All **268** ICL productions/fragments have a maintained local matching path.
- All **8** upstream ICL files have byte-identical counterparts in our tests.
- All **24** PDL command names occur in the separate PDL command table.
- There are no upstream `.pdl` fixtures at this pinned revision. Its unfinished
  PDL grammar is not used as an executable PDL oracle; its defects and normative
  differences remain listed in the PDL block review.

The real ANTLR 4.7.2 adapter ran all **6 tests** without skips, using a test-only
virtual environment under ignored build output. Differential comparison covered
**2,324 ICL files: 1,889 agreements, 270 local-only accepts, 165 reference-only
accepts**. Every reference-only case has a maintained rejection adjudication;
none is a classified positive rejected locally. Exit status 1 from the raw
comparison reports these known disagreements, not an unexplained regression.
No keywords were imported across language boundaries.

## Confirmed defect and correction

IEEE 1687-2014 §7.9.21, printed page 180 / PDF page 194, defines the iNote tags
`-comment` and `-status`. Both were accepted by PDL analysis but omitted from
the Vim plugin's `pdlOption` pattern. Vim and Neovim showed ordinary text for
direct commands and `tclEmbeddedStatement` inside command substitutions.

Added failing tests for both tags, an embedded command, and quoted text that
must remain a string. The two standalone editor tests failed before the change.
The fix adds only the missing alternatives to the existing PDL option regex;
all four standalone/vim-plug test methods then passed. The ICL syntax file and
PDL parser/token definitions were not changed.

The existing completed standard example
`tests/cases/standard/pdl/block_review/note_examples.pdl` is also selected by the
real-file highlight probes. Its declarations and procedure wrapper supply valid
context for the two short standard examples. Four new probes check command,
option and string groups without duplicating the fixture. Nested and quoted
controls remain in `tests/vim-icl-pdl-modern/test_syntax.vim`.

## Architecture and quality

Both ICL tools consume the same `icl::ParsedDocument`, AST definitions and
`FormatParsedDocument` formatting job. Both PDL tools consume the same
`pdl::Document`, command/word tree, analysis and formatter. LSP formatting reuses
the parsed document rather than reparsing through a CLI subprocess. Shared
protocol and CLI adapters do not merge language grammars or keyword inventories.

PDL uses non-evaluating Tcl parsing APIs; there are no authored PDL Flex/Bison
files to merge or duplicate. ICL's authored `.yy` and `.l` actions remain in
coverage measurement, while generated tables and external Tcl internals are
excluded. Grammar-rule reachability and handwritten line/branch coverage are
reported separately.

The previous indexed variable resolution and scoped completion fixes remain in
place. Their 32,000-variable performance regression and the roughly 1 MiB editor
scan workloads are retained. No new runtime dependency, duplicate parser,
workspace cache or broad refactor is justified by this correction.

Testing continues to use the documented Flex/Bison input-driven approach,
clang-format-style output/idempotence/content checks, and clangd-style diagnostic
and editor protocol checks; see [testing practices](icl-testing-practices.md)
and [PDL independent syntax checks](pdl-syntax-oracles.md).

## Final verification

The independent integrated review found no actionable correctness defect in the
change. It checked the shared document/formatter paths, nested/invalid input,
lexical containment, recursion guards and documented static-analysis limits.
Seven additional current-binary probes preserved opaque data and diagnosed the
malformed constructs as expected.

| Verification | Result |
|---|---|
| `make formatcheck namingcheck BISON=/usr/bin/bison` | Passed. |
| `ICL_REF_DIR=/tmp/icl-reference-honza-20261021 PATH="$PWD/build/combined-audit/reference-venv/bin:$PATH" make check-dynamic BISON=/usr/bin/bison -j4` | Passed, including all six real ANTLR adapter tests without skips. |
| ICL grammar/scanner reachability | 334/334 productions and 119/119 scanner rules reached by 2,324 files. |
| PDL independent syntax properties | 438 cases passed; collected corpus: 79 unique streams from 114 paths, with 73 positive and six expected negative streams. |
| Vim and Neovim full corpus | Each passed 38,487 ICL files and 713 PDL files, 525,625 lines and 43 explicit highlight probes. |
| `make static BISON=/usr/bin/bison -j4` | Passed; all four static outputs verified as statically linked ELF executables. |
| Static focused tests | Shared format CLI 12/12, shared LSP protocol 2/2, PDL tools 46/46 and ICL LSP 44/44 passed. |
| Static PDL reference lookup | 32,000 variables / 926,670 bytes, correct reference locations in 0.0097 s; below the existing 2 s bound. |
| `make coverage BISON=/usr/bin/bison -j4` | Functional checks passed; strict gate **failed** at 4,125/4,234 lines (97.4%) and 6,959/11,331 branches (61.4%), unchanged from the pre-audit baseline. Required thresholds remain 100%/90%. |
| `make install-dynamic BISON=/usr/bin/bison` | Installed all four version 1.0.0 executables into `/home/banana/.local/bin`. |
| Installed executable checks | Shared format CLI 12/12, shared LSP protocol 2/2, PDL tools 46/46 and ICL LSP 44/44 passed; all four `--help` and `--version` commands exited successfully within a five-second timeout. |

The existing editor benchmark also passed with roughly 1 MiB inputs. Observed
whole-buffer syntax scans were 0.37–0.42 s in Vim and 0.53–0.62 s in Neovim on
this host; these are regression measurements, not a speedup claim. See the
commands and platform assumptions in the maintained plugin test runner.

Authored Bison actions reached 570/570 lines; the scanner reached 211/213 lines
(uncovered lines 395–396). Their measured branch coverage remains 53.3% and
55.8%, respectively. `pdl_document.cc` reached 1,276/1,319 lines (96.7%) and
2,815/4,313 branches (65.3%). These figures must not be replaced with the 100%
grammar/scanner-rule reachability figure. No coverage thresholds or exclusions
were changed. The strict coverage requirement remains unmet.

No vendor script was evaluated. Dynamic command/script construction, escaped
switch-list elements and reconstructed multiword namespace scripts retain the
documented [PDL static-analysis limits](pdl-syntax-oracles.md). This phase did not
rerun a live coc.nvim session or a Tcl 8.6 build; actual stdio LSP services and
Vim/Neovim syntax integration were tested. Static ELF checks on this host do not
establish portability to every host, especially for glibc/Tcl runtime services.

The installer printed the shared [coc.nvim configuration](editor/coc-lsp.json).
Each language server supplies both language services and its corresponding
formatter; no separate formatter server is required. Personal editor settings
were not edited.

Raw logs remain under ignored `build/combined-audit/`; the execution checklist is in
[the plan](superpowers/plans/2026-10-24-combined-language-audit.md).

## Follow-up: scan-data lexical boundaries

The renewed audit starts from `605012d` and reuses the source inventory above.
IEEE 1687-2014 §7.7 (printed154/PDF168) and §7.9.12 (169–170/183–184)
require the `0b`/`0x` prefix to remain contiguous while allowing whitespace
between scan-data digits. §7.8 also permits uppercase prefixes. The pinned
upstream `pdl.g4` uses the same contiguous prefix; no new command or token is
needed in either language.

The PDL validator previously removed all scan-data whitespace before validating
the prefix, accepting invalid input such as `iScan scan 8 -si {0 b1010}`.
Four new negative fixtures exposed this through both real CLI and LSP services:
braced binary, quoted hexadecimal, a newline inside the prefix, and a quoted
continuation inside the prefix. A valid-control fixture covers both prefix
cases, spaces and continuations after the prefix, nested script bodies, decimal
data, unknown bits and dynamic/other escaped words. These constructed fixtures
live in `tests/cases/ours/pdl/block_review/` and are listed in `pdl_blocks.json`.

The fix checks the prefix in the original semantic word before compacting its
digits. The continuation case additionally exposed constant words being treated
as dynamic: Tcl text and continuation-only tokens now retain literal status.
Variable substitutions and other backslash escapes remain conservative. The
source word is not rewritten; both PDL tools use the same corrected document
and validator. Both checks remain linear in input size with no new dependency.

Before the fix, all four negative fixtures were accepted without CLI or LSP
diagnostics. After fixing prefix validation, the continuation case still failed;
after correcting literal classification, all 22 iScan checks passed. All 46
PDL executable test methods also passed. The complete sequential pass then
passed 46 ICL categories / 2,055 cases and 22 PDL categories / 197 cases.

ICL Enum and DataOutPort were checked first, followed by the other manifest
blocks. Existing nested and lexical fixtures remain unchanged. The ICL and PDL
ASTs and token inventories remain separate, and each formatter/LSP pair retains
its shared processing path. Vim's PDL number rules already require contiguous
prefixes; highlighting remains separate from parser diagnostics. New fixtures
are included automatically in the editor corpus scan.

Coverage inspection then identified the remaining level-0 error path for raw
interpolated scan data containing a continuation. Two additional LF/CRLF
negative fixtures exercise that path; they ensure an argument reference cannot
be embedded inside a level-0 number. This branch remains necessary for malformed
nonliteral inputs and was retained, rather than removed as supposedly dead code.
The final manifest has **199 PDL cases**, including **24 iScan cases**.

| Follow-up verification | Result |
|---|---|
| Independent integrated review | No actionable findings; checked prefix boundaries, LF/CRLF normalization, literal/dynamic classification, source ranges, shared adapters and highlighting. |
| Google style/naming and full `check-dynamic` | Passed, including real ANTLR adapter tests, ICL grammar/scanner reachability and the initial 197-case PDL manifest. |
| Final two error-path fixtures | Passed in the focused dynamic block, the complete static and installed 199-case block suite, and the repeated coverage run. |
| Vim/Neovim | Full regression passed 38,487 ICL and initially 718 PDL files per editor; the existing corpus driver then rechecked all 720 final PDL files / 76,561 lines with 29 probes and no errors in each editor. |
| Static build and focused checks | All four tools built; shared CLI 12/12, shared protocol 2/2, PDL tools 46/46, final PDL block suite 199/199 passed. |
| Reference-query performance | 32,000 variables / 926,670 bytes: 12.7 ms dynamic and 9.4 ms static on this host; both below the existing 2 s regression bound. This is not a speedup claim. |
| Strict coverage | Functional checks passed; gate still failed at **4,135/4,244 lines (97.4%) and 6,973/11,343 branches (61.5%)**. Thresholds remain 100%/90%. |
| Installation | `make install-dynamic BISON=/usr/bin/bison` installed all four 1.0.0 tools under `/home/banana/.local/bin`. |
| Installed checks | Shared CLI 12/12, shared protocol 2/2, PDL blocks 199/199, PDL tools 46/46 and ICL LSP 44/44 passed. All four help/version commands exited within five seconds. |

The new source logic was exercised by coverage. `pdl_document.cc` reaches
1,286/1,329 lines (96.8%) and 2,829/4,325 branches (65.4%). ICL authored action
line counts remain 570/570 for Bison and 211/213 for Flex. Existing semantic,
dynamic-script, live-coc/Tcl-version and platform limitations above still apply;
none of these results establishes complete language conformance. No generated
outputs are tracked and no vendor PDL was evaluated.

The
[follow-up plan](superpowers/plans/2026-10-24-language-boundary-followup.md)
records execution; raw logs remain under ignored `build/boundary-audit/`.
