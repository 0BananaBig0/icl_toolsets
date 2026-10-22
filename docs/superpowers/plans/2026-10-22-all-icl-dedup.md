# All-test ICL deduplication implementation plan

> Use superpowers:executing-plans inline under the user's standing autonomous-work authorization.

**Goal:** Audit every maintained ICL fixture, consolidate redundant files/modules,
and account for repetitions required by formatter tests and complete examples.

**Spec:** User's request to check/process duplicates across all collected tests;
existing requirement to preserve high grammar/formatter/LSP coverage.

**Architecture:** Extend the test-only shared-parser inventory with a tolerant
audit mode. A Python standard-library checker compares whole-file SHA-256 and
existing namespace-sensitive module IDs, enforcing a reviewed duplicate policy.
Rewrite source/conformance references to canonical files when removing copies.
Runtime parsers, language definitions and installed executables are unchanged.

## Constraints and decisions

- Reuse existing isolated worktree at ac55d66. No remote source mutations.
- Audit tests/cases and tests/golden, plus other maintained test ICL. Ignore
  tests/generated: benchmark generators deliberately repeat modules for scale.
- Byte-identical duplicates are redundant. Token-identical modules in different
  comment/BOM/CRLF/blank-line cases are distinct formatter inputs and remain.
- Preserve complete multi-module source examples and pinned original baselines;
  deleting their common helpers would weaken integration/source evidence.
  Record each retained duplicate group and exact file hashes, not a blanket skip.
- Rejected inputs receive whole-file duplicate checks; no reliable full module
  tree exists, so do not guess module boundaries or edit negative cases.
- New scripts live in tests/, with tests under tests/tests/. No dependencies.
- Scale: 2212 files; batch shared-parser audit once, hash-map comparisons. Do not
  run a fresh subprocess for each module or repeatedly parse shared baselines.

## Task 1: Shared audit and canonical-reference verification

- [x] RED: tests for file copies, namespace-sensitive modules, malformed inputs,
  opaque text, distinct comment variants, and policy drift.
- [x] Add --audit to tests/icl_corpus_inventory.cc (default remains strict).
- [x] Add tests/audit_icl_corpus.py and tests/tests/test_audit_icl_corpus.py.
- [x] Generalize EDA provenance check to canonical modules in existing multi-module
  baselines. Test real membership/context rather than file-name assumptions.
- [x] Focused GREEN; make corpuscheck includes full-corpus duplicate check.

## Task 2: Consolidate, document and verify

- [x] Remove 6 exact file copies and 47 imported standalone module copies; replace
  two copies of the Tessent chip2 mux example with canonical references.
- [x] Preserve old paths/hashes/comments in tests/cases/deduplication.json;
  update conformance/EDA manifests and documentation. Each grammar block keeps
  its test even when several blocks share one canonical file.
- [x] Review all retained groups and pin reason + occurrences in duplicate policy.
- [x] Run make check-dynamic BISON=/usr/bin/bison; preserve full rule coverage,
  original baseline hashes, golden formatting and LSP tests.
- [x] Independent final review; commit, fast-forward master, push under standing
  authorization. Report actual removals and retained test variants explicitly.

## Review focus

Namespace and opaque-body identity, invalid-input handling, strict importer
behavior, retained test coverage, source/category metadata across shared files,
stale policy entries, and untracked/new fixtures bypassing the audit.

## Verification evidence

- Removed55 files:6 byte copies,47 redundant imported definitions,2 repeated
  manual-example files. The remaining2157 files have no byte-identical copies.
- Exact before/after equality of all1289 accepted module identities. The25
  retained matching groups are reviewed, hash-pinned coverage/source variants.
- `make corpuscheck BISON=/usr/bin/bison`: all12 focused tests and global audit pass.
- `make check-dynamic BISON=/usr/bin/bison -j4`: exit0;1876 conformance cases,
  796 corpus assertions,13 golden outputs, original baseline hash checks and
  all existing LSP tests pass. Coverage reaches334/334 Bison productions and
  119/119 Flex rules over2144 corpus inputs; this is not full branch coverage.
- Local audit measurement:2157 files,0.50s elapsed,70644KiB peak RSS on this
  Linux workspace, using `/usr/bin/time` and the documented audit command.
- Independent final review found no actionable issue and independently checked
  all55 deletions, metadata preservation and exact module-set equality.
