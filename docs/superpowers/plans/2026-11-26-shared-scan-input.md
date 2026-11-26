# Shared Scan Input Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Retarget compatible broadcast loads through one physical serial input.

**Architecture:** Prove fixed equal nonzero disjoint inferred lanes, then equate
their symbolic input streams. Reuse concrete capture/shift/update and histories.

**Tech Stack:** C++17, existing Z3/Tcl, Python unittest.

**Spec:** docs/superpowers/specs/2026-11-26-shared-scan-input-design.md

## Global Constraints

- No new dependencies, resource-limit increases, transport or timing claims.
- All lanes participate in the same capture/shift/update.
- Keep storage overlap, intermediate taps, unequal/empty shared lanes, gated or
  switched shared routes, and named/client/TAP shared-input cases guarded.
- New tests live under tests/src/pattern/.

## Review Focus

- Opposite input/output polarities must use physical input agreement (test inversions).
- Equal total widths with different register boundaries must work (test split lanes).
- Partial writes may not change unrequested banks (test omitted-load preservation).
- Two input groups must stay independent (test grouped vector inputs).
- Failure after a successful group must retain the old output (test contradictions).

### Task 1: Coupled fixed lanes

**Files:** src/pattern/pattern_scheduler.cc; tests/src/pattern/test_shared_scan_inputs.py;
tests/src/pattern_scheduler_test.cc; Makefile; docs/remaining-work.md; docs/pattern-generation.md.

**Interfaces:** Existing inferred root discovery, Frame.loads and ScanStep lanes;
no public API changes. Produce equal physical input streams and unchanged actual
per-register modeled observations.

- [x] Write test_shared_scan_inputs.py: direct capture/status prefix replay,
  inversions, split lanes, partial/omitted loads, vector groups and contradictory
  writes. Add guarded topology/publication checks and representative scale. Add native literal-stream, independent scalar bit-shift, capture-order, copy and ownership checks in pattern_scheduler_test.cc.
- [x] Run the direct shared-input test against build/icl-pattern-generation.
  Expected: FAIL with existing overlapping-event scheduling diagnostic.
- [x] Implement bounded constructor proof and per-frame stream equalities in
  pattern_scheduler.cc; check concrete replay stream consistency.
- [x] Build and run new tests and existing shared outputs, simultaneous chains,
  client and ordinary-read-scope tests. Expected: all pass.
- [x] Run make check with the existing pinned Z3/static Tcl; run all 15 native
  sanitizer executables and affected CLI modules; run affected C++ quality and
  source-distribution checks. Expected: exit 0; disclose optional reference skips.
- [x] Measure nine lane/width combinations and four replay forms each, under
  the 10-second bound; compare existing disjoint inputs against master.
  Expected: exact replay, unchanged control output and timing recorded.
- [x] Record actual evidence and remaining bounds in the user docs, commit the
  selected diff using grouped messages, and run task-done with `python3 /tmp/shared-scan-input-check.py` (Expected: QUALIFIED, exit 0).

## Branch completion

- [x] Obtain one fresh whole-branch review. Grade every finding and declined area;
  fix Critical/Important once with RED→GREEN plus a green full suite; defer Minors.
- [x] Merge into recorded parent master, verify merged source/product identity
  and focused checks, push and delete the owned branch. Archive only this ledger.

Review repair: selector and disabled data-consumer omitted loads reproduce the
Important failure before implementation. Scope shared control permissions to
current operation dependencies, retaining real 4/256-bit controllers and their
unrelated bits. Four new CLI methods and native rollback/explicit-write checks
pass, then the full 40-command qualification passes with 221 suite runs and
3,413 reported tests (five optional skips). One negative-fixture precision Minor
is deferred; the single Important is resolved in one pass without rereview.
