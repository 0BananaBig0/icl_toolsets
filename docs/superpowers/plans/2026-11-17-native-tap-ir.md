# Native TAP Instruction Replay Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Execute qualified known ordinary IR goals without a whole-network symbolic frame.

**Architecture:** Extend existing TryNativeTapScan with a pure-IR case. Reuse
TraceSerialPath, NativeTapShape/control qualification and ReplayTapStep/Cycle;
all unsupported cases retain the current joint-planner fallback.

**Tech Stack:** Existing C++17/Tcl/Z3/Python; no dependencies.

**Spec:** docs/superpowers/specs/2026-11-16-native-tap-ir-design.md

## Global Constraints

Parent master7dc8dc1, branch task/native-tap-ir-scans. Task114 is merged and its
branch deleted. No public API, new FSM/walker/solver/RTL or raised guard. Preserve
all data-mode callers and exact same-option output. Selected64x4/512alias IR-only
profiles target peak RSS below128MiB on both measured Linux environments. Do not
apply optimized RSS bounds to sanitizer binaries. No1M IR or global optimum claim.

## Review Focus

- Already-equal IR values still require actual capture/shift/update; missing IR
  ControlEvents is not evidence of a missing update or a substitute for replay.
- IR qualification covers all data-interface phase/clock forwarding while data
  selection may be inactive/unknown; noncanonical controls retain fallback.
- Pure instruction detection cannot drop primary/address/data/group obligations,
  implicit BSDL comparisons, end-state or synthetic-target validation.
- Literal-X raw proof copies remain disposable; unknown hardware and retained
  controlling fill must survive known IR recovery without concretization.
- Failed native probes leave hardware, fill, execution and public output intact;
  authored comparisons differing from cached capture retain symbolic generation.

### Task 115: Native ordinary instruction replay

**Files:** Modify src/pattern/pattern_scheduler.cc,
tests/src/pattern_scheduler_test.cc, Makefile and generator/coverage docs. Create
tests/src/pattern/test_native_tap_ir.py and benchmark_native_tap_ir.py. Reuse
existing ICL/BSDL and collector; add no duplicate vendor fixture/certificate.

**Interfaces:** Rename private RawDataPath to
ConcretePath RawTapPath(bool instruction=false), preserving default data checks;
IR mode requires the complete instruction register and bound TDI. Extend private
NativeTapControls(const ConcretePath&, bool selected=true): default data behavior
unchanged; selected=false qualifies all existing RegisterInterfaces phase/clock
forwarding and skips data SelectPort activity. TryNativeTapScan emits a known
instruction step only when required/goals/expected contain IR alone. Existing
Solve validation/dispatch guards remain authoritative.

- [x] Record Task114 reference binaries and parent, task-start/brief and builder
      result. Preflight outer Solve dispatch and SetRegister event semantics.
- [x] Write benchmark RSS acceptance for64x4 and512aliases, sharing existing
      ordinary-program collector and exact source/flat/generated artifacts.
      Run saved Task114 as both reference/candidate: completing byte-equal runs
      must fail candidate131072KiB ceiling (actual baseline252-262MiB).
- [x] Add functional/native literal IR checks before implementation: normal,
      already-equal, read-only/all-X observers, partial retained fill, unknown IR
      recovery, protected rollback/retry, actual N+6 clocks/endpoints and
      noncanonical reset/control fallback. Compare ordinary/raw and all replay
      views; keep old data/literal-X behavior and authored mismatched comparison.
- [x] Extend existing path/control helpers and native candidate as specified.
      Set instruction flag, preserve known load/parity/expected masks, use actual
      shared replay and final IR value/update eligibility instead of unavailable
      IR ControlEvents. Keep data history checks/reset guards unchanged. Adopt
      complete copies only after all preservation/observation checks pass.
- [x] GREEN functional/native and memory acceptance, then measure4/32/256 IR
      bits,64x4,512aliases,1024unrelated and128 IR groups with matched saved
      reference options and full byte hashes. Recheck known-data/literal-X
      benchmarks and relevant IR/DR/builtin/multiple-TAP/resource/reset suites.
- [x] Format/tidy/corpus/offline, one independent final review and necessary fix
      pass; freeze and run fresh full/SAN/remote gates. Fresh genuine binary IR
      vendor recertification, verifying source/input/artifact/console hashes;
      retain existing identical snapshots. No new X vendor equivalence claim.
      Record actual evidence, commit/merge into master/delete branch; no push.
      Task-done runs final source/gate verifier; archive completed ledger.
