# Literal payload and opaque history oracle implementation plan (Task213)

> **For agentic workers:** Use superpowers:executing-plans inline with one fresh whole-branch review.

**Goal:** Make two maintained regression cases reject their known false positives.
**Architecture:** Add an unoverridden parameter-default case and canonicalize one existing SI fixture, reusing actual native replay helpers.
**Tech Stack:** Existing Python unittest, generator variants and owned raw evidence.
**Spec:** docs/superpowers/specs/2026-11-28-literal-history-oracles-design.md.

## Global constraints

Parentmaster693ce851bad93f374be2851e87ad8b5f0af651b2; branchcodex/literal-history-oracles.
No producer/Makefile/dependency changes. Preserve48 products,4133 unaffected
inputs,117 stashes and all original native limits. Private mirrored probes
under tests/tests/src/pattern are archived before packaging. Reuse existing
source/flat/generated helpers; no fabricated actual values or observations.

## Review focus

- Decimal default must reach observation without a V override or all-ones padding ambiguity.
- The literal160-bit observation and flattened payload must independently pin all bits.
- Canonical SI and SO must allow the omission marker to match in the wrong-order control.
- Both comment/status cases must reject eager execution while valid queued replay passes.
- Source/product identity and exact package members must justify the stated retained evidence.

### Task 1: strengthen literal/history regressions

**Files:** tests/src/pattern/test_wide_decimal_constants.py;
tests/src/pattern/test_opaque_root_serial_merges.py; tracker/spec/plan;
private tests/tests/src/pattern/.task213-* probes and tests/.task213-* helpers;
owned build/literal-history-oracles-20261128 evidence.
**Interfaces:** Existing native generator binaries and replay helpers; parent
4135input/48product manifests; unchanged Task208 production qualification.

- [x] Protect parent files/manifests. Write real-execution corruption meta-tests.
  Old tests accept zero-default and eager-order controls: outer2failures,0errors.
- [x] Add unoverridden160-bit parameter default with literal flattened payload;
  canonicalize SI6. Both meta-tests pass and record3inner assertion failures.
- [x] Run both full changed modules normal/static/SAN and hex controls:9reports,
  zero failures/errors/skips, with actual source/flat/generated replay. Verify
  all48 products and4133 unaffected inputs. Archive probes before make dist;
  check five exact changed package members and whitespace. Read diff and commit;
  task-done runs python3 tests/.task213-check.py, expected all checks pass.
- [x] One fresh whole-branch review; regrade, one blocking repair wave if needed,
  no rereview; ledger any Minors and final inherited count.
- [x] Merge master, verify merged tests/products/package identities, push,
  archive only own workspace/helpers, retire branch; preserve detached worktree.

Completed at `8abde53`, merged and pushed as `1d601ff`.
Independent review0C/0I/0M resolves two inherited gaps;23 findings remain.
12 qualification gates/9reports105methods and5merged checks/26methods pass.
117stashes preserved; evidence build/literal-history-oracles-20261128/.
