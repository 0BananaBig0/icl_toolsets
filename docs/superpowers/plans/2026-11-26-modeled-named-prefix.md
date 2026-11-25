# Modeled named-chain history replay implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking.

**Goal:** Preserve explicit-only modeled named-chain histories through generated replay.
**Architecture:** Mark existing physical implicit completions and serialize distinct
canonical metadata; reuse fixed-route normalization and adjacent consumption.
**Tech Stack:** C++17, Tcl, Python unittest, existing Z3/sanitizer builds.
**Spec:** docs/superpowers/specs/2026-11-26-modeled-named-prefix-design.md

## Global Constraints

- Original item6 bounded increment; direct parentmastercc38cbf,117stashes.
- Reuse .worktrees/bsdl-defaults and existing native simultaneous scans.
- Preserve old named/scalar/opaque metadata, actual SI/SO, flags and comparison counts.
- No new external dependency or scheduler engine; existing API additions remain compatible.
- Keep1048576-bit queue limit,10s/512MiB and256MiB single-wide controls.
- One inline implementer, one fresh final reviewer, one TDD blocker repair wave.
- Preserve36inherited Minors, excluded1 and broader unfinished inventory.

## Review Focus

- Implicit physical lanes must preserve absent histories and old asymmetric cared masks.
- Old named informational notes must not change Tcl call arguments or observations.
- Valid new direct metadata must follow required hoisting and canonical matching.
- Failed fixed-width/topology/reservation calls must retain output artifacts and state.
- Extra route validation must stay cached and within measured memory/runtime limits.

### Task 1: Modeled named-chain explicit history continuity

**Files:** Modify src/pattern/pattern_scheduler.cc, pattern_opaque.h/.cc,
pattern_runtime.cc, icl_pattern_generation_main.cc and Makefile as needed;
extend tests/src/pattern_scheduler_test.cc and tests/src/pattern_readback_test.cc;
create tests/src/pattern/test_modeled_named_prefix.py,
tests/src/pattern/benchmark_modeled_named_prefix.py and its reporter at
tests/tests/src/pattern/test_benchmark_modeled_named_prefix.py. Update the three
current guide/inventory docs, this plan and its spec.
**Interfaces:** Keep ApplyRawScans and NormalizeScan contracts. Reuse
raw_history_omitted for implicit completion; add a compatible named-modeled form
to RawHistoryOmissionText/recognition without activating old notes on modeled
roots. Keep original defaults for existing callers. Runtime may cache a native
normalizer only for new named-modeled metadata; replay link recipes must include
its real existing dependencies. OperationText's emission distinguishes native
named-modeled scans from opaque output. No second queue or observation source.

- [x] Step1: Add unequal first4/second2 absent-history and old read0000/mask1111
  continuation tests. Run against parent. Expected: real invented-history and
  silently replaced capture/mask RED in generated replay.
- [x] Step2: Carry omission intent on implicit native lanes, serialize distinct
  metadata and consume only canonical eligible fixed-root scans. Run focused
  and native checks. Expected: source/flat/generated actual-history GREEN.
- [x] Step3: Pin old notes/arguments, both tags, canonical discriminating direct
  merge notes, repeated/reset/canceled/later invalid calls, inversion/unknown
  capture/clear, default/local-reset replay and sentinel outputs. Add measured
  controls and reporter tests first. Expected: real frames, counts and no forged data.
- [x] Step4: Run full normal/static generators,30native, additional replay links,
  changed-C++quality, affected SAN CLI, corpus, retained/new matrices, guides and
  packaging. Expected: hashes/corpus provenance and all bounds pass.
- [x] Step5: Read selected patch, commit and obtain one fresh final review.
  Grade findings and every declined area; repair blockers once RED→GREEN/full
  suite, defer Minors. Expected: no unresolved Critical/Important and no rereview.
- [x] Step6: Merge/push to master, run integrated checks, detach reused tree,
  delete only the owned branch and archive own ledger. Expected: clean pushed
  master,117preserved stashes and bounded inventory claim.

Completion command: python3 /tmp/modeled-named-prefix-completion.py
Self-review: one connected named-root history flow; no shared task interfaces.
