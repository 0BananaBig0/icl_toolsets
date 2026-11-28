# Deferred readback call frames implementation plan (Task214)

> **For agentic workers:** Use superpowers:executing-plans inline with one fresh whole-branch review.

**Goal:** Retain queued operation call frames when a later getter reports failure.
**Architecture:** Append existing stored frames inside the existing Process error wrapper, using the project's current diagnostic format and caching.
**Tech Stack:** Existing C++17 provider, native tests and Python CLI tests.
**Spec:** docs/superpowers/specs/2026-11-28-readback-error-call-frames-design.md.

## Global constraints

Parentmaster19b7e3ea2eeaad28f3839e897a33dd1456646ed6; branchcodex/readback-error-call-frames.
No API/dependency/Makefile/scheduler/validation changes. Preserve existing error
cause/origin/callback prefix and exact empty-stack message; append stored frames
once in order. Preserve117 stashes, native limits and actual observations.
All new scripts under mirrored tests/ paths; archive private probes before package.

## Review focus

- A getter outside finished nested calls must report the failing operation's frames.
- Empty stacks and callback-preservation prefixes must retain their previous bytes.
- Sticky read/status failures must not duplicate or lose frames on repeated queries.
- Failed outputs/debug files must remain unchanged; successful patterns remain compatible.
- All three getter types and direct/merged nesting must use real native errors.

### Task 1: preserve failure stack and verify

**Files:** src/pattern/pattern_readback.cc; tests/src/pattern_readback_test.cc;
tests/src/pattern/test_opaque_root_serial_merges.py; tracker/spec/plan;
private tests/.task214-* QA helpers and build/readback-error-call-frames-20261128/.
**Interfaces:** Existing Operation.call_stack, provider Process catch, cached
error_, Runtime getters and current diagnostic conventions. No new API.

- [x] Add native full-message/ordered-frame/sticky-read+status tests and one CLI
  method for direct/merged nested calls across3getters, with artifact protection.
  Build native test with current source; both native and CLI must be RED on
  missing nested frames, not setup/parse failure. Archive exact logs/inputs.
- [x] Add the existing-style frame append loop. Build and run full native
  readback and focused CLI; expected GREEN with same causes/locations/output
  protection and exact empty-stack control. Preserve final code/input hashes.
- [ ] Run C++ quality, full make check, all15 isolated leak-native suites,
  affected sanitizer CLI, source build/install, package member/whitespace and
  final identities. Read selected diff and commit. task-done runs
  python3 tests/.task214-check.py, expected all required gates pass.
- [ ] One fresh whole-branch review; regrade, one verified blocking fix wave if
  needed, defer Minors, no rereview.
- [ ] Merge master, verify merged artifacts/tests/identities, push, archive only
  owned workspace/helpers, retire branch; preserve detached worktree.
