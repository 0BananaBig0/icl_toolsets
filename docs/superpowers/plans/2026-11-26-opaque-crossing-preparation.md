# Opaque-aware preparation implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking.

**Goal:** Complete active opaque leaves correctly during automatic parent opening.
**Architecture:** Retain opaque queues in preparation-only native trials and
reuse existing implicit ordinary completion before each actual preparation CSU.
**Tech Stack:** C++17, Tcl, Python unittest, existing Z3 and sanitizer products.
**Spec:** docs/superpowers/specs/2026-11-26-opaque-crossing-preparation-design.md

## Global Constraints

- Original items2–15 except1; parentmaster4de8a33 and117preserved stashes.
- Reuse isolated .worktrees/bsdl-defaults; no new dependency or invented capture.
- Preserve public signatures, scalar eligibility and aggregate1048576bits.
- Retain implicit reservation semantics and pre-planning public-ID validation.
- Measurements10s/512MiB; preserve existing single-wide256MiB controls.
- One inline implementer, one fresh final review, one TDD blocker-fix wave.
- Preserve33deferred Minors and broader unsupported-mode boundaries.

## Review Focus

A current raw request must not supply an earlier preparation completion.
Nonstable prior scans must leave implicit opaque lengths unknown.
Nested preparation must commit queue state per actual CSU without extra events.
Public reservations and failed trials must preserve state and caller outputs.
Native implicit serial bits must agree with generated replay and manual controls.

### Task 1: Opaque-aware preparation completion

**Files:** Modify src/pattern/pattern_scheduler.cc, existing native
tests/src/pattern/pattern_opaque_test.cc, test_opaque_child_preparation.py and
Makefile. Create tests/src/pattern/test_opaque_crossing_preparation.py and a
focused benchmark/reporter mirrored under tests/tests/src/pattern. Reuse the
existing measured runner with a concrete grid argument only if required.
Maintain the three current guide/inventory documents.
**Interfaces:** Preserve public ApplyScans and PrepareScans. TryGreedy's
prepare_only path uses an opaque-aware trial, copied queues and existing native
Opaque Apply completion; implicit writes remain implicit. Successful prepared
trial transfers queues with state/fill. Nonprepare solver behavior stays native.

- [x] Step1: Add two initially active-child/closed-sibling continuation tests,
  reset-default child0 versus prior stable child0 SI165, then child1 SI60. Require
  requestedX8 histories, no invented implicit history, zero masks/status and old root history. Frozen parent
  should reject the private prefix. Expected: two genuine route-boundary REDs.
- [x] Step2: Retain queues in preparation trials, permit only internal implicit
  opaque preparation and commit each successful trial's queue state. Remove the
  obsolete private-load rejection. Run focused/native checks. Expected: GREEN
  with one opening CSU and the actual default0/stable165 private prefix bits.
- [x] Step3: Add multiple/nested/mixed, alias/parity/care, scope/history,
  default/local-reset, reservation, failed-output/copy/rollback cases. Update the
  Task180 boundary regression for supported behavior. Add truthful matched
  manual-parent performance/reporting tests first. Expected: exact physical
  words, preparation inputs and source/flat/generated continuation agreement.
- [x] Step4: Run focused then complete normal/static generators,30native,
  affected sanitizer CLI, changed-C++quality, corpus, packaging, existing/new
  matrices and guides. Expected: exact qualified input/product hashes, measured
  bounds and unchanged corpus identities/classes.
- [x] Step5: Read finalized staged patch, commit and request one fresh review.
  Fix blockers once test-first, qualify and ledger all rulings/Minors. Expected:
  0unresolved Critical/Important without rereview.
- [ ] Step6: Merge/push, verify merged results, detach reused worktree, delete
  owned branch and archive own ledger. Expected: clean pushed master, accurate
  remaining inventory and117preserved stashes.

Completion command: python3 /tmp/opaque-crossing-preparation-completion.py
Self-review: one shared queue/native preparation flow; no cross-task interfaces.
