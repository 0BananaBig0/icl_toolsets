# Automatic Child Raw Preparation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Pure modeled-child iScan emits parent preparation before its exact final raw CSU.

**Architecture:** Reuse local RawRoute and prefix-only TryGreedy on a complete trial; expose prerequisite steps through a trailing opt-in output parameter and render them with existing append_steps.

**Tech Stack:** Existing C++17/Tcl/Z3/Python; no dependencies.

**Spec:** docs/superpowers/specs/2026-11-16-automatic-child-raw-design.md

## Global Constraints

Start only after Task112 merge/delete; preserve its frozen9 gates. Existing
strict defaults and public NormalizeScan stay strict. No dummy target CSU,
new walker/solver/FSM/clock/reset engine, raised budget or opaque/TAP/parallel
child support. New scripts under tests/; no RTL. Preserve existing ownership,
metadata deferral, source order and reset cancellation contracts.

## Review Focus

- Every automatic CSU appears in the returned prefix and generated PDL.
- Defaults do not silently change native APIs; output sentinels publish only
  after both preparation and final raw replay pass.
- Parent setup does not refresh authored child or protected ancestor ownership.
- Initial local path is only a planning hint; actual post-prefix geometry and
  pre-raw assigned IDs must be revalidated after side effects/cache changes.
- Overwritten/canceled requests, disjoint compatibility, aliases and unknown
  local routes must not cause stale preparation or per-client solver trials.

### Task 113: Automatic modeled-child route preparation

**Files:** Modify src/pattern/pattern_scheduler.{h,cc},
src/pattern/icl_pattern_generation_main.cc, tests/src/pattern_scheduler_test.cc,
Makefile, docs/pattern-generation.md and docs/pattern-model-coverage.md. Create
tests/src/pattern/test_automatic_child_raw.py and
benchmark_automatic_child_raw.py. Reuse qualified child ICL; unique source PDL
and case metadata under tests/cases/ours/pattern_generation/automatic_child_raw/.
The qualified vendor variant adds a unique parent-only model reusing RawSibLeaf;
retain the original known-capture strict trace difference (see design ruling).
Add genuine directional snapshots/registry only after vendor verification.

**Interfaces:** ApplyScan and ApplyScans gain trailing optional
std::vector<ScanStep>* preparations=nullptr after assigned. Existing defaults
stay strict. Non-null opts into modeled child setup and replaces the output
only on complete success. TryGreedy gains private prepare_only=false; existing
consumers unchanged. Local RawProfile/RawRoute, control masks, preserved maps,
copy/swap transaction and append_steps are reused.

- [x] Task-start after Task112 merge/delete, record master parent and save
      local/remote Task112 reference binaries. Read brief/spec and preflight
      public output/renderer, TryGreedy prefix boundaries and resource/cache flow.
- [x] RED native/source reset-closed parent SIB with8-bit child, SI96/SO0;
      ordinary manual-open reference succeeds. Opt-in returns exactly one parent
      setup step followed by final root raw packet, default strict call rejects.
      Literal independent original/flat endpoint/storage/shift/clock comparison,
      caller assigned/prefix sentinel rollback on bad width/stable/preservation
      followed by valid retry. Run before product changes; expect current closed
      child rejection rather than fixture/parser errors.
- [x] Add opt-in signatures and trial-only publication. Normalize deferred
      immutable queue metadata with last-value/bounds preserved, gather each
      surviving child local path using RawClientPorts plus existing RawRoute
      (dynamic local profile), and combine target IDs once. Root/TAP defaults
      and unsupported scope remain. Unknown local path fails without adoption.
- [x] Add prepare_only TryGreedy exits: already visible returns empty; otherwise
      use existing configuration/replay batches and adopt prefix immediately
      when all required IDs accessible, before final Solve. No dummy access.
      Keep preservation explicit_writes=false and existing progress/time/bit
      guards. Invalidate dynamic route caches after successful prefix planning.
      Reuse strict raw ApplyScans for actual final geometry/interval/stable/packet
      proof, then atomically publish full scheduler/prefix/assigned outputs.
- [x] Source front end passes prefix output, renders it via append_steps before
      returned raw operation; existing ordinary preparation remains first.
      Check owner refresh only through existing final AssignedScans. Verify
      source-order, procedure/owner return, reset/overwritten cancellation,
      generated/flat/reset replay and applicable local-reset rewriting.
- [x] GREEN focused native/CLI: nested parents, compatible disjoint children,
      incompatible alternatives/overlap, unknown local selectors, parity and
      ascending aliases, protected ancestor0 rejection, child partial history,
      stable rollback and retry. Existing modeled/root/TAP/resource/state/clock/
      reset/trace suites pass; default native strict tests remain unchanged.
- [x] Measure same-option explicit-manual saved Task112 reference versus implicit
      setup, with complete canonical and replay bytes.8/256-bit leaves,64 distinct
      parents, forwarding aliases and128 reopen groups must complete under
      existing guards. Measure1M leaf/1024 parents/97 levels and document actual
      limits rather than raise budgets. Record commands/platform/wall/RSS/hashes.
- [x] Style/corpus/offline, independent final review and necessary one fix pass;
      genuine strict vendor forward. Reproduce/document reverse internal-client
      limitation if still present; no comparator relaxation or unearned snapshot.
      Freeze and run fresh full/SAN/remote required gates, record verified evidence
      and rulings, commit/integrate directly into master/delete finished branch.
      No push; task-done verifies final source/gate evidence.
