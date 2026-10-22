# ICL syntax edge fixtures implementation plan

> Execute inline with superpowers:executing-plans; request one independent review.

**Goal:** Add maintained ICL files exercising lexical boundaries and recursive
grammar combinations beyond the existing rule-reachability corpus.

**Architecture:** Reuse `tests/cases/conformance.json`, the CLI conformance runner,
the live LSP diagnostics runner, metamorphic tests and corpus deduplication.
No new runner, dependency or runtime feature is needed.

**Scope/authorization:** User requested more syntax/regular-expression fixtures
and previously authorized autonomous repository edits. Maintain handwritten
fixtures under `tests/cases/ours/syntax_edges/`, classified by block and accompanied
by IEEE clause references. Do not copy generated matrices into Git.

- [x] Read existing coverage, lexer/parser and IEEE 1687-2014 normative rules.
  Baseline: 1,943/1,943 conformance cases pass in the existing isolated worktree.
- [x] Author block-by-block positives and single-fault negatives for enum widths,
  numeric token boundaries, strings/comments, recursive expressions, muxes,
  callbacks, hierarchy and AccessLink scanner-state transitions.
- [x] Register literal expected outcomes and provenance in the manifest; verify
  each block with the current real formatter before moving to the next block.
- [x] Run full dynamic regression, corpus uniqueness and grammar-rule coverage;
  review new expectations independently and record results/limits.
- [x] Commit and integrate the reviewed fixtures under standing authorization.

Review focus: reject invalid digits even after a valid token prefix; distinguish
unknown bits from forbidden significant-bit truncation; preserve comments and
escaped quotes; distinguish permitted Alias hierarchy from ordinary signal
references; leave Generic scanning mode before the next normal declaration.

Fixture-only work needs no production-code red/green change. Positive/negative
pairs must catch overly narrow regexes and overly permissive token boundaries.
If a real defect is exposed, preserve the failing fixture before changing code.

Ruling: the parameter-width overflow case confirms already documented deferred
elaboration (`NumberLiteral` deliberately defers parameter widths). Preserve it
as a separate LSP expected failure, with fixed-width overflow in the normative
manifest. Adding general parameter elaboration would exceed this fixture task;
do not label the semantic failure a valid IEEE input or count it as conformance.

Verification: full dynamic/style/naming checks exit 0; 2,015 conformance inputs,
29,177 metamorphic variants, 5,568 identifier/context cases, all 334/119 parser/
scanner rules. The LSP suite explicitly reports one expected failure. Corpus
audit: 2,297 files, no duplicate files or new unreviewed repeated modules.

Independent review found that an expected-failure annotation could also hide
transport failure for the semantic-limit fixture. Added an unmarked companion
publication/version test. Reviewer injected a timeout and confirmed normal suite
failure, then injected a future width diagnostic and confirmed unexpected
success. Citation correction and the existing size/base whitespace ambiguity
are documented; the final review reports no residual findings.
