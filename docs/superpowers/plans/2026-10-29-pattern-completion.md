# Pattern generation completion implementation plan

> **For agentic workers:** Use superpowers:executing-plans; implement and verify each task inline. Retain unfinished tasks as acceptance requirements.

**Goal:** Close the remaining pattern-generation requirements without weakening correctness, resource limits or regression checks.

**Architecture:** Extend the shared elaborated netlist, concrete state, symbolic state and existing scheduler. Keep source-preserving reset rewriting separate from normal flattened output. No additional parser or independent retargeter.

**Tech Stack:** C++17, Tcl, Z3, Python unittest, GNU Make.

**Spec:** [Approved design](../specs/2026-10-25-icl-pattern-generation-design.md), including subsequent user instructions permitting bounded greedy scheduling and Tessent behavior probes.

## Constraints and review focus

- Existing autonomous inline implementation authorization applies. No unrelated file changes or generated build products in Git.
- Google style; new test scripts mirror source paths under tests/. Keep ASan/UBSan/LSan enabled.
- Unknown state must not become known by using expected read data. Contending or unknown enables must not produce a valid scan route.
- Shared storage, resets and controls must be replayed together; merely removing an unsupported-case guard is insufficient.
- Each newly supported case must preserve flat-ICL replay and generated-PDL idempotency. Failure must preserve previous outputs.
- Bound traversal and solver work; assess representative larger networks before claiming a performance improvement.
- Immutable procedure bodies can make some caller-only reset repairs impossible. Diagnose such cases rather than changing the requirement silently.

## Task 1: One-hot serial selection

Files: pattern_data.h/.cc, pattern_state.cc, pattern_symbolic.cc, pattern_scheduler.cc, pattern_tap_execution.cc under src/pattern; existing native state/symbolic tests; new tests/src/pattern/test_one_hot_scan_scheduling.py and classified fixtures; Makefile. Update CLI help and current support documentation after verification.

- [x] Add failing native truth-table and CLI tests for single, nested, inverted, conflicting and unknown contributors, source/flat/generated round trips.
- [x] Reuse OneHotReadResolver for scan-output enables and nested groups. Read shift state for serial groups. Constrain a selected route to one known-active contributor and all others known-inactive.
- [x] Reuse the same selection rule for greedy path discovery and concrete scan replay; test dependency scheduling and state rollback.
- [x] Run focused native and dynamic/static CLI tests, complete regression, sanitizers, and independent review. Record evidence and limits.

## Task 2: Multi-apply merge batching

Files: src/pattern/pattern_merge.cc; tests/src/pattern/test_merge_scheduling.py.

- [x] Extend tests to require two disjoint multi-apply calls to merge by apply depth, preserving each call's operation order and fixed points. Unequal lengths, repeated writes, conflicting targets, empty applies, resource/read/wait barriers remain covered.
- [x] Recognize complete write-only streams and maintain disjoint target sets across the entire concurrent batch. Flush batches one apply per live stream per round; remove exhausted streams so work stays linear in operations after target lookup.
- [x] Keep the scheduler's existing replay and fresh-state serial fallback. Do not cross apply boundaries outside an iMerge or reorder opaque effects.
- [x] Run focused merge regression, scale checks, sanitizers, complete regression and independent review; document the supported scope.

## Task 3: Bit-level ScanRegister reservations

Files: pattern_resources.h/.cc, pattern_scheduler.h/.cc and icl_pattern_generation_main.cc under src/pattern; tests/src/pattern/test_resource_reservations.py, tests/src/pattern_scheduler_test.cc; resource fixtures and tests/src/pattern/probe_resource_slices.tcl.

- [x] Reproduce the unsupported indexed/alias take and the incorrect whole-register lock after an explicit partial write. Tessent 2026.2 probes: high-bit take succeeds, low-bit take blocks access, partial release succeeds, inverted alias succeeds, and taking the full register followed by a high-bit-only write leaves the low bit available.
- [x] Resolve aliases/slices with ResolvePdlTarget and store physical per-bit ownership, preserving inherited/persistent ownership and scoped release. Use per-node masks/counters rather than one allocation per bit. Protect only explicitly assigned taken bits.
- [x] Extend existing symbolic constraints and concrete replay to masked preserved values. Keep unprotected bits free; preserve old-to-requested transitions on protected bits; apply the same masks in greedy planning, idle replay, end states and reset prerequisites.
- [x] Emit partial reset-control writes when other bits of that register are taken but unwritten, so inserted source commands cannot create new reservations. Re-execute rewritten text and retain failed-output protection.
- [x] Test ascending/inverted/concatenated aliases, overlapping ownership, partial release, iState, reset rewriting, greedy paths, TAP/flat/generated replay and larger vectors. Run full regression, leak-enabled sanitizers and independent review; document normative rules separately from vendor observations.

## Task 4: Hidden serial storage events

Architecture decision: retain the existing path scheduler as the fast path;
when explicit interface controls clock storage outside that path, use bounded
symbolic capture/shift/update transitions and verify every committed register
against the existing concrete TAP replay. Do not enumerate possible patterns
or suppress replay mismatches. Use the same held/shift distinction as
NetworkState. Work is bounded by event count times modeled storage, with the
existing solver timeout and an explicit expression-work budget.

Files: src/pattern/pattern_symbolic.h/.cc, src/pattern/pattern_scheduler.cc;
tests/src/pattern_symbolic_test.cc, tests/src/pattern_scheduler_test.cc;
tests/src/pattern/test_hidden_scan_events.py; existing resource probe fixtures.

- [x] Add native symbolic/concrete tests with deliberately different held and shift bits, including mux selectors, aliases, inverted and one-hot serial returns. Extend SymbolicNetwork with a separate optional shift-state view while preserving existing callers.
- [x] Reproduce the original five slice probes on their ungated TAP network. Add capture/no-capture, hidden update, unknown source, protected hidden storage, reset and rollback cases with native expected states and original/flat/generated replay.
- [x] Reuse interface bindings to derive effective events; compute all affected register states from one event snapshot. Carry unknown state without inferring captured data from expected comparisons. Keep full-register state replay equality and per-clock reservation checks.
- [x] Measure representative larger gated and hidden networks; retain the fast path and fail with a resource-limit diagnostic when bounded event planning is not proved. Run focused checks, full regression, sanitizers and independent review; record any remaining model restrictions explicitly.

## Task 5: Explicit reset releases resource reservations

IEEE §7.9.9 resets controlling/expected state; §7.9.19 restricts retargeter
changes to taken resources. Tessent v2023.4 describes global reset and persistent
ownership across pattern sets only when no initial reset occurs. Twelve fresh
Tessent 2026.2 probes establish the remaining ordering detail: explicit async or
sync iReset is allowed after a taken/write, and a subsequent write does not
reacquire protection without a new iTake. Treat clearing ownership as observed
Tessent behavior where IEEE does not explicitly specify the interaction.

Files: src/pattern/pattern_resources.h/.cc, icl_pattern_generation_main.cc;
tests/src/pattern/test_resource_reservations.py; new
 tests/src/pattern/probe_reset_reservations.tcl; user/model docs.

- [x] Add failing tests for written and unwritten ownership across iReset,
  async/sync TAP reset, slice/alias/persistent takes, nested calls, explicit
  retake, pending-operation rejection, original/flat/generated and reset-only
  replay. A low selector written to zero after reset must remain free unless
  explicitly retaken; retaking it must still reject target access atomically.
- [x] Add ResourceReservations::Reset(), clearing owner masks, counters and
  protected values while retaining the active call-stack shape. Invoke only
  after scheduler.Reset succeeds. Keep unapplied-operation rejection and
  physical reset replay unchanged.
