# Modeled PIO readback implementation plan

> REQUIRED SUB-SKILL: superpowers:executing-plans. Implement inline; one fresh
> independent whole-branch review after all tasks, no implementation agents.

Goal: opt-in concrete PIO observations, masks and exact failure counts.
Spec: docs/superpowers/specs/2026-11-22-modeled-pio-readback-design.md
Parent master aa9a2af; design83912cb. Fresh builder check: nothing important left
(actual document relayed because its command host was unavailable).

## Global Constraints
C++17/existing dependencies. No RTL, hardware transport or invented observations.
Preserve default behavior, limits, procedure bytes, source indices and once-only
Tcl. Exclude storage/TAP/raw/merge/clock histories from this increment. Status is
a uint64 per-bit failure count; unknown counts fail without clearing. Keep solver
dependencies separate from existing parser/replay objects. Tests stay under tests/.

## Review Focus
- Actual observations differ from expected values; unknowns cannot prove status0.
- Sparse/inverted/overlapping history and manual overrides obey source order.
- Counts accumulate; prefixed -clear returns before clearing; reset cancels queues.
- Failed/unsupported observations preserve caches, procedures and output.
- Cursor/delta/batch processing avoids repeated prefix/history/graph scans.

### Task1: Core observation contract and shared queue
Files: runtime.h/.cc, scheduler.h/.cc, new pattern_access_queue.h,
generator, runtime/readback native tests and Makefile.
- [ ] Retain actual parent readback-unavailable evidence separately from compile RED.
- [ ] Add failing callback contract tests, then sparse validated history updates,
      optional uint64 status, global clear handling and unchanged default fallback.
- [ ] Extract existing QueuedAccesses unchanged; add unique physical ReadModel
      projection using last references/masks/inversions and queued assumptions.
- [ ] Add batched scheduler ReadBits via existing evaluator; test real queue behavior.
- [ ] Register native readback test/coverage wiring; focused dynamic/static runtime,
      queue/scheduler and connected CLI checks; commit and ledger.

### Task2: Lazy model adapter and explicit mode
Files: new pattern_readback.h/.cc, generator, Makefile, native readback test,
new tests/src/pattern/test_modeled_readback.py.
- [ ] Write failing real-model tests; distinguish missing interface/option RED from
      actual applied-readback failure before implementing the adapter.
- [ ] Implement lazy solver-layer factory, monotonic new-operation cursor,
      shared queue/resources/state/reset semantics and batched actual observations.
- [ ] Preserve per-bit source-order history and manual overrides with sparse deltas;
      count effective care-bit mismatches; unknown counts stay undetermined.
- [ ] Add --model_readback/help/diagnostics and stable normal-output iNote label;
      preserve local-reset source text and immutable procedure definitions.
- [ ] Test values/formats/aliases/masks/count/clear/unknowns/overlaps/queued state,
      cancellation/resources/prefix/files/once-only arguments, atomic failures,
      default and unsupported controls, original/flat/generated replay.
- [ ] Measure matched parent/current no-query controls, unused mode and queried
      mode at64/128 applies/32bits,3repeats/3forms; supported wider/sparse case.
      Record time/RSS/environment/hashes without unequal-workload speedup claims.
- [ ] Focused native/dynamic/static and connected callers pass; commit and ledger.

### Task3: Qualification, documentation and integration
- [ ] Freeze code/tests;14 native/14 leak-enabled ASan/UBSan suites, complete
      dynamic/static CLI suites, focused/main sanitizer checks, changed-code
      format/tidy/strict C++17 and packaging. Verify actual counts and identities.
- [ ] Update guide/model/README/tracker with bounded support and exact evidence;
      execute guide example; distinguish author walkthrough and code review.
- [ ] Complete ledger, one fresh independent final review on most capable model;
      regrade actual effects and declined items; one verified fix pass, no rereview.
- [ ] Merge direct parent master, verify qualified root hashes/focused checks,
      push/confirm origin, detach reused worktree/delete only completed branch.
      Archive only this ledger; preserve117 stashes and sibling workspaces.
      Coverage item1 remains excluded; continue other open inventory items.
