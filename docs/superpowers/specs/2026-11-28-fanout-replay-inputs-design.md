# Native fanout replay input work (Task208)

Task192 M3 identifies repeated native replay work. After suffix validation and
common padding, input_streams compares strings copied from the same representative
and discards the map. The ordinary shift loop assigns each physical input bit
once per observation rather than once per bit/tick. Existing runtime bounds pass.

Reuse longest_lanes, the current map from physical (node,index) to representative
lane. Iterate it when assembling shift input vectors, preserving untouched
sibling bits and the final per-node Drive calls. Keep original suffix-conflict
validation, canonical serial strings, padding, output comparisons, capture
sampling, controls and reset timing. Remove only the redundant input_streams map.
Leave ReplayTapStep's pin assembly unchanged; this optimizes the ordinary replay
branch used by qualified interface-free fanout and existing clients.

Alternatives: a second per-input cache duplicates the existing representative
index; retaining the duplicate loops leaves the recorded work gap. No API,
topology, clock/reset/event semantics, solver objective or limits change.

Move the maintained fixed fanout scale fixture into the new benchmark module
for reuse by its existing functional test. Add a manual benchmark under
tests/src/pattern that compiles a separate instrumented scheduler object using
the actual Makefile compile/link commands. Count executed physical-pin assignment
operations and the independent bit/tick budget (clocks times unique input keys)
inside ordinary Replay. The instrumented program must produce exactly the
unmodified reference's PDL. Save all source, commands, binaries, inputs, outputs
and counters. This measures logical assignment work, not CPU instructions or
instrumented throughput; no production instrumentation or hooks are added.

The old code must fail actual==expected across the existing 2/8/32 observations
and 4/64/256-bit grid, while functional output stays correct. Change the loop,
observe GREEN, then remove the redundant comparison as a green refactor and
verify the final source again. Preserve all normal/static four-form functional
checks and the existing 10-second bound. Measure three equivalent uninstrumented
scale runs before and after; report the observed values without a speedup claim
when noise dominates. Reuse the 512MiB normal source-process envelope.

Expected work changes from observations times clocks to unique input bits times
clocks; map lookup remains logarithmic in the number of input nodes. Scope stays
at the measured grid, not simultaneous larger maxima. Run C++17/style/tidy,
affected focused tests, fresh whole-project qualification and default leak-enabled
native/caller checks, source build/install and exact input/product identities.
Do not repeat an unchanged full baseline; completed Task207's 4134 inputs and
48 products establish it before the new fixture/benchmark and C++ changes.

Reuse standing completion-program, opted-out questions and inline execution
authorization. One independent whole-branch review, one verified Critical/Important
repair wave if needed, no rereview. Inherit 28 findings, candidate 27; preserve
117 stashes, sibling ledgers, foreign worktrees and excluded item1. Merge,
verify, push and retire only this branch before continuing.
