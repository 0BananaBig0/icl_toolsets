# Physical Replay Diagnostics Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Complete item13 with concrete errors while preserving native fallback.
**Architecture:** Retain errors at TryNativeTapScan boundaries and enrich existing comparisons.
**Tech Stack:** Existing C++17 scheduler, Python unittest and native tests.
**Spec:** docs/superpowers/specs/2026-11-20-physical-replay-diagnostics-design.md

## Global Constraints
- Parent master80fbfb9; branch codex/physical-replay-diagnostics.
- No scheduling/state/API/dependency changes; skip coverage closure item1.
- No extra signal reads for masked comparisons.
- Tests remain under existing project tests paths.

## Review Focus
- Native ordinary probing must keep falling back after the same failures.
- Failed physical comparisons must preserve state and published files.
- Unknown fill and closed targets need their names without setup transactions.
- Reservations must retain the original failure rather than a generic fallback.
- Successful original/flat/generated output must remain byte-identical.

### Task 1: Preserve physical replay failure causes
**Files:** src/pattern/pattern_scheduler.cc; tests/src/pattern_scheduler_test.cc; tests/src/pattern/test_argument_reset_sources.py; current guide/tracker.
**Interfaces:** unchanged TryNativeTapScan/ScanScheduler APIs.
- [x] Add RED assertions for compare bit1/expected1/observed0, retained unknown fill, named closed route with preserved artifacts, native resource cause and rollback.
- [x] Preserve explicit-mode exceptions and qualify direct guard failures; enrich observed comparisons only on real mismatch. Expected all focused tests GREEN and ordinary closed-route preparation succeeds.
- [x] Run native/generator dynamic/static, leak-enabled sanitizer, style/C++17 and relevant package checks. Expected0; coverage closure excluded.
- [ ] Update item13 evidence, commit, task-done, fresh whole-branch review, necessary fix pass; merge/push/delete branch.
**Verification:** python3 tests/src/pattern/test_argument_reset_sources.py build/icl-pattern-generation
