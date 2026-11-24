# Standard callback history notes

Task173 advances inventory6 within the existing explicit callback histories.
An empty iNote currently fails with vector::_M_range_check. Standard typed
callback-history notes fail to restore unavailable bits, while matching-looking
informational text can abort execution. IEEE1687-2014 section7.9.21, printed180,
requires note text to remain informational and malformed parameters to be
ignored or warned about. The parser already accepts zero or more arguments.

Use existing Runtime parsing and typed-note rendering. Emit callback-history
metadata as iNote -comment with quoted text. Restore valid two-argument
-comment/-status notes and existing single-argument legacy notes. Ignore empty,
extra-argument and malformed history notes without changing histories. Valid
metadata invalidates only the declared callback bits; it never supplies actual
data. Preserve actual ordinary endpoint histories, miscompares and status.

Approaches: an empty-argument guard alone leaves typed replay broken. A new
note parser or public API duplicates existing operation arguments and rendering.
The selected direct Runtime correction covers both faults without dependencies.
This is the executor's choice under the user's delegated design authority.

Scope: pattern_runtime.cc, existing read-callback tests, the corresponding
benchmark's direct-control note, and task documentation. No RTL or transport.
Do not change the model banner, callback invocation, scheduler or opaque notes.
Existing callback modes, scheduling guards and excluded coverage1 remain.

Check source/flat/generated and generated-prefix replay, both tags and legacy
notes, duplicates, empty/malformed notes, seeded sparse invalidation, ordinary
histories/status and local resets. Work remains linear in note length with
existing node lookup; no per-note subprocess or full-netlist scan. Reuse the
callback benchmark at its representative dimensions and unchanged bounds;
do not claim a performance improvement for this syntax correction.


Final-review correction: IEEE7.9.18(b), printed177, requires direct merge notes
before calls. Runtime retains resolved calls and their evaluated arguments
until the matching merge-end, then executes bodies in encounter order. This
also works across source files and nested call frames. Callback failure unwind
retains completed calls while reporting the original failure. Existing
generator notes-first ordering and scheduling remain unchanged. The expanded
tests include in-call getters and seeds, both tags/legacy, split prefixes and
modeled/unobserved variants; existing once-only and recovery tests still pass.

Compatibility correction within the same fixwave: complete literal merges
without valid history metadata retain immediate calls, including ordinary and
malformed literal notes. This preserves call-produced Tcl argument values.
Reuse the same CallbackHistoryNote validator for instrumentation and
restoration; uncertain or split merge contexts use notes-first deferral.
The walk is linear in source commands and literal note lengths. No scheduler
or public API change.
