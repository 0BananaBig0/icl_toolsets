# Renewed block-by-block ICL audit

**Goal:** Revalidate syntax blocks and their existing tests against IEEE1687-2014,
strengthen coverage evidence, preserve the shared AST/pipeline, and install and
verify both executables with coc.nvim configuration supplied at completion.

**Design:** Reuse the established scope and normative rulings in
`docs/icl-conformance-review.md` and the 268-production Honza comparison. No PDL
grammar is implemented. Syntax/local validation is distinct from parameter
elaboration and connectivity DRC. User authorized autonomous repository edits.

**Execution:** Inline using superpowers:executing-plans, with one independent
review at the end. Reuse the clean isolated `icl-conformance` worktree at3612110.
No runtime rewrite unless a source-backed failing test requires one.

- [x] Revalidate blocks in order: Enum; DataOutPort; each remaining port;
  registers; LogicSignal and muxes; groups/interfaces; Instance and AccessLink;
  Alias, parameters, attributes, namespaces/module; lexical and compatibility.
  Record per-block positive/negative evidence and recursive dependencies.
- [x] Strengthen test evidence where inspection finds gaps. Test any harness
  failure before its fix. Keep all inputs classified under tests and generated
  results ignored. Resolve stale test comments without removing unique coverage.
- [x] Rerun the pinned Honza keyword/production/input comparison and real parser
  adapter. Recheck IEEE/Tessent examples and the common AST/processing chain.
- [x] Measure handwritten line/branch/condition coverage, run full regression,
  review independently, and document any remaining limitations without lowering
  coverage thresholds or excluding handwritten logic.
- [x] Install the dynamic executables and verify installed binaries and editor
  integration. Both coc configurations are maintained for the final response.
  No user vimrc or coc settings were changed. Git delivery uses standing
  commit/push authorization after final review.

Expected scale: ~2,300 maintained inputs, merged baseline ~142KiB, largest benchmark ~673KiB;
existing trace jobs are bounded per file and generated matrices are batched.
Avoid a parser subprocess per production or repeated full corpus scans for each
block when a single ordered run can collect the evidence.

Review focus: false coverage from crashed processes, stale positive expectations,
recursive value/token boundaries, extensions confused with IEEE, and preserved
diagnostics/shared-AST behavior for both installed frontends.
