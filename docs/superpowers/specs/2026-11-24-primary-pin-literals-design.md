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

The first literal-word substitution did not close memory (338028KiB) and was
reverted. A retained Massif trace of the immutable parent reaches its existing
solver timeout under instrumentation; its tracked peak includes44.5MB in Z3
model conversion below execute_lex/execute_maxsat/SolvePrimaryWrites. This is
diagnostic allocation evidence, not a native acceptance timing/memory result.

Hypothesis2: skip constant primary fill-deviation objectives for full binary
pin goals at horizon1. Hard constraints already force the sole drive and word,
so that objective has the same value in every feasible solution. Removing a
constant lexicographic component preserves the ordering of every other
objective. Confirm the memory effect before keeping the change.

## Approaches and call

1. Substitute an existing full binary goal as a literal at horizon1. Rejected
   experimentally because memory remained338028KiB; implementation reverted.
2. Skip only the constant fill-deviation objective for such a pin at horizon1.
   Choose this profile-directed hypothesis; keep the existing symbolic word.
3. Add a direct PIO execution backend. Reject this scope: it duplicates read
   routes, reservations, implicit data/reset events and rollback.

## Contract and scope

Change only primary deviations collection inside SolvePrimaryWrites.
No dependency, public API, option or new solver. Skip a deviation only at
horizon1 with an existing goal for this pin and every goal bit binary.
Require every primary candidate to have a full binary goal and every requested
read bit to have exactly one candidate route. Removing constant objectives can
change Z3 tie choices despite preserving preference ordering. If any primary
candidate is unforced or any read route is ambiguous, keep the entire original
objective list. Unknown/partial-X internal goals and horizons beyond1 also
retain the same deviations. Keep count, data deviations and all other objectives. A later frame may require an intermediate
value different from the final goal, so do not extend the substitution there.

Keep read-before-write, implicit loads/resets, preserved and end-state goals,
unknown masks, controls/aliases, ascending widths, source/flat/generated/reset
replay, failure atomicity and existing resource guards. Do not optimize raw,
TAP, callback provenance, clock semantics or the inherited22 deferred Minors.
If the single objective exclusion fails the measured hypothesis, return to diagnosis;
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
