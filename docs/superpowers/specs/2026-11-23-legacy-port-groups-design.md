# Resolve and preserve legacy PortGroup membership

Task158 continues inventory11/12 from pushed master1acfbcb. The user delegated
routine implementation decisions and opted out of questions. These choices
are ours. Reuse Task157's fresh53-gate exact baseline; no dependencies changed.

## Outcome and existing evidence

The frontend accepts legacy LiU P1687 PortGroup comma lists of local port names,
optional integer-expression index/ranges, and comments. It stores inline lists
as AstStatement and commented lists as AstBlock. Current native elaboration
does not resolve membership and may misclassify comment-separated names as
properties; canonical serialization rejects all PortGroups. The shared frontend
navigation already recognizes member references (docs/icl-conformance-review.md).
This work implements the accepted compatibility form, not new IEEE syntax.

After Task157 actual full audit has1017canonical/55unsupported/691original
elaboration failures. Unsupported55 consists44unboundAccessLinks and11PortGroup
identities, including four legacy SIBs formerly blocked by LaunchEdge. Some
grammar-only inputs reference undefined a/b/c ports and must not become valid
model certificates. Initial fixture inspection suggests six valid models and
five undefined-member models; the actual complete audit decides the counts.

## Approaches and chosen representation

Copying group token text would retain hierarchical names without rewriting
references to flattened ports and leave semantic membership unchecked.
Putting annotation bits in NetlistNode.inputs would create fictitious data-flow
dependencies in cycle/alias and state callers. Choose one dedicated
std::vector<BitReference> port_group on NetlistNode, analogous to existing
scan_interface/one_hot_sources payloads. No new dependency, grammar, scheduler,
storage or transport. Existing Netlist.port_groups/struct PortGroup describes
segmented port functions and is a different existing mechanism; preserve it.

Native Connect extracts the complete braced list from node.ast's existing
source-token span, independent of AstStatement/AstBlock layout. Resolve through
existing Netlist::Resolve(... ReferenceUse::kAlias, max_bits), then require
each result to name an actual local ModelKind::kPort at a valid declared index,
no constants/virtual/inversion/alias/register/interface/group. Forward names,
parameter ranges, keyword names and segmented-port selections must work.
Preserve bit order and repeated references; no deduplication or sorting.
Skip generic property collection for actual PortGroup list bodies so a port
named LaunchEdge or Source is not interpreted as a property.

Keep existing1MiBit per-reference Resolve width bound. Introduce a4MiBit
aggregate resolved-member budget across all legacy groups, matching existing
one-hot/logic/address expansion budget scale. Debit during resolution before
allocation exceeds remaining budget. A new ValidatePortGroupMembers(const
Netlist&) shared helper checks total budget, group kind/representation,
nonempty membership, local scope, endpoint kind/index and absence of unsupported
properties/attributes. Invoke after RestoreFlatOrigins and before SerializeIcl
metadata/declarations so malformed public-API nodes and forged flat-origin
scope cannot silently corrupt annotations. Non-PortGroup nodes with nonempty
payload also reject. This helper is an internal shared C++ contract, no CLI API.

Declaration emits PortGroup FlatName(node.name) {Signals(netlist,port_group)}
using existing flattened signal rendering. References reconstruct from normal
flat ICL, then existing metadata restores group/endpoint original names/scopes
and kinds. No new metadata schema fields. Child input ports temporarily become
DataOutPorts during flat parse; membership still refers to real ports and must
validate again after restoration. Segmented vectors split across physical
flattened declarations and restore existing interval identities. Legacy groups
do not become wires, PDL access/reservation targets, scan clients or new state.

## Tests and performance

Native RED tests: actual valid group cannot serialize; undefined/nonport/range
members wrongly elaborate. Cover inline/comments/comma placements, scalar and
ascending/descending/vector/index/parameterized ranges, keyword ports including
LaunchEdge, duplicates, forward definitions, repeated/nested instances and
segmented mixed-function ports. Verify exact endpoint name/index/order, actual
canonical bytes, original scopes and no changed state behavior. Guard malformed
programmatic payload/empty/type/scope/index/constant/inversion/properties/attrs
and forged flat metadata; preserve ordinary Enum keyword behavior.

New maintained tests/src/pattern/test_legacy_port_groups.py reuses existing CLI
helpers for nonempty source/flat/generated writes/reads, hierarchical and
segmented cases, SIB control behavior, and atomic prior-output preservation
on invalid definitions and forbidden PDL group targets. Register in both full
generator variants; maintain existing native tests in place. Corpus registry
updates must be backed by actual fresh audit outcomes: promote proven canonical
models, remove newly precise original elaboration failures from unsupported
serialization declarations, preserve other unknown/missing dependencies. Never
manufacture ports, bindings, values or defaults; audit keeps errors visible.

Work is linear in resolved/emitted member bits plus existing indexed resolution,
not per-member process or a full module scan per group. Measure fresh normal and
configured leakSAN full corpus batch3runs each <=10s/512MiB, sameTask156/157
500ms unused-page release/fullquarantine. Also measure representative generated
networks at1/64/1024 groups x64 members and varied vector/segmented forms;
source/flat/generated exact output and measured budgets10s/512MiB per invocation.
Record counts,input/tool/environment/output hashes,walltime andRSS; no claimed
unmeasured speedup. Test the4MiBit boundary/beyond without raising any limit.

## Verification and completion

Split into native resolved-membership/canonical correctness; end-to-end CLI,
corpus/scale and complete qualification as independently checkable deliverables.
Use test-first fixes, existing C++17 format/tidy/native conventions. Full14native/
14leakSAN, both complete generator regressions, actualnormal/configuredSAN
parser+model corpus, producer8/protocol13/Make3, changedC++quality and source
packaging. Maintain guide/tracker with current acceptance and compatibility
limits; do not close whole11/12 or excludedcoverage1 from this increment.

Fresh builder check before plan. Inline implementation; one fresh final
independent review, one test-first Critical/Important fixpass, deferMinors,
no rereview. Final exact input/product evidence, merged checks, push, detach
preserved worktree/delete merged task branch, preserve117stashes/otherworktrees.
Continue remaining inventory after this increment.
