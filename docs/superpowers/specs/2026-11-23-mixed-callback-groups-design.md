# Mixed ordinary and explicit callback caller groups

Task161 continues remaining item4/6 from pushed master0c79c35. Task160 is
complete:58 gates,15normal/15leakSAN native suites,70x1136 each generator,
21quality checks,4031inputs/38products,177measurement/example processes and20
merged checks; its branch is deleted and only its ledger was archived.
The user delegated implementation choices and opted out of questions. Choices
below are ours, not claims of a separately approved design. Inline execution
and one fresh final independent review remain the selected workflow.

## Intent and evidence

A caller can queue separate ordinary iWrite/iRead operations and explicitly
requested DataRegister callbacks before one iApply. Both kinds of request must
reach their real endpoint/native frames without dropped caller goals, duplicate
callback invocation, fabricated logical observations or changes to the explicit
procedure Apply/reset order.

Immutable parent and four discovery invocations are under
build/mixed-callback-groups-20261123/{parent,discovery}/. With ordinary spare
DataInPort/spare_out and existing callback value->p/q, ordinary-first,
callback-first and an ordinary read plus callback write all reject
"mixed callback and ordinary accesses require separate Applies". The separate
control succeeds and observes spare3/q9. Parent binary/runtime hashes match
the completed master, so this is an actual unsupported path.

IEEE1687-2014 section6.4.6 callback rules x–z specify substitution of requested
data and instance-relative register names before procedure execution; existing
namespace/argument semantics remain unchanged. Local normative source is
/tmp/item11-1687.txt around6391–6404. Existing Runtime::ExecuteCallbacks
coalesces requests, preflights bindings/arity/cycles, expands procedures and emits
logical postconditions. Native Generate/ModelReadback already consume endpoint
operations and skip abstract logical markers. Reuse these parts.

## Approaches and choice

- Synthesizing a second mixed callback/native scheduler duplicates existing
  queue, frame, reset and observation semantics. Reject this.
- Flushing every ordinary caller request before running callbacks adds an
  unrequested CSU and changes read capture/reset/ownership ordering. Reject this.
- Keep ordinary requests live in the existing pending range while callbacks
  execute, distinguishing already selected callback markers by operation index.
  Native existing Apply/reset boundaries flush or cancel ordinary and endpoint
  requests together. Choose this; maintain explicit caller write constraints
  during callback execution and fail closed on incompatible endpoint overrides.

This increment covers separately queued ordinary operations and callback
requests, including ordinary-only aliases and callback-only aliases. A single
alias mixing callback and ordinary bits remains rejected by the existing
explicit guard. AccessTogether aliases, iApply -together, raw requests mixed
with callbacks, callback invocation inside an active iMerge, automatic
downstream invocation, physical PDL
and actual invoked ReadCallBack histories remain unsupported. Ordinary actual
histories within write callbacks and mixed groups are supported via existing
--model_readback. ReadCallBack generation keeps its existing non-model support.
Do not claim whole item4/6 or excluded coverage1 complete.

## Runtime queue and execution contracts

Use the existing QueuedAccesses for validation/coalescing, including iState
overwrites, original bit identities, sparse values and read-before-write.
Ordinary operations enter that queue but do not create callback Request entries.
Only callback bits feed callback arguments; extraction ignores ordinary bit IDs
rather than indexing a nonexistent callback position. Validate every ordinary
target/payload, all logical targets, bindings, arity and cycle checks before
invoking any procedure. Unsupported mixtures reject before procedure effects.

The operation stream remains append-only: do not revise flags or targets of
already emitted logical requests. Retain indices of callback operations already
selected for the current execution group; mark all selected source callback
indices before the first procedure runs. Nested Apply sees pending ordinary/
endpoint operations but does not execute those old callback markers again.
A newly queued recursive request still reaches the existing callback-stack cycle
check. Retain current first-request ordering and latest-by-bit argument merging.

