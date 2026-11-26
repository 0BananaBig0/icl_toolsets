# Oscillator Default Restoration Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Restore declared ICL oscillators with empty native clock overrides.

**Architecture:** Reuse the existing versioned oscillator override path and exact
period renderer; keep ordinary clock lowering and validation boundaries.

**Tech Stack:** Existing C++17, Z3, Tcl and Python unittest; no new dependencies.

**Spec:** docs/superpowers/specs/2026-11-27-oscillator-default-restore-design.md

## Global Constraints

Read the actual intrinsic ICL period, never a previous override or guessed period.
Preserve checked versions, source-less/factor-only/cycle/fanout/differential guards,
atomic failures, C++17, existing resource limits and immutable procedure bodies.
Retain item1 exclusion,117 stashes and38 inherited deferred Minors.

## Review Focus

- Restoring an oscillator reads its own declared period after arbitrary prior source or period definitions.
- Old checked descendants and separately restored vector bits retain their original versioned roots and ratios.
- Default lowering without an explicit period operation remains equivalent to original and associated-flat replay.
- Invalid source-less/factor-only/differential/hardware operations cannot commit definitions or publish artifacts.
- Canonical vendor references use explicit periods; raw omission rules and physical elapsed timing remain unqualified.

### Task 1: restore intrinsic oscillator defaults

**Files:** Modify src/pattern/pattern_clock.cc,
tests/src/pattern_scheduler_test.cc,
tests/src/pattern/test_clock_period_overrides.py and
tests/src/pattern/test_system_clocks.py;
update docs/pattern-generation.md and docs/remaining-work.md.

**Interfaces:** Consume SystemClocks::Override(const Operation&, z3::context&,
Operation*) and NetlistNode::clock_period. Produce the existing API and versioned
Definition with oscillator=true for an empty intrinsic-period override. When
normalization is requested, produce {target,-period,declaredMagnitude,unit}.

- [x] Maintain tests for ordinary source replacement followed by default restore,
  period-bearing replacement/restore, unit spellings, vector procedures/reset,
  independently checked descendants and a nine-point replay matrix. Keep only
  factor-only cases in the former oscillator empty/factor rejection loop.
  Expected: new positive cases fail on the qualified parent.
- [x] Extend native clock snapshots with exact old/new root and cycle assertions,
  restored normalized operands and failed-normalization sentinel preservation.
  Build/run --clock-snapshots before production edits. Expected: restore fails.
- [x] Route validated empty intrinsic-period overrides through the existing
  oscillator-definition path and declared period renderer. Expected: focused
  native and both generator modules pass without weakening other guards.
- [x] Run focused period/serialization/legacy/frontend/readback/reset checks.
  Expected: all exit0 and all five replay forms agree.
- [x] Measure all nine specified workloads and matched ordinary controls three
  times with artifact capture, repeat equality and parent/current identities.
  Run bounded explicit-period vendor reference/generated pairs. Expected:
  unchanged10s/512MiB budgets and literal expected operations pass.
- [x] Freeze actual inputs, run full project, affected leak-enabled sanitizers,
  changed-code C++ quality, corpus and source packaging. Expected: all exit0;
  record actual suite/test/skip counts and source/product hashes.
- [x] Update usage/evidence documentation, read the complete final selected diff,
  commit with grouped messages and run task-done using the evidence checker.
  Expected: checker confirms every current source/product/measurement identity.

## Branch completion

- [x] One fresh independent whole-branch review; grade every finding/declined
  area, one Critical/Important test-first repair pass if necessary; defer Minors.
- [ ] Merge into parent master, verify merged inputs/products and focused tests,
  push, detach reused worktree, safely delete only the task branch and archive
  only this plan's own ledger. Expected: clean aligned master/origin,117 stashes.
