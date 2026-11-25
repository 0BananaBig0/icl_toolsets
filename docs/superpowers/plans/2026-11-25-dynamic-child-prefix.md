# Dynamic modeled child prefix implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking.

**Goal:** Restore dynamic child histories using the geometry of their actual CSU.
**Architecture:** Retain one immutable pre-CSU RawProfile and local projection
caches. Distinct dynamic intent comments reuse Task177 provenance and neutral
readback, preserving actual old words after route changes.
**Tech Stack:** C++17, Tcl, Python unittest, existing Z3 and sanitizer builds.
**Spec:** docs/superpowers/specs/2026-11-25-dynamic-child-prefix-design.md

## Global Constraints

- Original2–15 except1; no RTL, dependencies, device transport or guard-only enabling.
- Parentmaster19af5bc; reuse .worktrees/bsdl-defaults and preserve117stashes.
- One inline implementer, one fresh final review and one blocker-fix wave.
- Reuse accepted dynamic scalar execution; no new clock/topology claims.
- Keep1Mbit aggregate and10s/512MiB bounds; single-wide controls256MiB.
- One shared immutable route profile, local caches; no per-child root snapshots.
- Retain30deferred Minors, including factory-address source compatibility.

## Review Focus

Pre/post route widths can differ; equal-length routes may reorder/parity-shift;
failed/copy executions must not leak geometry; stale notes must remain inert
and observation-neutral; default/flat/local-reset continuations must agree.
Each input class is pinned in Steps1–3.

### Task 1: Dynamic child prefix persistence

**Files:** Modify pattern_opaque.h/.cc, pattern_scheduler.h/.cc and
icl_pattern_generation_main.cc; reuse Runtime/readback callback contracts,
changing callers only where required. Create
tests/src/pattern/test_dynamic_child_prefix.py,
tests/src/pattern/benchmark_dynamic_child_prefix.py and its mirrored reporter
under tests/tests/src/pattern/. Maintain Makefile, native readback tests,
existing benchmark controls and three guide/inventory documents as needed.
**Interfaces:** Add kDynamicChildHistoryNote and a dynamic discriminator to
ModeledChildHistoryNote. Existing Is/ReadModeledChildHistoryNote parse both
bounded forms. ScanScheduler::DynamicRawHistory() const reports whether the
most recent scalar raw CSU captured dynamic geometry. Existing
ModeledRawHistory(note,root_scan) returns the projection of that committed CSU.
Private raw_history_profile_ is shared const RawProfile; child slice cache and
work budget belong to each scheduler copy.

- [x] Step1: Add genuine immutable-parent source versus original/flat/generated
  prefix failures:old9bit0b101001011/mask0b101001011/status6 after a child closes,
  plus a fixed8bit child whose root offset changes. Expected missing history RED.
- [x] Step2: Capture pre-CSU geometry and commit only on success; parse/emit
  dynamic intent and project actual root returns through old geometry.
  Expected initial focused/native GREEN with identical hardware/counts.
- [x] Step3: Pin nested widths, route/parity changes, unknown/canceled histories,
  stable claims, copy/rollback, invalid/duplicate/default-chain notes, merges
  and default/local-reset continuation. Add reporter/workload checks test-first.
  Expected exact results, unchanged prior behavior and measured bounds.
- [x] Step4: Run complete normal/static generators,30native normal/SAN, affected
  sanitizer CLI, corpus, changed-C++quality and source packaging. Check exact
  source/product hashes and all existing/new matched rows. Expected all pass
  with unchanged corpus classifications.
- [ ] Step5: Commit finalized diff and obtain one fresh whole-branch review;
  fix blockers once test-first and qualify the final state. Record all rulings
  and deferred Minors. Expected no unresolved Critical/Important, no rereview.
- [ ] Step6: Merge/push master, verify merged hashes/focused checks, detach the
  reused worktree, delete the owned branch and archive only this ledger.
  Expected clean pushed master,117stashes and accurate remaining inventory.

Completion command: python3 /tmp/dynamic-child-prefix-completion.py
Self-review:one shared route/history flow, no independent task interfaces.
