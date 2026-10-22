# Clang testing methods and current ICL evidence — 2026-10-22

## What Clang actually establishes

Neither clang-format nor clangd proves that every possible C++ program is handled
correctly. Clang tracks standard features and defect reports, tests representative
and problematic inputs, and fixes newly discovered failures. Its
[C++ status tables](https://clang.llvm.org/cxx_status.html) distinguish supported,
partial and unsupported features by language mode/release. A test-suite pass is
not a proof of language completeness.

The two tools also have different responsibilities:

- **clangd** builds on Clang's parser, AST and diagnostic machinery. Its own tests
  cover editor features, marked source positions/ranges, unsaved-buffer behavior
  and LSP transport. See the [architecture and testing walkthrough](https://clangd.llvm.org/design/code).
- **clang-format** uses formatting-oriented token/line analysis and cannot replace
  compiler validation. It checks expected output, stability on already formatted
  input, whitespace perturbations and incomplete input. See
  [FormatTestBase.h](https://raw.githubusercontent.com/llvm/llvm-project/main/clang/unittests/Format/FormatTestBase.h)
  and [UnwrappedLineParser.h](https://raw.githubusercontent.com/llvm/llvm-project/main/clang/lib/Format/UnwrappedLineParser.h).
- **Clang diagnostics** are tested with small inputs annotated with expected
  errors/warnings, including their text and locations; missing, extra or misplaced
  diagnostics fail verification. See the
  [diagnostic verifier documentation](https://clang.llvm.org/docs/InternalsManual.html#verifying-diagnostics).
- **LLVM testing** combines unit tests, targeted regression cases and broader
  program suites. Its [testing guide](https://llvm.org/docs/TestingGuide.html)
  describes lit/FileCheck and test organization. Clang also has
  [coverage-guided fuzzers](https://raw.githubusercontent.com/llvm/llvm-project/main/clang/tools/clang-fuzzer/README.txt)
  for malformed inputs and more structured generated programs.

These are transferable methods, not a requirement to adopt LLVM's build system
or add lit/googletest to this small C++/Python project. This repository continues
to share one parser and AST between its formatter and LSP as required.

## Mapping to this repository

| Method | Existing or added evidence | What it does not establish |
|---|---|---|
| Standard-feature inventory | Normative block review, 26 port kinds, pinned Honza keyword and 268-production mapping | That an implementation branch is correct in every combination |
| Positive/negative regression tests | Classified conformance manifest; first-error LSP range/version tests | Every malformed program or every diagnostic message |
| Formatter output/stability checks | Golden files, content preservation, reparse/idempotence, whitespace properties | Hardware connectivity or PDL behavior |
| Shared compiler frontend for LSP | Shared parsed document/AST; every manifest case sent through live edits | Full clangd-equivalent semantic analysis or multi-error recovery |
| Context variation | New 92-name × 58-context matrix plus malformed-name controls | Exhaustive combinations of every ICL construct |
| Independent implementation comparison | Real Honza ANTLR parser, with token exhaustion and lexer errors checked | The reference is not a normative oracle |
| Fuzzing and coverage | Deterministic mutation fuzzing, 334/334 reductions and 119/119 scanner rules | Current mutation fuzzing is not libFuzzer-style coverage guided; rule reachability is not branch coverage |

## New systematic context check

`make keywordcheck` takes 58 maintained templates in
`tests/cases/ours/keyword_contexts/` and substitutes 92 independently maintained
legal scalar names, including all 75 word spellings in the pinned comparison.
It also tries four malformed scalar spellings from IEEE examples. Expected
acceptance follows **IEEE 1687-2014 §6.3.3(b), printed 60/PDF 74**, which permits
keywords as identifiers, and **§6.3.10, printed 65–68/PDF 79–82**.

Every port declaration is included. Other contexts cover module/namespace names,
parameters and references, enum names/members, attributes, registers, muxes,
logic operands, aliases, instance module/namespace references, hierarchical
references, scan interfaces/chains, one-hot groups, and callback names. Context
seeds are syntax examples; they do not claim complete referenced hardware or PDL
models. No PDL lexer, grammar or shared token definitions were introduced.

This adapts the compiler practice of testing the same permitted construct in
multiple contexts. It is not claimed that LLVM uses this exact ICL matrix.
The runner uses batches of 64 files and one persistent LSP process. It checks
individual acceptance, CLI content preservation/reparse/idempotence, versioned
LSP acceptance, exact formatting text/ranges, and no edits on invalid input.
Each template is formatted once; renaming its identifiers derives the expected
formatted output for the matrix. This cross-checks the two frontends without
starting a CLI subprocess for every generated case.
Generated inputs/results stay under ignored `tests/generated/ours/keyword_contexts/`.

The initial run identified a wrong negative expectation: in a callback procedure
slot, `A$suffix` is procedure `A` plus a parameter argument, not one scalar name.
The complete statement is allowed by §6.4.9, printed 118/PDF 132, and the lexical
separation rules. The expectation was corrected after reading that production;
the parser was not weakened. The corpus audit also caught an identical module
in two new templates; the module-name template was reduced to an empty module,
preserving its purpose without adding a duplicate exemption.

## Fresh results

Base production behavior: commit `2ded0ce`. No executable behavior change was
needed during this follow-up; one obsolete parser comment falsely claiming
unsupported blocks were accepted was corrected.

- `make check-dynamic formatcheck namingcheck BISON=/usr/bin/bison -j4` passed:
  **1,943 classified inputs**, all also exercised through live LSP edits;
  **5,568 identifier/context cases**, zero failures; **27,364 metamorphic
  variants across 45 blocks**, zero unexpected results.
- Grammar/scanner traces reached **334/334 productions and 119/119 rules**.
  The deduplication policy passed with zero duplicate-file groups and no new
  unreviewed module repetition. Five matrix-oracle tests pass.
- The actual reference adapter passed **6/6 tests**, using
  `antlr4-python3-runtime==4.7.2` installed only into ignored `build/reference/python`
  and Honza revision `333f8bc7a54964138ac14a34fde9e942c496436d`.
- The corpus differential run covered **2,211 files**: **1,811 agreements**,
  **264 local-only accepts**, **136 reference-only accepts**. All 136
  reference-only cases are classified local negatives. The added diagnostic
  Enum-width mismatch explains the increase from the earlier 135; IEEE §6.5.3(a)
  requires consistent enum widths, which the reference grammar alone does not
  enforce. No reference-only case was a maintained positive rejected locally.
- Of 264 local-only accepts, 259 are maintained positives. The remaining five
  are existing semantic-LSP fixtures: keyword identifiers in
  `completion_keywords.icl`, `hierarchy.icl` and `review.icl` (IEEE 6.3.3), the
  standard AccessLink body in `interface.icl` (IEEE 6.4.17), and documented legacy
  PortGroup compatibility in `port_groups.icl`. These are expected differences,
  not evidence of 264 implementation defects.

The reviewed matrix took **3.09 s wall time / 28,200 KiB peak RSS**
on Linux 6.18.33.2 WSL2 x86_64, with existing `-O2` binaries and generated directory,
measured using `/usr/bin/time`. These are test-runner resource measurements,
not product performance improvements.

Reference commands (the comparison deliberately exits1 when differences exist):

```sh
PYTHONPATH=build/reference/python \
ICL_REF_DIR=/tmp/icl-reference-honza-20261021 \
python3 tests/tests/test_antlr_diff.py

PYTHONPATH=build/reference/python \
ICL_REF_DIR=/tmp/icl-reference-honza-20261021 \
python3 tests/antlr_diff.py --binary build/icl-format

make keywordcheck BISON=/usr/bin/bison
```

Independent review found and verified a false-green path in the first version
of the matrix: comparing whitespace-stripped LSP output ignored malformed
whitespace and replacement ranges. The final runner checks exact output and
full-document ranges against cached CLI formatting expectations. The reviewer
intercepted 92 positive responses, collapsed their whitespace and changed their
ranges to insertions: the old runner passed; the final runner detects all 92 and
returns failure. No remaining review findings were reported.

The final full regression ran with `PYTHONPATH=build/reference/python` and
`ICL_REF_DIR` pointing at the pinned checkout, so its six ANTLR adapter tests
actually ran. An archive produced by `make dist` also built and passed
`make keywordcheck BISON=false FLEX=false -j4`: all 5 oracle tests and all 5,568
matrix cases passed without invoking generators or linking static runtimes.
No executable behavior changed, so the existing installed programs and editor
settings were not changed in this testing-only follow-up. Static builds and a
new GUI/editor session were not rerun.

## Honest support statement

These checks found **no newly reproduced IEEE syntax defect**. All maintained
positive and negative expectations pass, and the new systematic context campaign
passes. That supports the reviewed/tested IEEE 1687-2014 ICL syntax scope; it does
**not** certify every recursive input or all normative semantics.

Known limits remain: no full parameter/width elaboration or connectivity DRC,
first-error reporting rather than multi-error recovery, opaque Generic AccessLink
payloads, and documented vendor/legacy extensions accepted alongside IEEE syntax.
The handwritten line/branch-coverage acceptance gate remains open as described in
[verification and limits](icl-verification.md). Coverage-guided fuzzing,
mutation-testing the implementation, and differential testing against a running
vendor tool were not added or claimed in this follow-up. Future failures need
small source-backed regressions before fixes, just as compiler projects do.
