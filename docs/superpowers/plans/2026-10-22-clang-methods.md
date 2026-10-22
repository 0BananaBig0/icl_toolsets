# Clang-style ICL conformance follow-up

Scope: explain LLVM's layered testing, check current ICL support using fresh
independent evidence, and adopt a missing systematic context check. Standing
user authorization permits repository changes and fixes without new approval.

- [x] Read official LLVM testing/architecture sources and normative IEEE ICL
  identifier rules; compare with existing checks and documented limits.
- [x] Add an independent keyword-by-identifier-context matrix with maintained
  source fixtures, batch CLI formatting/reparse checks and persistent LSP edits.
  Expected legal names come from IEEE 6.3.3 and 6.3.10, not our lexer tokens.
- [x] Run the real reference parser adapter and differential corpus comparison;
  adjudicate differences using the existing normative manifest and source.
- [x] Reproduce/fix concrete defects, run focused/full checks, obtain independent
  review, document evidence and limitations, then commit and push.

Use existing shared parser and test helpers. Generated combinations/reports stay
under ignored tests/generated and build. Keep ICL separate from PDL. Test matrices
must fail on an empty selection or tool error; a clean rejection of legal input
is a failure. No test count is a proof of complete syntax or semantic support.

Evidence: 1,943/1,943 conformance inputs, 5,568 context cases, 27,364 syntax
variants, 334/334 productions, 119/119 scanner rules; full dynamic/style/naming
suite and real six-test ANTLR adapter pass. Differential comparison of 2,211 files:
1,811 agreements, 264 local-only, 136 reference-only; every reference-only case is a
maintained negative. Independent reviewer reproduced a test false-green with
corrupted formatting responses; exact output/range assertions now detect all 92
corrupted positives, with no remaining review findings. Source archive builds
and runs keywordcheck with generators disabled. See
[the final report](../../clang-testing-and-icl-status.md) for limits.
