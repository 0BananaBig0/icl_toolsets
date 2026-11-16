# Automatic preparation for modeled child raw scans

Task113 follows completed Task112 merge/delete. Its original16-file source
freeze remains immutable through full/SAN/remote gates. User delegates execution
and routine design choices without confirmation; executor selects this approach.

## Intent and approaches

Pure iScan on a modeled child currently requires an already accessible root
route, even though explicit ordinary setup in the same Apply is supported.
Finish that remaining retargeting boundary by opening compatible parent controls
and emitting every real prerequisite before the final physical root raw CSU.
Known modeled local route and existing SI/fill restrictions remain required; no opaque/TAP/parallel
child, raw merge, general control/reset/clock engine or changed budgets.

A synthetic ordinary child access would execute an unwanted target CSU and
cannot safely be trimmed after adoption. A new parent-path solver duplicates
existing machinery. Chosen approach: reuse existing RawRoute for the known child
local path and add a prefix-only mode to existing TryGreedy configuration/replay.
The planner stops before executing target access, then existing strict raw
normalization, interval mapping and ReplayScan execute the actual packet.

## Public result and source integration

Preserve strict default behavior for existing native NormalizeScan/ApplyScan/
ApplyScans callers. Extend ApplyScan and ApplyScans with one trailing optional
std::vector<ScanStep>* preparations=nullptr. A non-null pointer opts into modeled
child preparation and receives the actual ordinary prefix on success. A null
pointer keeps existing strict accessible-only behavior. Source front end supplies
a local preparations vector, appends its steps with existing append_steps, then
emits the returned raw Operation and its Apply. Existing explicit ordinary queue
still runs first, in either source order; already accessible children add no
prefix. TAP/root-only requests retain their existing semantics. Mixed explicit
ordinary/raw -together stays rejected; supported pure raw final grouping remains.
No hidden state-changing CSU may be omitted from returned/generated output.

Use a complete scheduler copy for preparation plus raw validation/replay; publish
impl, assigned and preparations only after all phases succeed. Caller output
sentinels and hardware/fill survive any metadata, length, overlap, preparation,
stable, preservation or raw failure. Default calls cannot silently prepare.

## Reused path and prerequisite planner

Normalize immutable metadata/operands with existing deferred mode and existing
last-value/queue budgets before geometric validation. For each surviving child,
RawClientPorts supplies SI/SO. Build a local RawProfile using those endpoints,
dynamic=true and empty ordered maps, then use RawRoute with current state.
RawTemplate already qualifies all network nodes and records all supported mux
selectors, including inactive branches. No new serial walker. Known local
selectors/full registers/width/parity are required even while ancestors are
closed. Combine local register IDs for compatibility/accessibility planning.
Root-only requests do not infer new route geometry from packet length.

Add a private prepare_only option defaultfalse to TryGreedy. Existing consumers
retain exactly their final-goal behavior. In prefix mode, already-visible targets
return empty without solving/executing a target scan. Otherwise use its existing
configuration constraints, control masks, preservation rules, depth batches and
concrete trial replay; when all required registers become accessible, adopt
only accumulated prerequisite steps and return. Do not call final Solve on the
raw operation or dummy whole-register writes/reads. Preserve pending-progress,
15s,2s solver,4096 configuration-bit and other existing guards.

Call with nonempty raw-origin operation metadata, target ID set, no final data
goals, together=false and explicit_writes=false. Existing preserved masks remain
held throughout setup, without raw ownership refresh. Reject incompatible or
unreachable configurations by current bounded failure rules. Invalidate route/
segment caches after successful prefix planning, including failed native probes
followed by symbolic fallback, before strict RawSubchain/ApplyScans validation.
Revalidate actual local route/length after prefix side effects; the actual
pre-raw path determines assigned ownership, never the initial planning hint.

Existing ReplayScan maps full physical SI/SO and preserves surrounding parent
fill. Existing protected ancestor0 may not be automatically changed to1; an
explicit author preparation retains its current allowed refresh semantics.
Untaken/unwritten preparation does not create authored child or parent ownership.
AssignedScans publishes only explicit child pre-CSU IDs after successful raw
replay; -stable continues to constrain the final local raw transaction. Source
reset cancellation, overwritten width, ordinary ordering, procedures/owners and
local-reset rewriting reuse established front-end flow.

## Acceptance and performance

TDD native/source RED on reset-closed SIB plus eight-bit child, known SI96/SO0;
manual ordinary-open/root-raw reference already succeeds. Opt-in preparation
returns one exact parent-write CSU and final raw stream; default strict native
call still rejects. Independent literal original/flat pin/storage/shift/fill
checks, assigned masks, protected-ancestor rejection and output-sentinel/state
rollback after bad width/stable/overlap plus successful retry are required.
Cover nested parents, disjoint compatible children and incompatible alternatives,
unknown local selection, parity/ascending/aliases, owner return/partial fill,
explicit setup in either source order, overwritten/canceled pending geometry,
source/flat/generated/reset replay and applicable local-reset/source rewriting.

Compare same-option implicit preparation against saved Task112 explicit manual
setup artifacts. Representative8/256-bit child paths,64 distinct parents,
forwarding aliases and128 reopen/close repetitions must complete within existing
budgets. Measure1M leaf,1024 parents and97 nested levels where available; retain
actual resource limits without expanding bounds or promising unsupported scale.
Traverse each local path once and plan combined ancestors once; avoid per-client
solver trials or repeated full root scans. Record commands/environment/wall/RSS
and complete hashes. No global optimality claim.

Reuse an existing qualified modeled-child ICL rather than copy its modules.
New scripts under tests/; unique PDL fixture/case metadata may reference existing
ICL. Genuine strict vendor forward output is required; retain vendor internal-
ScanInterface reverse limitation if reproduced, without comparator relaxation.
Independent builder/final review, focused/native/style/corpus/offline and fresh
full/SAN/remote gates precede commit/integration/delete. No push or RTL changes.

Builder check: automatic_child_raw_builder_check reports nothing important left.

## Vendor qualification ruling

The original reused self-capture parent executes successfully in Tessent2026.2,
but strict forward comparison finds an added parent reset-value read in setup.
Do not suppress or normalize that difference. Keep its actual trace evidence.
The certified fixture reuses RawSibLeaf and adds a unique parent top with an
unknown capture source; its reset still closes the parent and automatic setup
opens it. Known-capture parent behavior remains covered by independent native
and source/flat/generated tests. This variant has a fresh strict forward
certificate; it does not qualify the known-capture vendor comparison difference.
Both original and variant reverse runs reject the internal ScanInterface.
The amendment changes fixture data and registry only and receives fresh corpus,
offline/harness and relevant source/static/SAN qualification after full completion.
