# Downstream write callback prerequisites

The user requested completion of the remaining inventory, except item1, and
authorized implementation, commits, pushes, direct-parent merges and cleanup.
This is the next implementation increment for item4. The user delegated design
choices and opted out of questions; choices below are the executor's, not specific
user design approval. Broader inventory work continues afterward.

## Intended behavior and evidence

IEEE1687-2014 clause6.4.9, printed119–121, describes invoking WriteCallBack when
the register must supply data to a downstream write. Local extracted standard:
`/tmp/item11-1687.txt:6152–6161`. Existing explicit callbacks execute correctly,
but direct downstream requests cannot discover their prerequisites.

At parentb9e59e2, selectable `dest` with `WriteDataSource value` and
`WriteEnSource en`, where `value` has WriteCallBack, rejects `iWrite dest 9`.
Explicitly write value9, Apply, then write dest9, Apply succeeds: store receives
0b1001, writes actual endpointp9, and the native scheduler enables dest afterward.
Combining the explicit and downstream requests currently rejects because the
downstream goal is enforced before its callback prerequisite is established.
A plain iRead expected9 is a comparison, not a request to drive its source.
Actually modeled callback-derived histories remain outside this increment.
An always-enabled discovery fixture fails even for its explicit control; do not
invent hardware support or treat it as a positive control. Discovery receipts:
build/downstream-write-callbacks-20261123/discovery/.

## Approaches and decision

1. General symbolic callback drivers plus a runtime scheduler preview cover more
expressions, but duplicate state/clock/resource replay and require a new solver/
PDL negotiation interface.
2. Trace transparent write-data paths, derive compatible callback masks, and stage
dependent ordinary operations around the existing dispatcher. Reuse resolution,
preflight, callback recursion, native scheduling and source spans.
3. Rewrite source into explicit calls. This cannot preserve dynamic procedure
scope, once-only arguments, caught failures or source-local reset transformation.

Choose2 for the first concrete downstream topology. This does not close item4:
symbolic logic/mux selection, derived clocks, callback-dependent addresses/enables,
automatic serial prerequisites and cyclic staged scheduling remain tracked work.
Do not silently enable these unsupported cases.

## Contracts

- Known iWrite bits can demand WriteCallBack bits through transparent Source/
Alias forwarding and selectable DataRegister WriteDataSource chains. Preserve
numeric index, declaration order, inversion, constants and instance identity.
Walk selected bits only; stop at real primary/scan storage boundaries owned by
native scheduling, and detect dependency cycles. No broad name inference.
- Resolve after effective ordinary last-wins masks for the Apply group are known.
Canceled and overwritten requests cannot invoke callbacks or constrain stale
values. All-X accesses create no automatic callback requirements.
- Aggregate per-register masks in first effective request order. Compatible
requirements coalesce. Contradictory demands through distinct downstream targets
reject before procedure effects. Explicit versus inferred demands agree on
overlapping known bits. Last-wins stays local to original target semantics;
do not drop a distinct downstream goal to resolve a conflict.
- Default explicit-write policy may invoke a prerequisite even if a prior logical
value matches; add no skip-write optimization or actual-value inference.
- Keep original logical markers and append-only operations/source identity.
Dependent ordinary requests enter the hardware queue after their prerequisites
complete: body Apply cannot flush them before logical postconditions exist.
Unrelated queued ordinary requests retain current caller-goal protections.
- Reuse the existing dispatcher, binding/arity/cycle preflight, namespace
parameters, <D>/<R> substitution, implicit tail Apply and consumed-index tracking.
Preserve Task162 pure and mixed explicit callback streams.
- Native scheduler and existing source/readback consumers own hardware/history/
reset semantics. Logical callback postconditions are planning assumptions, never
actual observations. Do not synthesize observed values.
- A body reset cancels pending dependent requests and goals; never restore them
after return. Completed frames stay completed. Keep current caught-failure,
poison/reset/retry behavior; add no automatic retry.
- Keep callback ownership restrictions and raw/together/AccessTogether/active
caller iMerge guards. Physical automatic callback output and modeled automatic
callback histories reject before artifact publication.
- Source actions own the prerequisite plus dependent expansion, preserving
arguments/origins/call stacks and once-only expressions. Source/flat/generated,
split-file and local-reset-source replay retain unchanged iProc definitions.
- Unsupported transparent-path cases must fail existing native proof or explicit
diagnostics; assumptions or ignored demands must not create false success.

## Implementation boundaries

Prefer private Runtime helpers and pending-group records. Keep public Operation/
Runtime APIs unchanged unless a concrete regression proves a need and this design
is refined first. Trace consumers before choosing marker bookkeeping. Register a
new CLI suite in Makefile; new scripts belong under tests/src/pattern. Maintain
existing native tests in tests/src/pattern_runtime_test.cc. README, guide, model
coverage and tracker must state actual topology/observation limits and keep item1.

The builder chooses private layouts, helper names and grouping. Settle cancellation,
source-span ownership and existing explicit coalescing together before coding.
Add no independent scheduler replay cache.

## Verification and performance

Test first against unchanged parent: direct/forwarded/sparse/inverted/duplicate/
ascending/segmented paths; multiple compatible targets; conflicts; superseded/
canceled/no-care goals; nested dispatch; preflight/cycles/failure reset; body Apply/
reset; unrelated caller goals; namespaces, split files, once-only arguments and
reset rewriting. Native tests check append-only stream/provenance and generated
logical scheduling. Check endpoint PDL/substituted data, not literal notes as
observations. Modeled/physical negatives protect existing artifacts.

Measure matched explicit-before controls first to choose representative counts;
target normal16/64/256 and sanitizer16 if actual baseline supports them. Record
sizes and justify refinements before implementation. Each selected process must
meet10s/512MiB; wide payload checks10s/256MiB. Work should track selected dependency
bits plus emitted operations, with map log factors. Reuse path results within a
group to avoid repeated whole-network walks. Follow existing graph-work limits.

Preserve Task162 mixed/group/callback/wide/corpus budgets and complete generator,
native, static, SAN, format/tidy/C++17 and package checks. Reuse baseline evidence
only with current source/product and exact passing-log proof.

Use one fresh builder check, inline execution without implementer subagents, and
one fresh whole-branch review. Fix blockers in one test-first pass and record
declined behavior/costs. Qualify final state; merge into direct parentmaster,
verify identical source/product hashes and integration, push, detach retained
worktree, delete completed branch and archive only this plan's ledger. Continue
the original inventory afterward.
