# Standalone Modeled Scan Readback Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking.

**Goal:** Return actual standalone serial observations through --model_readback.
**Architecture:** Sample concrete serial replay wires before shift edges, decode path intervals, and project physical captures to logical reads using the existing tentative observer.
**Tech Stack:** Existing C++17/Z3/Tcl and Python unittest; no dependencies.
**Spec:** docs/superpowers/specs/2026-11-22-modeled-scan-readback-design.md

## Global Constraints
Existing topology, event-work and clock guards stay in force.
Bound TAPs, raw scans, merge mode, timed histories, callback execution, interface/chain histories and real-device transport remain open.
Defaults and schedules remain unchanged. New maintained tests live under tests/.

## Review Focus
- Reset-held downstream cells must affect returned data, not just capture snapshots.
- Unequal lanes decode from tick zero and ignore trailing return padding.
- Inverted physical captures project to logical DataRegister bits exactly once.
- Repeated setup frames retain the first requested observation within an Apply.
- Failed Apply or unsupported bound TAP never publishes a partial result.

### Task 1: Serial observation integration
**Files:** Modify src/pattern/pattern_scheduler.{h,cc}, pattern_readback.{h,cc}, icl_pattern_generation_main.cc; tests/src/pattern_readback_test.cc, tests/src/pattern/test_modeled_readback.py, test_modeled_data_readback.py; create tests/src/pattern/test_modeled_scan_readback.py.
**Interfaces:** Consumes existing TraceSerialPath, NetworkState::ReadBits and tentative PioObservation. Produces Observation alias (old alias retained) and actual logical-bit observations.
- [x] Write literal-output scan tests: value9 expected0 gives read9/mask1001/count2, read-before-write gives9 then6, inverted/ascending/sparse/unknown lanes and DataRegister captures; add native serial event oracle and failure atomicity checks.
- [x] Run new tests against parent generator/native test; expect unsupported ScanRegister or missing observer failures. Keep raw logs.
- [x] Add CaptureComparison logical Key, pass observer into Replay, gather wanted monotonic per-lane intervals, sample actual outputs before each shift edge, decode suffix/order and emit selected logical bits. Restrict old post-state observation to parallel frames.
- [x] Admit ScanRegister storage, retain TAP/operation guards; migrate both legacy notes and compiled help; revise obsolete unsupported-scan test and cover TAP boundary.
- [x] Run focused new/existing readback, scheduler and scan-data tests against dynamic/static generators and native binary; expect all pass.
- [x] Commit feature and tests.

### Task 2: Qualification and delivery
**Files:** Modify docs/pattern-generation.md, docs/pattern-model-coverage.md, docs/remaining-work.md and relevant README text.
**Interfaces:** Consumes Task1 semantics; produces verified user guidance and retained qualification evidence.
- [x] Run required native/sanitizer, complete dynamic/static generator suites, affected formatting/tidy/strict compilation and source packaging; expect zero failures/skips beyond documented baseline.
- [x] Measure equivalent parent/current single-chain and larger multi-register inputs with/without queries; retain sizes, commands and timings.
- [x] Obtain fresh whole-branch independent review, resolve Important/Critical findings in one test-first verified pass, ledger dispositions.
- [x] Update docs with actual results and bounded remaining work; commit.
- [ ] Merge to master, verify integrated outputs, push and confirm upstream, detach reused worktree and delete merged child branch.
