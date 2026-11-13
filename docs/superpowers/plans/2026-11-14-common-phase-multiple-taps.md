# Common-phase multiple-TAP scheduling implementation plan

> Execute inline with superpowers:executing-plans under the existing authorization.

**Goal:** Schedule multiple TAPs with one TCK, distinct serial lanes and explicit
reset connectivity through the existing shared-state solver and concrete replay.

**Architecture:** Reuse simultaneous-chain scheduling: all TAPs follow the same
TMS/IR-or-DR phase and each TDI/TDO lane carries its own aligned data. Extend
per-instruction-register metadata and existing event constraints to every TAP.
Keep one NetworkState, one symbolic history and one TapExecution. Explicit reset
connectivity determines reset effects; do not synthesize virtual-reset ownership.

**Tech Stack:** Existing C++17, Z3, Tcl, native/Python tests; no dependencies.

**Spec:** [Approved design](../specs/2026-10-25-icl-pattern-generation-design.md),
shared-state, simultaneous-chain and multiple-TAP requirements. This is one
bounded stage; the remaining access-model requirements are not removed.

## Scope and approach

IEEE1687 §5.3 and §6.4.17(c) permit multiple access controllers/interfaces;
§6.4.6.17 requires equivalent logical TCK. Use the same actual top-level TCK
node initially. Distinct TMS pins or a shared TMS pin can carry the common
sequence; TDI/TDO lanes must have distinct physical scan storage/input paths.
Reuse CheckIndependentChains to prove the serial paths are disjoint while
retaining shared data, reset and control dependencies in the joint model.

Alternatives considered: one selected TAP per frame would need independently
parked state and a new frame selector; partitioned schedulers would lose shared
events. Common phases reuse the existing lane alignment, capture comparisons,
simultaneous update and resource checks. They may use more cycles than a schedule
that independently parks TAPs; no global optimality claim is made.

No inspected normative rule defines OR/AND/consensus for multiple implicit global
resets. First require reset dependencies to resolve through explicit input,
logic or storage connections, without virtual GlobalReset/AsyncGlobalReset.
Reject ambiguous virtual dependencies with a precise diagnostic. Preserve each
controller's BSDL instruction reset. iReset must also assert/release applicable
top-level ResetPorts; resetting only TAP controllers is insufficient. Root reset
ports follow their declared polarity, and explicit TRST wiring follows TRST.
Synchronous reset does not invent an asynchronous assertion on TRST-only wiring.
Mixed TRST availability requires common synchronous reset clocks when any TAP
needs them, ending with every controller in known Run-Test/Idle. Preserve the
existing rejection of TRST assumptions: ValidateStateTarget accepts only storage
and top-level DataInPort/SelectPort. CLI reproduction confirms TRST=0/x is
rejected before either shared state or the private controller can change; no
new readiness invalidation is needed. Do not broaden that assumption boundary.

Every lane participates in every common scan, including unrequested TAPs using
BYPASS, IDCODE or their current instrument path. Their capture, load, update,
fill, hidden events and reservations remain modeled. Do not drop an unrequested
lane or emit synthetic BSDL data registers as user PDL targets.

Independently parked controllers, equivalent but separate TCK roots, shared
intermediate serial storage, mixed IR/DR simultaneous phases and general implicit
multi-TAP reset ownership remain required follow-up work. Existing single-TAP
behavior and its implicit reset semantics must remain compatible.

## Files and interfaces

- Modify src/pattern/pattern_scheduler.cc: constructor chain setup and limits,
  per-IR metadata, Reset/Cycle/Idle, EventPins/ControlEvents/ScanEvents,
  Solve/Replay and target validation. Audit every existing tap_ use.
- Modify src/pattern/pattern_scheduler.h comments and CLI help for the proved
  boundary; preserve ScanLane/ScanStep output contracts.
- Reuse TapExecution's shared-edge fix from Task47. Do not split execution state
  or prematurely synchronize instruction/serial storage.
- Maintain tests/src/pattern_scheduler_test.cc; add
  tests/src/pattern/test_multiple_tap_scheduling.py and register its dynamic/static
  and coverage invocations in Makefile. Fixtures live under
  tests/cases/ours/pattern_generation/multiple_taps.
- Update current support/model/spec/plan records and retain vendor probe logs.

## Implementation and verification

