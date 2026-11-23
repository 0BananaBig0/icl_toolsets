# Preserve accepted legacy LaunchEdge in flat ICL

Task157 continues inventory11/12 from master544c0c6. The user delegated routine
implementation choices and opted out of questions; these are our choices.

The frontend already permits only ScanOutPort LaunchEdge Falling, retaining
its expression in NetlistNode.properties. Declaration currently rejects it
because it is neither a signal nor a constant. Preserve that accepted spelling
in canonical flat output using the existing property-expression token span.
No parser grammar, native event model, scheduler, physical timing or new
LaunchEdge value is introduced. Existing compatibility evidence is documented
in docs/icl-conformance-review.md; this change makes no new standards claim.

Approaches: retaining the current failure leaves accepted sources unable to
roundtrip. Adding a launch-event field and scheduler semantics would be a
separate behavioral project. Choose the existing expression-backed serializer
branch: print the one validated token, preserving the legacy declaration.
Require the existing kind and value contract at the serialization boundary;
unexpected programmatically constructed property expressions must fail rather
than silently serialize as Falling. No new dependency or product API.

Scope: src/pattern/pattern_flatten.cc; existing native flatten suite; new
tests/src/pattern/test_legacy_launch_edge.py; Makefile regression registration;
tests/cases/model_source_acceptance.json; current guide and tracker.
Native tests prove scalar/vector, inverted/source/Enable, repeated children,
scope and property preservation plus repeated canonical equality. CLI tests
prove nonempty source/flat/generated replay, invalid values/placements and
previous-output protection. Corpus audit must determine actual new outcomes:
one former LaunchEdge-only default becomes canonical; the other four SIB
models also contain PortGroups and advance to that explicit existing boundary
without being mislabeled canonical. Never erase that blocker.

Performance is linear in emitted properties, one existing token per property,
no extra process/scans. Measure actual full default audit against Task156's
10s/512MiB workload limits using normal and configured leak-SAN helpers, with
unchanged sanitizer quarantine and the documented500ms unused-page release.
Use three fresh runs each, recording input/tool/env/output hashes and RSS.
This is not a launch-timing or universal vendor-equivalence certificate.

Required verification: RED native+CLI before implementation; focused native
flatten/netlist/state and dynamic/static CLI after; full native14 and leakSAN14;
full dynamic/static generator regressions including the newly registered suite;
normal/configuredSAN corpus audits and8producer/13protocol/3Make tests;
changed C++ format/tidy/C++17 checks and source package tests.
Fresh independent final review, one test-first Critical/Important fix pass,
defer Minors, no rereview. Verify merged exact inputs/products, push and remove
the completed branch, preserve117 stashes, then continue remaining inventory.
