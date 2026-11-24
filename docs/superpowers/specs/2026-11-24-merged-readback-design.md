# Modeled readback through merge calls

Task167 advances inventory6 and the remaining modeled merge-history portion of
item4/8. The user delegated choices, opted out of questions and authorized repo
work, commits, direct-parent merges, pushes and owned-branch cleanup. Direct
parent is clean pushed master c0735a58e8ea4a889b22dfba3a919fa00d57fb1e.
These are the executor's design choices, not a separate user design approval.

## Intent and current evidence

Allow existing modeled readback to return actual native model observations
inside called merge procedures and after merged execution, with generated
frames that follow the same state trajectory. Preserve cached/manual histories,
unknown bits, status/clear, and ordinary batching when no merge history is
requested from the provider. This remains model-backed generation, not a device
transport or a claim of complete retargeting.

Raw and PIO serial controls return expected values. Wrapping the same valid
calls in iMerge fails on "modeled readback does not support iMerge histories".
Both pairs are retained at /tmp/icl-merged-history-discovery-ye50pasz/results.json.
The raw pair returns0x00 then0x96; the PIO pair returns9 then6.

Opened IEEE1687-2014 clause7.9.18, printed176–178, permits complete serialization
and requires resulting model state and per-call Apply order. Clause8.4.1,
printed183, returns the most recently applied per-bit read, not current
storage after later writes. Clause8.4.3, printed187, counts failing cared
bits and clears after returning the old count. Local extract:
/tmp/item11-1687.txt:9135–9210,9434–9467,9652–9682.

Current ModelReadback::Read preflights new operations before exposing any delta,
rejects iMerge, then Process replays native commands with scoped reservations.
Runtime invokes its supplied Readback callback lazily. Cached, never-read,
manual and provably-zero status paths can return without invoking that callback.
GenerateMerged later chooses batching independently. Ignoring markers alone
does not establish consistency between source-time observations and output.

A concrete ordering counterexample is retained at
/tmp/icl-merged-history-order-y238ommw/results.json. A DataRegister d loads
primary p on input events. Call1 drives q then reads d; Call2 drives p=9.
Serial execution captures0 in three Apply groups. Existing no-query batching
uses two groups and captures9. Both are valid scheduling choices before any
history-dependent Tcl flow; source-time observation must agree with the chosen
output. Tests must keep the two-group no-query control and require the
three-group trajectory when the provider returns the earlier read to Tcl.

## Approaches and decision

1. Observe use of the existing readback callback and serialize merge programs
   whose trace has actually reached that provider with merge markers. Preserve
   normal batching otherwise. Reuse native replay, cached/manual history paths
   and Merger's serial fallback. Choose this bounded, deterministic approach.
2. Jointly plan interleaving and model observations before evaluating Tcl flow.
   This needs a substantially larger execution/planning architecture. Defer it;
   no parallel speedup is required for newly supported observed merge programs.

## Design and responsibilities

- In ModelReadback, admit valid Runtime-generated merge markers for supported
  native models. Process them as event-free scope boundaries: retain the normal
  reservations.Enter(call_stack) transition, with no fabricated clock, reset,
  Apply, fill change or observation. Runtime remains the syntax/queue/reset
  validator; do not introduce a second merge parser or public API.
- Retain early rejection before observation deltas for physical mode, opaque
  roots/children, invoked read/automatic callback histories, unsupported timed
  commands and other existing model limits. Unknown abstract callback storage
  remains unknown; no expected value or ResetValue becomes captured data.
- In main, wrap the existing Runtime::Readback callback locally. Keep a cursor
  over newly appended original operations and a latched boolean indicating that
  a provider request has included a merge marker. Each original operation is
  inspected at most once; do not rescan complete prefixes on each getter. Keep
  borrowed state alive through Runtime execution and reset-source verification.
- Thread a private combine/serial decision through both GenerateMerged callers:
  initial output and callback reset-rewrite output. If a merge history was
  requested, use complete serial normalization for the whole program; otherwise
  keep existing batching/fallback behavior. A serial trial must not be repeated
  through the batching-error fallback. Preserve original requests, call identity,
  once-only argument evaluation and literal procedure bytes.
- Queries before any merge, model mode without provider requests, and cached or
  manual histories must not disable existing batching. Once an observed merge
  requires serialization, later merges in that program serialize too. This is a
  deliberate bounded policy; refine per-block planning only if measured need
  warrants it. No new user option, dependency or Runtime/Operation API is needed.

## Supported scope and retained boundaries

Cover existing PIO, DataRegister, serial and bound single/common-phase TAP
observations and already-supported raw routes. Cover getters inside iCall and
after iMerge-end, nested/repeated merges, prefixes/namespaces/split files,
sparse/inverted/unknown histories, raw/register plane separation, status/clear,
and Tcl branches driven by actual model data. Preserve reservations and queue
overwrite/cancellation, reset semantics and original/flat/generated/local-reset
replay. Existing explicit write-callback ordinary endpoint observations may
combine with merges; invoked read/automatic callback histories remain guarded.

Keep direct merge-body command rules, opaque merge guards, raw physical-PDL
guards, timed-history limits and unsupported topology checks. No new clock
period, host transport, physical timing or vendor equivalence is claimed.
This does not complete the broad original inventory.

## Verification, size and performance

Before product edits, add failing native readback and CLI cases for actual
merged PIO/raw observations plus a counterexample that would change frame
history if output were batched. Test no-request/cached/manual/pre-merge-query
controls to prove existing batching remains. Verify event-free scope handling,
implicit owner release, protected intermediate state, unknown/cared-bit status,
branching and once-only expressions, split inputs, repeated/returned call sites,
callback restrictions and failed pattern/debug/reset artifact protection.

Use the existing C++17/Tcl/Z3 implementation and test framework. Native tests
stay in their current test files. New maintained CLI and benchmark scripts live
under tests/src/pattern. Change ModelReadback, private main orchestration and
necessary callers/tests/docs only. Preserve all117 stashes and unrelated files,
worktrees, archived ledgers and products.

Measure representative16/64/128-call PIO/raw observed programs against matched
parent serial controls, current serial controls and no-observation batching
controls, with three original/flat/generated repeats. Include a smaller
leak-enabled sanitizer case and512/1024-bit normal values with high bits set.
Report actual observer queries versus generated literal notes, frames/scans,
shifted bits, input/product hashes and environment. Keep10s/512MiB regular and
10s/256MiB wide bounds; finish the matrix and retain any failures rather than
raising limits or claiming unmeasured scale. Tracking itself must stay linear
in appended operations and constant in added per-session state.

Run focused checks then full normal/static generator and normal/leak-enabled
sanitizer native suites, affected sanitizer CLI, corpus, C++ quality and source
packaging. ModelReadback changes require fresh native executions. Obtain one
fresh whole-branch final review, fix blockers in one test-first pass and ledger
Minors. Merge into master, check identical qualified inputs/products and merged
behavior, push, safely delete only the task branch, archive only its ledger and
continue the original inventory.
