# Status-clear prefix implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Preserve successful evaluated status clears across level0 prefixes.
**Architecture:** Ordered internal clear events render as standard comments;
Runtime validates replay intent against available actual status, while the
provider and scheduler retain clear positions without extra hardware.
**Tech Stack:** C++17, Tcl, Python unittest, existing Z3/sanitizer builds.
**Spec:** docs/superpowers/specs/2026-11-25-status-clear-prefix-design.md

## Global Constraints

- Continue original items2–15 except1. No RTL, dependencies or real transport.
- Reuse .worktrees/bsdl-defaults from master8d10bd3. Preserve117stashes,
  ignored products and other ledgers. Commit/merge/push/delete owned branch.
- Inline under delegated design authority. One fresh final reviewer, one
  test-first blocker-fix wave, no rereview; retain29deferred Minors.
- Generated PDL stays level0. No extra CSU, observed data or failure recount.
- Keep1Mbit and10s/512MiB bounds; single-wide controls256MiB.

## Review Focus

Clear after earlier failures and before later comparisons; zero/unknown status;
pending queues and resets; note validation/inert regeneration; called merge
order, namespaces and local-reset/provider boundaries. Map each to Steps1–3.

### Task 1: Status-clear prefix persistence

**Files:** Modify src/pattern/pattern_runtime.cc, pattern_readback.h/.cc,
icl_pattern_generation_main.cc and pattern_opaque.cc; inspect pattern_merge.cc
for event ordering, changing only necessary callers. Create
tests/src/pattern/test_status_clear_prefix.py and add it to Makefile's generator
suite. Maintain tests/src/pattern_readback_test.cc, existing status tests,
tests/src/pattern/benchmark_multiple_opaque_children.py and its mirrored
reporter, plus the three current guide/inventory documents.
**Interfaces:** Internal iGetStatus{-clear,canonical_count} Operation; shared
ReadStatusClearNote/IsStatusClearNote and canonical note-text construction in
pattern_readback.h/.cc. Runtime records successful events with invocation
context; provider clears known counts at their stream positions. Renderer
emits standard notes and keeps unconsumed reserved text inert.

- [x] Step1: Add real source-versus-prefix status/clear regressions for PIO,
  scalar raw and opaque child cases; run immutable parent. Expected missing
  clear-event failures and retained pre-clear actual histories.
- [x] Step2: Implement recording, bounded intent parsing, provider handling,
  rendering and necessary lowering/merge barriers. Expected focused normal
  tests pass with identical hardware requests and correct later counts.
- [x] Step3: Cover zero/repeated/unknown/unobserved, malformed/stale notes,
  pending queues/resets, bound TAP, namespaces/local resets and called merges.
  Maintain parent controls/live-clear benchmark rewriting test-first.
  Expected native and normal/static/SAN focused/reporting checks pass.
- [x] Step4: Run full normal/static generator,30native normal/SAN, affected
  SAN CLI, corpus, changed C++quality and source packaging. Record exact
  input/product hashes and complete matched benchmark/allocation/guide rows.
  Expected required checks pass under unchanged classifications and bounds.
- [ ] Step5: Commit finalized diff and obtain one fresh whole-branch review;
  fix blockers once test-first and rerun affected qualification. Record Minors.
  Expected no unresolved Critical/Important findings; no rereview.
- [ ] Step6: Merge/push master, verify merged hashes/focused checks, detach
  reused worktree, delete owned branch and archive only this ledger.
  Expected clean pushed master,117stashes and accurate remaining inventory.

Completion: python3 /tmp/status-clear-prefix-completion.py

Self-review: one shared event/data-flow task with no independent task
interfaces. Review classes map to Steps1–3; no implementer agents.
