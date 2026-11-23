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
with callbacks, callback iMerge, automatic downstream invocation, physical PDL
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
iteration rule for that batch. Calls without an inner Apply leave their
endpoint/ordinary requests for the outer Apply. A pending wait remains subject
to the existing unapplied-operation guard; no automatic flush.

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
emitted Applies, operand bits, endpoint writes and callback postconditions.
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
Bind exact current maintained inputs/products and all measurements. Merge
verified work into master, run identical-input/product integration checks,
push/delete the completed branch/archive only this ledger/preserve117stashes,
and continue the original inventory.
