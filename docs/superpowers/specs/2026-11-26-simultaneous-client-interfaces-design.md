# Simultaneous disjoint standalone clients

Original item2 still includes independently selected lanes. The current
multiple-client fixture has separate SI/SO/select lanes and shared capture,
shift, update and TCK pins, but a together write across two clients is rejected.
SelectScan requires exactly one selected client and normally one selected root.

Support explicit simultaneous groups across structurally disjoint standalone
client interfaces sharing one root TCK and the same declared phase-control pins.
All declared root client interfaces must satisfy this structural/control proof;
compatible subsets in otherwise overlapping networks remain outside this increment.
Keep ordinary requests, shared-storage interface switching and unsupported clock
relationships on their existing behavior. Standing user instructions delegate
implementation choices and authorize immediate commit/merge/push/branch cleanup.

Reuse CheckIndependentChains to prove serial separation and obtain register-to-
client ownership. A simultaneous group must actually span eligible clients.
Root SelectPorts must be independently selectable under IEEE1687-2014
6.4.16(m);coupled selector conjunctions cannot qualify, even on disjoint lanes.
Their optional proof examines at most1048576selector entries before retaining
ordinary one-client mode. Selector sources must be unconnected root inputs.
Carry its eligible client set into symbolic frames;permit multiple selections
only for that case, and count the selected clients' actual declared routes.
Preserve one-client prerequisites and constrain unrelated clients out of a
multiple-client frame. Use the existing concrete event model and retain read
aliases when emitted writes alone cannot preserve a capture group.

Globally allowing multiple clients would change ordinary selection/reset
semantics. A second scheduler would duplicate state and replay. The existing
joint solver plus a structural eligibility proof is the chosen approach.
No new dependencies, public scheduling API, clock timing assumptions or increased
resource limits. Metadata remains planning information, never hardware state.

Acceptance: original/flat/generated replay, exact asymmetric capture history and
once-only status, unequal widths/inversion, AccessTogether writes/reads, reset-
held members, local reset, unrequested held lanes and cross-client reset/capture.
Generated prefixes must preserve sparse read-history scope on later continuation,
including ascending/inverted aliases,overlapping groups,known old history and
both default/modeled producers consumed through original/flat models.
Reject overlapping/shared serial resources and incompatible clocks/controls
without changing a previous output. Prove common native update events and retain
the old one-client/shared-storage tests. IEEE1687-2014 7.9.14(c) and6.5.2 retain
their same-cycle/together obligations;serializing a requested group is not valid.

Measure2/4/8/16clients at8bits and2clients at32/64/256bits. The delegated bounded
target is10s per four-form point and512MiB runner/child-process-tree RSS;no
sanitizer scale guarantee. Reuse the existing complete-scan equivalence probe
over the full eligible selection domain;use event expansion when it cannot prove
equivalence. The initial forced-event256bit case measured14.317s/807128KiB.
Compare unchanged ordinary controls with the frozen parent;measure new groups
separately because the parent rejects them. Run normal/static, sanitizer, full
project, model/source audits, C++17 style and source-package checks. Qualify fresh
vendor comparisons only when the actual tool completes;retain failures/skips.
One fresh whole-branch review, one blocker repair pass, merged-result checks and
owned branch cleanup are required. Broader items2/3 and excluded1 stay open.

Fresh Tessent2026.2 reference probes accept the ordinary two-client control but
report no solution for the simultaneous request. The AccessTogether probe is
rejected at that keyword although6.5.2 explicitly includes its semicolon syntax.
6.4.16(m) requires unique selection;it does not itself establish simultaneous
execution or vendor portability. This is bounded local logical scheduling with
native edge evidence;no cross-tool equivalence or physical-output claim is made.
