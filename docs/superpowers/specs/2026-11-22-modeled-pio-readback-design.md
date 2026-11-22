# Modeled combinational PIO readback

Task148, parent master aa9a2af. This implements a bounded part of inventory6
within the authorized completion program. Stateful/chain observations, callbacks,
general clocks and actual device transports remain open; this increment does not
rename them complete. The user delegates implementation choices and continued
execution. Choices below are the implementer's decisions, not new user approvals.

## Problem and result

Runtime currently records normalized operations before the generator schedules
them. Applied reads mark history unavailable; iGetReadData/iGetMiscompares and
non-provably-zero iGetStatus cannot return observations. Expected comparisons
must never be substituted for observed data. Enable explicit --model_readback
for combinational PIO networks: obtain actual values from the existing concrete
scheduler, update per-bit last-read/miscompare history, and return exact failing
comparison counts. Default behavior remains compatible. Identify modeled execution
in CLI diagnostics and normal generated PDL; no live-device claim.

The mode excludes storage nodes and bound TAPs, and access histories containing
raw scans, merge groups or clock operations. Ordinary iRead/iWrite/iApply,
iState, iReset, iTake/iRelease and notes use existing semantics. Keep current
manual-history and provably-zero status behavior for unsupported/default modes.
Queries through unsupported modeled histories fail clearly. No guard is removed
to pretend a stateful network or hardware transport is supported.

## Evidence and semantics

IEEE1687-2014 clause7.9.10, printed165-166, says repeated reads overwrite the
expected observe model, including clearing care bits with X; an all-X read still
observes its target. Clause8.4.1, printed185-186, returns the most recent applied
observation per index, and first observation within a CSU sequence. Clause8.4.2,
printed186-187, defines bitwise miscompares and alias behavior. Clause8.4.3,
printed187-188, defines a decimal count of failing per-bit comparisons, not a
Boolean; -clear returns the current count and then clears it. Existing extracted
primary text is /tmp/item11-1687.txt and the selected excerpts are also in
/tmp/completion-data-register-normative.txt. Local Tessent v2023.4 iRead/iApply
HTML topics were read separately; no new vendor-execution equivalence is claimed.

Use the effective expected model after overlapping reads, masks and queued iState
overrides. Count each compared bit once in that effective observation per Apply.
Don't-care bits contribute zero mismatch/count. Preserve unknown observed data
as x; a known expectation against unknown data gives an unknown mismatch/count.
An undetermined status cannot be returned as a number; fail without clearing an
undetermined count. Never-read bits retain current x and formatting behavior.
Manual LastReadValue/LastMiscompareValue assignments override their own planes
in source order without altering the independent failure counter. Reset cancels
pending accesses; it does not silently clear the session's failure count.

## Approaches and architecture

A direct scheduler dependency in Runtime would pull SOLVER_OBJECTS into existing
parser/replay binaries; Makefile deliberately separates these objects. Replaying
the entire prefix for every query would make repeated queries quadratic. Choose
a lazy adapter supplied by the generator. This also leaves the ordinary no-query
path free of scheduler construction or replay.

Runtime accepts an optional callback returning newly observed per-bit history
updates and an optional uint64 failure count. A null/unavailable result preserves
the existing unavailable boundary. Use sparse updates with no-override sentinels
for either history plane. Do not copy/remerge the entire history on every query.
The callback owns its state for one Runtime's append-only operation stream; its
returned update storage lasts through the call. Validate update dimensions/data
before merging them so a rejected update does not partially overwrite history.

Put the concrete adapter/factory in the solver layer. It lazily creates the
existing ScanScheduler and ResourceReservations, consumes only new normalized
operations with a monotonic cursor, and never reevaluates Tcl or procedure bodies.
Extract the existing QueuedAccesses class from the generator into a shared header
and reuse it unchanged for overlaps, masks and queued-state rewriting. Process
ordinary applies, assumptions, reservations and resets in the same order as
Generate. Unsupported histories are diagnosed before reporting model results.

Without registers or TAPs, the existing primary scheduling path uses one frame.
Read values from its actual post-Apply concrete state, never from expected masks.
Expose a batched ReadBits accessor on ScanScheduler backed by NetworkState's
existing shared evaluator so overlapping/wide targets do not reevaluate a graph
per bit or object. Build the effective unique read-bit/expected model, then batch
the observation. History is per physical node/index; inversion affects read data
and expected data, not miscompare masks. Sparse delta indexing must preserve only
the latest update per bit without copying full history. Counter history persists across delta deliveries.

Current target validation permits reads of observable outputs/storage, while
manual history assignments permit storage and unconnected root inputs/selects.
With storage excluded these target sets are disjoint. Preserve that existing
boundary; the adapter need not add unreachable manual-output override machinery.
Core known manual histories continue to bypass unsupported modeled execution.

An explicit --model_readback flag creates the adapter; programs without queries
do not initialize its scheduler. Include the flag in help/current usage docs.
Normal PDL receives a stable iNote label after the standard generated header;
recognize an existing identical label so generated replay does not accumulate or
move labels. Local-reset source rewriting identifies the mode in diagnostics;
procedure bytes, source-action indices and once-only arguments stay unchanged.

## Reach and verification

Expected files: pattern_runtime.h/.cc (callback/history integration), new solver
readback factory header/source, shared access-queue header, scheduler.h/.cc batch
read API, generator CLI, Makefile, native and CLI tests, current guide/model/tracker.
The exact helper names and internal sparse container layout are builder decisions.
Use C++17 and existing dependencies. No RTL, new dependencies, device adapter,
clock periods, transport or speculative event framework.

Native and maintained CLI tests must first fail on actual applied readback, then
pass with model-derived observations distinct from intentionally wrong expected
data. Cover binary/hex/decimal, partial/unread history, aliases/inversions,
unknown values, count>1, accumulation/clear, overlapping reads/X masks, queued
state overrides, reset cancellation, reservations, prefix/instances, multiple
input files, once-only Tcl and immutable procedure/reset rewriting. Verify output
preservation on failures, unsupported stateful/raw/merge/clock histories, and
unchanged default-mode results. Prove original/flat/generated replay with the
mode label stable. Do not claim a full item6 closure or independent usability read.

Performance: consume every operation once; process only newly observed bits;
batch concrete evaluation and reuse existing queue indexing. Keep all limits.
Measure matched parent/current no-query controls and candidate modeled queries
at64/128 applies with32-bit targets, three repeats of source/flat/generated forms.
Include an unused-mode control and a wider/sparse target when supported by the
existing scheduler. Record command, input size, environment, time/RSS and hashes.
Only compare equivalent workloads; no speedup claim for newly supported queries.

Follow inline plan/TDD, focused checks then required native/dynamic/static/SAN,
changed-code quality and packaging gates, frozen identities, one fresh independent
whole-branch review and one verified fix pass if needed. Integrate/push/delete the
completed branch under the standing instruction. Coverage item1 remains excluded
with unchanged thresholds and its previous failed measurement retained.
