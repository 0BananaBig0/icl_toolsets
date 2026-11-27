# Write-only read scope implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans inline, task-by-task.

**Goal:** Keep incidental physical captures out of write-only logical history.
**Architecture:** Apply existing logical read reconstruction even when the incoming apply has no iRead; retain the physical/raw exclusions and implicit IR checks.
**Tech Stack:** Existing C++17/Z3 scheduler and Python/native regression framework.
**Spec:** docs/superpowers/specs/2026-11-27-write-only-read-scope-design.md

## Global Constraints

- Parent master f934097e2583da3fe6389a86bfbb776ab70bc285; reuse .worktrees/bsdl-defaults.
- Preserve 117 stashes, foreign worktrees, sibling ledgers and 44 inherited Minors; item1 excluded.
- No public API, format, dependency, resource bound or hardware assumption changes.
- Preserve required native events, explicit X observations and physical/raw masks.
- Maintain existing tests in place; new test scripts belong under root tests/.

## Review Focus

- Write-only preparation must leave unrelated previous histories unchanged.
- Requested all-X reads must still capture actual values.
- Reset-held scan members must not cause empty or missing native transactions.
- Ordinary TAP writes must retain their documented implicit IR capture checks.
- Source/flat/generated prefixes must retain logical histories over regeneration; refreshed vendor evidence must bind both actual executions and prior provenance.

### Task 1: Restore explicit-only histories for write-only applies

**Files:** Modify src/pattern/pattern_scheduler.cc, tests/src/pattern/test_ordinary_read_scope.py, tests/src/pattern/test_tap_pdl_replay.py, tests/golden/tessent_roundtrip/transitive_releases-forward, docs/pattern-generation.md and docs/remaining-work.md.
**Interfaces:** Solve consumes existing operations/read_scope/frame comparisons; emitted ScanStep.expected retains only logical observations plus existing implicit IR checks. Public ScanStep and physical/raw callers remain unchanged.

- [x] Add maintained write-only and automatic-preparation regeneration tests using the existing prefix/history helpers. Assert never-read xx versus explicit-read 11, miscompare masks/status, known prior history and exact source/flat/generated output. Run against protected Task199 generator. Expected: behavioral RED from extra a.r read/history pollution.
- [x] Use existing logical reconstruction on write-only applies. Rebuild and run the whole ordinary-read-scope module, native scheduler/readback, reset-dependency, feedback and independent-TAP checks. Expected: GREEN without changing explicit X, old-storage, native event or IR assertions.
- [x] Refresh the one affected directional certificate using the existing schema4 current/reference vendor proof. Expected: two successful Tessent2026.2 exports, identical physical results, 77 offline directional checks pass and distinct current native logical trace retained.
- [x] Run three-round 16/128-register changed-path measurements plus existing sparse-scale and feedback controls, fresh full make check, affected normal/static and leak-enabled suites, changed-C++ quality, corpus/editor and source package build/install. Expected: unchanged limits, explicit skips and exact source/product bindings.
- [x] Update guidance, commit the finalized selected diff and run task-done using python3 tests/.task200-check.py. Expected: all required qualification and artifact checks pass.

## One review repair pass

- [x] Reproduce loss of the final-group tap2 cared capture comparison in four wiring/mode cases before repairing production code.
- [x] Retain implicit IR checks on empty logical read scope, preserve mixed read-containing behavior and pass the focused regression.
- [x] Run fresh complete qualification and measurements on final repaired sources; no reviewed executable evidence is reused after the C++ change.
- [x] Commit the repaired selected diff and run task-done; defer the overview Minor, for 45 total.

## Branch completion

- [x] One independent review of 55d1732 completed: 0 Critical, 1 Important for the single repair pass, 1 new Minor deferred. No repaired-code re-review.
- [x] Merge, verify merged result, push, retire only the owned branch and archive only this workspace; continue remaining inventory.

Task200 integrated as `930f8e2` after 16 passing merged checks, then pushed and
its owned branch retired. Preserve the reusable detached worktree and sibling
ledgers; archive only this plan workspace and owned helpers. Final qualification:
48/48 gates, 227 Python suites, 3554 tests and five optional reference skips;
45 deferred Minors.
