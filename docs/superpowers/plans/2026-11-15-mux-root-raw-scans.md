# Gated Mux Root Raw Scans Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Execute exact root raw CSUs through known gated binary SIB routes.

**Architecture:** Cached immutable qualification plus current-state ordered route,
full-map native phase eligibility proof, active-only bulk shift controls and
post-update stable-route validation. Static child path and public APIs unchanged.

**Tech Stack:** C++17 existing netlist/Match/NetworkState/scheduler; Python unittest.

**Spec:** docs/superpowers/specs/2026-11-15-mux-root-raw-scans-design.md

## Global Constraints

No dependencies, new state engine or broadened TAP/parallel/reset/clock model.
Root Task99 frozen29 files remain unchanged. Keep current1Mbit/graph/register bounds.
New test scripts belong beneath tests/. Reuse actual stored update values, not
shift shadows or expected SO, for selectors. Preserve publication/failed-Apply rollback.

## Review Focus

- Bypass does not imply inactive: every omitted register needs zero effective events.
- Selector data must stay held during all shift edges; reject serial/phase dependencies.
- Update controls sample before SIB changes; opening/closing semantics differ.
- Stable requires unchanged ordered route/parity and phase eligibility after trial update.
- Current-state profiles must not become stale or trigger repeated full topology scans.

### Task100: Gated Binary SIB Root Scans

**Files:** src/pattern/pattern_scheduler.cc qualification/path/replay and the optional native
signal-reader hook in pattern_tap_execution.cc/.h;
maintain tests/src/pattern_scheduler_test.cc and tests/src/pattern/test_modeled_root_scan.py;
create unique gated_sib_raw fixture under tests/cases/ours/pattern_generation/;
update model/generator guides after evidence. Reuse ShiftComplete signature unchanged.

- [x] Independent detailed design/builder review against actual source.
- [x] Maintained intended RED and independent literal-reference tests.
- [x] Minimal qualification/route/phase/stable implementation; GREEN focused checks.
- [x] Connected checks, source-matched vendor probes and representative scale.
- [x] Independent final review and immutable style/leak/fullserial/remote/certificate gates.
- [x] Commit, fast-forward master and delete merged branch; preserve later work.

The separate explicit-comparison gated_sib_raw_roundtrip fixture passes genuine
Tessent2026.2 FORWARD and REVERSE run/strict record/strict check, all0 using the
unchanged harness and policy. Both schema3 snapshots are registered; all source,
artifact and submitted-input hashes were checked before import. The original
stable fixture and its rejected forward certificate evidence remain separate.
This qualifies the stated no-stable observation contract, not general vendor
stable preservation or child/IR/TAP/parallel portability.

Verified Task100: leak-enabled native7/main69/modeled35/opaque13/clocks51/client25/reset12/trace16/all57offline certificates all0. Full serial make check passes in 1487.5529853680055s. Final remote54 statuses all0, actual88 source/test/certificate inputs match. Independent review found no semantic blocker and one Important shared-control evaluation cost; bounded optional reader fix has maintained RED missing-hook compile, GREEN native equivalence/literal source-flat tests and complete suites. Equivalent wide-selector source1.04-1.05s before/.77s after; shared512aliases/1024children2.31-2.33s before/.38s after; canonical hashes unchanged. All25 scale commands pass across source repetitions/flat/generated, gated1Mbit .30-.33s and128 open/write/close repetitions .05s; peak168188KiB includes wide flattened selector form. Stable fixture accepted by Tessent but strict forward mismatch retained (dropped stable and added selector read); reverse0. Separate no-stable explicit-comparison fixture passes genuine Tessent2026.2 BOTH directions run/strict record/strict check0, unchanged policy, two schema3 certificates imported with all hashes checked. Source tree audit/style and initial16 connected checks passed; final broader gates cover the reader fix. Cached held-data selection, full-map inactive phase proof, pre-update eligibility and separate-copy stable proof preserve all state/fill rollback. Native helper hook is the necessary scope ruling beyond the initial scheduler-only file list; no separate state engine. Commit28 files, fast-forward master and delete merged branch, preserve next accessible-child work. Closed-child preparation, broader raw/ownership/events/reset/clock/lifecycle, strict coverage and final platform/vendor requirements remain open. No push.
