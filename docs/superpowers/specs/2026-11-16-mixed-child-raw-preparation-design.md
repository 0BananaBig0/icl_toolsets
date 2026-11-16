# Ordinary preparation for modeled child raw scans

Task107 extends completed root preparation to already-qualified modeled child
client interfaces. Explicit queued ordinary accesses may establish a child route
before its raw CSU, in either source order. IEEE1687-2014 §§7.3.3–7.3.4/7.9.12
and Tessent's documented ordinary-before-raw Apply rule remain the contract.
Reuse the existing ordinary planner, native replay, current RawSubchain/interval
checks, ownership assignment and atomic publication. No implicit child opening
when there is no ordinary preparation; pure closed-child execution still rejects.

Factor existing immutable child-client/pin checks out of RawSegmentFor into
RawClientPorts(size_t) const returning the same ScanIn/ScanOut IDs. Preserve every
existing condition and diagnostic; routing/contiguity/current length stays in
RawSegmentFor. Deferred normalization validates the immutable full RawTemplate,
finds the target, validates child metadata (root already validated), and uses the
existing shared operand/options/length-bound normalization. Rename the bool
parameter to defer_length for clarity; default strict NormalizeScan remains
unchanged in behavior and ABI. Current child route/geometry/reset readiness is
proved by existing strict ApplyScans after ordinary preparation.

The front end can then use deferred normalization for every modeled raw queue;
remove Task106's root-only predicate/mixed guard. Keep one ordinary queue and one
raw queue, existing last values/overrides, refresh exact ordinary reservations
before raw, and preserve child-only explicit assignment footprints. Existing
mixed together rejection becomes a generic diagnostic. Batch disjoint child
slices using existing interval checks; overlapping root/child requests still
reject after preparation. Stable remains local to the requested interface under
the existing pre/post proof. No new route solver, evaluator, cache, reset, clock,
TAP/IR/parallel/opaque/merge or resource-budget semantics.

Required tests: static and initially closed/SIB child preparation in both source
orders, aliases/unequal widths/inversions, native strict versus deferred cold/copy
and retry, protected surrounding/parent SIB and written child masks, owner return,
iState overrides, reset cancellation versus malformed/immutable errors, route
changes, stable failure and overlapping batches. Keep source/flat/generated/
local-reset replay and atomic artifacts. Measure1M physical route,1024 distinct
prepared child interfaces/512aliases,128 close/reopen groups and partial32 held
state. Compare complete output against saved Task106 explicit Apply boundaries,
with equal --debug options. Keep1M active/4096 registers/65536 graph/16M work.

Independent final review, genuine strict vendor qualification, style/corpus/
offline, sanitizer/fullserial/remote checks precede commit/ff/delete. Task106
Root frozen25 stays immutable; no local heavy work during its timed gates and no
shared remote mutation until its archive. Reuse the benchmark runner/provider;
all new scripts beneath tests/ mirror source. No dependencies or RTL edits.

Executor selects metadata factoring plus deferred current validation under the
user's ongoing inline authorization/confirmation opt-out. Validating current
geometry at queue time cannot admit a future child route; implementing another
child route planner would duplicate an engine already proved after Apply.
