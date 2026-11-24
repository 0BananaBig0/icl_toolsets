# Literal primary-pin goals

Task168 addresses the open inventory10 wide-PIO memory target. The user
delegated implementation choices and opted out of questions. This is the
executor's small private optimization design, not a new user approval.
Parent master is6bbbae13fc7e719bed135da954d76204668d0e81.

## Intent and evidence

Task167 retained36 correct1,024-bit PIO processes over256MiB, across
parent/current serial, observed/unobserved merges and three replay forms.
The final measured range is301332–337624KiB. The same cases complete below10s.
Keep the256MiB target and actual input/output/state semantics; do not change
fixtures, solver limits, resource accounting or accepted topology to hide misses.
The immutable parent binary is build/primary-pin-literals-20261124/parent.

CodeGraph and targeted source reads traced Generate/ModelReadback through
ScanScheduler::Apply, ApplyCallerGoals, ApplyNative and Solve to
SolvePrimaryWrites. For each horizon this solver creates fresh symbolic words
for all primary input pins, even when an explicit goal fully specifies a word.
For a primary pin, touched starts false. At horizon1, the final goal requires
touched, forcing that pin's sole drive true, and its full binary word must equal
the goal. Its remaining symbolic word is therefore not a genuine choice.
MinimizeBits still receives its fill-deviation expression in32-bit objectives.

Hypothesis: constructing that already-forced word as a literal at horizon1
removes avoidable optimizer work and memory. This remains a hypothesis until
the failing memory case passes and matched measurements confirm the effect.

## Approaches and call

1. Substitute the existing full binary goal only for its primary pin at
   horizon1, retaining the same drive Boolean, constraints, objectives, event
   model, concrete replay and commit. Choose this smallest local change.
2. Remove objectives for fully constrained pins. This leaves the avoidable word
   variables and requires separately proving when a frame fixes the objective.
3. Add a direct PIO execution backend. Reject this scope: it duplicates read
   routes, reservations, implicit data/reset events and rollback.

## Contract and scope

Change only the word-expression construction inside SolvePrimaryWrites.
Use SymbolicValue::Literal; no dependency, public API, option or new solver.
Require horizon1, an existing goal for this pin and every goal bit binary.
Unknown/partial-X goals, unforced pins and all horizons beyond1 retain the
existing symbolic construction. A later frame may require an intermediate
value different from the final goal, so do not extend the substitution there.

Keep read-before-write, implicit loads/resets, preserved and end-state goals,
unknown masks, controls/aliases, ascending widths, source/flat/generated/reset
replay, failure atomicity and existing resource guards. Do not optimize raw,
TAP, callback provenance, clock semantics or the inherited22 deferred Minors.
If the single substitution fails the measured hypothesis, return to diagnosis;
do not stack speculative fixes.

## Verification and performance

Add maintained tests under tests/src/pattern. First run a real1,024-bit PIO
memory check against the parent and require its retained256MiB failure.
Add focused exact-value/event/replay cases for full known pins, zero/matching
goals, aliases/slices/ascending ports, unknown/partial goals, unforced controls,
reservations/end states and sequential data/reset cases requiring later frames.
Use existing fixtures/helpers; no parallel implementation agent.

Reuse benchmark_merged_readback.py for its complete378-run matched matrix,
including16/64/128 calls,512/1024-bit high-bit data, source/flat/generated
repeats and smaller sanitizer cases. Parent memory misses remain nonzero;
require all current measured cases to meet10s/512MiB regular and10s/256MiB wide.
Do not claim speedups or closure outside measured inputs. The old256-call
callback latency target stays open unless a separate matched run proves it.

Run fresh15 normal/15 leak-enabled sanitizer native suites, full normal/static
generator checks, affected sanitizer CLI, corpus, C++ quality and packaging.
Obtain one fresh final review, fix blockers in one test-first pass, retain
Minors, merge into master, verify exact qualified sources/products, push and
delete the owned branch. Preserve117 stashes and ignored worktree artifacts.
