# Task153: explicit DataRegister callback execution

## Intent and scope
Continue item4 of the authorized completion program using Task145's typed
ReadCallBack/WriteCallBack declarations. Execute prewritten PDL iProcs for
explicit direct/aliased callback accesses at Apply, including nested instances,
subnamespaces, parameters, masks, queue replacement, cancellation, real endpoint
accesses, source/flat/generated replay and local-reset source rewriting.
The user has delegated implementation choices and opted out of further questions;
these design choices are ours. No product implementation before the builder check.

This increment supports callback-only caller Apply groups. Ordinary endpoint
accesses inside callback procedures use existing queues, applies, resets,
reservations and timing semantics. Mixed caller callback/ordinary/raw queues and
callback Apply -together reject before invoking any callback because this first
increment does not certify their shared declarative barrier. Ordinary access
groups before/after callbacks retain their path. Automatic callback invocation
while justifying downstream goals remains pending. Item4 is not closed by this
increment. Model-readback histories and physical-PDL callbacks remain unsupported.

## Primary semantics
IEEE1687-2014 §6.4.9 (printed pp116–121), local /tmp/item11-1687.txt
6141–6161 and6388–6417: callback registers are outside logical ICL connectivity,
accessed by prewritten PDL procedures. <D> substitutes requested data to write or
observe on ReadDataSource; <R> is the register name relative to the ancestor module
in iProc_namespace, without indices. Read/write callback types are distinct.
The callback procedure is responsible for the physical access protocol. Its
correctness is not established by executing its body. Tessent2023.4 lists these
register types unsupported; no vendor equivalence or live-device claim is made.

## Approaches
Choose deferred callback requests plus the existing iProc invocation machinery.
A separate interpreter would duplicate instance parameters, namespaces, Tcl state,
limits and source tracing. Executing at iWrite/iRead would invoke canceled and
overwritten requests prematurely. Preserve ordinary callback-free execution.

## Logical request markers and queue semantics
Append a default-false internal Operation flag identifying a logical callback
request; retain command, evaluated/qualified target, canonical data, origin and
call stack. The append-only public operation vector never rewrites records seen
by a readback consumer. Existing aggregate initializers remain source-compatible.
Only accesses touching a matching callback descriptor use this flag. Validate
target/data/options when queued, including discarded/canceled numeric data.
Normal operations retain their representation.

At Apply, collect the current pending span before any invocation. Reject a mixed
ordinary/raw access or an alias mixing ordinary and callback bits, and reject
-together for callback groups. Reuse QueuedAccesses' per-bit reference model,
adding a symmetric WriteModel helper, to combine aliases/inversion and latest
per-bit values; controlling iState patches queued requests as on existing paths.
Group each effective callback register/mode once with a full declaration-order
<D> mask, x in unrequested bits. Preserve first effective target occurrence order,
while last bit assignment wins. An all-x read remains a request to observe.
Validate every request, namespace and procedure binding before the first call.
Reset cancels logical requests without requiring a loaded procedure. Finish
retains the existing unapplied-command diagnostic.

## Binding and invocation
Resolve each callback's ancestor module and optional PDL subnamespace against
the node's actual instance ancestry, caching static binding metadata. Strip only
a leading global ::. Reuse ProcedureKey and current procedure-definition lookup
so definitions loaded/replaced between files behave like iCall. <R> is relative
to that ancestor instance, with no range. Pass constants/placeholders using Tcl
objects/EvalWords, preserving numeric widths/x and literal strings '<D>'/'<R>'
without reevaluation or injection. Reuse iCall's ICL parameter injection, level,
frame, origin, call stack, safe interpreter, deadline/command/recursion limits.
Factor shared invocation rather than duplicating an interpreter or call engine.

Isolate the caller's already-collected logical callback requests while invoking
each procedure: establish a fresh pending span for its body, so an inner Apply
does not recursively redispatch the caller. Support nested callback-only applies
with a register/mode invocation stack to diagnose direct/indirect callback cycles.
If a callback returns with queued endpoint accesses, apply that tail before
returning. This completes the callback's access before later callbacks or caller
operations run. Existing explicit body applies are retained. An outer source
Apply remains expressible even when the callback tail already applied.
Uncaught errors retain atomic output publication. Caught Tcl errors have iCall's
partial-side-effect semantics; no interpreter/transaction rollback is promised.
Restore caller bookkeeping on error so reset cancellation and explicit retry
are well-defined; an explicit retry may repeat completed procedure effects.

## Controlling state and readback boundaries
After a successful write callback and its endpoint applies, emit an explicit
iState -LastWrittenValue for the logical WriteCallBack register using the passed
mask. This is the callback's declared logical write postcondition outside the
modeled connectivity, not an actual read observation or substitute for its
executed endpoint operations. Unspecified x bits stay unknown; do not infer their
values from expected data, a prior call, defaults or endpoint similarity.
Existing downstream logic may use known declared postcondition bits.
Do not assign read callback data from <D>. Invalidate its read/miscompare history
on successful logical Apply and require an actual provider for subsequent values.

Generation skips logical request markers and emits real expanded endpoint
transactions plus declared write postconditions. ModelReadback explicitly rejects
callback markers and CLI model mode rejects an executed callback, preserving the
boundary against invented callback observations/counts. Physical mode rejects
callbacks explicitly. Manual state/history assumptions retain existing semantics.

## Reset rewriting and other consumers
Keep original logical request markers as source-action anchors. Extend their
owning completed main write/read/call action through the callback expansion and
outer Apply; use indexed begin-position lookup, not a whole action scan per
request. Active compound/call scopes naturally close after their nested expansion.
The existing reset group planner must see the whole callback access group so
prerequisites hoist before the original caller queue and never flush it early.
Filter logical markers in reset-prerequisite/local-reset target analysis and
skip their hardware lowering after processing their source-action anchor.
Preserve procedure bytes, evaluated arguments, prefixes and per-invocation
occurrence dispatch. Endpoint reservations and automatic reset releases keep
their existing enforcement. Direct take/release of unmodeled callback storage
is unsupported unless its current implementation already supplies a proven
contract; reject misleading access rather than declaring ownership established.

## Cost and verification
No dependencies or RTL edits. Work is O(total queued operand bits plus emitted
procedure commands and indexed metadata/action lookups); cache ancestor bindings
and retire overwritten bit references once. Use existing limits. Measure matched
parent/current callback-free and unused-declaration controls, and64/256/1024
callback targets/repeated applies with elapsed time/RSS, bounded input sizes and
replay checks. No per-bit subprocesses or repeated whole-program execution.

TDD native Runtime queue/binding/typed-placeholder/cancellation/redefinition/
cycle/error/source-action tests; actual CLI endpoint writes/reads, numeric
postconditions, aliases/masks/ascending ranges, nested namespaces/parameters,
same source site repeated with different data, split files and once-only Tcl,
body Apply/reset/tail completion, endpoint ownership, mixed/-together atomic
rejections, default/model/physical boundaries and local-reset byte preservation.
Compare source/flat/generated/reset-source forms. Preserve Task152's40-gate
native/sanitizer/dynamic/static/quality/packaging acceptance and coverage exclusion.
One fresh builder check, inline implementation plan, one fresh independent final
review, one verified blocking-fix pass, no rereview. Commit/merge/push/delete this
task branch; preserve117 stashes, other detached worktrees and sibling ledgers.