Do not clear the live pending range merely to invoke callbacks. An inner Apply
completes the ordinary/endpoint queue through the existing dispatcher; an inner
reset cancels unexecuted hardware requests and establishes the next pending
range. Selected callback requests are already an in-flight batch and retain
the existing fixed-batch iteration semantics; reset does not invent a new
iteration rule for that batch. Preserve InvokeProcedure's existing implicit tail Apply on callback completion:
callbacks without an explicit final Apply still flush pending endpoint/ordinary
requests there. The caller's outer Apply can therefore be an empty boundary;
do not coalesce distinct callback completions into one frame. A pending wait remains subject
to the existing unapplied-operation guard; no automatic flush. A group containing
only selected logical markers has no pending hardware requests, preserving
existing ordinary endpoint merges/waits inside its procedures. Mark each actual
read-invocation provenance record consumed at append time; newly queued logical
requests inside a body share execution flags but must still execute/cycle-check.

Read-history invalidation must occur on the actual Apply boundary for the
remaining pending hardware requests, including the callback tail. Abstract
callback markers are not ordinary reads. Do not invalidate a pending ordinary
read before a callback reset cancels it; preserve its prior history in that
case. An earlier inner Apply may already have delivered fresh observations to
a getter; outer completion must not invalidate those completed reads again.
Use existing ModelReadback incremental cursor and frame observations; expected/
requested/default data never becomes actual callback storage. Existing unknown
failure counts, -clear, sparse/inverted histories and read invocation markers
must remain correct. Do not introduce a new observation engine.

## Caller goals, ownership and failures

Capture the coalesced known ordinary caller-write bits after static preflight.
While executing that callback group (including nested calls), an endpoint
hardware write or controlling iState may not replace one of those known bits
with a different or unknown value. A compatible known value is allowed. Reject
a conflicting operation before appending/performing that operation, with the
caller bit identified; do not silently rely on last-write-wins across the
logical/endpoint boundary. Explicit caller x bits impose no known constraint.
This is a deliberately conservative boundary for otherwise conflicting groups;
general temporal restoration/reordering of conflicting callback endpoints is
not enabled.

A callback-body reset cancels pending ordinary write constraints as well as the
native pending queue. Retain constraints through ordinary inner Applies until
the group returns, so a later callback cannot silently undo an earlier caller
write. Existing callback failure poisoning, explicit reset recovery and committed
inner-frame behavior remain unchanged; no fictitious rollback of an executed
callback procedure. Preflight failures produce no callback effects. Reservations
keep their original call-stack ownership and existing actual native checks.

Consumed-marker tracking and caller constraints are scoped to active groups,
cleared at a safe outer Apply/reset boundary, not an ever-growing history.
Keep trace_source/callback_action_ends and once-only Tcl arguments intact:
local-reset rewriting must preserve procedure definitions, caller scopes,
split files, prefix/namespace and repeated/error/reset sites.

## Scope and implementation economy

Modify shared runtime logic in src/pattern/pattern_runtime.cc; keep the public
Operation/Runtime/Readback interfaces unless a concrete test proves a required
caller fix. Existing callback/native/readback/source-rewrite helpers win over
new abstractions. Expected caller surface: Generate, ModelReadback and source
action/reset rewriting, all verified; do not split large files or add a second
framework. Native tests remain in existing tests/src/pattern_runtime_test.cc.
Create focused tests/src/pattern/test_mixed_callback_groups.py and
tests/src/pattern/benchmark_mixed_callback_groups.py; register both generator
variants via the existing Make test macro. Update README/current guide/model
coverage/remaining tracker. No RTL, dependencies, new CLI flag or transport.

## Scale and acceptance

Use current1024-callback/four-group workload as the upper demonstrated target,
with4-bit independent ordinary input and actual spare_out observations. Normal
64/256/1024 callbacks and SAN64, three repeats/source-flat-generated, each
process <=10s/512MiB. Four generated probe boundaries must replace literal
notes with fresh ordinary getters; assert returned1/2/3/4 and callback counts.
Matched ordinary-only/unused/observer-off controls report actual request counts,
emitted Applies, actual scan CSUs (zero for this primary-only workload), operand bits, endpoint writes and callback postconditions.
Use fresh directories and bind binary/input/output hashes and environment.
Avoid repeated full-netlist/group scans per callback: work proportional to
queued bits plus emitted operations (ordered-map log factors acceptable), with
memory bounded by active groups. Preserve existing1024callback10s and16Kalias
256MiB budgets; no new speedup claim.

