# Raw merge calls implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking.

**Goal:** Run supported modeled raw scans inside valid merge-call bodies.

**Architecture:** Admit validated merge markers through modeled-raw preflight and
reuse existing serial Merger fallback and native raw scheduling/replay.

**Tech Stack:** Existing C++17, Tcl, Z3 4.13.3 and Python unittest; no dependency.

**Spec:** docs/superpowers/specs/2026-11-24-raw-merge-calls-design.md

## Global constraints

- Preserve native raw/TAP/clock state, ordering, reservation and reset semantics.
- No new API, dependency, resource-limit increase or physical/model-history claim.
- Direct parent master ee8a2b3; preserve117 stashes and unrelated outputs/worktrees.
- New scripts under tests/src/pattern; reuse existing fixture and replay helpers.
- Reuse approvals; one independent final review and one blocker fix pass.

## Review focus

- Raw markers must not allow invalid queues or transfer pending work to siblings.
- Nested/mixed calls must preserve exact raw frame order and ordinary batching.
- Prefix/namespace/split-input values must be evaluated once and replay unchanged.
- Raw reservations and TAP IR/DR transitions must retain native state checks.
- Existing opaque/history/physical guards and failed output artifacts must remain.

### Task 1: Admit and qualify native raw merge calls

**Files:** Modify src/pattern/icl_pattern_generation_main.cc and Makefile;
create tests/src/pattern/test_raw_merge_calls.py and
tests/src/pattern/benchmark_raw_merge_calls.py; update current evidence docs.
Maintain existing tests only for behavior deliberately changed by the spec.

**Interfaces:** Consume GenerateMerged(netlist,operations,modeled_readback),
Merger::Batchable and existing ScanScheduler::ApplyRawScans. Preserve signatures
and native behavior; raw streams remain serial through existing fallback.

- [x] Step1: Add12 CLI cases: scalar/nested/prefix/mixed/parallel/TAP,
  defaults/notes/local-reset/reservations/protocol failures/remaining guards.
  Add exact matched serial frame and canonical-output assertions.
- [x] Step2: Run new suite on parent; expect valid merged raw calls to fail
  specifically on modeled-raw iMerge preflight, while serial controls pass.
- [x] Step3: Admit iMerge to the raw preflight whitelist; change necessary
  callers only if a concrete failing case proves the need.
- [x] Step4: Build normal/static/sanitizer generator; run focused new cases
  and relevant existing raw/merge/reset suites; expect exact replay and guards.
- [x] Step5: Measure specified normal/wide/sanitizer matrices, all forms and
  matched controls; record complete outcomes and retain bound failures.
- [x] Step6: Run full required regression, prove any native reuse by exact
  dependency/product/log evidence, update current docs and commit.
- [x] Step7: Obtain one fresh whole-branch review; re-grade and fix blockers
  in one test-first pass; ledger Minors and declined behavior costs.
- [x] Step8: Merge into master, check merged exact inputs/products and focused
  behavior, push, delete owned branch and archive only this plan's ledger.

Whole-task completion command: python3 /tmp/raw-merge-calls-completion.py
Expected: required checks and hashes valid, measured limits explicit, merged
and pushed clean master, owned branch removed and117 stashes retained.

Self-review: one task and no cross-task interfaces; all changes follow the
same raw-merge responsibility. Native reuse is conditional, not a pass claim.


Final review adds a maintained legacy expectation update: empty merges now
compare complete serial replay, while three topology negatives remain.
This Python-only edit narrows the native shared proof from4,042 to4,041
inputs without changing native dependencies or executables. One Important
finding is fixed; two Minors remain deferred.