- [x] Run focused resource/native tests, full dynamic/static regression,
  leak-enabled sanitizers, formatting/naming and independent review. Record
  empirical behavior without claiming broader lifecycle or vendor coverage;
  commit/push and continue remaining requirements.

## Task 6: Data resource reservations across both scheduling backends

Scope: DataRegister and DataInPort/DataOutPort ownership, including slices,
aliases and instance membership, under the same assignment-driven protection
as ScanRegisters (IEEE §7.9.19–20). Continue rejecting unsupported clock/control
port lifetime semantics explicitly. Reuse existing state and symbolic models;
do not introduce a second ownership system.

Files: pattern_resources.h/.cc and pattern_scheduler.cc; new
 tests/src/pattern/test_data_resource_reservations.py; Makefile and support docs.

- [x] Add failing primary/scan DataRegister and primary-data-port tests:
  blocked implicit changes, allowed explicit replacement, release/retake,
  slice/alias and instance ownership, nested lifetime, end-state conflict,
  reset clearing, reads without invented hardware state, atomic failure and
  original/flat/generated replay. Include a shared-address bank to exercise
  preserved data across prerequisite scans and measure bounded larger inputs.
- [x] Extend reservation target/member classification to DataRegister and data
  ports. Share desired protected-value validation and end-state conflict checks
  between primary and serial scheduling. Pass protection through the primary
  solver, constrain every frame, and verify it during concrete replay. Access
  ScanStep loads only for ScanRegisters; data registers update through existing
  model events. Preserve explicit old-to-requested transition rules.
- [x] Run the focused suite, complete dynamic/static regression, leak-enabled
  sanitizers and independent review. Document supported data resources and
  remaining control/clock/lifecycle limitations; commit/push before continuing.

## Task 7: Mixed parallel-I/O and serial time frames

Tessent Shell Reference v2023.4 `iApply` specifies a sequence of Scan and
ParallelIO time frames, all writes satisfied at the end, and each read observed
once before subsequent changes. Reuse the existing state, route resolvers and
bounded solver. A serial transaction and a parallel input event must not be
modeled as interchangeable or simultaneously applied without evidence.

- [x] Probe primary-data/scan-enable and scan-data/primary-enable models with
  Tessent; retain scripts and self-constructed fixtures under tests/. Add failing
  CLI/native tests for both dependency directions, prior-value reads, protected
  storage, aliases, rollback, and original/flat/generated replay.
- [x] Extend shared scheduling with explicit parallel/serial frame kinds and
  shared state transitions; bound primary vector count/bit width and total frame
  work. Preserve scan-only/primary-only fast paths and existing goldens. Replay
  each frame through existing concrete models; do not concatenate independently
  planned schedules that ignore shared storage.
- [x] Validate TAP and non-TAP mixed events, read ordering, end-state and
  AccessTogether obligations; add representative scale checks, independent
  review, full dynamic/static regression and leak-enabled sanitizers. Record
  vendor evidence separately from local coverage and update support docs.

## Task 8: Primary-port AccessTogether groups

IEEE 1687-2014 §6.5.2(i), printed p.137, requires a common scan operation or
parallel-port update. Extend the existing group constraints and concrete replay;
no second scheduler or relaxation of simultaneous access. Ungrouped primary and
serial fast paths remain. The inherited 256-vector/4096-bit and temporal-work
bounds cover planning; measure 64 primary members with original/flat/generated
replays. Forwarded/computed write targets are covered by Task 9.

- [x] Add failing primary-only/mixed CLI tests for direct primary members,
  partial aliases and implicit member access, inverted/sliced nested groups,
  DataRegister common updates, incompatible scan/parallel groups, simultaneous
  output observations, reservations, failed-output protection and fixed points.
  Add native resulting-state and rollback assertions.
- [x] Route primary-only AccessTogether operations through the joint solver;
  recognize supported direct-primary write and output-read members, include
  implicit group pins in planning, and preserve group obligations on emitted
  operations. Keep state, resource and read-order checks shared.
- [x] Run focused tests, a representative scale measurement, independent review,
  full dynamic/static regression, leak-enabled sanitizers and Google checks.
  Update support documentation, commit/push, and continue remaining requirements.

## Task 9: Forwarded/computed data-port requirements in joint scheduling

Reuse SymbolicNetwork/NetworkState evaluation for logical port goals; do not
substitute a port's requested data for its hardware state or collapse independent
controlling histories into upstream registers. Cache guarded dependencies across shared logic,
retain bounded work, and require controller updates for written scan-driven ports
(IEEE §7.9.11 and §7.9.14(a), pp.167/173). Keep signal models shared with existing
primary-only writes. General time/clock/callback models remain separate work.

- [x] Add failing mixed CLI/native cases for forwarded/inverted/logic/mux/one-hot
  data ports, primary and scan sources, hierarchy, partial/X controlling history,
  reservations, end states, contradictory/disconnected writes, grouped updates,
  TAP and concrete rollback. Require original/flat/generated fixed points and
  a representative wide case.
- [x] Reuse logical value evaluation and controlling-fill rules in joint goals;
  discover required driver/control dependencies and support common-cycle checks
  for ports without pretending those ports contain storage. Replay and validate
  logical goals against concrete hardware before committing fill/state.
- [x] Run focused and full dynamic/static regression, leak-enabled sanitizers,
  naming/style, scale checks and independent review. Record limits and evidence,
  commit/push, and continue remaining requirements.

## Task 10: Merge independent observations with write streams

IEEE 1687-2014 §7.9.18(e,h,i), pp.177–178, uses shared controlling/expected state,
requires each call's apply order, and permits serialization for conflicts.
Extend `Merger::Batchable` in `src/pattern/pattern_merge.cc` to accept full direct
ScanRegister reads (including X masks and omitted expected data) alongside full known writes. Keep a common
occupied-register set across each entire call. Existing scheduler/replay and
fresh-state serial fallback retain ownership and event semantics; do not add a
second merge engine or enumerate interleavings. Other target kinds and barriers
remain conservative. Ordered-set checks and retiring exhausted streams retain
work proportional to operations apart from target lookup and set operations.

- [x] Add `tests/src/pattern/test_merge_observations.py` with literal expected
  two-round read/write groups, read-only batching, masks, same-target conflicts,
  note/resource/partial-read barriers, unequal streams and 64 two-apply calls.
  Five cases must fail the current serialization behavior; positive results must
  pass source/flat/generated replay. Record baseline count (128 applies).
- [x] Allow `iRead` in `Batchable`, validate its target/value with shared helpers,
  and retain the X prohibition for writes only. Adjust the obsolete read-barrier
  expectation in `test_merge_scheduling.py`; wire the new suite into both dynamic
  and static Makefile checks. Preserve all other conservative barriers.
- [x] Run both merge suites and native scheduler checks, full dynamic/static
  regression, leak-enabled sanitizers, Google style/naming and independent review.
  Measure the same 64-call workload, document apply-count reduction without a
  global optimum claim, update README/support docs, commit/push and continue.

## Task 11: Parallel control-port reservations

IEEE 1687-2014 §7.9.19(a,b,d,e), pp.178–179, covers port ownership, whole-instance
membership and inherited ownership. AddressPort/ReadEnPort/WriteEnPort already
have shared value and scheduling models; extend resource classification rather
than add a second ownership mechanism. Reuse masks, original logical targets,
shared symbolic constraints, concrete replay and atomic publication. Serial,
clock and other control types remain separate model work.

