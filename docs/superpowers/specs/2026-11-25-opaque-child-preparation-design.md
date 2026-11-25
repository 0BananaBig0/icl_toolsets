# Automatic parent-only preparation for opaque child scans

Original item8 remains open after Task179. A direct eight-bit child iScan from
reset currently fails when its scalar bypass is closed, while the modeled
equivalent emits a real one-bit parent opening followed by the requested scan.
Complete this case through the existing native planner and scan engine.

## Behavior

An opaque child request may prepare reachable modeled parent controls before
its raw CSU. Use existing PrepareScans/TryGreedy on a copied scheduler; preserve
the exact native preparation events and serialize their existing ordinary
writes. Only after preparation, project requests and complete active opaque
queues using the actual pre-raw route. The current request cannot provide data
or stable-length knowledge to an earlier preparation event.

Every preparation CSU must contain only public modeled storage. Inspect its
actual loads and reject any private opaque storage before committing the trial.
This bounded increment does not implement opaque-containing preparation. Reuse
the existing eligibility, aggregate1048576bit bound, reset and reservation rules.
Automatic preparation is available through the existing preparations API;
calls without that API continue requiring an already reachable route.

Commit the trial state, queue knowledge, assigned footprint and observations
only after the requested scan succeeds. Failed lengths, preserved controls,
unsupported preparation and copied schedulers must retain independent state.
Opaque observations remain unknown; SI, SO, defaults and stored payload never
become observed data. Preparation cannot invent a requested root raw history.
Keep opaque and modeled child history families and actual pre-CSU geometry in
source, flat, generated, default-provider and local-reset continuations.

## Implementation and performance

Reuse the existing C++17 scheduler, planner, queue maps, serialization and
readback provider. Defer OpaqueRawState until the chosen execution route has
been prepared. Keep public signatures unchanged; add no dependency or parallel
state engine. Check preparation loads in one pass with existing source-name
lookups, rather than rescanning all opaque leaves per preparation step.

Test representative1/2/8/32leaf networks at8/64/256bits and1/2/8leaves at1024bits,
three repetitions, sanitizer samples and real generated-history continuations.
Use an unknown-capture modeled twin as the frozen-parent control because the
parent cannot execute opaque automatic preparation. Verify every actual scan,
preparation CSU, word, history and status; disclose metadata normalization.
Retain10s/512MiB bounds and256MiB single-wide controls. Report measured values,
not an unmeasured performance or hardware-equivalence claim.

## Acceptance and remaining work

Pin initially closed requests, re-opening after nonstable closure, default and
stable restoration, disjoint and mixed clients, aliases/inversion, unknown care,
absent root history, reset/reservation errors, transaction rollback and copies.
Reject preparation through an already active opaque interval with unchanged
public outputs/state. Run native normal/SAN, complete normal/static generators,
affected sanitizer CLI, quality, corpus and packaging checks. One fresh final
review and one test-first blocker repair wave precede merge/push and deletion
of only this task branch. Preserve117stashes and32deferred Minors.

General opaque preparation, arbitrary black boxes, merges, independently phased
clocks/TAPs, parallel/timed execution, transport and remaining source acceptance
retain their original inventory requirements. Coverage item1 remains excluded.
