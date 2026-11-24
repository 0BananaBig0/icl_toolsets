# Standard history notes implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Keep informational notes harmless and restore callback history with standard typed metadata.
**Architecture:** Correct existing Runtime argument selection and validation, then reuse the existing typed-note renderer. Preserve legacy metadata input.
**Tech Stack:** C++17, Tcl, Python unittest; existing Z3 and sanitizer builds.
**Spec:** docs/superpowers/specs/2026-11-25-standard-history-notes-design.md

## Global Constraints

- Original items2–15 continue; coverage1 is excluded. No RTL/transport changes.
- Reuse .worktrees/bsdl-defaults; direct parent master412e933. Preserve117stashes and other ledgers/products.
- Execute inline under delegated design authority; one fresh final reviewer, one blocker fix wave, no rereview.
- Keep benchmark bounds and all actual-versus-expected history distinctions.

## Review Focus

Empty notes; extra arguments; both standard tags; legacy generated prefixes;
sparse seeded callback bits alongside unaffected ordinary histories. Each is
covered by Task1 tests below; existing suites retain local-reset/merge cases.

### Task 1: Standard callback history notes

**Files:** Modify src/pattern/pattern_runtime.cc,
tests/src/pattern/test_read_callback_histories.py,
tests/src/pattern/benchmark_read_callback_histories.py and task documentation.
**Interfaces:** Consume Operation.arguments and existing typed Command rendering;
preserve public Runtime APIs and existing invalidation masks.

- [x] Step1: Add tests for empty/malformed/extra notes preserving seeded histories
  and status, typed/status/legacy generated prefixes preserving sparse unavailable
  callback bits and actual q=9, duplicate metadata and quoted generated comments.
  Run the focused module on parent binary. Expected intended failures only.
- [x] Step2: Select valid legacy or typed note text before restoration, ignore
  malformed metadata, emit tagged comments and align direct benchmark controls.
  Run focused normal/static/SAN modules. Expected all pass and no invented data.
- [x] Step3: Run full normal/static generator,30native normal/SAN, affected SAN
  histories/callbacks, C++ quality and source packaging. Record hashes and results.
  Run callback benchmark source/flat/generated controls with unchanged bounds.
  Expected every required check passes; report actual classifications unchanged.
- [ ] Step4: Commit finalized changes and run one fresh whole-branch review.
  Fix Critical/Important once test-first and rerun affected verification.
  Expected no unresolved blockers; defer inherited27Minors and new Minors.
- [ ] Step5: Merge/push master, verify merged focused tests and qualified hashes,
  detach reused worktree, delete owned branch and archive only this task ledger.
  Expected clean pushed master,117stashes, original broad items remain accurate.

Completion: python3 /tmp/standard-history-notes-completion.py

Self-review: one existing implementation boundary, no inter-task interfaces;
all five review classes map to Step1. Small correction needs no builder review.
