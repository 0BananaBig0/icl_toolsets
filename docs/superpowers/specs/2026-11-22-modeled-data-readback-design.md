# Modeled DataRegister observations through PIO

Task149, direct parent master007a7a7. Continue authorized inventory6 work; item1
is excluded. The user repeatedly delegates continued implementation and asks to
proceed without questions. Choices below are implementer decisions, not new user
approvals. Use the existing C++17 solver/model and --model_readback flag.

## Intent and boundary

Task148 now collects actual first observations at selected concrete replay
frames, including multi-frame access groups. It still rejects every storage
node. Add DataRegisters accessed through existing PIO routes, keeping
ScanRegisters and bound TAPs outside this increment. No new clock/raw/merge
history support, callbacks, transport, physical timing or vendor-equivalence
claim. Preserve default mode and existing scheduling/resource/error limits.

The parent control assumes value9, queues a comparison against0, applies, then
requests read data. It fails explicitly at the storage guard. Retain this actual
semantic control under build/modeled-data-readback-20261122/. Expected values
must remain comparisons; the observation must be9 and status2 for1001 versus0000.

## Approaches and decision

Replaying emitted text or entire prefixes would duplicate routing and risk Tcl
reexecution/quadratic queries. Adding a separate state engine would duplicate
working concrete event semantics. Reuse the existing primary/general selected
frame observation hook, scheduler, queue and reservations. Extend eligibility
only after tests prove DataRegister values at read-before-write/feedback/reset
frames. The implementation must use actual concrete observations and preserve
the existing scan/TAP/callback boundaries; merely deleting a guard is insufficient.

## Source order and model semantics

The scheduler already retains stored values until their first requested read
and can update them in later frames. The adapter must return that first actual
observation, even if final storage is different. Existing DAG/feedback/enable/
reset rules decide reachability. Unknown stored data remainsx; known comparisons
againstx make status unknown. Unknown status cannot be numerically returned or
cleared. Reset cancels queues without clearing history/counter; a new applied read
can observe reset storage. Existing typed callback accesses keep rejecting.

Manual LastReadValue/LastMiscompareValue now overlap model-observed storage.
Runtime already applies manual history immediately; the lazy adapter may still
have older undelivered deltas. On a history-only iState, validate its physical bits
and suppress only that plane in its existing pending delta entries using the
current '?' no-override sentinel. Do not assign device state, change comparison
counts, or copy the complete history. A later applied observation replaces both
planes again. Alias inversion affects read data and expectations, never mismatch
masks. Root-input manual history remains compatible. Controlling iState and
LastWrittenValue keep existing scheduler/queued-assumption semantics.

Use the current per-callback sparse positions map. Each prefix operation is
consumed once; manual suppression costs addressed bits times map lookup, without
a prefix/history scan per query. Observation batches use the existing evaluator.
A failed execution publishes no partial delta and cannot silently resume from
partially applied model state.

## Mode identification and compatibility

Keep --model_readback, lazy initialization, normalized source arguments,
once-only Tcl and immutable procedures/local-reset sources. Update the normal
mode note/diagnostics to modeled PIO/DataRegister readback; no live device.
Recognize the former Task148 combinational mode note when filtering generated
input, so legacy generated PDL migrates to a single current note and then remains
stable. Default mode leaves ordinary notes unchanged. Update affected current
summaries and old guard fixtures, preserving historical checkpoint records.

## Verification and scale

New tests live under tests/; add a focused CLI module under tests/src/pattern/,
and extend the existing readback native suite. First retain real semantic REDs.
Cover stored values distinct from expectations, direct/output/alias targets,
formats/ranges, unknowns/count/clear, read-before-write, DAG and feedback events,
manual plane overrides before/after queued/applied reads and across lazy queries,
controlling/queued state, reset cancellation/storage reset, reservations, scopes,
multiple files and once-only arguments. Exercise source/flat/generated replay,
legacy-label migration, local-reset procedure preservation, atomic rejection,
and unchanged scan/TAP/clock/raw/merge/callback boundaries.

A native oracle applies only actual emitted pin events, checks observed physical
outputs at their read frame and checks final storage independently. Expected masks
must never assign oracle state. Qualify changed callers, all14 native/all14 native
leak-enabled sanitizer suites, all dynamic/static generator suites, focused/main
sanitizer cases, changed C++ format/tidy/C++17 and packaging. Freeze actual counts
and input/product identities; coverage1 remains excluded.

Measure immutable parent/current default controls, current unused mode and
queried mode at8/16 applies over a16-bit DataRegister, plus a supported larger
multi-register/sparse family. Three repeats and source/flat/generated forms.
Queries assert values/counts without changing normalized output. Default controls
must be byte equal; current unused/queried modeled outputs must be byte equal.
Record actual commands/input sizes/environment/time/RSS/hashes; do not compare
unlike workloads as speedups. Preserve existing resource/Tcl deadlines.

Write a scoped inline plan after one fresh builder check. Use one fresh
independent whole-branch review on the most capable model, one verified fix pass
and no rereview. Merge/push/delete the completed branch, archive only its ledger,
preserve117 stashes/sibling workspaces and continue the remaining inventory.

Fresh builder check: nothing important left. Host unavailable; actual full
design text relayed. No independent code/test run is claimed.
