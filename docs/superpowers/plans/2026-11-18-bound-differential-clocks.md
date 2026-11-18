# Bound Differential Clock Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Replace blanket bound differential-input rejection with a conservative
complementarity proof for the existing relative clock-only backend.
**Architecture:** Existing SystemClocks route cache, iterative dependency walk,
phase-origin/parity metadata, existing scheduler and replay APIs.
**Spec:** docs/superpowers/specs/2026-11-18-bound-differential-clocks-design.md

## Global Constraints

Parent master5187a3c; task/bound-differential-clocks. Only clock source/header,
existing clock CLI/native scheduler tests, generator/model guides, this plan
and its spec. No new dependency or public command/API. Retain raw coverage
thresholds and prior evidence. User authorizes merge/direct-parent cleanup;
push verified master to its configured upstream (user authorized 2026-11-18). Final task verification is python3 /tmp/bound-differential-verify.py.

## Review Focus

- Equal frequency from different phase origins must not establish complementarity.
- All explicit bindings along either dependency branch must be validated, even
  with cached shared nodes, mux changes and historical overrides.
- Failed checks and cycles must not cache a false valid target or mutate hardware,
  reservations or published files. Recovery must work after invalidation.
- Existing frequency arithmetic, snapshot lifetime and clock-only fanout/held
  constraints must remain intact. No recursive-depth or quadratic shared-DAG walk.

### Task 128: Qualify explicitly bound differential clock inputs

**Files:** src/pattern/pattern_clock.{cc,h};
tests/src/pattern/test_system_clocks.py; tests/src/pattern_scheduler_test.cc;
docs/pattern-generation.md; docs/pattern-model-coverage.md; this plan and spec.
**Interfaces:** private SystemClocks::Route/Resolve/Source; public Check/Wait
and native TestDifferentialClockDurations remain compatible.

- [x] Read exact callers and references, record baseline and brief. Add CLI/native
      positive and negative cases first. Expected genuine RED at blanket guard.
- [x] Extend private route proof and iterative resolution. Expected focused GREEN,
      including bounds/cycles/snapshots/rollback, without relaxing unsafe fanout.
- [x] Run changed C++ format/tidy/C++17, dynamic/static clock and native tests,
      leak SAN and representative performance. Expected all relevant gates0.
- [ ] Run full make check and fresh raw coverage serially; qualify current remote
      compiler and generator suites; archive/install checks. Expected normal0;
      report actual strict coverage result without exclusions/waivers.
- [ ] Record evidence and limits, candidate commit, one independent review and
      necessary fix pass. Final verification, merge master, delete branch,
      task-done/archive ledger and sync index. Expected clean master; push verified master to its configured upstream (user authorized 2026-11-18).

Pre-flight: one task; no shared task interfaces. Source-only metadata remains
private; all Resolve/Source callers have been inspected. No RTL changes.