Files: `src/pattern/pattern_values.cc`, `pattern_resources.cc`,
`pattern_scheduler.cc`; new `tests/src/pattern/test_parallel_control_reservations.py`;
Makefile, native scheduler tests and support docs if affected.

- [x] Run seven initial tests covering address/enable constraints in primary and
  mixed backends, inverted child ports, implicit parent changes, explicit child
  replacement, instance membership, masks/aliases, return/inherited/reset
  lifetimes and 64 forwarded enables. Confirm rejection of direct takes and
  silent omission from instance takes (15 RED subcases). Successful cases must
  preserve original/flat/generated fixed points and failures preserve outputs.
- [x] Extend shared reservable-resource classification/diagnostics. If mapped
  child enable writes lose logical explicit replacement in `DesiredPreserved`,
  reproduce then retain the original logical assignment while enforcing the
  actual source goal. Do not waive preservation on mapped ports. Keep work
  proportional to assignments/target bits and existing dependency/solver bounds.
- [x] Run focused resource/scheduler tests, full dynamic/static regression,
  leak-enabled sanitizers, style/naming, 64-port scale and independent review.
  Record evidence and remaining types; update docs, commit/push and continue.

## Task 12: Qualify computed ports, ownership and merge observations with Tessent

Reuse `tests/tessent_roundtrip.py` and `tests/tessent_oracle.py`, retaining strict
ordered-apply comparison and immutable certificates. Author one shared TAP
fixture and three PDL cases under `tests/cases/ours/pattern_generation/port_control_merge/`;
reuse the existing AccessProbe BSDL. Source inputs are self-constructed; vendor
exports enter Git only through explicit successful certification. Remote jobs
run serially inside the user's authorized `test_icl_toolsets/test_port_control_merge/`.

- [x] Prepare both directions for computed-port writes, reserved child-enable
  replacement and two merged read/write streams. Validate source locally before
  using Tessent. The initial fixture's invalid ResetSource syntax was caught and
  corrected to the existing implicit-reset model before remote execution.
- [x] Run all six jobs on Tessent 2026.2, retrieve complete evidence, and certify
  each passing direction to a new snapshot. Retain/report failures without
  weakening comparison or claiming universal behavioral equivalence.
- [x] Register successful standards in ordinary offline regression; verify source,
  flat ICL and generated PDL with the shared tools. Run full dynamic/static
  regression and review the recorded evidence; update documentation and commit/push.
  Outstanding model requirements remain listed below for subsequent work.

## Task 13: Indexed scan-input endpoints

Ruling: close the independently testable vector-input gap before shared-output
event planning. IEEE 1687-2014 §6.4.6.1 (printed p82/PDF96) uses `port_name`;
§6.4.7 rules d/f (p111/PDF125) permits width-matched slice bindings. A scalar
scan path may end at a selected bit of a vector ScanInPort. This does not authorize
independently clocking multiple outputs or bypassing the shared-control guard.

Reuse the bit-keyed scan graph and NetworkState::Drive. Preserve untouched input
bits when replay drives a selected endpoint; identify indexed endpoints in ScanStep.
The graph remains linear in reachable serial bits. Whole-vector drives reuse the
existing state API; measure a 4096-bit input with a 64-bit register before claiming
acceptable scale. No new parser, scheduler backend or state representation.

Files: src/pattern/pattern_scheduler.cc; tests/src/pattern_scheduler_test.cc;
tests/src/pattern/test_vector_scan_inputs.py; classified fixtures under
tests/cases/ours/pattern_generation/vector_scan_inputs/; Makefile and support docs.

- [x] Add native and CLI RED cases for ascending/descending indexed inputs,
  nonzero scalar indices, mux-selected/inverted endpoints and vector one-hot
  returns feeding one scalar chain. Assert actual register/input state, serial
  endpoint identity, untouched unknown bits, atomic failures and replay.
- [x] Recognize all top-level ScanInPort bits; replay a width-matched drive with
  only the selected position changed. Preserve scalar behavior and all existing
  multi-output/TAP/event checks. Run native and CLI tests: expected PASS.
- [x] Test source/flat/generated-PDL fixed points, reserved values and unknown
  one-hot controls. Run full dynamic/static regression, leak-enabled sanitizers,
  style/naming and independent review. Record scale and remaining restrictions,
  update README/docs, commit and push.

Review ruling: scalar-only EffectiveScanInterfaces inference can omit vector
children's declared controls. The reviewer reproduced a write through a SelectPort
tied low. Preserve the existing unsupported-event boundary with an additional
non-TAP guard over reachable serial scopes, not fabricated interface bindings.
The retained disabled-child test failed before this fix and now passes; the
reset-only child remains supported. Initial 64-bit fixture preparation also
corrected a mismatched 4-bit ResetValue before accepting its RED result.

## Task 14: Shared observations of one serial path

Continue the approved inline work in the current checkout (the preceding
addressed-scheduling plan records master authorization). Several ScanOutPort
bits may observe the same physical path, including inverted forwarding. They
do not create additional storage or clock events. Coalesce only transparent
single-edge output/forwarding vertices that converge before any register,
selector or enable; retain the first sorted output as the serial endpoint.
Distinct paths still require shared-event planning and remain open below.

Files: `src/pattern/pattern_scheduler.cc`, existing native scheduler tests,
`tests/src/pattern/test_shared_scan_outputs.py`, Makefile and support docs.

- [x] Add native and CLI tests for duplicate/inverted/vector/hierarchical
  outputs, shared mux paths, actual state and readback, source/flat/generated
  replay, reservations and rollback. Distinct or gated paths must still fail
  without replacing prior output. Confirm positive tests fail before editing.
- [x] Derive transparent path identity in existing graph order, once per
  vertex/edge; coalesce equivalent roots without dropping graph vertices or
  controls. Preserve replay, bounds and stable endpoint selection. Expected:
  focused tests pass, including a 256-output/64-bit scale case.
- [x] Run full dynamic/static regression, leak-enabled sanitizers, style and
  naming checks, and independent review. Record measured scale and limits;
  update current ledgers without marking distinct-chain/TAP work complete.

## Task 15: Merge sliced and aliased scan observations

Read slices and aliases do not assign controlling fill. Resolve them through
the existing PDL target mapper and conservatively occupy every underlying
ScanRegister for the whole call, retaining conflict ordering even for distinct
bits of one register. Keep AccessTogether aliases, partial/alias/X writes and
non-scan reads as barriers. Reuse shared value parsing, scheduling/replay and
fresh-state fallback. This closes an observation batching gap without changing
write-fill or resource semantics. Target resolution is linear in target bits
apart from existing alias traversal and ordered-set insertions.

Files: `src/pattern/pattern_merge.cc`, existing
`tests/src/pattern/test_merge_observations.py`, support docs.

- [x] Add RED cases for slices, inverted/nested/concatenated aliases, no-value
  reads, multi-apply order and 64 independent two-apply sliced observations;
  preserve physical-register conflict ordering and AccessTogether barriers.
- [x] Classify read targets by resolved physical ScanRegister bits; retain
  existing write restrictions and metadata. Expected: focused merge suites and
  source/flat/generated replay pass, with the 64-call workload using two applies.
- [x] Run full dynamic/static regression, focused leak-enabled sanitizers,
  style/naming and independent review; record bounds, results and remaining work.

## Task 16: Reset cancels unapplied reads and writes

