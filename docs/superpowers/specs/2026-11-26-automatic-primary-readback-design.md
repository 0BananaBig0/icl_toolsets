# Actual primary observations with automatic write callbacks

Original items4/6 still reject modeled histories after automatic WriteCallBack
prerequisites. Removing the guards is insufficient: generated en1 replay cannot
justify an enabled unknown callback load in the observation scheduler. A native
prototype proves the already-generated primary input frames can instead execute
through NetworkState with endpoint1001 and callback/downstreamxxxx, without
turning a callback assumption into observed data.

## Approach

For callback-containing primary-only networks with DataRegister writes, keep the
existing scheduler over the original model for planning and an independent actual
scheduler over ObservationNetlist for state/observations. Replay exact planned
primary frames through existing NetworkState input events. Carry selected logical
read bits and route indexes in in-process ScanStep metadata, then validate those
route guards against actual state before observing it. No new note syntax or
general scheduling engine is needed. Other models retain their existing path.

Planning iState assumptions may establish callback intent; actual callback storage
stays masked as before. DataRegister goal markers in a step are not input drives
and must never become actual iState values. Read-only frames emit no input event.
Use actual native reset propagation, feedback settling, comparisons and unknown
data. Publish observations only after successful complete execution.

Alternatives: deleting guards retains the proven failure; disabling downstream
updates loses old/reset data; treating callback intent as actual fabricates data;
guessing dummy callback values can choose wrong routes. Exact native replay uses
the existing planner's real frames while validating actual route conditions.
This is the implementation choice under the standing opt-out of design questions
and instruction to finish original work, not a new approval request.

## Scope and constraints

Qualify primary-only networks without TAPs, scan registers/interfaces/ports,
AccessLinks or clock objects. Retain physical/event/timed and broader serial
automatic-callback guards. Share one eligibility predicate between final main
preflight and provider admission, including no-getter programs. Preserve explicit
callback and ordinary readback compatibility rather than broadening all models.

Preserve source/flat/generated/local-reset behavior, once-only callback arguments,
namespace/call context, sparse/inverted reads, actual old/new data, cancellation,
reset, unknown masks/status, once-only clears, merged serial execution, resource
ownership and output preservation. Actual unproved read routes or held-data
preservation reject; no physical or transport equivalence is claimed.

## Interfaces and scale

Extend ScanStep with primary read provenance (logical BitReference and route
index), ignored by serialization. Add ScanScheduler::ReplayPrimary consuming the
original logical accesses, planned steps, preservation masks, observation callback
and caller goals. Reuse existing native route guards and state/rollback/subscriber
semantics. Preserve existing Apply/NormalizeScan signatures and ordinary planning
constraints, including unknown-load guards.

Keep C++17, existing256primary-port/4096primary-bit and1048576queued-bit limits,
117stashes,36inherited Minors and excluded1. Measure representative1/2/8 lanes at8/32bits,1/2at64bits and1at256.
Initial larger independent-callback controls hit existing solver resource/time
limits in both parent and current binaries; they are not supported-scale claims. Retain10s/512MiB
and256MiB single-wide bounds. Compare actual hardware to the frozen parent's real
default automatic-callback generation. Parent modeled automatic observations are
unsupported; do not claim otherwise or fake parent observations. Validate actual
unknown and ordinary known data separately with native per-event references.

Use RED→GREEN CLI/native tests, measured controls and reporter tests, complete
normal/static regressions,30native/SAN, changed-C++quality, affected SAN CLI,
retained/new matrices, corpus, guides and packaging with source/product hashes.
One fresh final review, one TDD blocker repair wave, then actual merged checks,
push, owned-branch deletion and own-ledger archive. Broader inventory remains open.

Measured environment correction: use recorded128MiBASANquarantine for new
budgeted measurements; default functionalSAN suites retain their configuration.
Normal RSS<157MiB; defaultSAN can reach593MiB, so unconfiguredSAN512MiB is not
claimed. This is no production memory improvement. The known larger planner
limits remain original item10 work. Retain unchanged same-task native/quality
evidence and report partial generator reuse explicitly after fixture-only edits.
