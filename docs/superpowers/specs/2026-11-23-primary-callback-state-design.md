# Task154: primary solver state for callback-heavy models

## Intent
Continue authorized inventory item10 after Task153. Preserve pattern/state,
observations, resets, ownership and errors while removing the measured repeated
symbolic work for unused callback storage. User delegated implementation choices
and opted out of further questions; the choices below are ours. Broad remaining
items and excluded coverage item1 stay unchanged.

## Evidence and success
Task153 code5dbdc014/roota71a2a7 passes43 qualification gates and63x1048 tests per
generator. Its maintained callback benchmark has1024 four-bit targets and four
caller Apply groups, producing4096 ordinary endpoint Applies. Median source
runtime25.9599s, peak107056KiB;64/256 targets take0.2523/2.0236s.
Owned gprof build /tmp/icl-callback-profile-20261123 and input/report
/tmp/icl-callback-profile-fixture-20261123 reproduce28.71s/105724KiB.
Profile counts:4096 SolvePrimaryWrites calls,8409088 symbolic literals,
12595200 assignment checks,12607488 concrete evaluator Value calls and12288
SymbolicNetwork constructions. Source shows every primary Apply materializes
all register literals and copies/checks them across three symbolic views.

Acceptance: same supported PDL bytes and concrete observations across
source/flat/generated replay; parent/current controls agree;1024 targets/four
groups median<=10s and at least2x faster than matched parent, RSS<=256MiB.
Three source repeats on equivalent builds/options; record environment, commands,
input/output/binary hashes. Retain current64/256/1024 controls and wide-alias
budget. Do not raise runtime/model limits or claim global linear scheduling.

## Approaches
Choose a conservative cached candidate index in ScanScheduler::Impl, limiting
symbolic state materialization in SolvePrimaryWrites. Retain existing solver,
objective order, event/reset settlement, concrete replay and observation phases.
A generic lazy SymbolicNetwork/NetworkState redesign touches all scan/TAP/mixed
backends and needs broader mutation/reset/cache semantics. A separate concrete
PIO backend duplicates scheduling/ownership/read-selection behavior. Neither is
needed to remove this measured work.

## Eligibility and retained state
Only exclude a DataRegister with a callback descriptor when it has no
WriteDataSource, no reset association or ResetValue, and no reference from any
other elaborated hardware field. Its storage is held across the primary query,
cannot affect another signal, and cannot be changed by a primary event.
Do not remove nodes or concrete storage/history/fill; fresh explicit iState
assumptions and callback postconditions still update the existing state.

Build the structural reference index once per immutable netlist/scheduler:
inputs, mux-choice sources, one-hot contributor bits and register identities,
logic instruction signals, reset references, interface ports/chains, and TAP
datapath/control/pin identities. Scope membership/PDL namespace metadata is not
a hardware reference. Count aliases conservatively even when unused; referenced
callback storage retains the old solver treatment. Retain unknown/ordinary
storage, every physically writable or resettable register and potential driven
input ports. No new dependency graph solver or value cache.

For every Apply, union the cached candidates with actual goal/read/end-state
and reservation/desired-preserved target IDs before applying the existing
state-seed predicate (register, primary input or actually driven input). Thus a
direct query obligation for otherwise unused callback storage remains fully
modeled. Obtain literal values from current NetworkState each time; cache only
topology/IDs. Preserve deterministic ID order. On models with no eligible
callback storage, retain the existing materialization behavior.

Keep full netlist/model resource validation and all unsupported-case guards.
Do not bypass reset settlement, address/one-hot/route checks, reserved intermediate
states, objective/fill intent, concrete state/fill replay or observation timing.
The solver snapshots may omit only the proven irrelevant held storage; concrete
NetworkState remains complete and omitted values must remain unchanged.

## Interfaces and scope
Modify src/pattern/pattern_scheduler.cc only for private candidate metadata and
primary-state seeding; reuse existing NetlistNode fields/Values/NetworkState.
No public API, Operation flags, RTL, dependencies or SymbolicNetwork semantics
change. Copying ScanScheduler copies or shares immutable candidate IDs safely.
Topology does not mutate; Reset/Assume change values but never invalidate IDs.
Other backends retain their current path. Alias-heavy/referenced storage may
retain current costs; broader dependency slicing is not this increment.

New maintained CLI tests under tests/src/pattern/test_primary_callback_state.py
and a matched performance driver under tests/src/pattern/
benchmark_primary_callback_state.py; reuse benchmark_callback_execution.inputs.
Maintain native scheduler tests where they can prove complete concrete state,
copy/reset/assumption behavior and required query state. Register CLI tests in
Makefile; retain all Task153 callback tests and both existing scale drivers.
Guide/README/current inventory publish measured improvement and conservative
remaining limits, without closing broad item10.

## Verification and workflow
Write a maintained performance budget check and run1024 targets on parent for
meaningful RED; unchanged-behavior correctness tests may pass on parent and must
be recorded as controls. Test unreferenced/aliased/referenced callback state,
logical state feeding output/logic/mux/one-hot/reset dependencies, explicit
assumptions between Applies, scheduler copies, ordinary storage, pending/end
goals, ownership and actual observations. Unsupported callback ownership remains
unsupported. Add only valid ICL/source-contract fixtures, not manufactured
hardware semantics for callback procedures.

Inline plan/TDD, one fresh independent final whole-branch review on the most
capable model, one verified blocking-fix pass, defer Minors, no rereview.
Run all14 native/14 leak-enabled sanitizer suites, full dynamic/static generator
regressions, affected sanitizer CLI checks, format/tidy/strict C++17, packaging,
hash and matched scale checks. Preserve partial evidence separately.
Merge/push into direct parentmaster, remove this completed branch and archive
only this plan's ledger; preserve117 stashes and unrelated worktrees/ledgers.

Builder check: fresh read-only /root/primary_callback_state_builder_check reports
nothing important left. Inline implementation and user-delegated choices continue.
