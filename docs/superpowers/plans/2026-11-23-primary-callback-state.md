# Primary Callback State Index Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking. Preserve inline execution.

**Goal:** Remove repeated irrelevant callback-state symbolic materialization while preserving all supported scheduling behavior.
**Architecture:** Cache conservative topology-only state candidates in ScanScheduler::Impl; primary solves union their actual obligations and seed fresh values through the existing solver/replay path.
**Tech Stack:** Existing C++17, Z3/Tcl, Python standard library. No dependencies or RTL.
**Spec:** docs/superpowers/specs/2026-11-23-primary-callback-state-design.md

## Global Constraints
- Direct parentmastera71a2a7; existing reused worktree, one task branch.
- User opted out of questions and delegated choices; record decisions as ours.
- Exclude only unreferenced callback DataRegisters without WriteDataSource/reset/ResetValue; retain conservative aliases and all structural/metadata hardware references.
- Cache only IDs/topology, never values; current Assume/Reset/postconditions remain complete and fresh.
- Restore goal/read/end-state/preserved/desired-preserved IDs before the original state seed predicate; retain deterministic order.
- Keep all ordinary state, state-changing registers, potential driven inputs, solver objectives, reset/events, concrete replay, observations and resource guards.
-1024 targets/four callback groups: three-repeat median<=10s, >=2x matched parent speed, RSS<=256MiB; identical supported pattern bytes. No global linear-runtime claim or raised limits.
- Preserve117 stashes, other worktrees/ledgers and excluded coverage item1.
- New maintained scripts under tests/; no public API/Operation/SymbolicNetwork semantics changes.
- One fresh most-capable final whole-branch review, one blocking-fix pass, Minors deferred, no rereview.

## Review Focus
- Structural references through alias/mux/logic/one-hot/reset/interface/TAP fields retain callback storage, including unknowns and scope-specialized flattened models.
- Goal/read/end-state and both reservation masks restore otherwise excluded IDs, rather than becoming unconstrained unknown storage.
- Cached IDs survive scheduler copies and state changes while literal values remain fresh.
- Omitted storage remains in complete concrete state and never changes during primary events; no false observation or hidden reset/event bypass.
- Benchmark comparisons use identical options/input scale, full output checks, stable executable/input hashes and mode-specific workload accounting.

### Task 1: Conservative primary-state candidates and measured compatibility
**Files:** Modify src/pattern/pattern_scheduler.cc, tests/src/pattern_scheduler_test.cc, Makefile. Create tests/src/pattern/test_primary_callback_state.py and tests/src/pattern/benchmark_primary_callback_state.py.
**Interfaces:** Add private Impl topology-only candidate cache/helper using existing NetlistNode reference fields and existing Values. Keep SolvePrimaryWrites signature/public ScanScheduler API. New native TestPrimaryCallbackStateSnapshots, focused test-binary selector --primary-callback-state. New performance driver imports benchmark_callback_execution.inputs, accepts --binary/--parent/--output/--repeats and --sizes (defaults64,256,1024); retains all measured inputs/artifacts/hashes and correct mode workload dimensions.
- [x] Write matched performance driver. Same --debug options for parent/current plain/unused/callback cases; three source repeats, current flat/generated replay once per size, exact parent/current pattern equality and callback postcondition count. For1024 callbacks enforce median<=10s,>=2x speed and RSS<=256MiB. Run parent as candidate with --sizes1024 --repeats1: expected performance RED against Task153 baseline, with actual output otherwise correct. Preserve log/report.
- [x] Write native snapshots/copies/Assume/Reset/preserved-mask checks using public scheduler APIs and actual observations from referenced callback-backed storage. Add focused selector without changing existing selectors/default complete suite. Add CLI controls for fresh postconditions/assumptions across Applies, known/unknown held values, nested/flattened scopes, alias/logic/mux/one-hot/reset consumers, ordinary writable storage, end-state/ownership and modeled observations with no actual callback invocation. New fixtures must respect existing source contracts. Run on parent: compatibility controls may pass; record that rather than inventing functional RED.
- [x] Build conservative candidate cache once per immutable netlist for callback models; scan all reference containers in the spec, keep ordinary/potential driven state and reset/writable/referenced callback storage. Retain legacy materialization when no callback is eligible. Union actual obligations, preserve ID order and original register/primary/driven predicate, and seed current state literals in SolvePrimaryWrites. Use existing solver, constraints, reset/feedback, replay and observation paths unchanged.
- [x] Build dynamic/static generators and native scheduler; run focused native selector and new/Task153 callback, observation, queued-state, resource, data/reset, end-state CLI controls on both variants. Run matched64/256/1024 performance with3 repeats; expected exact bytes and the stated1024 budget. Run existing wide-alias memory driver and guide example. Fix only measured/required issues. Commit after green checks; task-done records the full focused gate.

### Task 2: Qualification, review, documentation and integration
**Files:** README.md, docs/pattern-generation.md, pattern-model-coverage.md, remaining-work.md and this plan/spec; owned build evidence and plan ledger.
**Interfaces:** Consume Task1 candidate implementation, test module and exact matched benchmark binary/hash. Broad item10 and all unrelated unfinished capabilities remain pending.
- [x] Run all14 native/14 leak-enabled ASan/UBSan native suites, complete dynamic/static generator suites, affected primary/callback/observation/queued/resource/readback/main sanitizer modules, changed-code format/tidy/strict C++17, source packaging and input/product hashes. Expected full green receipts; retain interrupted/failed runs separately.
- [x] Obtain one fresh independent whole-branch review with this Review Focus verbatim and all ledger rulings. Re-grade by user effect; fix Important/Critical in one test-first pass plus complete required suite, defer Minors and rule explicitly on every declined-to-judge line; no rereview.
- [x] Record reproducible parent/current medians/RSS, conservative scope and remaining costs. Update current guide/inventory without closing broad item10. Verify runnable examples and exact source/flat/generated outputs. Expected evidence matches qualified binary hashes; no unmeasured speedup claim.
- [ ] Commit, merge into direct parentmaster, verify source/product hashes and focused integration checks, push, detach reused worktree, delete only this merged branch and archive/remove only its own ledger. Expected clean master==origin/master and117 preserved stashes. Finish then continue remaining authorized inventory.
