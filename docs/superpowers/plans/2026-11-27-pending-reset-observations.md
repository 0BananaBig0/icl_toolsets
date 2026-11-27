# Pending reset observations Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Preserve pending storage observations through ordinary reset preparation.

**Architecture:** Compare requested logical observations at capture with original storage under current access controls. Retain the original snapshot across split TAP setup and existing intermediate feedback constraints.

**Tech Stack:** Existing C++/Z3 scheduler and native/Python regression framework.

**Spec:** docs/superpowers/specs/2026-11-27-pending-reset-observations-design.md

## Global Constraints

- Existing resource limits, event formats and public CLI/PDL/observation interfaces remain unchanged; SymbolicNetwork gains a defaulted internal access-context argument.
- Preserve 117 stashes, foreign worktrees and 44 inherited deferred Minors; item 1 excluded.
- New test scripts belong under root tests/; maintain existing tests in place.

## Review Focus

- PIO preparation changes reset before the old frame snapshot.
- Acyclic data writes change scan reset after the old pending-value check.
- Observations after capture remain allowed to change storage.
- Unknown and mismatching expected data must not determine actual values.
- Failed scheduling preserves state, callbacks and output atomically.

### Task 1: Restore unread storage retention

**Files:** Modify src/pattern/pattern_scheduler.cc, tests/src/pattern/test_data_reset_dependencies.py, tests/src/pattern_scheduler_test.cc, docs/pattern-generation.md and docs/remaining-work.md.

**Interfaces:** Consumes existing pending_reads, data_read_done and symbolic state/fills in ScanScheduler::Impl::Solve; produces the same ScanStep and observation APIs.

- [x] Add CLI ordinary-data-reset and direct-PIO-reset destructive-preparation regressions, asserting atomic rejection for known/unknown expected values; run against the protected Task198 generator and inspect behavioral RED.
- [x] Add native state/callback rollback assertions and safe-preparation/capture-before-update positives. Expected: the unsafe case fails before the implementation; positives retain actual storage1 even for expected 0.
- [x] Snapshot pending register storage at mixed-frame entry and enforce held values after final reset settling using current data_read_done. Expected: new regressions pass without weakening existing feedback-wave checks or constraining actual readback.
- [x] Run focused Python/native suites, normal/static builds, full make check, affected leak-enabled sanitizers, changed-C++ quality, reset/feedback scaling and source packaging. Freeze and check exact input/product identities; record actual command exits and optional skips.
- [x] Update usage/tracker and commit finalized selected diff with grouped messages; run task-done with `python3 tests/.task199-check.py`. Expected: all final required checks pass on exact inputs/products.

## One review repair pass

The independent review of 084a827 found four Critical and two Important issues.
The single repair pass replaces the superseded whole-word frame hold above:

- [x] Reproduce selected-client, direct CaptureSource, partial-bit, inactive-route,
  data-selector and split-TAP-prefix findings in maintained tests before repair.
- [x] Compare requested bits against original storage at capture, including direct
  scan CaptureSource, and pass the same origin through implicit TAP setup.
- [x] Retain current forwarding-port availability while preserving original
  OneHot selection/payload enables; add and pass its concrete repair regression.
- [x] Run fresh full qualification on the repaired sources, including normal/static
  and leak-enabled independent-TAP cases, 12 C++ quality checks and both scale sets.
- [ ] Record all six dispositions, commit the reviewed repair and run task-done.

## Branch completion

- [x] One fresh independent whole-branch review completed on 084a827; regrade all six findings for one Critical/Important repair pass, retain 44 inherited Minors. No repaired-code re-review.
- [ ] Merge, verify merged result, push, detach reused worktree, delete only owned merged branch and archive only this workspace. Expected: clean aligned master, 117 stashes preserved; continue remaining inventory.
