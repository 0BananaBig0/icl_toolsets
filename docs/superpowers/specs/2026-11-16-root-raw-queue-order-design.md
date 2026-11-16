# Root raw scan queue ordering

Task106 completes root mixed-queue ordering under IEEE1687-2014 §§7.3.3–7.3.4,
7.9.12 and Tessent Shell Reference2023.4 iScan: source command order does not
choose CSU order; Tessent schedules raw loads after queued ordinary retargeting.
Task104 already does this when the ordinary commands appear first. The source
front end still rejects the reverse order and validates root geometry too early.

Reuse both existing queues and Task104 preparation/apply path. Identify a root
scan from its resolved netlist node/scope. Normalize every queued root scan with
the existing deferred-root option, preserving immutable qualification, options,
length/payload bounds and lexical validation immediately. Current route, reset
readiness, knownness and exact length are checked at actual Apply after ordinary
preparation. Public NormalizeScan's default strict API stays strict. Reset can
cancel a queued geometry check; malformed operands and immutable models still
fail before cancellation. Existing last raw payload and last ordinary bit win in
their respective queues. The final raw CSU follows ordinary preparation, including
when ordinary and raw name the same modeled storage; source order does not cause
a hidden intermediate apply.

Allow ordinary writes/reads after pending root scans. Preserve the existing
explicit child-mixed rejection in both orders; pure child scans retain strict
normalization and their existing batches. No child preparation, opaque/TAP/IR,
parallel/raw merge, together or implicit reset extension. Resource ownership,
iState queue overrides, owner-return lifetime, atomic publication and diagnostic
logs use the existing paths. No scheduler or evaluator changes.

Required checks: all source order permutations, future1→9 route and overwritten
raw lengths, last ordinary masked writes, cross-call queues and caller/persistent
ownership, reset cancellation versus malformed operands, together/child scope
boundaries. Maintain source/flat/generated/local-reset replay. Compare large1M,
1024distinct/512alias,128repeat and partial32 output byte hashes to saved Task105
canonical ordinary-before-raw reference; characterize time/RSS with no increased
budgets. Reuse the native benchmark runner. Genuine vendor strict certificates,
independent final review, style/corpus/offline and frozen sanitizer/full/remote
checks precede integration. Root Task105 frozen26 remains immutable; no local
heavy work while its gates run, and no shared remote mutation before its archive.

Executor selects native front-end reuse under the user's continuing authorization
and explicit opt-out of confirmation questions. Re-parsing at Apply would duplicate
existing normalization and lose early malformed-input diagnostics; always executing
raw immediately would contradict queued semantics. This design preserves the
existing engine and changes only front-end queue eligibility/validation timing.