IEEE 1687-2014 §7.9.9(a), p164/PDF178, resets controlling and expected state.
Six Tessent 2026.2 probes (write/read/both, async/sync) confirm pending operations
are discarded, not applied before or after reset. Retained script:
`tests/src/pattern/probe_pending_reset.tcl`; remote evidence under
`test_icl_toolsets/test_pending_reset/run_2026_11_13`, local ignored copy under
`build/pending-reset-probe`. This observed queue rule supplements the standard.

Reuse Runtime pending tracking, Generate's queue and ResourceReservations::Reset.
Validate queued target/value spelling without scheduling canceled operations.
Reset-only grouping treats reset as a queue boundary; ResetTargets excludes
reads/writes canceled before apply, without dropping earlier applied accesses.
Keep immutable procedures, call-origin diagnostics and staged output publication.
Work is linear in operations apart from existing target/value resolution.

Files: `pattern_runtime.cc`, `icl_pattern_generation_main.cc`,
`pattern_scheduler.cc`, `pattern_values.h/.cc`, native runtime tests, existing resource tests,
new `tests/src/pattern/test_pending_reset.py`, Makefile and support docs.

- [x] Add RED tests for async/sync queues, end-of-stream reset, queues across
  files/calls, resource release, reset-only rewrite without spurious preambles,
  post-reset access, malformed discarded target/value and atomic publication.
- [x] Clear runtime/generator queues at successful reset, retain immediate
  target/value validation, and exclude canceled accesses from reset planning.
  Expected: native/runtime, focused CLI and resource checks pass; original,
  flat/generated and reset-only replay agree.
- [x] Full dynamic/static regression, focused leak-enabled sanitizers,
  style/naming and independent review; document vendor observations separately
  from certification and retain remaining model requirements.

## Task 17: Enabled observations of one physical scan path

IEEE 1687-2014 §6.4.6.2 qualifies observation with ScanOutPort Enable; two
observations converging before storage do not add a clocked register or shift
event. Reuse the existing selected-root solver and concrete serial replay.
Retain each gated observation and its enable constraint, while computing a
separate physical-path identity through transparent forwarding. Permit several
roots only when those physical identities agree. Distinct paths still need
the simultaneous event work below; never remove that guard indiscriminately.
Compute identities in existing graph order in O(V+E), with no per-root traversal.

Files: `src/pattern/pattern_scheduler.cc`, existing native scheduler and
`tests/src/pattern/test_shared_scan_outputs.py`, user/model documentation.

- [x] Add RED tests for constant, unknown and changing enables, forwarding,
  inversion and vector observations. Test all-disabled/unknown-only failure,
  reservation rollback, actual native state, source/flat/generated replay and
  prior-output preservation. Retain distinct-path rejection.
- [x] Separate physical identity from equivalent-observation deduplication;
  reuse existing root selection, enable constraints and replay. Expected:
  focused CLI/native cases pass without bypassing declared interface controls.
- [x] Verify full dynamic/static regression, focused leak-enabled sanitizers,
  formatting/naming, scale and independent review; record results and continue
  the remaining tasks.

## Task 18: Standalone client scan-interface controls

Reuse the existing single-path scheduler before extending its transaction shape.
IEEE 1687-2014 §6.4.16 defines interface membership, §6.7 defines omitted control
behavior and §6.8 defines active levels. Tessent v2023.4 IJTAG User's Manual,
"Checking ICL Module Ports", defines anonymous single-chain interface inference.
Retain EffectiveScanInterfaces and the concrete executor's register bindings.
For a standalone client, supply the external capture/shift/update phases at its
unconnected top-level control ports. Evaluate explicitly connected child
controls and clocks rather than ignoring them. Missing controls retain the
existing active-path defaults.

Extend the current phase-equivalence check to standalone clients: each modeled
event must agree with the selected complete-scan path. Independently timed or
hidden events that fail that proof remain for the general event task below.
Concrete replay must derive controls from actual interface bindings and sampled
clock levels independently of the symbolic proof, and check all storage and
reservations. Do not implement this by deleting the declared-control rejection
and retaining unconditional path-only replay.

Files: `src/pattern/pattern_scheduler.cc`, shared interface-control code only
where needed, existing native scheduler tests, new
`tests/src/pattern/test_client_scan_controls.py`, Makefile and support docs.

- [x] Add RED native/CLI cases for explicit and inferred clients, nested
  connected controls, top-level control pins, inverted/gated enables,
  disabled/unknown select/shift/update and absent clock edges. Cover actual
  register state, primary/scan interaction, original/flat/generated replay,
  reset rewriting, reservation rollback and failed-output preservation.
- [x] Reuse interface membership and phase constraints for standalone clients;
  implement concrete phase replay with actual controls. Keep unsupported
  client-TAP, unmatched controls and distinct-chain diagnostics. Expected:
  focused tests pass, including negative cases that would pass if controls were
  ignored. Measure a 64-register explicitly controlled network.
- [x] Run full dynamic/static regression, focused leak-enabled sanitizers,
  style/naming and independent review. Record remaining event limitations and
  continue to simultaneous-chain/control event support.

## Task 19: Simultaneous independent chains in one client interface

IEEE §6.4.16 requires the chains in one interface to react simultaneously.
Reuse the selected-root graph, per-register constraints and client phase proof.
Use the declared `Chain` SI/SO pairs; select all chain outputs together.
Unlisted observation ports cannot replace a declared output. This increment requires exactly one top-level client and disjoint serial
storage/input paths. Shared storage/inputs and multiple controllers retain an
explicit diagnostic pending general serial-event solving.

Concrete replay derives one input stream per selected path, pads shorter chains
at the beginning and clocks every active chain together for the longest path.
Capture/update occur once and state/reservations are checked after every edge.
Expose lane metadata without changing existing single-lane ScanStep fields.
Use graph-order path lengths to optimize actual simultaneous clock count.
Bound graph processing by existing limits, avoiding per-chain graph scans.

- [x] Add failing native and CLI tests for unequal widths, indexed input/output
  lanes, inverted paths, gated observations, cross-chain SIB prerequisites,
  shared primary/scan controls, grouped access, partial writes and reservations.
  Include shared-input/storage rejection, output safety, actual input/register
  state and source/flat/generated fixed points.
- [x] Reuse serial graph ownership and common-state constraints; implement
  simultaneous concrete replay and per-lane metadata. Prove constant-path
  phase equivalence and retain independent state/replay checks.
- [x] Measure representative many-lane inputs; run focused/full dynamic/static
  tests, leak-enabled sanitizers, Google checks and independent review. Update
  support records and continue shared/overlapping path and controller work.

## Task 20: Hidden storage events on standalone clients

Extend the existing TAP capture/shift/update unroll to standalone clients,
including the disjoint simultaneous chains from Task19. Keep held and shift
storage separate and reuse EffectiveScanControl in independent replay. Derive
which primary scan-input bits are driven and each selected chain's length from
the same selected-route graph; pad shorter streams with zero, matching concrete
replay. Preserve unused bits and retain the last driven input across frames.
Bound symbolic event work by clocks × modeled storage/input width and the
existing solver timeout; retain the proved complete-scan fast path.

This step handles hidden storage with phase-stable controls. Serial-derived or
otherwise phase-varying event sequences still need the general event work; do
not label those complete or reinterpret failures as hardware unreachability.

- [x] Add RED native/CLI cases for hidden capture/shift/update, disabled capture,
  hidden unknown values, resource preservation and rollback, indexed inputs,
  and unequal simultaneous chains. Verify actual hidden state as well as
  source/flat/generated replay; preserve outputs for unknown/blocked controls.