- [x] Review this design against actual scheduler callers and reset semantics.
  Finish the small vendor loading probes; distinguish loader failures from model
  semantics and do not treat them as a standards prohibition or certificate.
- [x] Add failing native/CLI cases: two independent TDRs written/read together,
  shared TMS, unequal IR and DR lengths with aligned lanes, explicit IR writes
  and reads on each TAP, source/flat/generated replay and canonical fixed points.
  Assert held/shift values and one common update for cross-TAP AccessTogether.
  Also target only one TAP while an unequal-width unrequested lane contains held
  and reserved data. Explicitly read one IR while a differently sized IR retains
  its distinct implicit BSDL capture comparison.
- [x] Add reset tests for both polarities of explicit root ResetPort, TRST-only
  wiring, async/sync controller reset, missing initial reset and ambiguous virtual
  dependencies. Add shared reset/data/control dependencies and reservations,
  preserving state and all existing output files on failure.
  Cover mixed TRST availability and TRST=0/x assumptions followed by scans/idle,
  including a later TRST=1 assumption without reset. Reject separate TCK roots
  and shared serial storage atomically as well as virtual reset dependencies.
- [x] Replace the blanket multiple-TAP guard with the proved topology/reset
  boundary. Build common-phase root/input pairs with existing chain validation;
  index every IR and synthetic BSDL data register, and protect every TAP pin from
  primary-write planning. Retain bounded solver/event work.
- [x] Apply phase/instruction/capture metadata for every TAP. Reuse existing
  longest-lane padding and symbolic serial inputs. Replay all lane TDI bits before
  each common edge and drive all TMS pins consistently. Preserve common capture,
  update, reset settlement, per-edge reservations and trial-only publication.
- [x] Keep IR/DR access-class validation across all TAPs: multiple IRs can share
  a group, as can multiple DRs; a mixed IR/DR group needs independent-phase work.
  Emit implicit IR capture comparisons only for IRs not explicitly requested.
  Disable only the single-path greedy shortcut for multiple TAPs until it has a
  multi-path proof; retain joint bounded planning and clear budget diagnostics.
- [x] Validate mixed primary/scan frames, hidden events, end states, reset-only
  rewriting and resource lifetimes on the new boundary. Run 4/16/32-TAP scale
  cases with representative unequal widths, recording input size, wall time,
  memory and toolchain. Existing limits remain mandatory.
- [x] Obtain independent source review; run focused checks, complete dynamic/
  static regression, leak-enabled sanitizers and source style/naming. Update
  evidence, limits and vendor differences accurately; commit only verified work.

Review focus: stale first-TAP-only logic; unequal lane padding/IR capture widths;
simultaneous cross-TAP updates and reads; reset ownership versus scan ownership;
protected hidden storage and atomic failure. Each is covered by the tests above.

The initial Tessent2026.2 probe passes a single explicit-reset TAP but both
two-AccessLink cases fail at set_current_design with an empty filename diagnostic
and SIGSEGV. The separate-clock case fails the same way. A single TAP referring
to the second entity fails earlier: the loader reads ProbeA and rejects requested
ProbeB, despite both entities being in the file. Thus this loading route cannot
provide multi-entity semantics evidence; it does not show the IEEE model invalid.
Evidence is retained in build/multiple-tap-probe-evidence and
build/multiple-tap-loading-probe-evidence. The initial implementation is isolated in .worktrees/tap-edge-snapshot, based on2d08124.

Independent design review confirms feasibility and the common-phase, unrequested
lane and per-IR comparison obligations recorded above. Its suspected TRST
assumption gap was disproved by the shared validator and focused CLI tests;
the initial failed test expectation is retained, then corrected to preserve the
existing rejection. The single-TAP root ResetPort defect does reproduce.

## Implementation findings

The first13 CLI methods and native source/flat state checks pass. Common IR/DR
phases, per-IR captures, aligned lanes, explicit root resets, shared TMS and
mixed TRST availability are implemented. Unrequested synthetic BSDL storage stays
in internal loads/replay/fill; output filtering rejects meaningful comparisons
and physical scan frames with no expressible remaining PDL access.

