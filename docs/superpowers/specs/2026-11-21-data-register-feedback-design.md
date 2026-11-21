# DataRegister feedback settling

Item4 includes cyclic/self-referential data updates and callbacks. This increment
implements feedback updates; callbacks remain a separate following increment.
IEEE1687-2014 6.4.9 (printed119/PDF133) explicitly shows WriteDataSource local_sel
on local_sel and immediate evaluation of cascaded data registers after UpdateEn.
Do not replace existing cascades with a single old-state clocked snapshot or
remove the cycle guard while leaving unconstrained symbolic fixed points.

Build immutable strongly connected update groups once from the current bounded
source/enable/address/reset dependency walk. Reuse the existing iterative
Kosaraju implementation through one small internal utility shared with the reset
planner. Preserve the old name-ordered acyclic path and reset ordering exactly.
Flatten groups into the existing order view for analysis/fill callers; propagate
primary/scan input dependencies over whole groups and their predecessors.

Evaluate acyclic singleton groups once as before. Evaluate every cyclic group's
members simultaneously from the previous delta snapshot, then repeat until
stable. Start from actual pre-event storage, retaining reset priority, enable
uncertainty and 0/1/X information. A self hold retains its previous value; do not
select an arbitrary algebraic fixed point. Use at most32 delta rounds and4M
feedback-bit evaluations per native event or symbolic query. These new bounded checks do not raise the
existing4096-register, dependency, netlist, primary or solver limits. A failure
to prove convergence is a resource/model diagnostic, not proof of unreachability
or physical timing stability. Known oscillations reject; unknown values remain
unknown. Native event changes commit only on success.

Use the same groups and limits in primary and scan/mixed symbolic execution.
Unroll actual state transitions and require final stability, with bits/known
masks; do not solve only simultaneous final equations. Reuse current update,
reset, controlling-fill and active-event semantics, accounting for the last
definite known write/reset through all rounds. Partially known intermediate
loads retain controlling fill; the settled snapshot cannot discard known state. Permit early exit when expressions are
provably unchanged. Reuse one symbolic state view with transactional assignment
and derived-cache invalidation, avoiding a full netlist copy/scan per delta.
Existing immutable symbolic users retain their current memory behavior.

Check held resources at intermediate cyclic snapshots in both symbolic and
concrete replay. Propagate scoped check callbacks through existing state/TAP
event calls; retain the scheduler's transaction boundary and NetworkState's
ApplyInputs/Clock rollback. Do not store borrowed callbacks in persistent state.
Precompute external storage-reset references reached from feedback groups.
Require those reset levels and known masks to remain invariant through each
delta; changing external reset events reject atomically until item3 supplies
general event semantics. Masked/invariant paths remain eligible, while in-group
reset priority continues to participate in simultaneous settling.
Capture still observes pre-update storage; feedback evaluation adds no clocks,
invented source periods, hardware observations or RTL timing claims. Existing
independent-clock/event guards remain. Source and associated-flat models derive
the same groups without new metadata.

Acceptance: normative self hold; convergent self shift and mutually dependent
groups; enable0/1/X, known/unknown/reset values and declaration order; preservation
of old cascades/defaults/fill; primary and scan-driven events, capture-before-
update, holds and rollback/retry after nonconvergence or failed checks. Use native
state/symbolic/scheduler assertions and maintained CLI source/flat/generated/
reset replay, including negative artifact sentinels. Add scripts only under
tests/src/pattern/, reusing existing native test files.

Expected scale: existing data-register bounds, representative16/128 feedback
groups with small words and matched acyclic controls. Record environment,
commands, input/output sizes, wall time and RSS; no unmeasured speedup claim.
Qualify focused RED/GREEN, full native and dynamic/static generator suites,
affected leak-enabled sanitizer checks, changed-code format/tidy/strict C++17,
source package and exact input/product identities. One independent branch
review and one necessary blocking pass; merge/push/delete when qualified.
Tessent2026.2's recorded self/mutual-cycle rejection remains a compatibility
limit; do not call it normative or fabricate vendor success. Coverage1 excluded.
