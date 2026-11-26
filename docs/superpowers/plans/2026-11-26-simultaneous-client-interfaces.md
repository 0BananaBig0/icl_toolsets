# Simultaneous client interfaces implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans inline, task-by-task.

**Goal:** Execute simultaneous register accesses across eligible standalone clients.
**Architecture:** Reuse static lane proof and ownership, per-frame selection
constraints and the shared symbolic/concrete event model;preserve ordinary mode.
**Tech Stack:** C++17,Z3,Tcl,existing Python/native tests and vendor harness.
**Spec:** docs/superpowers/specs/2026-11-26-simultaneous-client-interfaces-design.md

## Global Constraints

- Parent pushed master ba94302aed7c110dc0181231efc7b1888c428e0c.
- Reuse .worktrees/bsdl-defaults;preserve117stashes and sibling workspaces.
- No new dependencies, public scheduling API, invented timing or resource increases.
- Only disjoint lanes with one shared root TCK and shared phase-control pins.
- Retain ordinary one-client and shared-storage switching behavior.
- New maintained tests under tests/ with source-relative paths.
- One final reviewer;one Critical/Important repair pass;defer Minors;no rereview.

## Review Focus

- Shared serial resources or coupled selectors must never qualify as independent clients.
- A simultaneous frame must not activate an unrelated held client's controls.
- Selection-driven resets and hidden capture must follow actual native events.
- Read aliases with reset-held members must retain their simultaneous capture.
- Ordinary requests and shared-storage switching must preserve prior behavior.

### Task 1: Select eligible client groups and replay their shared events

**Files:** Modify src/pattern/pattern_scheduler.cc (constructor lane proof,
CheckIndependentChains,Frame,SelectScan,ConstrainScanControls,Solve/read grouping);
src/pattern/icl_pattern_generation_main.cc (current help constraints);
tests/src/pattern/test_multiple_client_interfaces.py (replace obsolete rejection);
tests/src/pattern/test_multiple_client_chain_groups.py (replace corresponding group rejection);
tests/src/pattern/test_simultaneous_scan_chains.py (optional CLI flags in reused helper);
tests/src/pattern_state_test.cc (native shared-event proof);Makefile;
docs/pattern-generation.md,pattern-model-coverage.md,remaining-work.md.
Create tests/src/pattern/test_simultaneous_client_interfaces.py. Add vendor
fixture/certificates only when genuine tool execution qualifies them.
**Interfaces:** Internal static proof may return register ownership through an
optional output map;Frame carries eligible client IDs. Public Apply/ScanStep and
generated PDL command syntax remain compatible.

- [x] Step1: Write test_two_clients_update_together using existing network(),
  values13/6 and one -together group;require original/flat/generated equality.
  Run python3 tests/src/pattern/test_simultaneous_client_interfaces.py build/icl-pattern-generation.
  Expected: maintained RED on current one-client constraint.
- [x] Step2: Implement structural/control eligibility, register ownership and
  per-frame client/root counting using the existing helpers;retain one-client
  defaults. Run the focused test. Expected: GREEN with one shared update.
- [x] Step3: Add every acceptance case and Review Focus regression, actual
  native update proof, ordinary/shared-storage controls and publication safety.
  Run focused normal/static/native checks. Expected: exact replay/history/status.
- [x] Step4: Run three paired scale rounds and applicable vendor qualification;
  run full make check,15SANnative/focused suites,audits,style and packaging.
  Expected: evidence retains actual limits/skips and binds source/product hashes.
- [x] Step5: Reconcile all three current guidance records;read the selected
  patch,commit and run completion contract. Expected: checks and claims agree.

Implementation verification: python3 /tmp/simultaneous-client-interfaces-check.py

## Branch review and integration

- [ ] Obtain one fresh final review;grade every finding/declined area;fix blockers once.
- [ ] Merge,verify merged result,push,detach/delete owned branch,archive own ledger.

Self-review: one scheduler deliverable plus its proof and consumers. Static
separation/control eligibility is required before relaxing selection cardinality.
