# Callback benchmark postcondition plan (Task207)

> **For agentic workers:** Use superpowers:executing-plans inline with one fresh independent whole-branch review.

**Goal:** Pin serialized callback identities/payloads to their physical Apply frames.
**Architecture:** Strengthen the existing shared pass, reuse its result in the merged wrapper and add literal corruption controls.
**Tech stack:** Existing Python unittest/Make/native benchmark generators; no dependency or C++ change.
**Spec:** docs/superpowers/specs/2026-11-28-callback-postconditions-design.md.

## Constraints

- Parent master dcd802c6e8a47fa82c46f096aa24332002378661; branch codex/callback-postconditions in .worktrees/bsdl-defaults.
- Preserve physical_counts/checked_frames signatures, returned metrics and physical frame tuples. Empty logical Applies do not advance physical positions.
- Change two existing benchmark scripts, Makefile, one new mirrored metatest and task docs. All 48 products stay identical; one added test path makes 4134 inputs.
- Preserve existing 10s/512MiB regular and 10s/256MiB wide caps; no producer/performance-limit change, speedup or new event/vendor claim.
- Inherit 30 findings, candidate 28; preserve 117 stashes, sibling ledgers, foreign worktrees and excluded item1.

## Review focus

Literal pair identity, early/late physical-frame placement, unused callbacks,
empty logical Applies and wide payloads must retain their fixture semantics.
Task1 covers each through literal controls and actual existing consumer matrices.

### Task 1: strengthen shared callback benchmark assertions

**Files:** tests/src/pattern/benchmark_downstream_write_callbacks.py, benchmark_merged_callbacks.py; Makefile; new tests/tests/src/pattern/test_benchmark_downstream_write_callbacks.py; docs/remaining-work.md and this spec/plan.
**Interfaces:** physical_counts(text, targets, width, invoked) and checked_frames(...) keep return (dimensions, frames); callers are both benchmarks, benchmark_word_model_decoding.py and benchmark_primary_decode_calls.py.

- [x] Protect actual fixtures and all 48 parent products. Add four metatest methods with literal valid/unused/empty-Apply cases and payload/name/early/late corruptions; old checkers must fail rejection assertions without tooling errors. Retain exact corrupted inputs and parent script bytes.
- [x] Collect ordered target/value/physical-position triples in the existing pass, require the fixture sequence and derive notes from it. Preserve integer counts and wrapper metadata while deleting its duplicate scan. Wire the metatest into pattern-scheduler-check; all metatests must pass.
- [x] Run downstream, merged and word-model benchmark matrices with three repeats and existing caps, the 256-target decode counter and 24 static replay forms. Run Makefile checks, source build/install and exact input/product/package manifests. No unrelated native/event suite rerun.
- [x] Mark implementation progress, read the complete selected patch, commit with grouped policy and run task-done using tests/.task207-check.py.

## Branch completion

- [ ] One independent whole-branch review; regrade findings and declined areas, one Critical/Important RED-to-GREEN repair wave if needed, defer Minors without rereview.
- [ ] Merge master, verify merged behavior and unchanged qualified identities, push, archive only this ledger/helpers, retire this branch and continue the original inventory.
