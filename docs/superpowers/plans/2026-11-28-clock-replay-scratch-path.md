# Clock replay scratch location implementation plan (Task212)

> **For agentic workers:** Use superpowers:executing-plans inline; one fresh whole-branch review.

**Goal:** Show the scratch directory before a copy/setup failure can hide it.
**Architecture:** Move the existing flushed print immediately after mkdtemp.
**Tech Stack:** Existing Python standard library and published guide.
**Spec:** docs/superpowers/specs/2026-11-28-clock-replay-scratch-path-design.md.

## Global constraints

Parent master4e1f8d26ece787aa1ce7286ab06832d54afe2515; owned branch
codex/clock-replay-scratch-path. Preserve4135 inputs/48 products/2617 canonical
files, assertion preflight, cleanup and all native command arguments/limits.
Private probes under tests/docs/ are archived before make dist. No producer,
dependency or archived-driver changes. Preserve117 stashes and unrelated state.

## Review focus

- Copy failure must print the actual new scratch location and remove the active script.
- Failures before active creation must still expose an allocated scratch path.
- Waiting interruption must retain diagnostics and propagate its original exception.
- Optimized execution must reject before any scratch allocation.
- Native byte identity must justify retained evidence; package checks must cover final docs.

### Task 1: move scratch announcement

**Files:** guide, tracker, spec/plan; private tests/docs/.task212-* probes,
tests/.task212-check.py and build/clock-replay-scratch-path-20261128 evidence.
**Interfaces:** Same wrapper and archived Task209 driver; parent Task211
qualified manifests, canonical files, replay and source-build evidence.

- [x] Write location regression on the actual published block. Run old code:
  expected one copy-case failure, wait positive, no setup error.
- [x] Move exactly the print line. Both lifecycle methods and two preflight
  methods pass; compare parent/current bytes excluding exactly that line.
- [ ] Archive probes; run make dist, exact four document members,4135/48/2617
  identity and whitespace checks. Read selected diff and commit. Run task-done
  with python3 tests/.task212-check.py; expected all gates pass.
- [ ] One fresh whole-branch review, regrade, repair only blocking findings once
  with RED-to-GREEN tests, defer Minors, no rereview.
- [ ] Merge master, verify final tracked/package/input identities, push, archive
  owned ledger/helpers and retire branch, retaining the detached worktree.
