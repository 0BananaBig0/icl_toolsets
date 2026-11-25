# Modeled static child prefix implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Preserve actual static modeled child raw histories across prefixes.
**Architecture:** Post-CSU intent notes name explicitly requested children;
shared scheduler geometry validates projections from an actual root return.
A neutral preview validates metadata before Runtime marks pending histories.
**Tech Stack:** C++17, Tcl, Python unittest, existing Z3 and sanitizer builds.
**Spec:** docs/superpowers/specs/2026-11-25-modeled-child-prefix-design.md

## Global Constraints

- Original2–15 except1; no RTL, new dependency or real-device adapter.
- Parentmaster124752a; reuse .worktrees/bsdl-defaults; preserve117stashes.
- One inline implementer, one fresh final reviewer, one blocker-fix wave.
- Static fully modeled scalar routes only; preserve root omission and counts.
- No observed values in notes, extra CSU or per-child root snapshots.
- Keep1Mbit aggregate and10s512MiB bounds; retain29deferred Minors.

## Review Focus

Wrong/stale note identity must not change scheduling or old histories;
inverted child masks must use local parity; same-type instances remain distinct;
resets/pending overwrites must not fabricate observations; metadata must remain
inert when rejected or normalized. Each is pinned in Steps1/3.

### Task 1: Static modeled child prefix persistence

**Files:** Create tests/src/pattern/test_modeled_child_prefix.py and maintain
Makefile. Modify pattern_opaque.h/.cc (bounded parser), pattern_scheduler.h/.cc
(projection using cached RawSubchain), pattern_runtime.h/.cc (neutral optional
history validation and pending widths), pattern_readback.h/.cc (one actual root
buffer, validation and projection), icl_pattern_generation_main.cc (post-CSU
notes, inert rejected text). Maintain native readback tests, mirrored benchmark
reporter and three guide/inventory documents as the change requires.
**Interfaces:** ModeledChildHistoryNote{root,client,expected};
ReadModeledChildHistoryNote(Netlist,Operation) and IsModeledChildHistoryNote;
RawHistoryProjection{target,expected,offset,inverted};
ScanScheduler::ModeledRawHistory(note,root_scan) returns optional projection.
Runtime::HistoryReadback validates (operations,note) without publishing;
MakeModeledReadback shares that callback and StatusReadback with one model.

- [x] Step1: Add immutable-parent source versus generated/flat-prefix
  regressions. Source child8read165/miscompare165/status4; generated getter
  currently fails. Expected genuine missing-history RED, root histories intact.
- [x] Step2: Implement bounded intent notes, static geometry validation,
  retained actual root word and neutral Runtime callback. Expected initial
  focus/native GREEN with identical hardware and once-only failure counts.
- [x] Step3: Add disjoint/inverted/latest/unknown/partial-mask, malformed/stale
  note, no-provider, reset/cancel, merge and local-reset tests; add repeatable
  matched workload/reporting checks. Expected exact actual results and bounds.
- [x] Step4: Run full normal/static generators,30native normal/SAN, affected
  SAN CLI, corpus, changed C++quality and source packaging. Record exact source
  and product hashes plus full matched measurement rows. Expected all pass
  with unchanged corpus classifications and existing workloads.
- [ ] Step5: Commit finalized diff, obtain one fresh whole-branch review,
  fix blockers once test-first and qualify final state. Expected no unresolved
  Critical/Important findings; retain Minors, no rereview.
- [ ] Step6: Merge/push master, verify merged hashes/focused checks, detach
  reused worktree, delete owned branch and archive only this task ledger.
  Expected clean pushed master,117stashes and accurate remaining inventory.

Completion command: python3 /tmp/modeled-child-prefix-completion.py
Self-review: one shared event/history task, no independent task interfaces.