Scale evidence:4/16 TAPs with implicit setup pass four source/flat forms in
0.574609/3.716664s.32 TAPs pass in6.308875s with explicit IR setup followed by
joint DR writes. Implicit joint32-TAP setup reaches the unchanged2s solver limit;
the initial log is retained. A two-TAP hidden-SIB case likewise needs explicit
IR/gate setup within the current budget. These are measured planning limits,
not full optimization or reachability claims. No timeout was increased.

Required mixed/end-state testing exposed an existing single-TAP defect too:
a scan override can expire on a later generated primary apply, adding an
unverified scan during regeneration. Constrain each emitted frame so a known
ScanRegister end-state override has an actual represented write in that frame;
exclude no-event padding. Cover primary and IR frames and reset-held updates.
Do not add scan writes to a primary frame without modeling their events.

Independent review also identified separate TRST pulses as order-dependent when
one register's reset changes another TAP's reset gating. Assert/release same-phase
root reset/TRST levels as a batch and settle once. Reuse that batch pin drive for
common TMS/TDI levels before each clock edge; do not invent DataRegister write
events through ApplyInputs. A native gated-reset regression precedes this fix.

Corrected implementation review is clean. Native gated-reset RED2 fails only
when the resetting TAP sorts after the dependent TAP; batched pin snapshots
make both orders pass. The first LogicSignal probe was invalid because TRST is
not a data expression; the reproducer uses a legal DataMux source instead.
Native state/scheduling pass in build/multiple-tap-batch-green-build-native.exit0.

The existing single-TAP mixed/end-state defect is retained in
build/multiple-tap-end-state-existing-red.log/.exit1. Its corrected single/multiple
case and IR-after-override checks pass. New end-state and synthetic-output
validation blocks were extracted to satisfy the existing function-size check;
no threshold was changed. build/multiple-tap-naming3.exit0 and formatting pass.
Final focused checks pass:18 new CLI methods, TAP9, end-state11, mixed-I/O15,
hidden-event4 and main33. Evidence: build/multiple-tap-reviewed-focused.log/.exit0.

Root integration was frozen for full and leak-enabled checks under
build/multiple-tap-final-full and build/multiple-tap-final-sanitizer-*;
final outcomes are recorded below. The unchanged limitations and measured
failed implicit large-plan attempts remain part of acceptance work.

Final integration findings: the first full run failed corpuscheck because the
four submitted probe files repeat TapCell. The exact occurrence/hash-bound group
is now recorded in tests/cases/deduplication.json; independent review confirms
all four sources match the submitted probe bundles. No validation rule changed.
build/multiple-tap-corpus-green.exit0. The first failed full log/exit2 remains;
the rerun uses build/multiple-tap-confirmed-full. A final cross-TAP AccessTogether
alias test passes, bringing the new CLI suite to19 methods.

Sanitizer native state/TAP/scheduler/flatten, main33, TAP9 and hidden-event4 pass.
The19-method multi-TAP suite has18 functional passes and a4-TAP scale pass, but
its16/32-TAP scale subcases exceed the unchanged2s Z3 budget under ASan. Retain
build/multiple-tap-final-sanitizer-multiple.log/.exit1; it contains resource-limit
errors and no sanitizer finding. Optimized4/16/32 cases pass, with32 using explicit
IR setup. Independent review accepts focused safety verification at this measured
boundary; do not claim the entire instrumented scale suite passed. The separate
18-functional-method, end-state11 and mixed-frame15 sanitizer runs pass;
build/multiple-tap-final-sanitizer-functional-all.exit is0.

Optimized scale3 ran on local Linux/GCC16.2.1:6907/26431/52703 model bytes,
4/16/32 TAPs, four source/flat replay forms each,10.65s total wall and80220KiB
maximum RSS. The models use unequal4–6-bit IRs and4–8-bit DRs. The retained
implicit32-TAP failure and instrumentation overhead are performance limitations,
not changes to correctness assertions or solver budgets.

Final full regression passes: build/multiple-tap-confirmed-full.log/.exit0,
941.461s wall,40 dynamic/static CLI suites,51 directional/seven legacy goldens,
2523 ICL334/119 rules and six editor tests in178.774s. Five optional ANTLR tests
were skipped. New19-method CLI runs take18.274/17.845s in the full integration.
The earlier corpus-policy failure remains exit2. All required focused functional
sanitizers, style/naming and independent reviews pass, with the explicitly
recorded instrumented16/32-TAP scale limitation retained. Strict coverage and
Rocky8 qualification were not rerun for this change.
