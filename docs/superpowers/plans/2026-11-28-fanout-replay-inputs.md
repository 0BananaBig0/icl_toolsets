# Native fanout replay input plan (Task208)

> **For agentic workers:** Use superpowers:executing-plans inline with one fresh independent whole-branch review.

**Goal:** Assign each physical scan input bit once per ordinary replay tick.
**Architecture:** Reuse longest_lanes for input assembly; preserve canonicalization and output sampling. Measure actual assignment work in a separate compiled probe.
**Tech stack:** Existing C++17/Make/Python unittest/native generators and pinned Z3/Tcl; no dependency or production instrumentation.
**Spec:** docs/superpowers/specs/2026-11-28-fanout-replay-inputs-design.md.

## Constraints

- Parent master 8c74e2d1d23c98b0ec5ad51c21094f126a23e382; branch codex/fanout-replay-inputs in .worktrees/bsdl-defaults.
- Only the ordinary Replay input loop and redundant second map change in production. Keep suffix checks, padding, every output comparison, controls, reset timing, sibling bits and TAP assembly.
- Reuse the exact 2/8/32 by 4/64/256 scale fixture, four replay forms and 10-second bound; reference source-process cap 512MiB. No new topology, API, solver limit or end-to-end speedup claim.
- Inherit 28 findings, candidate 27; preserve 117 stashes, foreign worktrees, sibling ledgers and excluded item1.

## Review focus

Unequal/inverted lanes, multiple physical keys including vector siblings,
unrequested histories, suffix conflicts and per-output sampling must retain
their behavior. Existing native/shared-input/output/fanout suites cover these;
extend only when current inspection identifies an actual uncovered interface.

### Task 1: remove duplicate replay input work

**Files:** src/pattern/pattern_scheduler.cc; tests/src/pattern/test_fixed_scan_fanout.py; new tests/src/pattern/benchmark_fixed_scan_fanout_work.py; docs/remaining-work.md and this spec/plan.
**Interfaces:** Keep Replay/ReplayTapStep and public scheduler APIs. Benchmark fixture(count,width) returns source, complete body and expected write pairs for the maintained test. The standalone benchmark uses actual Makefile compile/link flags and a separate generated scheduler object, never installed producers.

- [x] Protect all parent inputs/products and original scheduler/test bytes. Extract the scale fixture without changing its data. Add the assignment-work benchmark with unique source hooks, positive execution counters and exact reference-PDL comparisons. Old code must fail only the work bound on all nine coordinates. Save three uninstrumented parent scale runs.
- [x] Iterate longest_lanes when assembling ordinary replay inputs, one assignment per (node,index)/tick. Run the instrumented benchmark GREEN with matching reference PDL. Keep the duplicate map until that GREEN is observed.
- [x] Remove the redundant input_streams comparison as a green refactor. Verify final instrumented counts, three equivalent current scale runs, focused normal/static fanout/shared-input/output modules, C++17/style/tidy and actual source/vector coverage before freezing.
- [x] Run fresh whole-project normal/static qualification with its normal native suites, default leak-enabled native and affected CLI checks, source build/install and exact manifests. Run applicable timed probes before concurrent broad regression; no duplicate full baseline or native suites already covered by the whole run.
- [x] Mark verified implementation progress, read the complete selected patch, commit with grouped policy and run task-done using tests/.task208-check.py.

## Branch completion

- [x] One independent whole-branch review; regrade findings and declined areas, one Critical/Important RED-to-GREEN repair wave if needed, defer Minors without rereview.
- [x] Merge master, verify merged behavior and qualified identities, push, archive only this ledger/helpers, retire this branch and continue the original inventory.

Completed at `eb3c90e`, merged and pushed as `6ad6ad4`. Independent review:
0 Critical, 0 Important, 0 new Minor issues; one inherited resolution and
27 remaining findings. All 44 qualification gates and eight merged-result
checks pass; 117 stashes are preserved. Evidence:
`build/fanout-replay-inputs-20261128/`.
