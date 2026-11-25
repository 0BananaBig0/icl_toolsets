# Standalone opaque-root serial merges and explicit history scope

Original items6/8 retain the standalone opaque-root merge boundary after Task182.
Actual scalar and named-root probes execute sequential calls successfully and
reject their merge wrappers. A further probe shows that generated implicit
named-chain fills invent an unrequested history on replay. Complete this one
queue/lowering/provider flow, including the history defect exposed by it.

## Approach

Reuse existing MergeOperations(...,false) inside eligible root opaque lowering,
then execute the established opaque queue. The provider accepts event-free merge
markers and keeps actual queued observations. New scheduling or opaque batching
would duplicate existing serial semantics and cannot justify an optimality claim.
This is the implementation choice under the user's standing opt-out of design
questions and instruction to finish the remaining work.

Implicit completion must carry the existing raw-history omission intent.
Generation and replay retain this intent for named root chains as well as scalar
root scans, using existing canonical metadata and normalization. Explicit chain
requests replace only their own histories; unknown captures remain unknown and
prior cared masks remain intact when another chain is implicitly filled.
Malformed/informational metadata stays informational, while valid direct metadata
retains normative before-call ordering. Preserve the legacy callback guard.

## Scope and constraints

Support standalone scalar clients and already-supported disjoint named opaque
chains, fixed or explicitly supplied variable widths. Keep reset defaults,
stable completion, unknown-length rejection, canceled queues, nested/unequal
calls, namespaces, actual care/masks and once-only status clears. Original,
flattened, generated and local-reset forms must agree without invented data,
implicit histories, hardware events or weakened publication validation.

Keep physical mode, clock/event restrictions, arbitrary black boxes and
independent/gated/shared/TAP topologies at their existing boundaries. No public
signature, dependency or note syntax change. Keep C++17 and the1048576-bit
aggregate queue limit,117stashes,34deferred Minors and excluded coverage1.

## Performance and verification

Use the frozen Task182 parent on identical opaque models with ordinary sequential
calls. Measure scalar and1/2/8/32named chains at8/64/256bits and1/2/8chains at
1024bits, with three repetitions, source/flat/live-generated replay and sanitizer
samples. Every call in the measured all-explicit control requests every lane;
functional/native checks separately pin implicit-history scope and stable fills.
Compare actual complete words, flags, CSU counts and returned histories exactly;
no operation/data normalization. Retain10s/512MiB and256MiB single-wide bounds.
Work remains proportional to actual serialized payload/output, with one cached
interface lookup rather than repeated model scans per request.

Test-first acceptance covers scalar/named groups, implicit history absence and
old cared masks, direct metadata order, unknown comparisons and clears, nested
and unequal calls, variable widths, failed later calls, queue limits and
continuations. Run complete normal/static generators,30native, affected SAN CLI,
changed-C++quality, corpus, packaging, retained/new measurements and guides.
One fresh final review and one blocker repair wave precede verified merge/push,
owned-branch deletion and archival. Broader original work remains open.
