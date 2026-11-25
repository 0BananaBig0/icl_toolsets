# Opaque child parent-only preparation implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking.

**Goal:** Automatically open modeled parent bypasses for fixed opaque child scans.
**Architecture:** Prepare a trial native scheduler, reject preparation loads in
private storage, and compute opaque completions on the prepared pre-raw route.
**Tech Stack:** C++17, Tcl, Python unittest, existing Z3 and sanitizer products.
**Spec:** docs/superpowers/specs/2026-11-25-opaque-child-preparation-design.md

## Global Constraints

- Original items2–15 except1; no new dependency, RTL or fabricated observation.
- Parentmaster24c7510; reuse .worktrees/bsdl-defaults and preserve117stashes.
- Fixed opaque scalar binary bypass networks; preparation contains public storage only.
- Aggregate1048576bits;10s/512MiB and256MiB single-wide measurement bounds.
- One inline implementer, one fresh final reviewer, one TDD blocker-fix wave.
- Preserve32deferred Minors and all broader unsupported-mode contracts.

## Review Focus

Current raw payload must not satisfy an earlier preparation CSU.
Failed preparation or raw execution must not commit state or publish observations.
An active opaque sibling cannot silently enter a supposedly parent-only prefix.
Generated ordinary opening must reproduce native capture/update without extra CSUs.
Mixed client histories and old root history must survive the route transition.

### Task 1: Parent-only opaque child preparation

**Files:** Modify src/pattern/pattern_scheduler.cc, existing native
tests/src/pattern/pattern_opaque_test.cc and Makefile. Create
tests/src/pattern/test_opaque_child_preparation.py; extend or reuse existing
benchmark_dynamic_opaque_routes.py and its mirrored reporter only as required.
Maintain docs/remaining-work.md, pattern-generation.md and pattern-model-coverage.md.
**Interfaces:** Preserve ScanScheduler::ApplyScans and Impl::PrepareScans,
OpaqueRawState/CommitOpaqueState signatures. Use existing actual ScanStep.loads,
trial swap, queue completion and RawObservation commit behavior.

- [x] Step1: Write initially closed single/two-child continuation tests. Direct
  child8SI165 SO0bx returnsX8/mask0/status0 and no requested root history; require
  one real parent opening and one raw scan. Run against frozen parent.
  Expected: actual inactive-child raw-route failures, not fixture errors.
- [x] Step2: Defer OpaqueRawState until after trial preparation, inspect prefix
  loads for private storage and retain commit-on-success. Run the new CLI and
  native opaque/readback/runtime checks. Expected: exact two-CSU GREEN.
- [x] Step3: Add closure/reopen, mixed/disjoint, stable/default, unknown-care,
  aliases/inversion, reservation/reset, rollback/copy, private-prefix rejection,
  default/flat/generated/local-reset checks. Add truthful automatic-preparation
  benchmark coverage and reporter checks test-first. Expected: actual words and
  observations agree with matched unknown-capture modeled controls.
- [x] Step4: Run focused then complete normal/static generators,30native,
  affected sanitizer CLI, changed-C++quality, corpus, packaging, all existing
  matrices and the new representative preparation workload. Expected: passing
  exact source/product hashes and measured bounds, unchanged corpus classes.
- [x] Step5: Read finalized staged patch, commit and obtain one fresh final
  review. Re-grade, fix blockers once test-first, qualify and record rulings.
  Expected:0unresolved Critical/Important and accurate deferred-Minor inventory.
- [x] Step6: Merge/push, verify merged hashes/checks, detach reused worktree,
  delete only owned branch and archive own ledger. Expected: clean pushed master
  and preserved117stashes; broad inventory remains accurate.

Completion command: python3 /tmp/opaque-child-preparation-completion.py
Self-review: one shared preparation/queue transaction; no cross-task interfaces.
