# Equivalent TCK snapshots implementation plan

> Execute inline with superpowers:executing-plans under existing authorization,
> after Task48 integration is verified and committed.

**Goal:** Extend common-phase TAP scheduling to distinct top-level TCK ports
that represent the same logical test clock, preserving joint events and state.

**Architecture:** Add an explicit synchronized-clock snapshot entry point to
TapExecution and reuse its existing single-edge implementation. Stage all old
clock levels before sampling child clocks, then all new levels before computing
controller actions and effective controls. Keep one NetworkState::Clock per
logical edge. The scheduler drives all top-level TCK roots together and assigns
all corresponding symbolic clock pins at each phase.

**Basis:** IEEE1687-2014 §6.4.6.17(a), printed p101, requires TCKPorts in one
module to be equivalent and have the same polarity; its explanation describes
one logical test clock even when physical clock entries serve different power
domains. §6.4.6.18 gives the corresponding ToTCKPort equivalence. Cached source:
build/combined-audit/ieee1687.txt lines6335–6375. This does not authorize arbitrary
independent phase/frequency behavior or invent clock-gating enables.

**Spec:** [Approved generator design](../specs/2026-10-25-icl-pattern-generation-design.md).
Shared storage, controls and resets stay in one state model. Task48's remaining
independent TAP phases, shared serial storage and implicit reset ownership are
separate required work.

## Scope and choices

The current scheduler rejects different TAP clock-node IDs. Calling the existing
single-clock Edge once per root would execute shared state sequentially and could
sample controls after another TAP has already changed them. Instead batch the
clock levels and reuse the existing staged actions/control publication. Preserve
the native single-clock API for callers reporting individual observed edges.

The scheduler should collect every unconnected top-level TCKPort, including a
root explicitly connected to instrument clocks but not named by a TAP's BSDL.
The equivalent-clock rule covers those ports too. Keep actual child-clock edge
qualification and inverted-phase diagnostics. Do not replace a disabled/unknown
gated clock with a manufactured edge. Validate the selected root set before
mutating a batch; one batch advances each affected controller once.

Reuse scalar TCK declarations and existing frame phase metadata. BSDL maximum
clock specifications are not actual waveform periods; no timing/frequency
qualification or new timing model is claimed. Standalone client clock batching
already exists and should remain compatible. No dependencies or state partitions.

Files: src/pattern/pattern_tap_execution.h/.cc; src/pattern/pattern_scheduler.cc;
CLI help and model/guide records; existing native state/scheduler tests and
 tests/src/pattern/test_multiple_tap_scheduling.py. Reuse Task48 fixtures and
source/flat helpers. No new vendor certificate is implied by earlier loader
failures with distinct clocks.

## Implementation and checks

- [x] Independently review clock scope, old/new snapshot order, single-clock
  compatibility and all symbolic clock assignments before implementation.
- [x] Add failing CLI cases for distinct TAP TCK roots and an additional root
  driving a child clock. Use unequal IR/DR widths, common capture/update,
  source/flat/generated fixed points, reset and idle replay.
- [x] Add native cases that forward control/data between clock-root domains,
  reversing root/TAP names and checking old-state sampling and exactly one
  joint update. Cover a subset native edge, invalid batch input without mutation,
  known-disabled/unknown gates and retained inverted-child-clock diagnostics.
- [x] Reuse the existing Edge pipeline for synchronized root sets. Batch both
  clock levels, then all controller actions/outputs, one joint state clock and
  existing IR/reset settlement. Keep shared-register ownership diagnostics.
- [x] Replace only the same-clock-node scheduling guard; drive/assign every
  equivalent root in Cycle, EventPins, ControlEvents and their callers.
  Retain common TMS/IR-or-DR phases, explicit reset dependencies and budgets.
- [x] Verify reservations and failures preserve all state/output artifacts.
  Measure representative4/16 equivalent-root networks and existing wide/long
  native replay without claiming an unmeasured improvement.
- [x] Obtain independent source review, run focused and required full dynamic/
  static tests, leak-enabled sanitizers and style/naming. Update scope/evidence
  and commit only verified work.

Work remains proportional to the explicit root set, controllers and existing
control/state evaluation per edge; no repeated whole-network Edge per root.
Planning limits and solver history costs remain unchanged.

Independent design review confirms the scalar, unconnected top-level boundary.
Extra roots need not own a TAP if the batch affects at least one controller.
A singleton native call must not advance other controllers. Include a single-TAP
regression whose instrument clock comes from another equivalent root, and verify
invalid batches leave state unchanged. Implementation and independent source review are complete. Focused native and
all22 CLI methods pass. Leak-enabled native state/TAP/scheduler/flatten,
main33/TAP9/hidden4/functional20/end11/mixed15 checks pass; full integration
passes in build/equivalent-tck-final-full.log/.exit0 (1047.081s). Both optimized scale methods are excluded from the20-method
functional sanitizer run, preserving Task48’s known instrumented solver limit.

Final integration:40 dynamic/static generator suites,51 directional/seven legacy
goldens,2523 ICL334/119 parser/scanner rules and editor6 in178.636s pass. Five
optional ANTLR tests skip because ICL_REF_DIR is unset. Final sanitizer evidence
is build/equivalent-tck-final-sanitizer-*.log/.exit0; functional20 takes30.660s.
Both optimized scale methods remain excluded from that functional sanitizer run.
Focused native/all22CLI, naming, formatting and independent source/evidence
review pass. Strict coverage, vendor certification and Rocky8 qualification have
not been refreshed. Root six product/test files match the reviewed worktree.
