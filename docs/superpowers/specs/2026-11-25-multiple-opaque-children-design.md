# Multiple opaque children on one static serial route

Task172 advances inventory6/8 from master800a41f8873f37224fe48af4feb29cad55f53a79
on codex/multiple-opaque-children. The user's question opt-out, delegated design
choices and native inline execution continue. This is one extension of the
existing private opaque interval backend, with one final independent review.

A retained two-child source and PDL in
/tmp/icl-multiple-opaque-children-20261125 fails on the parent as disconnected
parent storage. Both repeated ChildLeaf instances have their own eight-bit
DefaultLoadValue and share the scalar root controls. The required behavior is
a single complete physical route with independent child defaults, explicit
payloads, stable completion, lengths and actual opaque histories.

IEEE1687-2014 section6.4.16 pp131–132 requires black-box client interfaces and
default lengths/values. Section7.9.12 pp168–170 requires precise active lengths
from the current request, previous stable request or reset default. The local
supplied /tmp/item11-1687.txt contains the inspected clauses. This extension
models those contracts; opaque hardware contents remain unknown.

Use the existing OpaqueChildModel private expansion and OpaqueScanQueue.
Replace its single client/storage/default with an interval collection indexed
by actual instance/client identity. Validate candidate leaves, scalar pins,
defaults, one root route, controls and side-channel privacy once for the whole
network. Every selected opaque interval must occur exactly once on the static
route. Keep source node IDs and source/flat exports untouched.

The scheduler owns one queue per child. Prepare queue candidates and all
completions before native execution; commit every queue/state only after the
whole group succeeds. Ordinary parent accesses include all required default/
stable loads and hide every private register from public frames/observations.
Raw requests project into intersecting child intervals, with exact root and
local inversion handling. Index intervals by root offsets; avoid per-request
full scans of all children. Copy/reset and failed subscriber semantics retain
the existing transactional contract.

Alternatives: calling the single-child expansion repeatedly repeats whole
network validation and cannot certify disconnected intervening opaque nodes;
a separate raw backend duplicates native state and observations. An interval
collection reuses existing behavior and keeps one state owner. This is the
executor's implementation choice under delegated design authority.

Inputs: qualify repeated two/eight/thirty-two children, mixed widths8/64/256
and1024-bit words, optional modeled siblings, aliases and route inversions.
Storage is O(nodes + total payload); route/privacy validation is O((nodes +
connections) log nodes), using existing ordered maps. Raw projection is O(children log
children + requests log children + intersected payload), not a full
children-by-requests scan. Use existing one-megabit scan/payload and traversal/
solver/event limits. Each measured process must meet10s/512MiB; wide controls
retain their existing tighter256MiB bound. Report measured scale only.

Required tests: independent defaults and stable payloads, repeated module
instances, root/local histories and unknown mismatch status, simultaneous
disjoint requests, reset/cancellation, variable-length rejection, native copy/
rollback/subscriber commit, parent reservation/preservation, inversion/sparse
data, namespaces/local-reset/split files, private targets and exports,
disconnected/overlapping/invalid/side-channel topologies, unknown-length
completion and mixed modeled siblings. Source/flat/generated replay must
agree on complete output and actual data/status.

Keep existing dynamic/gated/parallel opaque, multiple-TAP, timed, physical and
merge boundaries unless already supported independently. Do not redefine
whole inventory items as complete. No RTL, dependencies, public options,
arithmetic changes or live-device adapter.

Run focused checks, full normal/static generator variants,15normal/15
leak-enabled SAN native suites, affected SAN CLI/corpus, changed-range C++
quality, source packaging and representative new scale/replay measurements
with parent-supported one-child controls. Baseline reuses the just-completed
Task171 88gates/846measurements and identical products. Preserve117stashes,
27deferredMinors and all unrelated ledgers/ignored products. One final review,
one test-first blocker pass, merge/push/delete owned branch and archive ledger.
