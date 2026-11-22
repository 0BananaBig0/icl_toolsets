# Task155: modeled root parallel raw scans

## Intent and evidence
Continue inventory items6/8 after Task154. Execute named parallel raw scan chains
against actual modeled ScanRegister state and publish real raw return histories.
User delegated choices and opted out of further questions; the design choices
below are ours. Broad items6/8 remain pending for other topologies/transport.

Parentmasterd68226f is clean/pushed; Task154 branch deleted,117 stashes preserved.
The native ordinary scheduler already replays unequal parallel lanes, while
Task152 opaque queues support named parallel fields with unknown observations.
The modeled raw route is still scalar. A valid4-bit/2-bit named-chain probe with
self capture, common controls and iReset rejects at the scalar-root guard with
old output retained. Inputs/command/hash/diagnostic:
build/modeled-parallel-raw-discovery-20261123/probe.json.

IEEE1687-2014 §7.9.12, printed168–170, local /tmp/item11-1687.txt lines8735–8858:
iScan may address modeled modules; -chain selects a named subchain, different
chains of the same interface may have different lengths; each length must match
its current path; scan data is LSB-first, input and expected planes are distinct;
-stable/defaults concern repeatable controlling state. Existing native parallel
lane construction prefixes short SI streams and appends don't-care SO padding.
No new vendor/live-device/timing-signoff claim.

## Approaches
Choose exact native root-parallel replay using the existing concrete hardware
engine, ClientControls and NetworkState capture/ShiftComplete/update behavior.
Add a vector result so all canonical chain commands share one Apply.
Ordinary goal solving can optimize/omit/add CSUs and is not an exact raw protocol.
Opaque lowering cannot supply actual register state or observations.
A separate hardware simulator would duplicate existing reset/clock/state rules.

## Supported topology and bounds
One explicit root client ScanInterface with named chains and common interface
controls (explicit direct root pins or existing virtual defaults). Each Chain
binds one unique SI/SO bit; controls are common, not chain-specific. Fixed,
unconditional pure-forward scan paths may use indexed/vector ports, static
aliases and inversion. Trace from each declared SO to its matching SI with the
existing graph/path machinery. Reject mux-selected/gated/dynamic routes, shared
input bits, overlapping ScanRegister storage, ambiguous returns and uncovered
ScanRegister state. Declared fixed paths must include every modeled ScanRegister.
No TAPs, AccessLinks, child interfaces or hierarchy in this increment.

No event-driven DataRegister storage (WriteDataSource). Held DataRegisters and
unrelated data inputs/capture-only combinational logic may remain only when they
cannot change controls/routes during a CSU; use the existing concrete engine
for their capture values. Every register reset must be direct/virtual and known
inactive at execution. Common control terminals must be direct root pins or
virtual defaults; no storage-derived clocks/resets/enables or other stateful
control feedback. Preserve existing unsupported iMerge/readback/physical guards and the raw
iApplyEndState exclusion; exact raw CSU must not silently ignore a directive.

Require iReset before actual modeled replay. Normalization validates target,
chain, options, numeric data and payload bounds before replacement/reset cancel.
Match existing defer_length: canonicalize queue operands before reset without
executing; enforce current physical chain length on actual Apply preflight.
A named default chain is a real parallel name, distinct from scalar default alias.
Aggregate effective explicit+implicit scan payload<=1048576 bits, checked before
large payload allocation. No raised model, graph, event or interpreter bounds.

## Grouping, controlling state and returned data
Latest request wins independently by (IR mode, interface, chain). IR on a
non-TAP parallel client rejects before effects. All requests in a group preflight
before capture. -together is allowed for the raw-only parallel group; existing
mixed ordinary-preparation plus -together rejection stays.

Every active field participates in one common CSU with max(field lengths) shift
clocks. For a shorter lane, the existing ordinary native convention supplies
leading SI padding and ignores extra trailing SO positions. Only its declared
length belongs to that field's input/return. No per-field independent CSU.
For an untargeted fixed modeled chain, reapply current native per-register
controlling fill (same policy as existing scalar raw embedding), normalized to
its SI polarity, with all-x expected data and a proven -stable marker. This
uses known modeled topology/fill, not invented black-box observations. Explicit
-stable flags remain unchanged; fixed modeled path knowledge stays intrinsic.

For each path's output-to-input ordered (register,suffix parity) list, decode its
normalized SI string into per-register loads using the same offset/parity rules
as scalar raw replay. Capture once through existing Clock/ClientControls. Obtain
each field's actual returned bits from captured shift state and actual SO parity,
never from -so, -si, defaults or last-written history. Pure-forward fixed paths
shift uninterrupted, so the captured snapshot equals the first field-length
return samples. Prove this against small explicit per-clock reference replays,
including unequal lengths, inversion, unknowns and constant capture sources.

