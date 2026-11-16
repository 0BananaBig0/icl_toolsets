# Literal unknown SI on qualified raw TAP scans

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans.

**Goal:** Stop rejecting all literal X input on otherwise qualified raw TAP paths,
without concretizing unknown serial bits or changing known-binary behavior.

**Architecture:** RawTapRequest supplies an existing ScanStep/ScanLane, shared
ReplayTapStep/Cycle performs actual three-valued replay on a complete copy.
Reuse/factor existing canonical control/reset/path qualification and raw
preservation/fill semantics. No new FSM, walker, solver or raised guard.

**Tech Stack:** Existing C++17/Tcl/Z3/Python only.

**Spec:** docs/superpowers/specs/2026-11-16-unknown-tap-si-design.md

Parent master f37363d; branch task/unknown-tap-si. Task113 gates and recovery
evidence are complete. Builder check: nothing important left. Preflight decisions are recorded in
the spec and ledger.

## Global Constraints

Existing C++17/Tcl/Z3 only. No new engine, walker, FSM, dependency or raised
limits. Binary raw TAP behavior and ordinary callers remain unchanged. New X
profile is canonical scalar single-root TAP with inactive immutable resets;
unknown data/control-dependent selection stays bounded failure. Reuse actual
three-valued replay and known-only controlling fill; never concretize SI.
Source and test edits stay in this isolated worktree; no RTL.

## Review Focus

- SI X is physically replayed as X; controlling fill changes only for known SI
  positions, while complete hardware state includes unknown and off-path effects.
- Every actual edge uses existing shared TAP replay; no duplicated phase engine
  or unjustified assumption that missing known ControlEvents means no update.
- Exact requested SI/SO, single-CSU, parity, stable post-route, assigned pre-route
  footprint and caller output rollback remain enforced.
- Selection/reset/control qualification remains valid for every unknown serial
  bit and intermediate value. Unsupported shapes fail explicitly.
- SO remains authored comparison; its difference from cached capture does not
  become an unsupported generation-time assertion.
- Established protected X assignments reject, surrounding protected storage is
  checked on every edge, builtin privileges remain exact-ID and call-local.

### Task 114: Unknown raw TAP SI

**Files:** src/pattern/pattern_scheduler.cc, tests/src/pattern_scheduler_test.cc,
existing tests/src/pattern/test_raw_tap_{ir,dr}_scan.py obsolete X rejections,
new tests/src/pattern/test_unknown_tap_si.py, a minimal benchmark reusing the
existing candidate-only TAP collector, Makefile and generator/coverage docs.
Reuse existing ICL/BSDL when qualifying source PDL. Add snapshots/registry only
for genuinely successful strict vendor directions.

**Interfaces:** Add private bool NativeTapShape() by extracting existing
TryNativeTapScan qualification. Add private std::vector<ScanStep>
ApplyLiteralTapScan(const RawTapRequest&, const std::map<size_t,std::string>&).
ApplyTapScan dispatches X only to it; all-binary Apply stays unchanged. A
disposable ordinary proof validates existing groups/privileges/end goals and
exact phase/endpoints/known positions; only literal ReplayTapStep is adopted.
No public signature changes.

- [x] After Task113 merge/delete, branch from master and record parent; save
      local/remote reference binaries, task-start/brief and builder preflight.
      Inspect actual IR/builtin update/unknown semantics before committing to
      this draft's candidate construction. Resolve findings in ledger.
- [x] True CLI/native runtime RED: test_literal_dr_unknown_input uses
      8bit SI0b10x101x0 and SO0; test_literal_ir_unknown_input uses4bit
      SI0bx011/SO0bxx01 then known instruction3 recovery. Known binary controls
      work. Native TestRawTapUnknownInput compares every public endpoint and
      actual register/shift value against literal N+5/N+6 replay. Literal original/flat
      model expectations include phase, clocks, pins and three-valued storage.
- [x] Implement smallest literal raw candidate with existing qualification and
      shared ReplayTapStep, retaining all-binary branch behavior. Preserve
      known-only fill updates and per-edge resources; no false known update
      requirement for X. Final raw proof/stable checks precede publication.
- [x] GREEN source/flat/generated/reset, original/flat literal native replay,
      all-X/mixed masks/parity/ascending/multiple registers, retained controlling
      fill after later partial write, IR recovery, rollback/retry and protected
      bits. Keep unsupported controls/reset/builtin cases explicit. Run all
      connected binary TAP/raw/resource/state/reset suites.
- [x] Representative8/32/256bit,64x4,512alias,1024unrelated and repeated-packet
      candidate-only timings/memory plus exact artifact/state checks. Keep old
      X rejection as RED, never a completing baseline or speedup ratio.
- [x] Format/tidy/corpus/offline, one independent final review and necessary
      fix pass. Freeze; full/SAN/remote and genuine vendor checks. Record actual
      vendor output/support limits. Commit/merge directly into master/delete
      branch, retain recovery evidence; no push. Task-done evidence verifier.
