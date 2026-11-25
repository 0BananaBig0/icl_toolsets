# Opaque prefix histories implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Preserve explicit opaque-child histories through root-scan lowering.
**Architecture:** Emit validated request-identity notes; restore actual unknown
observations from committed native interval comparisons via the existing
readback delta path. Pending Runtime intent contains no observed data.
**Tech Stack:** C++17, Tcl, Python unittest, existing Z3 and sanitizer builds.
**Spec:** docs/superpowers/specs/2026-11-25-opaque-prefix-histories-design.md

## Global Constraints

- Original items2–15 continue; coverage1 excluded. No RTL/dependencies/transport.
- Reuse .worktrees/bsdl-defaults; direct parent masteref8d28d. Preserve117stashes,
  ignored products and other ledgers. Commit/merge/push/delete owned branches.
- Execute inline under delegated design authority; one fresh final reviewer,
  one test-first blocker fix wave, no rereview. Inherited27Minors stay recorded.
- Actual opaque data stays unknown. No additional CSU or global failure count.
- Keep1Mbit and existing performance limits; O(total interval bits) per CSU.

## Review Focus

Root-only scope; cared/don't-care masks and clear status; notes after reset or
with mismatched payloads; existing histories under invalid notes; native copy
and subscriber commit. Each maps to Step1/3, not an assumed guard-only result.

### Task 1: Opaque prefix histories

**Files:** Modify src/pattern/pattern_opaque.h/.cc, pattern_scheduler.h/.cc,
pattern_runtime.cc and pattern_readback.cc; use the existing generator note
emission. Maintain tests/src/pattern/test_multiple_opaque_children.py and
native opaque/readback tests as needed, plus corresponding benchmarks/docs.
**Interfaces:** Shared optional parsed root/client/expected metadata; native
OpaqueRawHistory(note) returns a validated ScanReadbackUpdate without hardware
execution. Existing Runtime/scan APIs and private storage boundaries persist.

- [x] Step1: Add source-versus-generated-prefix continuation tests for one/two
  children, actual x data, sparse miscompares/status, persistent old histories,
  both tags/duplicates, invalid/stale/unanchored metadata and no-provider mode.
  Run on immutable parent. Expected genuine missing-local-history failures.
- [x] Step2: Implement shared bounded metadata parsing, native committed
  compare snapshots/identity emission and readback delta publication. Register
  Runtime pending intent only after a data-scan apply, preserving existing history
  until validated provider updates. Expected focused normal/static/SAN pass.
- [x] Step3: Cover native copy/reset/rollback/subscriber behavior, inversion,
  namespaces/split files and local resets; adjust parent benchmark comments
  without changing hardware requests or weakening full-output checks.
  Expected exact histories/status/scan counts and retained privacy.
- [x] Step4: Run full normal/static generator,30native normal/SAN, affected SAN
  CLI/corpus, C++quality and complete source packaging; record exact hashes.
  Rerun multiple-child and one-child measured replay matrices with unchanged
  bounds. Expected all required checks and actual corpus classifications pass.
- [x] Step5: Commit and obtain one fresh independent whole-branch review.
  Fix blockers once test-first, rerun affected qualification and record Minors.
  Expected no unresolved Critical/Important findings; no rereview.
- [ ] Step6: Merge/push master, verify merged qualified hashes/focused checks,
  detach reused worktree, delete owned branch and archive only this ledger.
  Expected clean pushed master and117preserved stashes; broader work accurate.

Completion: python3 /tmp/opaque-prefix-histories-completion.py

Self-review: one shared data-flow extension; no independent task interfaces.
Review classes map to Step1/3; bounded design needs no separate builder review.