- [x] Reuse the event equivalence probe and bounded unroll for clients. Derive
  per-input selection and path lengths without per-input graph scans. Replay
  actual client events, publish only visible writes, and compare all storage.
  Keep clock/pin histories consistent across frames and input padding explicit.
- [x] Measure representative larger hidden storage and the work-limit failure;
  run focused/full dynamic/static checks, leak-enabled sanitizers, Google checks
  and independent review. Record remaining event/model requirements.

## Task 21: Masked and aliased merge writes

IEEE §7.9.18 shares controlling state across merged calls while preserving each
call's apply order. Extend the existing Batchable target classification to
ScanRegister write slices, aliases and X masks. Retain whole-physical-register
occupancy across each stream so interleaving calls never share controlling fill
for one register. Keep AccessTogether, resource, wait, empty-apply and opaque
barriers, normal operand validation, shared scheduling and fresh-state fallback.
No dependency or alternate merge engine is needed; conflict lookup remains
ordered-set based and batch traversal linear in operations.

- [x] Add RED tests for partial/X/alias writes, inherited fill and last-write
  order within a call, observations across apply depth, physical alias conflicts,
  fresh-state fallback, failure publication and 64-call scale. Update prior
  conservative expectations to the newly supported behavior.
- [x] Remove only the full-direct-known-write restriction once resolved physical
  target bits all name ScanRegisters. Parse operands as before and retain the
  shared scheduler's fill semantics, replay and barriers.
- [x] Verify both merge suites, full dynamic/static regression, leak-enabled
  sanitizer replay, style/naming, scale and independent review. Retain broader
  target/event optimization requirements separately.

## Task 22: Whole-command reset prerequisites for main Tcl compounds

Retain the existing source-preserving rewriter and trace complete top-level Tcl
commands as caller actions. Hoist release prerequisites before a main loop or
conditional, capturing its entry prefix and the whole executed operation span.
Retain literal-site actions as a fresh-state fallback for already repairable
loop bodies that reset internally. Verify either rewritten result against the
expected operation stream before publication. Procedure definitions remain
byte-for-byte intact. This handles repairable
compound sites without inserting an apply between queued writes on successive
iterations. Word-substitution resets and internal reset reassertions still need
separate classification; they must never silently change execution order.

Approaches considered: rewriting loop bodies changes repeated execution;
separately evaluating each top-level command changes Tcl return semantics;
marking command entry/exit in the existing instrumentation preserves Tcl's
normal evaluator and needs no new parser. Use the latter only when source tracing
is requested. Close an outstanding command at file evaluation completion, and
retain whole-merge spans if a compound starts a merge continued by later source.
This is an implementation choice under the existing autonomous authorization.

Files: pattern_runtime.cc/.h; tests/src/pattern_runtime_test.cc; new
 tests/src/pattern/test_compound_reset_sources.py; Makefile; support docs.

- [x] Add RED whole-loop/prefix/conditional/queue tests, native source-span
  assertions, procedure preservation, merged/multi-file cases and output safety.
  Check generated/flat/reset-source replay and rewrite fixed points.
- [x] Record complete main-command actions in existing instrumentation. Keep
  ordinary PDL sites, procedure execution, Tcl control flow and safe interpreter
  validation unchanged; retain original text and final operation-order checking.
- [x] Verify focused runtime/CLI, full dynamic/static checks, leak-enabled
  sanitizers, style/naming, representative loop scale and independent review.
  Document repairable scope and remaining caller-only rewrite requirements.

## Task 23: Merge independent DataRegister access streams

Extend the existing physical-register occupancy classification to DataRegisters
as well as ScanRegisters. Reuse ordinary primary/scan/mixed data scheduling,
read obligations, partial-write controlling fill and fresh-state serial fallback.
Shared address/data/enable signals remain scheduler constraints; they are not
independent storage ownership. Keep all existing opaque/resource/timing and
AccessTogether barriers. This increment does not add computed-port target
batching, whose underlying physical ownership needs separate analysis.

Alternatives: leave all data calls serialized (misses existing common-frame
capability), add another merger (duplicates ordering/state), or extend the
current supported-register classification. Choose the latter under the existing
autonomous scope. Target 64 independent two-apply calls, with constant-width
registers and existing bounded solver/planner behavior; no optimality claim.

Files: pattern_merge.cc; new tests/src/pattern/test_merge_data_registers.py;
Makefile and support records. Reuse existing merge/data fixtures and test helpers.

- [x] Add RED tests for independent primary/serial/mixed registers, read/write
  apply order, partial/aliased/X targets, shared bus constraints, conflicts,
  resource barriers, fallback/failure safety and representative scale.
- [x] Extend physical-register classification only; retain validation, whole-
  register occupancy, source order, common scheduler and concrete replay.
- [x] Verify source/flat/generated fixed points, focused merge/data tests, full
  dynamic/static regression, leak-enabled sanitizers, style/naming and independent
  review. Record measurements and remaining broader merge requirements.

## Task 24: Merge direct primary-input access streams

Reuse the same whole-object ownership pass for unconnected top-level DataInPort,
AddressPort, ReadEnPort and WriteEnPort targets. Alias slices resolve to those
physical inputs before classification. Retain a barrier for connected child
ports and computed outputs until their underlying ownership is resolved; never
classify a forwarding port as independent from its source. Shared scheduling
still checks reads, register effects, reservations and final controlling state.

Use the existing reservable-kind classifier plus explicit top/input/unconnected
checks, with no new dependency graph or scheduler. Expected scale: 64 two-apply
calls, constant-width ports; measure replay and retain ordered-set complexity.
Files: pattern_merge.cc; new tests/src/pattern/test_merge_primary_inputs.py;
Makefile and current support records.

- [x] Add RED cases for direct/aliased/masked primary writes, physical conflicts,
  primary control ports, mixed register streams, connected-child barriers,
  invalid operands/output safety and representative scale.
- [x] Extend Batchable only for actual unconnected primary input storage. Keep
  shared operand validation, per-call ordering, opaque/resource barriers and
  fresh-state fallback; do not admit DataOutPort or connected input targets.
- [x] Run focused and full dynamic/static checks, leak-enabled sanitizer replay,
  style/naming, scale and independent review; update limits accurately.

## Task 25: Use the complete generator suite in strict coverage

The coverage recipe currently runs only six generator CLI suites while dynamic
and static checks run the growing complete list. Reuse one ordered recipe/list
for all three binaries so current tests reach the authored-code coverage gate.
Keep 100% line/90% raw-branch thresholds and the existing native/editor coverage;
missing coverage remains an acceptance failure, never a reason to lower limits.

Files: Makefile; existing tests/tests/test_makefile.py for dry-run command-set equivalence;
tests/coverage.py and its tests for complete authored-source accounting;
tests/lsp_test.py for an isolated workspace; coverage evidence and remaining-gap ledger.
A shared Make recipe removes duplicate
lists without changing suite order or dropping exit failures. Do not override
the generator executable variable in a recursive make, which would relink the
coverage binary using ordinary objects.

- [x] Reproduce missing coverage suites with a dry-run test that compares the
  actual planned generator invocations for dynamic/static/coverage builds.
- [x] Share the generator suite recipe, keeping all existing suites and native
  checks; verify command-set/order equality and first-failure propagation.
- [ ] Run instrumented tests and the unchanged strict gate, retain the report,
  address uncovered reachable authored paths and record residual platform or
  tool constraints honestly. A run below thresholds is unfinished acceptance.

## Task 26: Source distribution, installation and Rocky Linux 8 qualification

