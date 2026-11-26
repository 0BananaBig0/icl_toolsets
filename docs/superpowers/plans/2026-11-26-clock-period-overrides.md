# Explicit Clock Period Overrides Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Retarget explicit oscillator-period models without losing clock snapshots.

**Architecture:** Shared period-literal validation, versioned oscillator roots and
retained clock statements only for period-bearing programs. Reuse existing route
safety, actual PIO observations and recursive local-reset rendering.

**Tech Stack:** C++17, existing Tcl/Z3, Python unittest; no new dependencies.

**Spec:** docs/superpowers/specs/2026-11-26-clock-period-overrides-design.md

## Global Constraints

- Programs without period overrides retain existing normalization and omissions.
- Mixed source/ratio overrides in period-bearing programs require all three options.
- Retain physical/raw/opaque, differential, stateful-time and unsafe-clock guards.
- No resource-limit increase, inferred period or hardware/timing-signoff claim.
- New test scripts live under tests/src/pattern/.

## Review Focus

- Two logical targets checked across a root period change retain distinct snapshots (literal clock-program test).
- Flat-origin and nonzero vector indices retain correct target/source names (hierarchical/vector replay).
- Dynamic Tcl period values and separate units share frontend validation (parser and procedure cases).
- Mixed old partial overrides cannot silently change their omission meaning (explicit mix and rejection tests).
- Pending writes, reset rendering and unrelated modeled PIO retain ordering and output safety (prefix/invalid cases).

### Task 1: Period-aware clock program replay

**Files:** src/pdl/pdl_document.h, src/pdl/pdl_document.cc;
src/pattern/pattern_clock.h, src/pattern/pattern_clock.cc;
src/pattern/icl_pattern_generation_main.cc; tests/src/pattern_scheduler_test.cc;
tests/src/pattern/test_clock_period_overrides.py; tests/src/test_pdl_tools.py;
Makefile; docs/pattern-generation.md; docs/remaining-work.md.

**Interfaces:** Add shared pdl::NormalizeClockPeriod(value, unit="") returning a
positive compact time token. Extend SystemClocks::Override with an optional
Operation* normalized output, default nullptr; add canonical TargetName(text).
Add a period-root marker to Definition, retaining checked version lookup.
Append a retain-clock-program flag to private Generate and forward it through
its recursive local-reset call. No public CLI switch.

- [x] Add failing CLI/default/unit, checked-target snapshot and hierarchy/vector
  tests in test_clock_period_overrides.py, reusing SystemClocks replay helpers;
  add frontend option cases to the existing PDL test module.
- [x] Run the basic period case on the parent executable. Expected: FAIL with
  invalid/incomplete iClockOverride option; retain old output artifacts.
- [x] Share period-literal validation between frontend and clock backend. Accept
  only the exclusive period form and validate resolved Tcl values consistently.
- [x] Extend versioned oscillator-root recognition and optional normalized output,
  keeping existing source/ratio defaults, safety/cycle checks and invalid-update
  behavior. Add native root/version/copy/rollback assertions.
- [x] Detect a period-bearing executed program once, retain canonical clock
  statements/logical waits and pass this policy into recursive rendering;
  reject partial mixed source/ratio forms. Keep ordinary lowering unchanged.
- [x] Complete independent modeled PIO, guarded clock histories, procedure/reset, physical/raw/unsafe, invalid-value
  and failed-artifact tests. Run new normal/static tests and existing clocks,
  PDL frontend, readback and local-reset checks. Expected: all pass.
- [x] Run full make check, all native and affected CLI ASAN/UBSAN checks,
  changed C++ format/tidy/C++17 checks and packaging. Expected: exit 0; disclose skips.
- [x] Measure the nine 2/8/32-bit by 8/128/1024-update programs, five forms each,
  under 10 seconds/512 MiB; compare matched ordinary parent/current programs.
  Expected: exact canonical replay/control bytes with recorded time/RSS/hashes.
- [ ] Record actual evidence, read the final selected diff and commit with grouped
  messages. Run task-done with python3 /tmp/clock-period-overrides-check.py.
  Expected: QUALIFIED, exit 0 with final input/product identity.

## Branch completion

- [ ] One fresh whole-branch review, grading every finding and declined area;
  one Critical/Important test-first repair pass plus a green full suite; defer Minors.
- [ ] Merge into parent master, verify merged identity and focused checks,
  push and delete only this owned branch; archive only this task ledger.
