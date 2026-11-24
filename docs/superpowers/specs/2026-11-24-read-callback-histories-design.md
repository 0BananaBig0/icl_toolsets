# Invoked read-callback endpoint histories

Task170 advances pending inventory4/6. Parent master is
249150971ffb6fb4c3674fd6d21e23085fb560a0; owned child is
codex/read-callback-histories. The user delegated decisions, opted out of
questions and selected native inline execution. This is a small guard-lowering
change using existing execution, not a new device transport or observer.

## Evidence and intent

IEEE1687-2014 §6.4.9 rules w–z (printed p.121 in the supplied extraction)
bind callback namespaces and substitute expected read data for <D> and the
relative register name for <R>. The expected data is not an observation.
Current Runtime already expands actual read callbacks into ordinary endpoint
operations, tracks nested/read execution across resets, and invalidates logical
callback histories instead of inventing results. ModelReadback already executes
expanded write-callback endpoints, masks abstract callback storage and commits
only native observations. Physical, automatic and timed histories have separate
guards. CodeGraph/source trace confirms these shared paths.

Immutable parent probes retain both rejections: an invoked read callback with
no getter fails the main global read-callback check; a getter inside its body
fails ModelReadback's read-execution check. Both preserve an existing output.

Allow actual histories of ordinary native endpoints inside/after invoked read
callbacks. A callback asked to compare0 against endpoint9 must observe9 and
two mismatches. Do not infer a logical callback register's value from its
expected argument or from unrelated endpoint reads.

## Approaches and executor call

1. Let already-expanded read bodies use the existing ordinary observer and
   native scheduler, retaining all other guards. Choose this smallest approach.
2. Add a second callback observer or interpret callback Tcl again. Reject the
   duplicated execution/state model and risk of repeated side effects.
3. Populate logical callback histories from <D>. Reject fabricated observations.

## Contract

Remove only the ReadCallbacksExecuted part of main's model-mode callback guard
and callback_read_execution from ModelReadback's preflight rejection. Keep
automatic prerequisite, physical callback, timed/opaque route and unexpanded
callback-target checks. Preserve logical history invalidation and abstract
storage masking. Keep merge provider latching at both generation calls, native
reset events, reservation constraints and atomic output/debug publication.

Use existing test helpers. Add12 focused methods covering actual values and
mismatches, body getters, no-query generation, unknown/masked/inverted data,
nested callbacks/namespaces, selectable and serial endpoints, resets,
split/local-reset replay, merged calls and preserved negative boundaries.
Update obsolete read-callback rejection tests only with actual-value/replay
assertions; automatic/physical/timed negatives stay.

Reuse benchmark_merged_readback's runner through a fixture-factory argument
and a private wide-call-count argument, with unchanged defaults. The base
fixture accepts an optional owner for factory compatibility. A small new read
callback fixture returns direct ordinary endpoint accesses for parent controls
and equivalent expanded read callbacks for current/SAN cases. Wide cases use
two calls so raw histories actually capture a high-bit payload on the second
scan. Keep all forms, exact parent controls, unobserved-note filtering, repeats,
reported failures and10s/512MiB regular or10s/256MiB wide bounds.

Run full normal/static generator checks,15 normal/15 leak-enabled SAN native
suites, affected CLI, corpus, changed-range C++ quality, source packaging and
existing allocation/reporting/work probes. One fresh final review and one
test-first blocker fix pass, then merge/push/delete the owned branch. Preserve
117 stashes, ignored artifacts and24 inherited deferred Minors. Broad topology,
clock/event and model inventory remains open; coverage item1 stays excluded.
