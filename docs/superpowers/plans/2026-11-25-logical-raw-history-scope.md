# Logical raw history scope implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Preserve absent and earlier root histories across child-only lowering.
**Architecture:** Carry a logical-history omission flag through native raw
lowering, encode it as an exact adjacent pre-scan comment and suppress only
logical history publication. Physical comparisons and status stay intact.
**Tech Stack:** C++17, Tcl, Python unittest, existing Z3/sanitizer builds.
**Spec:** docs/superpowers/specs/2026-11-25-logical-raw-history-scope-design.md

## Global Constraints

- Continue original items2–15 except1. No RTL, dependencies or real transport.
- Reuse .worktrees/bsdl-defaults from master5b8be34. Preserve117stashes, ignored
  products and other ledgers. Merge/push/delete only the owned task branch.
- Inline executor under delegated design authority; one fresh final reviewer,
  one test-first blocker fix pass, no rereview. Retain29deferred Minors.
- Preserve1Mbit and existing10s/512MiB bounds, single-wide controls256MiB.
- No extra hardware transaction, changed masks or failure recount.

## Review Focus

Absent and old root histories; status independent of publication; exact-note
matching and canceled/stale intent; metadata copy/normalization/subscribers;
source/flat/prefix/local-reset/provider boundaries. Cover each in Steps1–3.

### Task 1: Logical raw history scope

**Files:** Modify src/pattern/pattern_runtime.h/.cc, pattern_opaque.h/.cc,
pattern_scheduler.cc, pattern_readback.cc, icl_pattern_generation_main.cc;
maintain tests/src/pattern/test_multiple_opaque_children.py,
test_modeled_raw_readback.py and pattern_opaque_test.cc as needed. Maintain
affected existing benchmarks/reporters and docs; new test scripts belong in
tests/ mirroring source paths.
**Interfaces:** Operation::raw_history_omitted defaultfalse, shared
RawHistoryOmissionText(const Operation&) returns canonical annotation text.
Native lowerer marks physical-only roots. Runtime binds exact adjacent typed
notes to root data scans; provider counts comparisons but omits raw publication.

- [x] Step1: Add absent/previous-root source-versus-generated continuation
  regressions to opaque and modeled suites; run immutable parent.
  Expected real root-scope/mask failures before implementation.
- [x] Step2: Implement metadata propagation/rendering and Runtime/provider
  history omission without modifying hardware or status. Expected focused
  normal checks pass and exact generated replay is idempotent.
- [x] Step3: Add exact/malformed/stale/duplicate typed notes, resets/canceled
  queues, explicit-root, suffix clear, copies/subscriber and unobserved cases.
  Maintain benchmark controls and live getters. Expected normal/static/SAN
  focused and reporter checks pass with unchanged hardware counts.
- [x] Step4: Run full normal/static generator,30native normal/SAN, affected
  SAN CLI/corpus, changed C++quality and source packaging. Record input/product
  hashes and complete matched replay/allocation/guide matrices.
  Expected every required check and unchanged corpus classifications pass.
- [ ] Step5: Commit finalized diff; obtain one independent whole-branch review.
  Fix blockers once test-first, record Minors and rerun affected qualification.
  Expected no unresolved Critical/Important findings; no rereview.
- [ ] Step6: Merge/push master, verify merged hashes/focused checks, detach
  reused worktree, delete owned branch and archive only this ledger.
  Expected clean pushed master,117stashes and accurate broader inventory.

Completion: python3 /tmp/logical-raw-history-scope-completion.py

Self-review: one shared metadata/publication flow; no independent task
interfaces. Every review class maps to Steps1–3; no implementer agents.
