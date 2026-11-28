# Callback failure evidence implementation plan (Task220)

> **For agentic workers:** Use superpowers:executing-plans inline with one fresh whole-branch review.

**Goal:** Qualify callback timeout cleanup, once-only reset replay and ownership errors.
**Architecture:** Existing GNU process-group timeout, corrected reset helper
arguments and precise native diagnostic assertions with real failure controls.
**Tech Stack:** Existing Python unittest/drivers, GNU timeout/time and unchanged generators.
**Spec:** docs/superpowers/specs/2026-11-28-callback-failure-evidence-design.md.

## Global constraints

Parent master 27996489119825e19b7666efcaa326ecff91f207;
branch codex/callback-failure-evidence. Four Python files plus tracker/spec/plan.
Preserve 4,131 unaffected inputs, 48 products, 117 stashes and resource bounds.
Driver outer timeout stays 30s; inner GNU timeout is 27s plus 2s kill grace.
No producer/API/dependency/build-rule changes. Archive private nested probe
before package. Retain native/build/broad evidence by identity only.

## Review focus

- Deadline tests must prove the measured candidate actually started and child cleanup occurred.
- Normal/control native runs retain the real deadline, limits and complete output validation.
- Combined reset rewrites already contain the second input and execute it exactly once.
- Ownership diagnostics must identify the real storage violation with protected artifacts intact.
- Final module, product and package bytes must justify the retained evidence.

### Task 1: callback failure evidence

**Files:** tests/src/pattern/benchmark_merged_callbacks.py;
tests/src/pattern/test_merged_callbacks.py;
tests/src/pattern/test_mixed_callback_groups.py;
tests/src/pattern/test_primary_callback_state.py; tracker/spec/plan;
private tests/tests/src/pattern/.task220-* and tests/.task220-* helpers;
build/callback-failure-evidence-20261128.
**Interfaces:** Existing benchmark command/JSON receipts, split PDL/reset rewrite
and native ownership failure. No public interface changes.

- [ ] Add real descendant deadline and split-replay tests plus native diagnostic
  corruption control. Run parent code; expect three failure controls with no
  setup errors and retain actual original error for its exact cause.
- [ ] Wrap measured command in GNU timeout, remove duplicate second input from
  reset rewrite replay and assert exact ownership cause. Expected focused GREEN.
- [ ] Run all three complete modules normal/static/default-leak SAN (nine reports),
  archive private probe, package seven exact members, verify final inputs/products
  and whitespace. Commit selected diff; task-done runs python3 tests/.task220-check.py.
- [ ] Fresh review; regrade and repair blocking findings in one wave if needed.
- [ ] Merge, verify, push, archive owned ledger/helpers and retire branch.
