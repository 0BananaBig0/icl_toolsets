# Serial native opaque-child iMerge execution

Original items6/8 retain an explicit opaque-child iMerge boundary. A two-call
raw example works in the modeled twin, using two serial native CSUs, while the
opaque version rejects both default generation and modeled histories. Complete
that native serial case through the existing merge traversal and scan engine.

## Behavior

Native networks with fixed opaque children may use iMerge call groups. Preserve
call order and execute their existing raw or supported parent-register frames
serially. Force the established MergeOperations(..., false) path, rather than
claiming simultaneous opaque execution or introducing a new merge scheduler.
Nested merge traversal, direct-note ordering, call-local reservations and
ordinary validity checks retain the existing merger's semantics.

The modeled provider may accept valid begin/end merge markers for native
opaque-child networks, then process the actual serial frames as before. Observe
only actual native captures: opaque returns remain unknown, expected SO controls
care and masks, and status clearing occurs once. Preserve requested interface
scope, old root/sibling histories and canonical generated child-intent notes.
Defaults, stable loads, unknown-length rejection, queue copies and transactional
state remain the existing native scheduler's responsibility.

Default generation, original/flat/generated continuations and local-reset
rewriting must preserve frames, histories, procedure definitions and source
call context. Invalid calls, protected controls, unsupported event modes and
failed output transactions must still reject. An empty merge group is a no-op.
No physical operation, observed data or count may be invented to enable it.

Standalone opaque-root execution remains on its existing separate queue/lowering
path and retains its merge boundary. Physical mode and unsupported clocks,
arbitrary black boxes, independent TAP/parallel/timed/transport cases remain
unchanged. This feature does not promise optimized batching for opaque calls.

## Implementation and performance

Reuse existing C++17 GenerateMerged, MergeOperations serial traversal and
ModelReadback frame execution; retain public signatures and add no dependency.
Remove only native opaque-child merge preflight exclusions after forcing serial
ordering. Local-reset Generate may inspect markers only in its existing edits
mode. Keep standalone root and physical-provider checks intact.

Use the frozen Task181 CLI with explicit sequential iCalls as the matched parent
control, on the same opaque model. Current source wraps those calls in iMerge;
both must produce byte-identical native output and actual observations. No real
operation or observation is removed or normalized. Measure1/2/8/32leaves at
8/64/256bits and1/2/8leaves at1024bits, three repeats, source/flat/live-generated
forms and sanitizer samples. Retain10s/512MiB and256MiB single-wide bounds.
Work proportional to actual serialized frames and output size is expected;
avoid extra full passes or per-bit processes beyond existing checks.

## Acceptance

Pin opened and automatic-preparation call groups, mixed parent/raw calls,
nested groups, unequal call lengths, direct notes, scoped aliases/reservations,
unknown care/status and clear ordering, old histories, default/local-reset
continuations and failed outputs. Retain standalone/physical/event rejection.
Run complete normal/static generators,30native normal/SAN suites, affected
sanitizer CLI, C++ quality, corpus, packaging, old/new measurements and guides.
One fresh final review and one TDD blocker-fix wave precede verified merge/push,
deletion of only this branch and archival of its own ledger. Preserve117stashes
and34deferred Minors; original broader work and excluded coverage1 remain open.
