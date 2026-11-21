# Invocation-sensitive caller reset repair

Item9's existing opposing-reset loop has identical evaluated write/call
arguments but different required release preambles. Keep procedure definitions
byte-for-byte and retain the three cheaper literal/argument rewrite attempts.
Add a final recorded-invocation fallback using existing SourceActions(false),
including empty calls and caught failures. Evaluate original argument words
once in caller scope, check the recorded arguments/count, consume the invocation
before running the original call, insert its actual preamble, and retain return
and error behavior. Existing pending-group/reservation rejection remains.

Capture preambles by actual SourceAction identity. Intern distinct
(argument-list,preamble) records and emit one sequence of record IDs per modified
site. Reuse anonymous apply/uplevel wrappers, not new user procedures. A guarded
private namespace holds counters and immutable tables across ordered files.
Choose a spelling absent from literal source names in one bounded scan; reject
collisions/unexpected helper contents rather than overwriting or deleting user
state. Verify final invocation counts and exact array keys/immutable values before
cleanup. Guard mismatches remain fatal even when caught by the caller. An
independent verification postlude must attest completed cleanup; otherwise reject
before publication. Validate the actual joined artifact's operations/final level.
Preserve final PDL level through a small existing-pattern runtime getter, and
retain existing file/continued-EOF boundaries. Do not wrap the entire source.

This is trace-specific specialization. Exact rewritten operation equality and
strict Generate/reservation replay must pass before any artifact is published;
unexpected invocations, changed argument/order behavior, namespace interference
or impossible queue/ownership transformations reject atomically. No universal
equivalence for arbitrary external state or introspective Tcl is claimed.

Expected scale: existing bounded Tcl execution,64/256 repeated calls and a
larger representative sequence within the five-second runtime budget. Work and
output scale with source bytes, recorded invocations and distinct records; do
not duplicate preambles per invocation in output or add a full source scan per
site. Measure wall time/output size and matching controls, without speedup claims.

Acceptance: direct writes and immutable iProc calls, alternating state with the
same arguments, empty/partial-error calls, argument side effects, nested vector
substitutions/prefixes, multiple files and level0 EOF, helper collision/cleanup,
unexpected/missing invocation rejection, original/flat/generated/reset replay
and idempotence. Existing impossible queue/reservation cases stay negative.
Run focused RED/GREEN, affected native and connected reset suites, full native
and dynamic/static generator checks, leak-enabled sanitizer focused checks,
changed-hunk format/tidy/C++17 and source packaging. One independent branch
review, one necessary blocking pass, then merge/push/delete. Coverage1 excluded.
