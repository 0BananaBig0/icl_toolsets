# Primary DataRegister event consistency

Task147, parent master2833ac9. The retained item10 counterexample has a writable
unrelated primary pin and an enabled constant-source two-bit DataRegister.
After iState value0, iWrite value3 rejects in the primary-only solver; an
equivalent feedback register settles through an explicitly emitted unchanged
pin write. The mixed planner also counts actual enabled data events.

Reuse the existing NetworkState::ApplyInputs event semantics and symbolic
activation. In SolvePrimaryWrites, a definite enabled write caused by an actual
emitted pin event satisfies the DataRegister activation requirement even if that
pin is outside the data/control dependency set. Activation already includes
any_drive and inactive reset. Preserve actual emitted writes and concrete replay;
never settle on an empty/read-only group or invent a clock/event/observed value.

Approaches: adding a separate artificial event would expand PDL semantics;
restricting feedback to the DAG rejection would remove working modeled behavior.
Align the primary-only touched predicate with mixed/feedback activation instead.
This is the implementer's decision within the authorized completion scope.

Reach: shared scheduler primary path, native scheduler tests, maintained CLI
replay suite/Makefile and current guide/model/tracker. Existing accepted models,
resource limits, queues, state/history, ownership and output protection remain.
No RTL, dependency, callback invocation, transport or vendor-equivalence claim.

Acceptance: original constant-source counterexample generates an explicit real
pin event and reaches the requested concrete state. Cascaded constants, reset,
masks/aliases and mixed-path controls agree in original/flat/generated replay.
No writable event source, disabled enable, contradictory goals and protected
side effects continue to reject with old output/state preserved. Read-only
groups do not synthesize an event. Test activation with an independent native
NetworkState replay, rather than trusting expected comparisons.

Performance: reuse the existing enabled expression, remove repeated per-goal
dependency loops, retain all limits. Measure matched16/128 independent DAG
register workloads with writable-source parent/current controls and candidate
constant-source cases, three repeats/source-flat-generated forms, recording
walltime/RSS/hashes without claiming a speedup.

Require focused RED/GREEN, native/dynamic/static/SAN/quality/package checks and
frozen inputs/products, one fresh independent whole-branch review, then merge,
push and delete only the completed branch. Broad inventory items stay pending
until their stated acceptance is met. Coverage item1 remains excluded.

Final fix pass: share explicit DataRegister goal retention across primary and
mixed paths when no emitted controller write carries its intent. Native checks
apply only real events, then check markers against actual storage. Two additional
methods prove four mixed/four source-ordered primary RED cases, then seven-method
GREEN. Require33 fresh gates,56x913 per variant and108 scale processes. Preserve
pre-review/interrupted receipts; no second review or physical timing claim.
