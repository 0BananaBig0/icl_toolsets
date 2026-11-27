# Feedback reset pulses

Task198 advances remaining items3/4. Item1 stays excluded. Continuous inline
implementation, repository/owned temporary changes, commits, parent merges,
pushes and completed branch deletion are already authorized. The choices below
are the executor's within that scope.

## Problem and existing semantics

The maintained TestFeedbackResetPulse starts a two-bit DataRegister at11 and
enables WriteDataSource0,value[1]. Its simultaneous feedback snapshots are
11→01→00. Their XOR briefly asserts an external ScanRegister's reset, although
the first and final reset levels are zero. Current concrete execution rejects
the changing reset; symbolic execution constrains the reset invariant. Removing
either guard would lose the pulse or make planning disagree with actual replay.

IEEE1687-2014 6.4.9, printed117–119/PDF131–133, defines selectable data registers
and immediate evaluation after UpdateEn, including cascades and self hold. The
existing bounded logical delta model is established by
2026-11-21-data-register-feedback-design.md. Extend that model; do not infer
physical pulse width or introduce a timed simulator. Recorded Tessent2026.2
feedback-cycle rejection remains a compatibility limit, not normative invalidity.

## Qualified family

Support pulses from one convergent data-feedback SCC into an acyclic closure of
independent modeled ScanRegister resets. Include parallel sinks, cascades,
hierarchy, aliases, indexed words and active-low reset connections. Scan sinks
must not feed any writable DataRegister's data, enable, address or reset cone.
Another writable data group must not influence the same reset closure.

This increment qualifies interface-free models with ordinary data, scan, reset,
address, read-enable and write-enable ports. Models with TAPs, AccessLinks,
ScanInterfaces or explicit clock/scan-control ports keep the existing changing-
external-reset guard. Coupled data/reset feedback, shared ownership between
groups and cyclic scan-reset closures also retain that guard. Masked/invariant
cases must keep working everywhere they already work. These boundaries remain
open inventory work; this increment does not complete broad item3 or4.

## Shared dependency metadata

Extend the existing bounded external-reset dependency walk in DataRegisterPlan.
Build register-to-reset-consumer edges, including implicit OneHot read conditions
through the existing resolver. Propagate origin data-group ownership over those
edges with a three-state lattice: no origin, one origin, multiple origins.
Only two distinct origins need to be retained. Each ownership state changes at
most twice, avoiding a complete reset-network walk for every data group.

Build the reverse set of scan storage consumed by writable data groups once.
For each feedback group, collect its uniquely owned scan-reset closure; reject
qualification on other storage, data consumers or a reset cycle. Qualified
closures are disjoint. Preserve external_resets for the old guard on unqualified
groups. Add DataRegisterGroup::reset_targets (vector<size_t>) and
reset_consumers (map<size_t,vector<size_t>>) for qualified sinks and their
internal propagation edges. DataRegisterPlan::feedback_reset_targets is the
union used by primary state/fill seeding and replay validation.

## Concrete and symbolic transitions

After each simultaneous feedback update, settle only the qualified reset sinks.
Start with all sinks, compute each wave from one prior snapshot, publish the
wave together, and queue consumers of changed storage. Check held values before
and during reset propagation, including a transient between cascade waves.
Do not manufacture a transient by updating independent sinks one at a time.
Preserve unknown reset joins, reset priority and actual reset/control events.
Failure leaves the original state, fill and histories unchanged.

Add private NetworkState::ResetFeedback(const DataRegisterGroup&,size_t*,const
Check&). Keep public Reset's existing behavior. In the scheduler, add
SettleFeedbackResets(const DataRegisterGroup&,SymbolicNetwork*,FeedbackResult*,
const z3::expr&,z3::optimize*,size_t*,const
std::function<void(SymbolicNetwork&)>&). Reuse ResetValue, SymbolicNetwork::Assign
and scoped preservation checks; do not copy or rescan the whole netlist per wave.

SettleDataFeedback must retain affected scan state/fill alongside the data SCC.
Both primary and mixed callers must publish those entries. Primary-only pin writes and DataOutPort reads may use the primary planner when
every scan register is a qualified independent sink. Treat that route as an
optimization candidate: retain the mixed planner when scan preparation or end
goals prevent primary-only scheduling, and publish only successful observations.
Known explicit pin goals
in a single frame are logically forced; use their literal values instead of
existential choices. For a qualified read-only request without unmet end goals, try
one no-drive frame first; on unsat or unknown, continue the existing search.
Use existing route and preservation checks, without matching actual data to
expected comparisons. Other primary choices retain their existing behavior.
Primary seeding must include scan sink fills; primary concrete replay must validate them. Share
ResetFill(size_t) for known ScanRegister ResetValue bits over initial controlling
fill, preserving the existing DataRegister reset-fill convention. Update the
exact primary replay path as well as symbolic primary/mixed replay. Capture
continues to observe the pre-update snapshot. An empty apply or observation
frame without driven inputs creates no data update event; required preparation
still executes its real primary or scan events.

## Bounds and verification

Keep C++17, existing dependencies and all existing limits:32 data delta rounds,
4M feedback-bit work,4096 data registers and current dependency/reset/solver
budgets. Charge reset-wave work to the existing bounded feedback work account.
Metadata construction and concrete propagation use indexed edges; symbolic work
is bounded by the existing budget and only visits the affected cone. No new
unbounded event queue or per-item subprocess is introduced.

Native assertions must inspect actual storage, controlling fill, reset events,
copies and rollback. Maintain CLI tests under tests/, using literal expected
values and source/associated-flat/generated/reset-only replay. Cover known and
unknown pulses, parallel simultaneous sinks, cascades, held transient conflicts,
scan-driven updates, old capture values, source order and unqualified guards.
Use model_readback for actual observations; expected comparisons are not data.

Measure three rounds of16/128 independent groups,2/8-bit feedback words and1/4
scan sinks in a reset cascade. Retain actual inputs/outputs, environment, commands,
walltime/RSS and binary/test identities. Compare masked parent/current controls
and verify candidate reset outcomes independently. Every qualified point must
meet unchanged10second/512MiB bounds; report resource exhaustion honestly.
Run focused RED/GREEN, full project, affected leak-enabled sanitizers, changed-C++
quality, corpus/editor and source packaging on final inputs. Reuse passing
broader results for focused claims only when the actual case and conditions ran.

## Integration

Parent masterac0bbc9a0e3d8cf300d7d7e533d8c4e23460499e; owned branch
codex/feedback-reset-pulses in the existing .worktrees/bsdl-defaults workspace.
Reuse the just-verified Task197 baseline:39 qualification commands,9 merged
checks,4125 input/44 executable identities. Preserve117 stashes, foreign
worktrees, sibling ledgers and44 inherited deferred Minors.
Use one fresh independent whole-branch review, one Critical/Important test-first
repair pass if needed, defer Minors and do not re-review. Verify the merged
result before push and safe owned branch deletion; archive only this plan's
workspace and continue the remaining inventory.
