# Readback factory compatibility implementation plan

> **For agentic workers:** Use superpowers:executing-plans inline.

**Goal:** Restore the original three-argument factory function-pointer API.
**Architecture:** One forwarding overload alongside the existing overloads;
retain the four-argument model owner unchanged.
**Tech Stack:** C++17, GNU make, native readback tests.
**Spec:** docs/superpowers/specs/2026-11-26-readback-factory-compatibility-design.md

## Global Constraints

- Base pushed master de119ef; reuse .worktrees/bsdl-defaults.
- Preserve117stashes and other task ledgers; no dependency or behavioral changes.
- Modify existing tests in place; no new test-script location needed.
- One fresh final reviewer; one blocker repair pass, no rereview.

## Review Focus

- Three-argument calls must not become ambiguous after adding the overload.
- Existing one/two/four-argument factories must continue to compile and link.
- Status preview must share model state with returned readback, retaining deltas.
- Null status output and absent history output must remain valid.
- No changed netlist lifetime, physical flag or execution-state ownership.

### Task 1: Restore the original factory overload

**Files:** src/pattern/pattern_readback.h, src/pattern/pattern_readback.cc,
tests/src/pattern_readback_test.cc; this plan/spec and docs/remaining-work.md.
**Interfaces:** Runtime::Readback (*)(const Netlist&,bool,Runtime::StatusReadback*)
forwards to the existing four-argument factory with nullptr history output.

- [x] Step1: In TestStatusPreviewRetainsUnpublishedHistory, assign the factory
  to the exact three-argument function-pointer type and invoke that pointer.
  Compile the test; Expected: no matching three-argument overload diagnostic.
- [x] Step2: Declare/define the explicit three-argument forwarding overload;
  remove the fourth argument default. Expected: the same test compiles/links.
- [x] Step3: Run pattern-readback-check, compile/link typed one/two/four-argument
  pointers with null output cases, then make check using the installed compiler
  and existing pinned Z3/Tcl dependencies. Expected: all checks pass; unchanged
  status-preview counts4then5 and once-only observation deltas.
  Verification amendment: the full command exposed five inherited forward
  child-history golden mismatches, reproduced with the qualified baseline.
  Prepare those exact five cases, execute licensed Tessent on the authorized
  lianghuaxiao account in an owned /tmp directory, record into fresh local
  certificate destinations, and replace only the five validated snapshots.
  Expected: existing failed golden target passes; no policy weakening or manual
  certificate edits. Preserve the full run's passing unaffected checks; disclose
  its initial exit2 rather than claiming a fresh all-green full-command run.
- [x] Step4: Record the resolved inherited finding31to30, inspect the complete
  selected diff, and commit using the grouped policy. Expected: surgical change.

Implementation verification: python3 /tmp/readback-factory-check.py

## Branch integration

- [x] Obtain one final review; grade every finding/declined area and fix blockers
  once with RED/GREEN evidence; keep any new Minors deferred.
- [x] Merge and run merged native/API checks, push, detach/delete owned branch,
  archive own ledger and verify clean pushed master with117stashes.

Self-review: one task; existing overload consumers remain unchanged. The native
test uses real model state rather than mocks or declaration-text assertions.

Final review amendment: fresh full check passes every prerequisite but exposes
an omitted pattern_readback.cc source in tests/coverage.py. Its existing test
is RED then GREEN after the one-entry inventory fix. No excluded thresholds
change. Rerun the entire main recipe with completed prerequisites retained;
expected exit0, actual exit0. All project checks are now covered;the initial
full-command failures remain explicit. One new Minor is deferred.
