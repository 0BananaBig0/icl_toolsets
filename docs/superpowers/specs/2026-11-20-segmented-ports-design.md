# Preserve disjoint vector-port functions
Repair the item11 defect confirmed against IEEE1687-2014 6.4.1g/h (printed80/
PDF94) and bounded Tessent2026.2 probes. This increment closes that concrete
defect, not independent event scheduling or the whole completion program.
User-delegated choices and existing repository authorization apply.

Use one existing real typed NetlistNode per declared function. Keep its AST,
kind, attributes, enum, defaults, source, clock/reset semantics and declaration
order. Never merge functions into an artificial hardware or alias node.
Single declarations keep their current identity. Repeated indexed port names
receive canonical full-fragment target identities (p[7:4], p[3:0], p[1]).
The original qualified base maps to sorted immutable member intervals and
whole-bus bounds. Scalar/non-port duplicates, overlaps, gaps, inconsistent
nontrivial orientation, namespace collisions and width exhaustion reject.
All-single-index groups default to descending whole-bus order; a nontrivial
range determines the common direction. Reversed explicit slices retain existing
selection behavior. Preserve the bounded mixed input/output case with each
selected real bit checked for the operation's access/direction contract.

Reuse one numeric-range-to-real-bits resolver in ICL expressions, PDL targets
and InputPort bindings. Existing alias traversal, hierarchy/output restrictions,
scan-register rightmost-bit rules and target eligibility apply per real bit.
Bindings distribute a single validated source vector over each selected real
fragment, retaining duplicate binding checks and the existing hole policy.
Logical DataIn concatenation and automatic interface enumeration retain original
function declaration order, independent of whole-base reference order.
Named PDL enum values require one uniquely identified declaration; a whole
heterogeneous group must not inherit its first fragment's enum. LogicSignal
enum ranges remain rejected under IEEE6.4.10c/k (printed123-124/PDF137-138).
Vendor-generated enum subranges are documented compatibility differences.

Flat hardware names encode each full-fragment identity with existing FlatName.
Derive interval views from canonical indexed-port original identities
in existing version1 node metadata instead of a redundant new field. Only
validated port/indexed/bounds identities may use a fragment suffix; at least
two members must reconstruct each original group. Strictly reject malformed,
duplicate, missing/colliding, overlapping, gapped or contrary identities.
Existing ungrouped metadata serialization and restoration stay compatible.
Do not accept arbitrary indexed names for registers/interfaces/logic/aliases.

Update only callers that require the aggregate view: source/PDL resolution,
bindings, clock literal/flat decoding and displayed bit names, partial reset
release names, and enum uniqueness. Direct scalar BSDL/TAP pin uniqueness
remains. Keep unambiguous AST binding checks; grouped scan-input conflicts must
be checked on selected elaborated bits before TAP wiring, allowing bindings of
unrelated port functions. Encoded clock references must remain inside the decoded
physical fragment even when the logical group contains other selectable bits. Scheduler storage/interface maps continue using actual node identities.

Build groups once after source declaration expansion, before connections.
Bind pending instance targets by name so child group resolution can wait until
this shared construction step. Restore flattened groups once after identities.
Sort intervals once; query one starting interval then stream selected bits/
intervals. O(N + sum(G log G) + selected bits), excluding existing map lookups;
no full-fragment scan for each selected bit, no repeated global rebuild per
instance, no new solver or dependencies. Retain existing object/bit/depth limits.
Measure a representative many-fragment source, nonempty source/flat/generated
replays, and memory/runtime versus a matching single-port control. Report
measurements and resource limits without unmeasured speedups.

Native and CLI acceptance: descending/ascending/single-index source order,
parameterized bounds, more than two functions, mixed kinds, input/output,
fragment attributes/defaults/enums, child whole/sliced bindings, Source/Alias/
LogicSignal references, interface membership, ordinary writes/reads, histories,
resource holds, reset partial release and clock source/target/indexed flat
names. Negative atomic cases cover scalar/non-port duplicate, overlap/gap/
contrary direction, out-of-range, wrong target kind, enum ambiguity, forbidden
hierarchy, duplicate bindings and forged flat metadata. Retain old output
sentinels on failure. Corpus11 grouped-model reproductions must pass this
boundary and record any next actual unsupported event/scheduling boundary.

Reuse Task141 base products and required checks by identity only where no
relevant source changes. Focused RED before implementation; GREEN/native/
dynamic/static, all generator suites, native and focused leak-enabled SAN,
changed-hunk format/tidy/strict C++17, source package. One independent whole-
branch review and one necessary fix pass; archive rulings and deferred minor
findings, merge/push/delete the finished branch. Coverage item1 remains excluded.
No new tests outside tests/; mirror source paths for new scripts.
