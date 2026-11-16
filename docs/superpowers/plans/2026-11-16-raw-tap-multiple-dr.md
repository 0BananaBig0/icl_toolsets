# Multi-register Raw TAP DR Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** One physical TAP DR CSU spanning a known ordered multi-register path.

**Architecture:** Share existing ordinary concrete-path traversal, map physical
packet slices/parity to full-register goals and reuse exact one-CSU proof.

**Tech Stack:** Existing C++17/Tcl/Z3/Python; no dependencies.

**Spec:** docs/superpowers/specs/2026-11-16-raw-tap-multiple-dr-design.md

## Global Constraints

One bound scalar root TAP, known SI, complete nonoverlapping register path. No
new walker/FSM/clock/reset/solver, implicit retargeting or widened public targets.
Task110 frozen17 immutable; new scripts under tests/. Existing budgets remain.

## Review Focus

- Packet slicing uses SO-to-SI order and distinct SI/SO parity, including indices.
- Shared traversal preserves existing decode and ActivePath callers' semantics.
- Exactly one requested data CSU, no extra IR/primary/setup or storage changes.
- Assigned/ownership footprints refer to pre-CSU path, even after topology change.
- Stable post-route completeness/length supports parity but rejects disconnection.

### Task 111: Ordered Full-path Raw TAP DR

**Files:** scheduler.cc shared path/result and multi-register request/proof;
native scheduler tests, maintain DR/IR/builtin CLI; new multi-DR CLI/benchmark
and unique fixture/snapshots; registry/guides. Public signatures unchanged.

- [x] Task-start after Task110 merge and builder check. Native/CLI RED for3/5
      full path, SI96, all endpoints/masks and ownership; old multiple reject.
- [x] Factor/reuse ordered graph path with legacy controls; slice packet/parity
      into map goals, exact one-CSU transaction and pre-route assigned set.
- [x] Stable post-route and inversion positive/constant/alternate negatives;
      partial/ascending/alias/failure/copy/ordinary-order regressions GREEN.
- [x] Matched-option representative scale, style/corpus/offline and genuine
      strict vendor direction; independent review and necessary fix pass.
- [x] Freeze source; full/SAN/remote all0, certificates qualified, commit/ff
      master and delete completed branch before next task.
