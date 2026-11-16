# Native TAP Data Scan Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Complete known selected64-register256-bit TAP data transactions within15 seconds, preserving exact physical replay and fallback behavior.

**Architecture:** Reuse ordered concrete paths and construct one ordinary candidate; extract existing TAP cycle replay once for symbolic/native consumers. Keep joint planning for unsupported candidates.

**Tech Stack:** Existing C++17/Tcl/Z3/Python, no new dependencies.

**Spec:** docs/superpowers/specs/2026-11-16-native-tap-data-scans-design.md

## Global Constraints

Task111 frozen32 stays immutable through completed gates/integration. Start only
after Task111 merge/delete. No new TAP FSM, clock/reset/event engine, serial
walker or solver. Existing event/structural budgets and public signatures stay.
One ready bound scalar root TAP, selected instrument DR, known complete loads;
unsupported prerequisites/IR/builtin/primary/data/reset cases retain fallback.
New test scripts under tests/, repository root is project root. No RTL.

## Review Focus

- Native attempt cannot bypass public synthetic-target or AccessTogether checks.
- Physical SO is sampled before shift edge, including final exit bit and parity.
- Reservation old-to-desired checks run through capture/shift/update/idle edges.
- Failed candidates leave every pin, storage bit, fill and caller footprint intact.
- Off-path events, unknown fill and direct-reset histories fall back rather than
  adopting incomplete fill bookkeeping or rejecting previously supported work.

### Task 112: Native selected TAP data transaction

**Files:** Modify src/pattern/pattern_scheduler.cc (private native dispatch and
shared TAP replay helper), tests/src/pattern_scheduler_test.cc (literal phases,
all state, failure/fallback), tests/src/pattern/benchmark_raw_tap_multiple_dr.py
(keep previous geometries; add64x4 after saved-baseline RED), docs/pattern-generation.md
and docs/pattern-model-coverage.md. Create tests/src/pattern/test_native_tap_data.py
and register in Makefile. Reuse Task111 fixture; add no duplicate fixture models
unless genuine independent vendor evidence requires one.

**Interfaces:** Consumes RawDataPath()/TraceSerialPath ordered SO-to-SI register
IDs/prefix parity, WriteFill, resolved Solve goals/expected/required and
DesiredPreserved; existing Cycle/TapExecution. Produces private optional native
ScanStep sequence through existing Solve dispatch; no public API.

- [x] After Task111 merge/delete, branch from master with parent recorded, run
      task-start and save Task111 optimized local/remote reference binaries.
      Read this brief and spec. Preflight shared replay/rollback/fill interfaces.
- [x] RED: tests/src/pattern/test_native_tap_data.py generates64 r[3:0] self-capture
      chain, reset, IR3/read1, raw256 SI2643/SO0, final partial write/read history;
      source, flat and generated complete bytes agree and each invocation has
     15-second subprocess-group timeout. Run on saved Task111 reference and
      record intended timeout124 before product edits. Native literal3/5-bit
      selected path tests capture masks,16 parity/index combinations, exact
      N+5/TDI/TDO/TMS/TCK/all-state/idle values and copy/failure/retry.
- [x] Extract the existing concrete tap_replay block from Replay into one helper,
      preserving its exact leading100/final-exit/update/idle sequence and
      writable/reset-held checks. Provide optional physical observation at the
      existing shift sample point; inspect NetworkState scan reads first.
      Both symbolic Replay and native attempt call this helper; no second loop.
- [x] Add private TryNativeTapScan attempt using resolved maps and existing path.
      Qualify one ready bound TAP, all required in selected non-builtin path,
      full known0/1 goal-or-fill loads, no primary/DataRegister/end-state/group
      expansion. Reuse current single-path budget; no limit increase. Build
      exact maps/serial/parity on copied execution, use existing reservation
      callback and compare sampled SO. Require all path loads physically written.
      Reject native eligibility on direct reset or off-path events unless existing
      correct fill updater can be reused; return nullopt for original fallback.
      Adopt full execution/state/fill only after all checks. Raw proof unchanged.
- [x] GREEN: focused CLI/native tests and existing raw multiDR/DR/builtin/IR,
      modeled/root/resource/state/clock/reset suites. Test wrong SO and unknown
      fill, direct-reset/off-path events, failed reserved update followed by valid
      retry, retained parity/partial fill, source-order IR preparation and public
      synthetic targets in original/copy before-after. Expected all0; fallback
      cases preserve existing visible behavior and diagnostics where applicable.
- [x] Measure local/remote matching flags, peak RSS, complete source/flat/generated
      hashes at64x4 (<15s),16x16/64x1,1024 unrelated registers,512 aliases and128
      repetitions. Compare saved prior ordinary artifacts for completing shapes;
      retained prior timeout is baseline for64x4, not output equivalence evidence.
      Keep explicit4096 guard regression and all existing benchmark shapes.
- [x] Style/corpus/offline, independent final review and one necessary fix pass;
      genuine strict Tessent evidence, source freeze/full/SAN/remote qualification.
      Expected all required checks0 with declared inherited limits retained.
      Record evidence/rulings, commit, integrate directly into master and delete
      finished branch. No push. Final task-done verifies exact source/gate hashes.
