# Fixed Scan Fanout Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Retarget compatible fixed fanout with unequal and overlapping lanes.

**Architecture:** Prove fixed shared-input groups, constrain shorter load prefixes,
and broadcast the longest concrete stream. Reuse unique register events/history.

**Tech Stack:** C++17, existing Z3/Tcl, Python unittest.

**Spec:** docs/superpowers/specs/2026-11-26-fixed-scan-fanout-design.md

## Global Constraints

- No new dependencies, resource limits or hardware/timing claims.
- Preserve existing equal disjoint groups and scoped controller permissions.
- Keep intermediate register taps, switched/gated sharing and named/client/TAP sharing guarded.
- New geometries require invariant inactive constant/global or root ResetPort resets.
- New test scripts live under tests/src/pattern/.

## Review Focus

- Shared storage must clock once and report each explicit logical read once (native oracle and partial read/status tests).
- Unequal words require MSB-prefix agreement, with physical inversion (unequal polarity and fork tests).
- Wire observations must not fabricate zero input prefixes (wire capture test).
- Unrelated controller bits must retain omitted loads (reuse shared-control regressions plus fanout omitted-load tests).
- Active, unknown or serial-dependent resets must not publish a fabricated result (reset boundary/sentinel tests).

### Task 1: Fixed fanout retargeting

**Files:** src/pattern/pattern_scheduler.cc; tests/src/pattern/test_fixed_scan_fanout.py;
tests/src/pattern/test_shared_scan_inputs.py; tests/src/pattern/test_shared_scan_outputs.py;
tests/src/pattern_scheduler_test.cc;
Makefile; docs/pattern-generation.md; docs/remaining-work.md.

**Interfaces:** Existing CheckIndependentChains, inferred root grouping,
shared_input_paths_, Frame.loads and Replay; no public API changes.

- [x] Add test_fixed_scan_fanout.py reusing existing replay helpers. Pin literal
  actual captures/status and prefix continuation for shared A4/B4, unequal A4/B2,
  fork A4->B2/C4, wire observers, inversions and partial read histories.
- [x] Run new tests against the parent binary. Expected: legacy overlapping-event
  rejection for compatible inputs; retain sentinel evidence for incompatible inputs.
- [x] Extend inferred fixed-group proof; retain strict independent-chain checks
  for named/client/TAP callers. Track only new geometry reset checks and prove
  reset references stable through constants/global/root ResetPort forwarding.
- [x] Replace equal-word coupling with scan-conditional MSB-prefix coupling;
  canonicalize each physical input's concrete serial stream before global padding.
  Keep actual output comparisons and unique register clock events.
- [x] Update old unsupported-geometry fixtures now covered by positive tests;
  retain valid gated/intermediate cases and check their specific diagnostics.
- [x] Add native literal eight-tick fork oracle, emitted serial stream, copy,
  ownership and rollback checks; reset and omitted-load CLI boundaries.
- [x] Build and run focused normal/static suites. Expected: all pass, including
  existing shared inputs/outputs, simultaneous chains/clients and ordinary reads.
- [x] Run full make check, affected leak-enabled ASAN/UBSAN, changed C++ quality,
  corpus and source packaging. Expected: exit 0; disclose optional skips.
- [x] Measure 2/8/32 observations at 4/64/256 bits, four replay forms, under
  10 seconds and 512 MiB; compare equivalent disjoint parent/current controls.
  Expected: literal replay, exact control bytes, recorded time/RSS and binary hashes.
- [x] Record actual evidence, read final selected diff, commit with grouped
  message, then task-done using python3 /tmp/fixed-scan-fanout-check.py.
  Expected: QUALIFIED, exit 0 with input/product identity verified.

## Branch completion

- [ ] One fresh whole-branch review; grade every finding and declined area.
  One Critical/Important RED→GREEN repair pass with green whole suite; defer Minors.
- [ ] Merge into recorded parent master, verify merged source/products and
  focused tests, push and delete the owned branch. Archive only this task ledger.
