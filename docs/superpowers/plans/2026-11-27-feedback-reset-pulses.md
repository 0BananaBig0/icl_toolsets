# Feedback Reset Pulses Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Preserve qualified external scan reset pulses during feedback settling.

**Architecture:** Extend the shared immutable data dependency plan with affected
reset sinks and consumer edges. Settle those sinks in simultaneous waves in
concrete and symbolic replay and carry their state/fill through both planners.

**Tech Stack:** Existing C++17, Z3, Tcl and Python unittest; no new dependencies.

**Spec:** docs/superpowers/specs/2026-11-27-feedback-reset-pulses-design.md

## Global Constraints

Preserve old acyclic evaluation, immutable procedure definitions, capture-before-
update, unknown information, controlling fill, ownership and atomic publication.
Keep32 data deltas/4M feedback-bit work/4096 data registers and all existing
dependency/reset/solver limits. No new clocks, elapsed-time engine or transport.
Only uniquely owned acyclic scan-reset closures without writable-data consumers
in the specified interface-free family are newly qualified. Other changing
external resets retain their guards; old masked/invariant cases remain valid.
All new regression scripts belong under repository-root tests/; maintain existing
tests in place. Preserve117 stashes and44 inherited Minors. Item1 stays excluded.

## Review Focus

- Parallel sinks update simultaneously; cascade snapshots expose real transient holds without inventing single-sink ordering.
- Reset targets and implicit OneHot control dependencies retain physical identity across repeated instances, vector bits and associated-flat names.
- Primary and mixed planners preserve affected scan storage and controlling fill through actual native replay, including capture-before-update and read-only groups.
- Unknown pulses, failed checks and nonconvergence preserve information, histories, copies and rollback without substituting expected data for observations.
- Shared data origins, reset cycles, writable-data consumers and explicit controller/clock contexts retain their old guards while masked cases remain valid.

### Task 1: preserve feedback-generated reset events

**Files:** Modify src/pattern/pattern_data.h/.cc, pattern_state.h/.cc and
pattern_scheduler.cc; maintain tests/src/pattern_state_test.cc,
tests/src/pattern_scheduler_test.cc, tests/src/pattern_readback_test.cc and
tests/src/pattern/test_data_register_feedback.py and its benchmark driver; update docs/pattern-generation.md
and docs/remaining-work.md. Reuse the existing Makefile feedback module wiring.

**Interfaces:**
- Extend DataRegisterGroup with vector<size_t> reset_targets and
  map<size_t,vector<size_t>> reset_consumers; empty targets retain the old guard.
- Extend DataRegisterPlan with set<size_t> feedback_reset_targets for the union.
- Add private NetworkState::ResetFeedback(const DataRegisterGroup&,size_t*,const Check&).
- Add scheduler ResetFill(size_t) const returning string, and
  SettleFeedbackResets(const DataRegisterGroup&,SymbolicNetwork*,FeedbackResult*,
  const z3::expr&,z3::optimize*,size_t*,const function<void(SymbolicNetwork&)>&).
  FeedbackResult remains Values state/fill, now including affected scan sinks.
- Seed qualified scan fills in SolvePrimaryWrites, publish sink entries from
  SettleDataFeedback in both callers, and validate/reset their concrete fill in
  exact primary replay and primary symbolic replay. Mixed replay already checks
  reset ScanRegister fill; reuse its known ResetValue overlay through ResetFill.

- [x] Replace the maintained primitive pulse rejection with actual value00 and
  victim0 assertions; keep the mask0 victim1 control. Add native parallel sinks,
  cascades, unknown reset joins/events, scoped transient failures and copy/retry
  assertions for ApplyInputs and falling scan updates. Assert metadata identities
  and unqualified cases. Run protected-parent/compiled native tests first.
  Expected: the valid pulse fails at the old reset-event guard or new API missing.
- [x] Add CLI positive pulse, cascade, hierarchy/index/active-low, model-readback,
  mixed/capture and controlling-fill cases with literal expected states. Add held
  sink/cascade transient sentinel failures and unsupported shared/data-consumer/
  cyclic/controller cases, keeping masked controls. Use the existing helper for
  three forms and explicitly replay associated-flat generated/reset-only forms.
  Run new positives on the protected parent. Expected: old reset-event/planner
  rejection; negative and masked controls retain their existing outcomes.
- [x] Build bounded reset-consumer metadata in the existing dependency walk,
  including OneHot conditions. Propagate no/unique/multiple group ownership once,
  index scan data consumers, qualify disjoint acyclic closures and build their
  internal consumer edges. Expected: native metadata identities and guard tests pass.
- [x] Implement transactional concrete reset waves after each feedback delta,
  charging existing work budgets and checking simultaneous intermediate states.
  Expected: primitive/cascade/unknown/event/copy/rollback native tests pass.
- [x] Implement symbolic reset waves on the existing mutable state view, carry
  sink values/fills through primary and mixed callers and seed primary fills.
  Reuse known ScanRegister reset-fill overlays in native replay and verify sink
  values/fills against modeled frames. Expected: normal/static feedback, actual
  observation and controlling-fill cases agree without invented events.
- [x] Run focused native state/scheduler/readback and existing data reset,
  feedback, observation, reservation and callback tests. Expected: all pass;
  an empty apply or event-free observation frame performs no feedback event;
  required primary/scan preparation still executes and publishes once.
- [x] Measure three rounds of16/128 groups ×2/8 feedback bits ×1/4 cascaded scan
  sinks, plus masked parent/current controls, retaining actual artifacts and exact
  coordinates. Expected: known candidate resets, identical control outputs and
  unchanged10second/512MiB budgets. Record unsupported limits without relabeling.
- [x] Freeze final inputs and run full project, affected leak-enabled sanitizers,
  changed-C++ quality, corpus/editor and packaging. Reuse a broader pass for a
  focused claim only when that case ran under the required conditions.
  Expected: all required commands exit0 with exact source/product bindings.
- [ ] Update guide/tracker and read the complete finalized selected raw diff;
  commit grouped messages and run task-done with
  python3 tests/.task198-check.py.
  Expected: final behavioral, source/product and actual performance evidence agree.

## Branch completion

- [ ] One fresh independent whole-branch review; regrade findings/declined areas,
  one Critical/Important test-first repair pass if needed; defer Minors.
- [ ] Merge into parent master, verify the merged result, push, detach the reused
  worktree, safely delete only this owned branch and archive only this workspace.
  Expected: clean aligned master/origin,117 unchanged stashes and no task backlog;
  continue the remaining inventory.
