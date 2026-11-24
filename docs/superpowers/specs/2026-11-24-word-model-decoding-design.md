# Whole-word scheduler model decoding

Task165 advances inventory10. The user delegated choices, opted out of questions,
and authorized commits, parent merges, pushes and task-branch cleanup. This is
the executor's small-change design; no separate user design approval is claimed.
Direct parent: master at92e62956878c653f4cde96926c7a339075e549b2.

## Intent and evidence

Reduce repeated Z3 model-evaluation work without changing modeled state, solver
constraints, objectives, reset events, reservations or emitted schedules.
Three parent runs per connected automatic-callback fixture give64 targets about
1.17s,128 about4.06s and256 about15.65s. Matching outputs and exact PIO frames
are retained in build/connected-callback-scale-20261124/discovery/results.json.
Callgrind on the same16-target fixture attributes36.53% of instructions to
Decode, which evaluates known/value expressions separately for every bit.
This is instruction attribution, not a measured wall-time speedup.

## Approaches

1. Evaluate each complete bitvector once, use installed Z3 as_binary conversion,
   restore leading zeros and overlay the known mask. Numeral literals need no
   model evaluation; wholly unknown values need no value evaluation. Choose this
   local change: O(width) local work and at most two model evaluations per word.
2. Prune more referenced state or bypass primary solving. Both change scheduling
   responsibilities and need dependency/event proofs; defer them to other work.

## Contracts and reach

Change private Decode in src/pattern/pattern_scheduler.cc and its tests/docs only.
All primary, serial, parallel, TAP and opaque replay callers retain their checks.
Return the declared width in MSB-first order, with leading zeros and x exactly
where known is zero. Support widths beyond machine integers and zero-valued
words. Preserve model completion and errors for invalid model evaluation. Do not
add a public API, dependency, persistent cache, resource-limit increase or state
omission. Keep all117 stashes and unrelated worktrees/outputs.

## Acceptance

Before implementation, retain a failing256-target10s performance assertion.
Add correctness cases for widths1/31/32/33/63/64/65/127/128/1024, unequal serial
widths, leading zeros and mixed/all-unknown held callback state. Check source,
flat and generated replay and repeated model updates. Preserve exact parent
outputs and complete ordered physical frames/postconditions for the measured
connected workload; compare three equivalent parent/current runs at64/128/256.
Target256 completion within10s and512MiB; if measured evidence cannot establish
that, record the remaining limit rather than claiming this scale is qualified.
Also measure wide512/1024-bit values, with10s/256MiB normal bounds, and a smaller
leak-enabled sanitizer case. Report environment, commands, hashes, runtime/RSS.

Run focused tests then full normal/static generator checks, all normal and
leak-enabled sanitizer native suites, affected sanitizer CLI suites, corpus,
C++ quality and source packaging. Use one fresh whole-branch review, one
test-first blocker fix pass, and ledger Minors. Merge into master, verify merged
inputs/products and checks, push, delete only this branch, archive its ledger.
This does not close the broader inventory10 or the other pending inventory.

## Measured outcome

The shared decoder change passes all required behavioral checks. Three-run
256-target source medians fall from 15.9208s to 10.0586s (36.8%). All 72 complete
parent/current source/flat/generated outputs match, including 1,028 physical
frames and 1,024 callback postconditions. Four 256-target current runs miss the
unchanged 10-second bound, so 256-target latency closure remains open as allowed
above. The 64-target sanitizer case exceeded 512MiB; the smaller matched sanitizer
matrix uses 16 targets, preserving prior callback qualification practice.

An actual budget miss made the review's early-exit reporting finding Important:
one test-first fix collects the complete matrix and returns failure at the end.
The real-generator regression fails at 2 rows instead of 8 before the fix, then
passes on normal/static/sanitizer products. Timeout child cleanup is still a
deferred Minor; 17 total Minors include 16 inherited. All 68 final gates pass with
75x1211 tests per generator, 4,044 input hashes and 38 executable hashes. Unchanged
build/test inputs and products justify retaining 64 actual gates after the
benchmark-only fixes; packaging is freshly rerun and 3 reporting gates are added.
No state is pruned and no resource threshold or solver constraint is weakened.
