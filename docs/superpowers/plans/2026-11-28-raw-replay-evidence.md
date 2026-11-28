# Raw replay evidence implementation plan (Task219)

> **For agentic workers:** Use superpowers:executing-plans inline with one fresh whole-branch review.

**Goal:** Pin independent reset artifacts and actual upper-bit raw replay behavior.
**Architecture:** Separate reset paths, high-bit benchmark fixtures, two wide raw
captures, and native-backed regression/evidence checks.
**Tech Stack:** Existing Python unittest/benchmark drivers and unchanged generators.
**Spec:** docs/superpowers/specs/2026-11-28-raw-replay-evidence-design.md.

## Global constraints

Parent master fc5837a65c850406f4a686a41b69cd08ea04a32c;
branch codex/raw-replay-evidence. Preserve 4,131 unaffected inputs, 48 products,
117 stashes and all limits. Four existing Python modules and three Markdown files
change. No producer/dependency/build-rule changes. Archive private nested probe
before source package. Retain prior native/build/broad evidence by identity.

## Review focus

- Flat reset replay must never overwrite the original rewrite before comparison.
- Both distinct wide payloads must pin bits above 64 without changing 4-bit cases.
- Wide raw history must capture the preceding high-bit load, not expected/reset bits.
- PIO dimensions and larger requested raw-call counts keep their established meaning.
- Failed evidence stays failed; package/identity proofs cover final qualified bytes.

### Task 1: strengthen raw replay evidence

**Files:** tests/src/pattern/test_raw_merge_calls.py;
tests/src/pattern/benchmark_raw_merge_calls.py;
tests/src/pattern/benchmark_merged_readback.py;
tests/src/pattern/test_benchmark_merged_readback.py; tracker/spec/plan;
private tests/tests/src/pattern/.task219-* probe and tests/.task219-* helpers;
build/raw-replay-evidence-20261128 evidence.
**Interfaces:** Existing fixture tuples, ordered frame parser, raw captures and
source/flat/generated/local-reset replay helpers. No new public interface.

- [x] Add native reset mutation control and maintained high-bit/matrix tests.
  Run against parent code; expected assertion failures, no setup errors.
- [x] Separate/compare reset rewrites; set high/middle payload bits above width4;
  select at least two raw wide calls while retaining PIO and larger counts.
  Expected focused controls GREEN with actual native observations.
- [x] Run all affected modules normal/static/default-leak SAN and representative
  wide raw matrices. Archive probe; package seven exact members and verify
  4,135 final inputs/48 unchanged products/whitespace. Commit selected diff;
  task-done runs python3 tests/.task219-check.py, expected all checks pass.
- [x] Fresh review; regrade and repair blocking findings once if necessary.
- [x] Merge, verify, push, archive owned ledger/helpers and retire branch.

Review repair: preserve the low nibble at widths 5–7 by choosing the second
added bit at max(4, width//2). At width 5 only bit 4 lies above the low nibble,
so both payloads use that upper position while their low nibbles distinguish
them. Widths 6–7 use bit 4 for the second payload. Literal three-call checks
cover widths 5–16; a native 5/6/7-bit matrix and all three full-module variants
verify the fix. Previously measured 4/512/1,024-bit fixtures remain byte-identical.

Completed at `74a9aa6`, merged and pushed as `782ef08`. Independent
review: 0 Critical, 1 Important (fixed), 0 new Minor issues. Three inherited findings
are resolved and 14 remain deferred. All 15 qualification gates and seven
merged-result checks pass. All 117 stashes are preserved. Broader generator
evidence remains retained by identity.
Evidence: `build/raw-replay-evidence-20261128/`.