Use existing ShiftComplete for bulk shifting when the topology/control proof
makes intermediate events invariant; preserve final SI bits while keeping
unrelated vector bits. Do not materialize channels*max_length padding strings
or introduce per-bit full-netlist scans. If any required protected intermediate
condition needs per-edge replay, reuse the bounded existing event-work path or
reject before commit when that proof/bound is unavailable. No silent bulk skip.
Update once and idle through the same native control semantics.

Retain numeric reservation masks throughout all relevant phases. Explicit field
assignments may update their requested owned storage as on scalar raw; implicit
fields preserve held values and do not claim assignment. Report assigned IDs for
explicit requested chains only. Reuse existing root-interface port-effect and
ownership policy; do not invent a new lifecycle. On failure, no raw state/fill,
assigned IDs or observations commit. Subscriber exceptions after commit preserve
completed state, and route caches are valid for the following request.

## Interfaces and consumers
Add public vector-returning ScanScheduler::ApplyRawScans with existing preserved,
assigned, preparation and RawObservation parameters. It returns canonical scans
only; caller emits one original Apply. Nonparallel calls delegate to existing
ApplyScans and return one scan. Preserve all scalar/child/TAP behavior. Existing
single-operation ApplyScan/ApplyScans remain scalar APIs and reject parallel
groups before effects with a clear vector-API diagnostic.
NormalizeScan dispatches valid root-parallel requests to the shared generic
normalizer/ReadScanFields and the cached static path metadata; old scalar/TAP
normalization stays intact. No new PDL command, Operation flags or dependencies.

Update both main generation and ModelReadback raw queues to typed
(IR,interface,chain) keys and ReadScanFields payload offsets, replacing their
scalar hardcoded offsets. Both use the vector API. Native API and default CLI
perform real state replay; default applied getters remain unavailable without
a provider. --model_readback supplies only explicit requested field histories,
per-chain variable-width planes, real unknowns and exact cared-bit miscompares.
Implicit completion clears expected data and does not invent source observations.
Retain source origins/call stacks, latest replacement, split-file queues,
cancellation, empty Applies and atomic CLI publication. Physical raw remains
unsupported. Existing opaque queue behavior stays unchanged.

## Tests, scale and workflow
New maintained tests under tests/src/pattern/test_modeled_parallel_raw.py and
benchmark_modeled_parallel_raw.py; maintain native scheduler/readback/runtime
tests as needed, shared source/native consumers and Makefile registration.
TDD parent rejection then real behavior:4/2-bit unequal lanes in one Apply,
first/second capture histories, known/unknown/inverted/ascending streams,
multi-register paths, unrelated vector bits, constant capture contradicting
expected data, partial/all-x care and status/clear, latest requests, named default,
untargeted fixed fill, reset cancellation, split files and ordinary preparation.
Exercise ownership, rollback/retry, postcommit subscriber failure and atomic
negative artifacts for length/overlap/dynamic/IR/hierarchy/limit/physical cases.
Compare source/flat/generated patterns and actual state; do not infer observations
from expected comparisons. Scalar/TAP/opaque regressions remain required.

Measure8/32/64 chains at representative32/512/4096-bit widths within1MiB,
three matched source/flat/generated replays, elapsed time/RSS, inputs/products/
commands/hashes; use existing scalar and opaque controls without claiming a
parent parallel pass. Working-memory cost must track aggregate field bits and
metadata, not chains*maximum length. Preserve Task154's<=10s1024-target callback
budget and original wide-alias memory budget on the exact qualified executable.

One fresh builder check, inline implementation/TDD, one fresh most-capable
whole-branch final review, one verified Important/Critical fix pass, defer
Minors, no rereview. Full14 native/14 sanitizer suites, complete dynamic/static
generators, affected sanitizer CLI, changed-code format/tidy/strict C++17,
source packaging, hashes and integration checks. Merge/push into direct parent
master, detach reused worktree, delete this completed branch and archive only
its own ledger. Preserve117 stashes and unrelated worktrees/ledgers. Coverage1
and broader pending capabilities retain their current acceptance.

Builder check: fresh read-only /root/modeled_parallel_raw_builder_check reports
nothing important left. Inline execution and user-delegated choices continue.

Execution checkpoint: native and16-method CLI compatibility gates pass;93 fresh scale/control processes and the runnable example pass exact replay. All47 qualification gates pass after one test-first four-Important fix pass; two Minors are deferred with no rereview. Exact-byte identical optimized products retain verified full regressions/performance; changed sanitizer products were freshly checked. Broader inventory remains pending.