TDD must cover source orders, ordinary writes/reads, latest replacement/iState,
compatible and rejected conflicting endpoint writes (including unknown masks),
initial static-invalid preflight without effects, nested callbacks/cycles,
explicit/implicit inner Apply, body reset cancellation, body error and reset
retry, last-read preservation, actual status/masks, native/source/flat/generated/
reset-source replay, namespaces/prefix/split inputs, reservations, procedure
immutability and once-only arguments. Keep mixed alias/raw/together/merge/
read-callback model/physical guards tested and outputs atomic.

One fresh builder check precedes the plan. Inline task execution then one fresh
final independent review; one Critical/Important TDD fix pass, defer Minors,
no rereview. Final required checks:15normal and15leakSAN native suites, both
whole generators, selected SAN callback/runtime/model/raw histories, corpus/
producer/protocol/Make, affected C++ format/tidy/C++17 and source packaging.
Bind exact current maintained inputs/products and all measurements. After a
fresh complete build, a previously passed native gate may be reused only when
its executable bytes, test source and runtime fixtures remain unchanged; store
the exact hash/log provenance and rerun every changed native product. Whole
generators and affected CLI/source packaging always run on the current code. Merge
verified work into master, run identical-input/product integration checks,
push/delete the completed branch/archive only this ledger/preserve117stashes,
and continue the original inventory.


## Native caller preservation refinement

A concrete Task2 probe (build discovery in /tmp/mixed-indirect-caller-jilzuem2)
shows why a runtime target check alone is insufficient: after an inner Apply,
iWrite q9 can drive q's source p from the caller's3 to9. Add the minimal
required interface handoff: Operation::callback_preserved is a default-empty
map<size_t,string> of desired ordinary caller masks by original node ID,
never observed values. Snapshot it on callback Apply/controlling iState/idle
boundaries. Source operation entries remain immutable; no public PDL command.

Add ResourceReservations::CallbackValues(const Operation&,
const QueuedAccesses&) const -> optional<map<size_t,string>>. Merge these
caller constraints with existing reservations; known direct pending writes
matching the desired mask need no prior-state protection on their first frame.
A conflicting direct queued value rejects. Remaining known bits use existing
native preserved-resource proofs, including derived ports/clocked effects.
No extra writes or CSUs are introduced. Both Generate and ModelReadback use
the same helper; annotate caller context on failures without hiding the cause.

Add ScanScheduler::Assume(target,bits,preserved) as an overload. Empty masks
retain the old fast path; guarded assumptions use a native trial, existing
resource-value validation/concrete preservation and commit only on success.
Reject private opaque IDs as at the other public native boundaries. Callback
reset clears masks; actual invoked-read history/physical/raw boundaries remain.
Protect waits through existing preserved-map Idle/Wait APIs. Preserve masks
through merge processing or retain annotated Applies as barriers if required
by the actual regression. This extends the initially expected runtime-only
file surface to pattern_runtime.h, pattern_resources.h/.cc,
pattern_scheduler.h/.cc, main/readback and the affected native tests; the
existing private-state/solver machinery remains the sole event authority.

Native test TestMixedCallbackFrameGoals verifies exact Apply/state snapshots,
no prior-operation mutation and reset cancellation. Existing opaque native
privacy test verifies guarded-Assume rollback/private-ID rejection. Same-method
CLI regressions cover both derived-write directions, compatible derived writes,
indirect controlling state and a merged ordinary endpoint body. Run fresh checks
for every rebuilt/changed native product; reuse eligibility is byte-based, never
assumed from filenames.

## Final review correction
A matching pending write exempts only prior-state preservation during the first
native frame; it never exempts the complete caller mask from the frame's final
concrete proof. Guarded state assumptions retain bits already established in
native state even if a future queued write repeats them. Failed final proofs
restore hardware state, fill, TAP execution and opaque queue before any
subscriber delivery. Enclosing Apply masks are captured after nested expansion,
so reset cannot resurrect canceled goals.

The masks retained on annotated boundaries cost caller width times boundary
count. Qualification fixes the protected caller at four bits and varies callback
targets through1024; it does not establish additive scaling in both dimensions.
The primary-only benchmark's zero scan-CSU/shift counts are expected fixture
dimensions rather than independently observed scan counters.
