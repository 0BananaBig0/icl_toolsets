# Queued state assumptions

Task36 completes the pending-access portion of iState integration. IEEE1687-2014
7.9.22 (printed180–182, PDF194–196) defines event-free model assignment and
forbids creating a device operation; it does not require an empty apply queue.
The runtime already retains pending operations across iState. The generator's
blanket rejection prevents this integration, and reset-source planning also
needs to account for assumptions inside a pending group.

Reuse immediate ScanScheduler::Assume and ResourceReservations::Assigned while
retaining the queued access obligations. Default iState overwrites controlling
and expected values of overlapping target bits (§7.9.11, §7.9.22d); update
existing queued operands through resolved bit identity, including aliases and
slices. Connected outputs retain distinct expected state. Do not append an
operation for state-only bits. Emit the state statement before the eventual
expanded transaction; do not flush/cancel the queue or invent an apply. Existing
merge handling treats state-bearing streams as nonbatchable. Retain operand,
resource, alias/slice and atomic-output checks. Observation-history flags and
hardware readback remain separate requirements.

The alternative of applying queued actions before the assumption changes device
behavior and conflicts with the command contract. A second queue representation
is unnecessary unless focused tests expose an actual ordering gap. Index resolved bit identity to pending references. Retire overwritten references
once and serialize values at apply; work is amortized over queued references
plus state bits, with ordered-map lookup factors. Do not scan the full queue
for every assumption.

- [x] Reproduce pending scan/data read/write, X/slice, reservation, procedure/
  multi-file/merge and reset-rewrite cases; retain invalid-input checks.
- [x] Remove the blanket boundary using existing state/reservation operations;
  repair necessary ordering consumers only if tests establish a defect.
- [x] Run focused and required integrated checks, sanitizer, vendor probe and
  independent review. Document exact limits and update the command guide.

The user delegated implementation choices and requested uninterrupted completion.
Tests may be prepared during Task35 validation; source changes wait for its
active generator checks to finish. Combined verification may cover both tasks.


Review corrected the initial assumption that pending write strings remain
unchanged. The standard's controlling/expected overwrite rule takes precedence.
Tessent2026.2 rejects pending iState outright and does not implement LastReadValue;
the four exploratory probes therefore cannot decide overlap semantics or certify
this extension. Evidence: build/queued-state-probe-evidence and remote
test_queued_state/run_2026_11_13_01/queued-state-probe-bundle.

Before inserting reset prerequisites for a source group containing state and
access commands, preview that group on copied scheduler/reservation state with
strict local-reset checking. A passing group needs no preamble; otherwise retain
existing rewrite strategies. A whole-stream fast path was rejected in review:
later required repairs would still make earlier legal groups fail.

Scheduler copies rebuild derived graph/context and copy hardware, fill, global
reset, TAP execution/readiness and dynamically accumulated control masks.
Derived route caches may be rebuilt lazily. Preview is not stream termination,
so it skips final end-state restoration. It never mutates the live state.
Only state-bearing access groups use this preview; broader caller-only rewriting
requirements remain in the main plan.

Tests cover copied TAP instruction/fill and independent writes/resets, a legal
assumption group followed by an independent group needing repair, and 64 storage
registers with 16 source groups. Index scaling uses 1,024/4,096 alternating
write/state pairs. Preserve each command's existing value/width/resource bounds.

Integrated15 CLI methods and native scheduler-copy tests pass. Existing main25,
resource35, compound-reset and argument-reset checks pass on the reviewed
isolated implementation. Integrated leak-enabled native/new15/main25/resource35/
argument12/reset13 checks pass; the initial main-suite runner typo is preserved
as an invocation failure, superseded by the corrected main exit0. Formatting,
naming and independent review pass. Full dynamic/static verification passes in build/queued-state-full.log/.exit(0),
including36generator suites,48directions/seven legacy standards,334/119rules
over2,516ICL files and six editor tests(182.910s).

The first generated vendor fixture lost an empty apply boundary and failed
exact comparison. The revised fixture observes both groups and passes unchanged
forward certification; original pending-iState input remains unsupported by
Tessent2026.2. The guide records both evidence sets.
