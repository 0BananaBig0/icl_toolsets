# Physical Reset Replay Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Finish replay of vendor reset-masked scan loads without changing ordinary PDL.
**Architecture:** Explicit CLI mode reuses the scheduler's concrete TAP simulation; parsing and publication stay shared.
**Tech Stack:** C++17, Tcl, existing Z3 and Python unittest.
**Spec:** docs/superpowers/specs/2026-11-20-reset-physical-replay-design.md

## Global Constraints
- No dependency or RTL changes.
- Ordinary iWrite final-value semantics remain unchanged.
- Exact physical packet boundaries, binary loads and pre-update compares.
- Existing resource and publication checks remain enforced.
- Keep the raw 100/90 coverage failure visible.

## Review Focus
- Reset during shifting must affect the next packet's state and fill.
- Wrong expected output must fail without committing state or artifacts.
- Route changes and unknown loads must not create a setup transaction.
- Reservations must remain checked across every clock.
- Flattening must retain real reset and TAP behavior.

### Task 1: Exact physical reset replay
**Files:** src/pattern/pattern_scheduler.{h,cc}, src/pattern/icl_pattern_generation_main.cc; tests/src/pattern/test_argument_reset_sources.py; README.md and docs/pattern-generation.md as applicable.
**Interfaces:** Add a trailing opt-in physical flag to Apply/Solve and Generate; existing defaults remain unchanged.
- [x] Write self-contained TAP/reset replay tests, run against current binaries; Expected: new physical option is rejected.
- [x] Implement the shared concrete replay path and CLI mode; Expected: physical export, exact output, flattened replay, default rejection and negative tests pass.
- [ ] Run focused static and sanitizer checks, native scheduler, normal full regression, raw coverage, style/C++17 and source package; Expected: executable checks pass, known raw 100/90 gate fails and is retained.
- [x] Qualify recorded four export/model combinations and repeated larger input; Expected: all explicit physical replays pass with exact scan payloads.
- [ ] Document semantics and limits; commit, fresh whole-branch review, address important findings once, verify and integrate/delete the finished branch.
**Verification:** python3 tests/src/pattern/test_argument_reset_sources.py build/icl-pattern-generation
