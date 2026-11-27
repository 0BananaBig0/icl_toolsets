# Replay boundary regression plan (Task204)

> **For agentic workers:** Use superpowers:executing-plans inline, with one fresh independent whole-branch review.

**Goal:** Close four recorded event, polarity, reset-rewrite and flat-clock test gaps.
**Spec:** docs/superpowers/specs/2026-11-28-replay-boundaries-design.md.
**Architecture:** Reuse maintained fixtures and existing continuation/five-form helpers; add only the missing inputs and observable assertions.
**Tech stack:** Existing Python unittest, native C++17 generators and existing ASAN/UBSAN builds; no new dependencies or production changes.

## Constraints

- Parent master bfe5e4fd7936ef240f8c2f785c1c1caf90652d17; branch codex/replay-boundaries in .worktrees/bsdl-defaults.
- Preserve 117 stashes, sibling ledgers, foreign worktrees and excluded item1. Inherit 35 findings; candidate remainder 31.
- Change exactly the three named Python tests plus this task's documentation. All 44 products and 4130 unaffected input hashes must match completed Task203.
- Opaque actual data remains unknown; comparison polarity does not create device observations. Clock checks/waits retain version and component semantics. Rewritten reset prerequisites must be actual commands before the original procedure call.
- Create new private QA scripts under project-root/tests and keep fault artifacts out of source packages.

### Task 1: protect replay boundaries

**Files:** tests/src/pattern/test_fixed_scan_fanout.py, tests/src/pattern/test_opaque_child_preparation.py, tests/src/pattern/test_clock_period_overrides.py; docs/remaining-work.md and this task's spec/plan.
**Interfaces:** Keep helper signatures, existing public metadata and production APIs. Reuse the physical-clock helper for encoded names and the data-reset fixture for real local-reset rewriting.

- [x] Verify four finding identities and protected fixtures. Record the old event-count blind spot and current expected polarity, prerequisite and mixed encoded-clock behavior before maintained edits.
- [x] Add two-apply preservation, cared inverted comparison, real reset-prerequisite and mixed encoded vector regressions. Require actual current behavior to pass and relevant controlled omissions/polarity errors to fail.
- [x] Run all three complete modules under normal/static generators and selected new functional methods with default leak-enabled instrumentation. Verify existing fanout timing bounds, unchanged production and all 44 products.
- [x] Package/install the final source, check exact changed members/private-script exclusion and record actual qualification and all four dispositions. Stage/read the complete selected diff, commit with grouped messages and run task-done with the artifact checker.

## Branch completion

- [ ] One independent whole-branch review; regrade findings and declined areas, one verified Critical/Important repair wave if needed, defer Minors without rereview.
- [ ] Merge into master, verify actual merged identities and affected behavior, push, retire only this branch and archive only this workspace. Mark verified progress and continue the original inventory.