Use the existing source archive and manifest-based installers in isolated local
and remote prefixes. The authorized lianghuaxiao host is Rocky Linux8.10 with
GCC8.5/default Make4.2.1, installed GCC Toolset15, CMake3.26.5 and Tcl8.6.8.
Use the installed C++17 toolset; build the required newer GNU Make and the already
pinned Z3 release only inside test_icl_toolsets/test_platform or /tmp. Reuse the
verified local Z3 tarball hash. No personal installation or editor configuration
changes. Bound dependency builds to two parallel jobs.

- [x] Prepare isolated prerequisites and retain versions, archive hashes and
  commands. Build the actual source archive on Rocky8 with Bison/Flex disabled.
- [x] Install all five commands into a temporary prefix; run installed formatter,
  LSP and pattern original/flat/generated/reset-rewrite checks. Verify uninstall
  preserves unrelated files. Repeat local dynamic/static installation checks.
- [x] Run static release binaries on the actual Rocky8 host and inspect dependency
  behavior. Rebuild/fix genuine compatibility failures, preserve matching Tcl
  headers/libraries, and record the precise tested toolchain and remaining limits.

## Remaining completion work (not yet completed)

- Multiple TAPs and intermediate storage taps: extend common-state event planning beyond selected clients, named-chain groups and ShiftEn-only selection completed in Tasks29–31.
- General independently gated/serial-derived events, feedback/register callbacks, system-clock/time behavior: establish each model's documented event semantics before implementation.
- Additional reservations and protected reset/lifecycle behavior: extend resource ownership and transitions consistently across backends.
- General caller-only reset rewriting: classify repairable source sites and impossible immutable-procedure cases; retain diagnostics for genuinely impossible transformations.
- Broader merge optimization: preserve reads, last-write semantics, barriers and reservations while grouping compatible work; benchmark representative scale.
- Synchronize current requirement/model coverage ledgers; complete defaults/reference evidence and vendor qualification. Do not call corpus parser coverage generator coverage.

These items require their own concrete test-first steps after inspecting their connected code. This plan does not redefine them as optional or completed.

## Verified execution record

