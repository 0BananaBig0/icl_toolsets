# Modeled DataRegister Readback Implementation Plan

> For agentic workers: REQUIRED SUB-SKILL superpowers:executing-plans.
> Preserve the established inline method; no implementation agents.
> One fresh independent final reviewer on the most capable model.

Goal: actual DataRegister observations through PIO, with source-ordered manual
history and exact counts under --model_readback.
Architecture: reuse selected concrete replay frames and sparse lazy deltas;
suppress older undelivered planes superseded by manual history.
Tech Stack: C++17, existing Tcl/Z3/concrete model; no new dependencies.
Spec: docs/superpowers/specs/2026-11-22-modeled-data-readback-design.md

## Global Constraints
Parent master007a7a7; isolated .worktrees/bsdl-defaults on task branch
codex/modeled-data-readback. Preserve default semantics, current limits,
once-only Tcl and immutable procedures. No ScanRegister/TAP observation,
clock/raw/merge/callback execution or physical/vendor equivalence widening.
Archive only own ledger; preserve117 stashes/sibling workspaces.
Standing proceed instruction covers continued inventory6 work.

## Review Focus
- Earlier read differs from final storage: compare actual emitted events.
- Lazy manual planes do not leak older undelivered observations.
- Partial/inverted history changes its own plane, never counter/device state.
- Unknown/clear/reset semantics cannot invent known data or zero status.
- Legacy notes migrate once; no-query/default paths remain compatible.

### Task1: DataRegister observations and manual planes

Files:
- Modify src/pattern/pattern_readback.cc/.h: eligibility, suppression, note.
- Modify src/pattern/pattern_scheduler.h: PIO data observation contract.
- Modify src/pattern/icl_pattern_generation_main.cc: old/current note filtering.
- Modify tests/src/pattern_readback_test.cc: emitted-event/state native oracle.
- Modify tests/src/pattern/test_modeled_readback.py: current note/scan guards.
- Create tests/src/pattern/test_modeled_data_readback.py; wire Makefile.

Interfaces: existing Runtime::Readback/ReadbackUpdates '?' plane sentinels,
QueuedAccesses::ReadModel and ScanScheduler::PioObservation/Apply stay in place.
Same factory/flag API; no new state engine/history format.

- [x] Add native/CLI REDs: assumed9/expected0 returns9/status2; read9/write6 in
      one apply observes9/leaves6; delayed manual read15 leaves mismatch1001/
      count2. Cover partial/inverted/manual-mismatch history.
- [x] Run python3 tests/src/pattern/test_modeled_data_readback.py build/icl-pattern-generation;
      retain actual semantic failures separately from API/build failures.
- [x] Allow DataRegister, exclude other register kinds/TAPs. In history-only
      Process(iState), validate physical bits and suppress only the matching
      plane of existing pending deltas using '?'.
- [x] Current mode note becomes modeled PIO/DataRegister readback; no live
      device. Filter former combinational note too; copy only when needed.
- [x] Cover unknown/count/clear, overlaps/X, queued/control state, reset/
      reservations, DAG/feedback, prefix/files/once-only Tcl, local-reset
      procedure bytes and unsupported boundaries. Prove three-form replay and
      legacy migration. Native oracle never assigns expected/goal markers.
- [x] Focused new/existing CLI, native readback/runtime/scheduler callers and
      changed-code format/tidy/C++17 pass; commit and ledger.

### Task2: Scale, qualification, documentation and integration

- [ ] Measure parent/current defaults and current unused/queried mode at8/16
      applies x16bits plus larger/sparse registers;3 repeats x3 forms. Save
      command/input/environment/hash/time/RSS, control equality and failures.
      Preserve limits/deadlines; no unlike-workload speedup claims.
- [ ] Freeze code/tests; all14 native/all14 leak-enabled native SAN suites,
      full dynamic/static CLI, focused/main SAN, C++ quality and packaging.
      Record actual suite/test counts and input/product identities.
- [ ] Execute guide example as author walkthrough; update README/current
      command table/model/tracker with DataRegister support and remaining limits.
- [ ] One fresh independent final review; regrade real effects/declined items.
      One verified fix pass with appropriate fresh qualification; no rereview.
      Retain pre-fix/partial receipts without claiming complete passes.
- [ ] Merge master, verify qualified root hashes/focused callers, push/confirm
      origin, delete completed branch, detach reused worktree and archive only
      own ledger. Retain117 stashes. Coverage1 excluded; continue inventory.
