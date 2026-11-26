# Independent TAP phases implementation plan

> **For agentic workers:** Use superpowers:executing-plans inline.

**Goal:** Meet mixed IR/DR simultaneous writes across independently controlled TAPs.
**Architecture:** Per-TAP frame phases, reused native ownership and edge replay;
one shared symbolic choice for existing common-phase schedules.
**Tech Stack:** C++17,Z3,Tcl,existing Python/native regression and Tessent harnesses.
**Spec:** docs/superpowers/specs/2026-11-26-independent-tap-phases-design.md

## Global Constraints

- Base pushed master ee890c7;reused .worktrees/bsdl-defaults;parent master.
- No new dependency, resource-limit increase, transport or timing invention.
- Keep excluded1,117stashes and sibling task workspaces unchanged.
- New tests under tests/ with source-relative paths;existing tests stay in place.
- One fresh final reviewer;one blocker fix pass;no rereview.

## Review Focus

- Shared TMS pins cannot execute different IR/DR paths on the same edge.
- Unrequested lanes and held registers must survive the extra DR idle edge.
- Actual captures/status must follow the executed route and count once.
- Same-TAP impossible accesses and ambiguous reset instructions stay rejected.
- Existing common-phase schedules must retain behavior and bounded solver cost.

### Task 1: Model and replay distinct TAP phases

**Files:** src/pattern/pattern_scheduler.cc/.h,pattern_tap_execution.h;
src/pattern/icl_pattern_generation_main.cc (help text only);
tests/src/pattern/test_independent_tap_phases.py,tests/src/pattern_state_test.cc;
Makefile and focused fixture/certificates under tests/cases/ours/pattern_generation/
independent_tap_phases and tests/golden/tessent_roundtrip when qualified;
docs/pattern-generation.md,pattern-model-coverage.md,remaining-work.md.
**Interfaces:** ScanStep instruction metadata identifies selected IR TAPs;
existing Apply/Cycle/Replay entry points and generated PDL remain compatible.

- [x] Step1: Add a maintained regression for tap0.INSTRUCTION and c1.value in
  one -together group after setting tap1's instruction. Expected: current binary
  rejects mixed IR/DR and leaves a previous output intact;test expects success.
- [x] Step2: Implement per-TAP symbolic choices and owner-specific events, shared
  TMS equality, and native IR1100/DR0100 replay;retain common choices otherwise.
  Expected: first regression passes with one shared update and real replay.
- [x] Step3: Add reverse orientation, unequal widths, AccessTogether, actual
  observations/status, held/untargeted lanes, local resets and negative shared-TMS/
  same-TAP cases. Add native per-edge proof. Expected: source/flat/generated
  outputs and actual state agree;failed programs preserve previous output.
- [x] Step4: Measure2/4/8/16TAP and8/64/256bit bounded cases against the frozen
  parent for unchanged controls;qualify normal/static/SAN,full project checks,
  audits,style and package. Expected: no unsupported claims or silent skips.
- [x] Step5: Reconcile current guidance and open inventory;read finalized diff,
  commit and run completion contract. Expected: evidence matches changed scope.

Implementation verification: python3 /tmp/independent-tap-phases-postreview-check.py

## Branch review and integration

- [x] Obtain one final review;grade every finding/declined area;fix blockers once.
- [ ] Merge,check merged result,push,detach/delete owned branch,archive own ledger.

Self-review: one scheduler change with its consumers and proof. Model metadata
must not become an assumed hardware state. Wider original item2 remains open.