Tasks 1 and 2 passed the final full dynamic/static `make check`, including the
reviewer's late-failure rollback test: `build/pattern-completion-regression-reviewed.log`
(exit 0). Seven new one-hot tests and eleven merge tests pass under
ASan/UBSan/LSan; native symbolic/state tests pass with the same sanitizers.
Independent review found no Important/Critical issues; the retained wide-group
constraint complexity is documented with the measured 32-contributor scale.
Google formatting, changed-source naming and diff whitespace checks pass.
See [implementation evidence](../../pattern-generation.md#one-hot-serial-paths-and-multi-apply-merges-2026-10-29).
No fresh vendor certification, installation or strict coverage run was performed.
The user authorized committing and pushing this verified increment. The remaining
requirements above are still open and execution continues after publication.


Task 3 passed the final full dynamic/static `make check` (exit 0):
`build/resource-slices-regression-reviewed.log`. All 31 reservation cases and
native scheduler tests also pass Clang ASan/UBSan/LeakSanitizer with leak checks
enabled. Independent review found and verified the fix for unknown unwritten
reset bits; no findings remain. Google formatting, changed-source clang-tidy and
diff whitespace checks pass. Five fresh Tessent probes establish bit semantics,
but are not certified round-trip goldens; their ungated hidden TAP storage
exposes an additional outstanding event-model limitation. See
[bit-level evidence](../../pattern-generation.md#bit-level-reservations-2026-10-29).
The remaining completion work above is still required, not marked complete.


Task 4 passed full dynamic/static regression (`build/hidden-regression-final.log`),
29 directional and seven legacy offline vendor standards, and leak-enabled
ASan/UBSan native and CLI tests. Eight newly recorded Tessent 2026.2 certificates
cover four original shadow-storage reservation probes in both directions.
Independent review found no correctness issues. Wide shadow paths use existing
dependency-guided batches above 32 scan bits; smaller paths retain joint planning.
A 32/128-bit original/flat/generated test and 1024-bit work-limit rejection pass
in 2.89 s / 131484 KiB. Original joint planning timed out on those wide cases.
Final additional native coverage exercises protected hidden storage rollback and
reset for both 4-bit joint and 32-bit greedy paths. Current event restrictions
and the remaining requirements are retained in the generator/model guides.


Task 5 passed `make check` dynamic/static regression (exit 0,
`build/reset-ownership-regression.log`) and all 34 resource tests under Clang
ASan/UBSan/LeakSanitizer (`detect_leaks=1`, 43.288 s). Google formatting,
changed-source naming and whitespace checks pass. Independent review found no
blocking issue and additionally checked local/persistent post-reset retakes.
Vendor observations are scoped to ordinary takes; other lifetime variants have
local coverage and are not individually vendor-certified. No broader lifecycle
or other remaining requirements are marked complete.


Task 6 passed full dynamic/static `make check` (exit 0,
`build/data-resources-regression-reviewed.log`), all nine new data-resource
cases, all 34 original reservation cases and native scheduler tests under
Clang ASan/UBSan/LeakSanitizer. Independent review found a stale protected-value
handoff between address batches; full/partial 32-register replacement tests
failed first, then passed after the fix. Formatting, naming and whitespace
checks pass. Source/flat/generated replay and failed-output protection are
covered; no additional vendor certification is claimed. See
[data-resource evidence](../../pattern-generation.md#data-resource-reservations-2026-10-29).


Task 7 passed the final full dynamic/static `make check` (exit 0,
`build/mixed-io-regression-final.log`), including 31 directional and seven legacy
offline vendor standards. Native scheduler tests, 15 mixed-I/O, nine data-resource,
34 reservation and four hidden-event cases passed ASan/UBSan/LeakSanitizer with
`detect_leaks=1` (`build/mixed-io-sanitizer-final.log`). Focused native, mixed and
25 scan-data tests also passed after the final primary-input helper extraction.
Google formatting, changed-source clang-tidy and whitespace checks pass. Two
fresh Tessent forward certificates qualify the mixed fixtures; reverse apply
comparisons remain failures and no reverse standards were recorded. Independent
review findings were reproduced and fixed, with actual-state and TAP rollback
coverage. No broader model completeness or shortest-pattern guarantee is claimed.


Task 8 passed full dynamic/static `make check` (exit 0,
`build/primary-access-regression-reviewed.log`), 31 directional and seven legacy
offline vendor standards, and native plus 13 primary-group, 15 mixed-I/O and nine
data-reservation cases under ASan/UBSan/LeakSanitizer (`detect_leaks=1`).
Independent review found overly strict implicit-pin goals; a new partial-alias
case reproduced the failure before the fix. The initial full regression failed
an obsolete unsupported-feature assertion; the replacement verifies actual whole-
group drives and output state, and the final full run passed. Google formatting,
changed-source clang-tidy and whitespace checks pass. No new vendor certification
or completion of the remaining model requirements is claimed.


Task 9 passed final full dynamic/static `make check` (exit 0,
`build/mixed-port-regression-verified.log`), including 31 directional and seven
legacy offline vendor standards and the complete Vim/Neovim corpus checks.
Native scheduler plus 26 mixed-port, 13 primary-group, 15 mixed-I/O and nine
data-resource CLI cases pass ASan/UBSan/LeakSanitizer with `detect_leaks=1`
(`build/mixed-port-sanitizer-final.log`). Formatting, naming and whitespace
checks pass. Independent review findings were reproduced and repaired, including
knownness activity and reconvergent controller dependencies; no findings remain.
The 64-level graph replay takes 0.15s / 59040 KiB. No new vendor certification or
completion of the broader remaining model requirements is claimed.


Task 10 passed final full dynamic/static `make check` (exit 0,
`build/merge-observations-regression-final.log`), 31 directional/seven legacy
standards and full editor corpus. Native scheduler plus ten new and eleven
existing merge cases pass Clang ASan/UBSan/LeakSanitizer with leaks enabled
(`build/merge-observations-sanitizer-final.log`). Google style/naming/whitespace
checks pass; independent review has no findings and its reset-conflict fallback
probe is retained. Initial regression failed the omitted-expected read case on
a stale static binary; the final full rebuild/run passes. The 64-call/128-read
example drops from 128 applies to two; three-replay timing 0.27s / 60288 KiB.
No global optimality or new vendor certificate is claimed.


Task 11 passed full dynamic/static make check (exit0,
`build/parallel-control-regression.log`), 31 directional/seven legacy standards,
full editor corpus, native scheduler and9/9/34 CLI suites under ASan/UBSan/LSan
with leaks enabled (`build/parallel-control-sanitizer.log`). Naming/style/diff
checks pass. Independent review confirmed scalar/partial parent-protection holes;
original logical per-bit assignments fix both, with RED/GREEN cases and actual
state/rollback checks. Initial invalid wire-inversion/data-input bindings were
corrected against IEEE6.4.6.25(c)/26(d), without relaxing AddressResolver.
64 enable ports/three replays:0.16s/58848KiB. No new vendor certificate or broader
serial-control/clock/lifecycle support is claimed.

Task 12 passed full dynamic/static `make -j2 check` (exit 0,
`build/port-control-vendor-regression-final.log`), including 37 directional and
seven legacy offline standards, 1,916 ICL regression cases, 2,180 conformance
cases and the Vim/Neovim corpus checks. All 334 parser and 119 scanner rules
were reached across 2,501 ICL files. Five optional ANTLR integration tests were
skipped because `ICL_REF_DIR` was unset; this run does not establish that
independent comparison. The three new fixtures pass source/flat/generated
replay under ASan/UBSan/LeakSanitizer with `detect_leaks=1`
(`build/port-control-vendor-sanitizer-final.log`). An extra sanitizer invocation
initially selected an obsolete binary and encountered the sandbox's LSan
restriction; the final run uses `build/local-reset-sanitize/icl-pattern-generation`
outside that restriction without disabling leaks. Independent review checked
all six certificates against staged inputs, hashes and vendor logs and found no
actionable issue. Whitespace checks pass. These are fixture-specific certificates,
not completion of the remaining model requirements.

Task 13 passed corrected full dynamic/static `make -j2 --jobserver-style=pipe
check` (exit 0, `build/vector-input-regression-final.log`), including 37
directional/seven legacy offline standards and Vim/Neovim corpus checks. All
334 grammar and 119 scanner rules were reached across 2,504 ICL files. Five
optional ANTLR tests remain skipped because `ICL_REF_DIR` was unset. Native
scheduler tests, seven new vector-input cases and seven existing one-hot cases
passed ASan/UBSan/LeakSanitizer with `detect_leaks=1`
(`build/vector-input-sanitizer-final.log`). Changed-source Google formatting,
clang-tidy naming and whitespace checks pass. Independent review's declared-
control bypass was reproduced RED and repaired GREEN before the final regression.
A 4096-bit input/64-bit register through three replay forms took 0.16s/57,700 KiB.
No new Tessent certificate or simultaneous vector-output support is claimed.

Task 14 completed in `3fa4630`. Full dynamic/static `make check`, native scheduler
and five CLI cases under ASan/UBSan/LeakSanitizer, style/naming and independent
review passed. The 257-output/64-bit case took 0.154 s for three replay forms.
Distinct paths and multiple TAPs remain open. See
[shared-output evidence](../../pattern-generation.md#shared-scan-output-observations-2026-11-13).

Task 15 completed in `85745b9`. Full dynamic/static `make check`, 13 observation
cases under ASan/UBSan/LeakSanitizer, style/naming and independent review passed.
The 64-call sliced-read case uses two applies instead of 128; no runtime speedup
or general optimality is claimed. See
[merge-slice evidence](../../pattern-generation.md#sliced-and-aliased-merge-observations-2026-11-13).

Task 16 passed the final full dynamic/static `make check` (exit 0,
`build/pending-reset-regression-final.log`), native runtime/scheduler tests and
seven reset/34 resource CLI tests under ASan/UBSan/LeakSanitizer with leak
detection enabled. Independent review's canceled target-kind validation issue
was reproduced RED and fixed before final verification. Style/naming/whitespace
checks pass. Six Tessent 2026.2 probes establish queue discard for write/read/both
and async/sync reset; these are observations, not new qualification certificates.
See [reset-queue evidence](../../pattern-generation.md#reset-cancels-queued-accesses-2026-11-13).
All broader remaining requirements above remain open.

Task 17 passed full dynamic/static `make check` (exit 0,
`build/enabled-output-regression.log`), native scheduler and nine CLI cases
under ASan/UBSan/LeakSanitizer with leak detection enabled, style/naming/
whitespace and independent review. Positive gated observations failed before
implementation. The 257-output/64-bit case took 0.234 s for three replay forms.
Five optional ANTLR checks remain skipped. See
[enabled-output evidence](../../pattern-generation.md#enabled-observations-of-one-scan-path-2026-11-13).
Task 18 verification is recorded below; Task 19 and the broader requirements remain open.

Task 18 passed full dynamic/static `make -j2 --jobserver-style=pipe check`
(exit 0, `build/client-controls-regression-verified.log`), 38 directional/seven
legacy offline standards, all334 parser/119scanner rules over2505ICL files and
editor corpus checks. Five optional ANTLR checks remain skipped. Native scheduler
and ten CLI cases passed ASan/UBSan/LeakSanitizer with detect_leaks=1
(`build/client-controls-sanitizer-{native,cli}-verified.log`). Google formatting,
naming and whitespace checks pass. Independent source and certificate review
found no remaining issues after the parallel-frame phase fix. The64-register
case took0.167s for three replay forms; full10case suite4.56s/60504KiB. Only
reverse Tessent ordered-apply equivalence certified; see
[client evidence](../../pattern-generation.md#standalone-client-scan-controls-2026-11-13).

Task 19 passed full dynamic/static `make -j2 --jobserver-style=pipe check`
(exit 0, `build/simultaneous-chains-regression.log`), including the native and
nine CLI cases, seven legacy vendor standards, all 334 parser/119 scanner rules
over 2,506 ICL files and editor corpus checks. The full run checked 38 directional
standards; the separately added certificate passed all 39 offline standards
(`build/simultaneous-chains-certificates.log`). Five optional ANTLR checks remain
skipped. Native scheduler and nine CLI cases passed ASan/UBSan/LeakSanitizer with
leaks enabled. Google style/naming/whitespace and independent source/certificate
review passed. See [simultaneous-chain evidence](../../pattern-generation.md#simultaneous-independent-client-chains-2026-11-13).
Task 20 verification is recorded below; the remaining model/qualification requirements stay open.

Task 20 completed the full dynamic/static regression
(`build/hidden-client-regression-reviewed.log`), including 40 directional/seven
legacy standards, all 334 parser/119 scanner rules over 2,507 ICL files and
editor corpus checks. Five optional ANTLR checks remain skipped. Native
scheduler, eight hidden-client and four TAP-shadow CLI cases passed
ASan/UBSan/LeakSanitizer with leaks enabled. Google formatting/naming/whitespace
and independent source/certificate review passed after the primary-enable
proof fix. See [hidden-client evidence](../../pattern-generation.md#hidden-standalone-client-events-2026-11-13).

Task 21 passed full dynamic/static `make -j2 --jobserver-style=pipe check`
(exit 0, `build/merge-masked-regression.log` and `.exit`), including 40 directional/
seven legacy standards, native tests, all grammar/scanner rules over 2,507 ICL
files and the editor corpus. Five optional ANTLR checks remain skipped. Both
merge suites (20 + 11 cases) passed ASan/UBSan/LeakSanitizer with leaks enabled;
formatting/naming/whitespace and independent review passed. The full regression
also verifies the committed Task20 implementation. See
[masked-write evidence](../../pattern-generation.md#masked-and-aliased-merge-writes-2026-11-13).

Task 22 passed reviewed full dynamic/static `make -j2 --jobserver-style=pipe
check` (exit 0, `build/compound-reset-regression-reviewed.log` and `.exit`),
including 40 directional/seven legacy standards, native tests, all parser/scanner
rules over 2,507 ICL files and editor corpus checks. Five optional ANTLR checks
remain skipped. Native runtime, twelve compound-source and 25 existing CLI
cases passed leak-enabled ASan/UBSan/LeakSanitizer. Formatting/naming/whitespace
and independent review passed; the merge-overlap finding was reproduced before
its fix. The earlier full run was stopped with143 for that fix and is not final
evidence. See [compound-source evidence](../../pattern-generation.md#main-tcl-command-reset-prerequisites-2026-11-13).

Task 23 passed full dynamic/static `make check` (exit 0,
`build/merge-data-regression.log` and `.exit`), including thirteen new data-merge
cases, 40 directional/seven legacy standards, native tests, all parser/scanner
rules over 2,507 ICL files and the editor corpus. Five optional ANTLR checks
remain skipped. All new cases passed leak-enabled sanitizers (initial twelve
plus the added final-state/revised fallback checks); existing merge and data
suites, style/naming/whitespace and independent review pass. The reviewed reset
interleaving follows IEEE7.9.18 common-state semantics, verified with explicit
post-merge reads and a reserved serial variant. See
[data-merge evidence](../../pattern-generation.md#merged-dataregister-streams-2026-11-13).

Task 24 passed full dynamic/static `make check` (exit 0,
`build/merge-primary-regression.log` and `.exit`), including eight primary-merge
cases, 40 directional/seven legacy standards, native tests, all parser/scanner
rules over 2,507 ICL files and editor corpus checks. Five optional ANTLR checks
remain skipped. Eight new cases passed ASan/UBSan/LeakSanitizer with leaks
enabled; all44 existing merge cases, formatting/naming/whitespace and independent
review pass. No new vendor certificate is claimed. See
[primary-merge evidence](../../pattern-generation.md#merged-direct-primary-input-writes-2026-11-13).

Task25's invocation and build fixes passed independent review, twelve Makefile
tests, seven coverage-reader tests and the full instrumented test sequence.
The strict gate remains failed at95.8% lines/61.4% raw branches; the final
checkbox stays open. See
[coverage evidence](../../pattern-generation.md#complete-generator-coverage-invocation-2026-11-13).

Task26 passed source-archive build and local dynamic/static installation,
Rocky8 native dynamic and transferred-static installation checks, and all26
generator CLI suites under both the host EDA runtime and matching system
Tcl8.6.8. Python3.8's missing Path.hardlink_to was repaired with os.link.
See [platform evidence](../../pattern-generation.md#source-archive-and-rocky-linux-8-qualification-2026-11-13).
Task25 coverage acceptance and the broader model requirements remain unfinished.

Task27 follows the [computed-port merge plan](2026-11-13-computed-port-merges.md).

Task27 passed the reviewed full dynamic/static regression (exit0,
build/merge-computed-regression-final.log),12 new CLI cases and native tests
under leak-enabled sanitizers, style/naming/whitespace and independent review.
The addressed one-hot ownership and shared analysis-budget findings were fixed
before final verification. See
[computed-port merge evidence](../../pattern-generation.md#merged-connected-and-computed-ports-2026-11-13).

## Task 28: Reset releases after argument evaluation

Completed [the focused plan](2026-11-13-argument-reset-sources.md). All three
source candidates retain fresh-runtime ordered-operation validation. New12CLI
cases, native metadata checks, full dynamic/static regression, leak-enabled
sanitizers, style/naming and independent review pass. Tessent2026.2 accepts and
exports whole/sliced generated wrappers; no additional equivalence certificate
is claimed. Immutable internal reassertions and other recorded limits remain.

## Task 29: Independently selected standalone clients

Completed [the focused plan](2026-11-13-multiple-client-interfaces.md). Selection,
route constraints, parallel frames, reset settlement and concrete replay share
one hardware state. Seventeen CLI cases and native SI/SO/state assertions cover
ordinary/shared storage, reset cascades, controlling history, resource limits
and failed-output preservation. Full dynamic/static checks, native/new17/old17
leak-enabled sanitizers, style/naming and independent review pass. Two Tessent
reverse certificates bring the offline total to42 directions plus seven legacy
standards. The guide records exact evidence and remaining limits. Task30 will
add named-chain groups to selectable clients; its tests remain outside this
checkpoint.

## Task 30: Named chain groups on selected clients

Completed [the focused plan](2026-11-13-multiple-client-chain-groups.md).
Selected clients enable all their chains with declared SI/SO pairing and common
events. Shared hardware state, output sharing across clients, padding and route
repair remain checked through concrete replay. Eleven CLI cases and native
lane/state assertions pass. Full dynamic/static regression, leak-enabled
native/new11/old17/old9 sanitizers, naming/style and independent review pass.
One reverse Tessent certificate brings the offline total to43directions plus
seven legacy standards. The guide records exact evidence and limits.

## Task 31: ShiftEn-only client selection

Completed [the focused plan](2026-11-13-shift-enable-clients.md).
Clients without SelectPort use ShiftEn membership, with all-owner uniqueness
constraints and phase-qualified symbolic/concrete pins. Eleven CLI cases and
native lane/state/rollback checks pass. Full dynamic/static regression,
leak-enabled native/new11/old17/old11 sanitizers, naming/style and independent
review pass. One reverse Tessent certificate brings the offline total to44
directions plus seven legacy standards. See the guide for exact evidence.

## Task 32: Unsized literal inversion validation

Completed [the focused plan](2026-11-13-source-literal-validation.md).
Shared constant/signal boundaries enforce explicit normative6.3.12(m) before
contextual sizing, including even inversion counts. The guide records the
conflicting6.3.11 example and Tessent2026.2 acceptance; vendor rejection agreement
is not claimed. Eight CLI and native cases, full dynamic/static regression,
leak-enabled sanitizers, naming/style and independent review pass.
Broader source-kind/hierarchy validation remains open.

## Tasks33–34: All-port reservations and DataRegister state

Completed the [reservation plan](2026-11-13-all-port-reservations.md) and
[state plan](2026-11-13-data-register-state.md). Membership covers every ICL port
without broadening read/write/state target contracts. Event-free DataRegister
state assignments reuse alias/slice/hierarchy handling; read-only planning
preserves state and fill in primary, mixed and hidden-event backends.

Eight new reservation methods,35existing reservation methods,11state methods
and native tests pass. Combined full dynamic/static regression, final
leak-enabled sanitizers, naming/style and independent review pass. Two reverse
certificates bring the offline total to46directions plus seven legacy standards.
The guide records exact evidence. DataRegister-controlled scan reset dependency
ordering is the next task; strict coverage and broader requirements remain open.
