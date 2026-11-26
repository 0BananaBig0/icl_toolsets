# Automatic primary callback observations implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking.

**Goal:** Report actual primary histories after automatic write callbacks.
**Architecture:** Existing planning frames execute in a separate actual native
state; selected read-route provenance validates actual observations.
**Tech Stack:** C++17, Tcl, Python unittest, existing Z3/native/sanitizer products.
**Spec:** docs/superpowers/specs/2026-11-26-automatic-primary-readback-design.md

## Global Constraints

- Direct parentmasterbce04e4; original4/6 bounded work,117stashes and36Minors.
- Reuse .worktrees/bsdl-defaults; no new external dependency or scheduler engine.
- Primary-only callback/DataRegister networks; preserve other mode/topology guards.
- Planning intent never supplies actual callback data or actual read-route proof.
- Keep C++17,256primary ports/4096bits/1048576queued bits,10s/512MiB/256MiBwide.
- One inline implementer, one fresh final review, one TDD blocker repair wave.
- Excluded1 and broader unfinished inventory retain scope.

## Review Focus

- Callback assumptions and logical goal markers must never become actual data.
- Unknown actual read selectors or preserved storage must not inherit planner proof.
- Per-frame sparse/alias reads must occur at their actual selected frame.
- Reset/canceled/merged callbacks and default no-getter preflight must remain consistent.
- Extra actual/planning state must retain bounded memory and existing ordinary fast paths.

### Task 1: Primary automatic callback actual execution

**Files:** Modify src/pattern/pattern_scheduler.h/.cc, pattern_readback.h/.cc and
icl_pattern_generation_main.cc; extend tests/src/pattern_scheduler_test.cc and
tests/src/pattern_readback_test.cc; maintain newly supported guard fixtures and
the three current guide/inventory docs. Create
tests/src/pattern/test_automatic_primary_readback.py,
tests/src/pattern/benchmark_automatic_primary_readback.py and reporter
tests/tests/src/pattern/test_benchmark_automatic_primary_readback.py; update Makefile.
**Interfaces:** Add per-ScanStep primary logical-bit/route provenance. Add
ScanScheduler::ReplayPrimary(original accesses, planned steps, preserved masks,
observation callback, caller goals) with native state rollback and post-commit
subscriber notification. Keep existing Apply/NormalizeScan contracts. Add a shared
primary callback eligibility predicate to readback and final-main preflight.
ModelReadback uses original-model planning and masked actual native state only
for that bounded class; all other models retain their existing paths.

- [x] Step1: Add source and generated prefix cases for automatic dest9 writes,
  actual endpoint1001/downstreamxxxx, masks/status and callback-local old/reset
  reads. Expected: real automatic guard and unknown-load replay RED.
- [x] Step2: Carry selected primary read provenance and replay real native input
  frames, checking actual guards/preservation. Wire bounded planning/actual state
  and shared admission. Expected: original/flat/generated actual data GREEN.
- [x] Step3: Pin sparse/inverted/logical reads, namespaces/once-only arguments,
  repeated/reset/canceled/nested/merged callbacks, resource failures, unknown
  selectors, source bytes/local-reset and sentinel outputs. Add native per-event
  references, measured parent hardware controls and reporter tests first.
  Expected: no invented actual data/events and retained unsupported boundaries.
- [x] Step4: Run complete normal/static generators,30native/SAN, changed-C++
  quality, affected SAN CLI, retained/new scale matrices, corpus, guides and
  packaging. Expected: all bounds and exact provenance pass, no false parent
  modeled-history or global optimality claim.
- [ ] Step5: Read selected patch, commit and get one fresh final review. Grade
  every finding/declined area; repair blockers once RED→GREEN/full suite, defer
  Minors. Expected: no unresolved Critical/Important, no rereview.
- [ ] Step6: Merge/push to master, verify merged checks, detach reused tree,
  delete only owned branch and archive own ledger. Expected: clean pushed master,
 117stashes and bounded original-inventory claim.

Completion command: python3 /tmp/automatic-primary-readback-completion.py
Self-review: one connected primary planning/actual observation flow; no task interfaces.
