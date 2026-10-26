# ICL Pattern Generation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Deliver all requested `icl-pattern-generation` behavior, preserving existing tools.

**Architecture:** Reuse ICL `ParsedDocument`/AST and PDL `Document`/command tree.
An ordered registry feeds a typed, source-attributed elaborated netlist. A
separate Tcl/PDL runtime feeds state-aware scheduling and deterministic emitters.

**Tech Stack:** C++17, existing Bison/Flex ICL frontend, existing Tcl PDL frontend,
GNU Make, Python unittest regression.

**Spec:** [Requirements and design](../specs/2026-10-25-icl-pattern-generation-design.md)

## Global constraints

- All original requirements remain acceptance targets; intermediate milestones
  are not permission to drop model coverage, optimization or round-trip checks.
- Reuse existing parsers and AST/CST, keeping languages and runtime execution separate.
- Resolve behavior from IEEE 1687 first; when it is not explicit, consult the
  release-specific Tessent IJTAG, ATPG, user and command/reference manuals.
  Record source/version and distinguish vendor behavior. Ask the user if these
  sources leave a material ambiguity. Inspect Honza255/icl_parser retargeting
  capabilities and tests for applicable implementation ideas, not authority.
  Keep findings in [pattern generation references](../../pattern-retargeting-references.md).
- Google C++ style; generated outputs remain ignored; test scripts under `tests/`.
- No implicit hardware reset; unknown initial logic state; source-located errors.
- Model component ports, function/state and external connections only. Treat
  primitives such as ScanRegister as behavioral units, without internal
  flip-flop/gate connectivity; preserve real inter-component ICL connections.
- Preserve existing formatters/LSPs/Vim plugin; complete regression after changes.
- No commit or push without a new explicit request.
- Proceed inline under existing autonomous-edit authorization. Keep the current
  working-tree documentation; do not discard it or move unrelated checkouts.

## Working assumptions after proceed

- Emit register/SIB `iWrite`/`iApply` operations; preserve original hierarchical
  target identities. Generated artifacts must be legal PDL.
- Reject hardware-dependent execution without observed values, rather than
  substituting expected read data or inventing successful status.
- Optimize scan loads/parallel operations first, scan cycles second. Prove small
  cases against exhaustive search; report a search limit separately from no solution.
- `--local_reset` is an additional output, retaining original procedure text.
  Independent resets use legal separate writes under a shared apply unless an
  existing alias permits one write.

## Review focus

### Tessent corpus refresh (user-authorized 2026-10-26)

- [x] Commit and push the verified TAP datapath checkpoint (`ab5f0a8`).
- [x] Collect all ICL/PDL from the flat, hierarchical-with-SSN and hybrid-TK/LBIST
  2026.2 projects read-only, preserving all 465 source paths in the snapshot.
- [x] Reuse existing parser-backed module and complete-stream importers; compare
  against the maintained corpus and retain historical source revisions.
- [x] Add 124 distinct ICL module fixtures and 58 byte-unique PDL streams, with
  provenance and regression registration. No duplicate-policy exceptions added.
- [x] Reproduce and fix the documented `iScan -skip_model_state_simulation`
  syntax/highlighting gap; retain five focused positive/negative cases.
- [x] Independently review source preservation, deduplication, test wiring and
  parser/editor changes; no actionable findings.
- [x] Complete the final full regression and record its outcome: `make check`
  exited 0 (`build/tessent-refresh-regression.log`), including 2,180 ICL
  conformance cases, 212 PDL block cases, all 137 vendor PDL streams and both
  editor corpus passes. Focused static PDL corpus/block and Google style checks
  also passed. Grammar/scanner reachability remains 334/334 and 119/119.

See [ICL corpus provenance](../../../tests/cases/EDA_CORPUS.md) and
[PDL corpus provenance](../../../tests/cases/pdl_eda/README.md). These are syntax,
formatting and editor fixtures; importing them does not establish retargeting
equivalence or complete the generator tasks below.
Strict authored coverage was not rerun for this refresh; its last unchanged
100% line / 90% raw-branch gate remains unmet (96.0% / 61.2%). The user requested
committing and pushing this verified continuation after `ab5f0a8`.

### Tessent oracle extension (user-authorized 2026-10-25)

- [ ] Run isolated remote narrow/equal-width and direction-control probes with
  Tessent; use fresh outputs, checked return codes and tool-error detection.
- [ ] Add `tests/tessent_oracle.py` and mirrored
  `tests/tests/test_tessent_oracle.py`: test manifest/path validation, safe Tcl
  emission, complete corpus accounting, stale/missing/error outputs, and local
  reset input routing before implementation. Use Python's standard library and
  the existing shared PDL frontend rather than a new PDL parser.
- [ ] Maintain complete fixtures under `tests/cases/ours/pattern_generation/`
  and source-classified existing-network cases. Run Tessent scripts remotely
  in a fresh project-owned directory, retrieving evidence into ignored `build/`.
- [ ] Integrate the generator comparison when Tasks 4–6 provide the executable
  and reset rewrite; until then report it as blocked, never passing. Run focused
  harness tests and the complete repository regression after edits.

1. Identical module names in different ICL namespaces must never collide (task 1).
2. A failed later input must not damage input files or publish partial results (task 6).
3. Flattened names must not lose original procedure binding or bit order (task 2/7).
4. ResetValue must not silently initialize hardware, and X write bits preserve state (task 4).
5. Procedure redefinition must not retroactively change an earlier call (task 3).

### Task 1: Ordered source registry and shared tree access

**Files:** `src/pattern_design.h`, `src/pattern_design.cc`,
`src/pdl_document.h`, `tests/src/pattern_design_test.cc`, `Makefile`.

**Interfaces:** `pattern::Design::AddIcl(path, text)`,
`SelectTop(name)`, `modules()`, `warnings()`; immutable `ModuleDefinition`
owns a shared `icl::ParsedDocument`, module declaration index and qualified key.
PDL `Document::commands()` exposes its existing immutable command tree.

- [ ] Add tests for qualified identities, replacement warnings and retained source
  ownership, malformed input, top selection after overrides, missing modules,
  ambiguous/zero roots and dependency cycles. Verify failures before implementation.
- [ ] Implement the ordered registry using declarations from the shared parser,
  not lexical rescanning or a new grammar. Resolve `UseNameSpace` with source scope.
- [ ] Add read-only PDL tree access for subsequent runtime consumption.
- [ ] Run `make pattern-design-check BISON=/usr/bin/bison` and existing unit checks.

### Task 2: Typed netlist elaboration and serialization

**Files:** `src/pattern_netlist.h`, `src/pattern_netlist.cc`,
shared typed AST additions in `src/icl_ast.h`, `src/icl_driver.*`,
`src/icl_parser.yy`; `tests/src/pattern_netlist_test.cc`.

**Interfaces:** `Netlist Elaborate(const Design&)`; nodes/edges have stable IDs,
instance scope and original source references. `SerializeIcl(const Netlist&)`
returns canonical ICL with validated versioned origin attributes.

- [ ] Inventory IEEE/Tessent model/default rules and complete source pairs in
  `docs/pattern-model-coverage.md`; link every obligation to test cases.
- [ ] Write failing parameter/bit-range/alias/constant/instance-connection tests;
  extend shared AST metadata where missing, then elaborate typed endpoints.
- [ ] Implement hierarchical inference before flattening: reset/TRST/TMS,
  scan/control/clock/data associations, documented precedence and ambiguity errors.
- [ ] Add register/mux/logic/access-interface model families, explicit connection
  and source relationships, model-state fields and parameter specialization.
- [ ] Add collision-free flat identifiers and original scope metadata. Verify
  `SerializeIcl(Elaborate(Parse(flat))) == flat` and original target lookup.

AccessLink integration checkpoint (2026-10-26):

- [x] Validate existing TAP port kinds/TDO sources and selected scan-input conflicts.
- [x] Insert parsed leaf behavioral model scopes before ordinary connection
  resolution, preserving source trees, parameters, ordered overrides and identity.
- [x] Verify actual BSDL-derived datapath insertion, ownership and flat replay.
- [x] Synthesize/reuse BSDL TAP ports and connect instruction-selected ordered
  IJTAG client serial paths, with indexed endpoints and structural flat replay.
- [ ] Connect child control paths to controller events; support remaining
  interface kinds without inventing controller state or timing.
- [ ] Connect controller/IR behavior to live DR execution and preserve runtime
  associations through flat replay; component replay alone is insufficient.

### Task 3: Sequential PDL execution

**Files:** `src/pattern_runtime.h`, `src/pattern_runtime.cc`,
`tests/src/test_pattern_runtime.py`, `tests/cases/ours/pattern_generation/`.

**Interfaces:** `Runtime(const Netlist&)`, `Execute(path, pdl::Document)`;
produces `Operation` records containing literal target/value, source ranges,
invocation stack and apply/merge/timing/resource boundaries.

- [ ] Add failing before/after override tests, module/PDL namespace distinctions,
  arguments/defaults, prefixes and nested calls. Calls select current definitions.
- [ ] Register Tcl callbacks for PDL; evaluate in a constrained, bounded interpreter
  only here. Reuse the PDL command tree and its bracket/index distinctions.
- [ ] Test/implement ordinary Tcl expressions, loops and procedures needed for
  static expansion. Hardware readback and external side effects fail explicitly.
- [ ] Test source-attributed nested failures and loop/recursion limits; keep exact
  original definition spans for the local-reset artifact.

### Task 4: State model and retargeting scheduler

**Files:** `src/pattern_scheduler.h`, `src/pattern_scheduler.cc`,
`tests/src/pattern_scheduler_test.cc` and source-classified fixtures.

**Interfaces:** `Schedule(const Netlist&, const vector<Operation>&)` returns an
applied configuration trace with causal requirements and deterministic costs.

- [ ] Write failing known/unknown-state, last-write, X-mask, read-obligation and
  capture/shift/update tests; implement separate controlling/expected/logic state.
- [ ] Derive scan paths and access constraints through SIBs/muxes, Boolean logic,
  clocks, parallel/address access and supported TAP models. Preserve all barriers.
- [ ] Test nested/reset-controlled SIBs and blocked/disconnected targets; compute
  prerequisite operations and report a PDL call stack plus ICL cause on failure.
- [ ] Independently enumerate small reference-state spaces and assert minimum
  operation/cycle costs; implement deterministic search without unproven success
  claims. Resource exhaustion is not proof of unreachability.
- [ ] Expand and jointly schedule merge streams, preserving reservation, timing,
  reset and observation semantics; optimize with adjacent compatible operations.

### Task 5: Local-reset-only rewrite

**Files:** `src/pattern_local_reset.h`, `src/pattern_local_reset.cc`,
`tests/src/test_pattern_local_reset.py`.

**Interfaces:** consumes runtime invocation/source records and netlist state;
returns rewritten top-level text with original `iProc` definition bytes intact.

- [ ] Test independent/chained releases, polarity, feedback, control aliases and
  parameterized calls, plus incompatible release groups and internal reassertions.
- [ ] Derive/reset release prerequisites and insert legal writes/apply groups
  before affected top-level writes/calls, without SIB configuration in this file.
- [ ] Verify no duplicate insertions on replay and no changes to procedure bodies.

### Task 6: CLI, output safety and build integration

**Files:** `src/icl_pattern_generation_main.cc`, `src/pattern_output.*`,
`Makefile`, existing installation helper, `tests/src/test_pattern_cli.py`.

**Interfaces:** executable options exactly match the requirements. Emission uses
the netlist/runtime/scheduler APIs; diagnostics go to stdout and optional log.

- [ ] Write failing argument/default-path/help/version/logging/debug tests.
- [ ] Implement multi-file ordered options and safe output staging with source,
  log/debug/output identity checks (including hardlinks and symlinks).
- [ ] Test malformed later inputs, unwritable output and failure publication;
  emit canonical PDL/ICL and reset artifact only after successful analysis.
- [ ] Add dynamic/static build and install/uninstall/source-distribution rules;
  preserve Tcl-free ICL editor builds and standalone Vim distribution.

### Task 7: Round trips, real inputs and full regression

**Files:** `tests/src/test_pattern_generation.py`, maintained fixtures/expected
outputs and source provenance; `docs/pattern-generation-verification.md`.

- [ ] For complete existing ICL/PDL pairs, assert original and flat ICL yield
  byte-identical ICL/PDL, then generated PDL with original ICL is idempotent.
- [ ] Replay selected traces in an independent model, comparing actual state,
  read/reset effects and sequencing; byte stability alone cannot pass this check.
- [ ] Add source-classified IEEE/Tessent and external examples; document incomplete
  fragments separately, not as successfully retargeted corpus entries.
- [ ] Run focused dynamic/static tests, `make check-dynamic BISON=/usr/bin/bison -j4`
  with the established reference environment, static tests and coverage. Investigate
  failures; retain existing strict thresholds and report outstanding gaps.
- [ ] Measure expanded netlist size, operations, search states, runtime and memory.

### Task 8: Independent review and delivery

**Files:** `README.md`, `docs/README.md`, `docs/requirements.md`, generator user
guide and verification report.

- [ ] Obtain one independent source/test review, fix material findings with
  failing regressions, then rerun affected and complete required checks.
- [ ] Rewrite user documentation around implemented behavior only. List every
  remaining acceptance gap; no unsupported model is reported as completed.
- [ ] Install and verify dynamic/static binaries and help/version behavior using
  the existing installation conventions; record paths and relevant editor config.
- [ ] Leave reviewed changes uncommitted until the user requests commit/push.

## Execution record

Implementation is in progress. A limited ordinary serial backend now connects
ordered loading, runtime expansion, scheduling, CLI delivery and end-to-end round
trips. Native TAP event execution is tested separately. Full TAP/interface,
address/one-hot, local-reset artifact and remaining PDL transaction integration
remain unfinished; the core executable does not complete the original scope. Actual commands and results are recorded in the ignored
progress ledger; passing foundation checks does not complete later tasks.

### Task 2 continuation: BSDL metadata dependency

AccessLink's IEEE 1149.1 model needs an external BSDLEntity (IEEE 1687-2014
6.4.17 and the inspected Tessent AccessLink/read_icl topics). Continue the
provisional separate `--bsdl` input design; no existing CLI behavior changes.

- [x] Add a separate BSDL metadata reader, with independent lexical rules and
  no ICL/PDL token reuse. Read static entity attributes, scalar TAP port roles,
  opcode alternatives and widths, capture/ID codes and register associations.
  Retain other attributes for later model construction; this is not a complete
  VHDL compiler or proof that an AccessLink can be exercised.
- [x] Test the maintained Tessent-accepted BSDL companion, comments/strings,
  case-insensitive VHDL identifiers, constant string concatenation, opcode
  alternatives, malformed/unsupported required values and resource limits.
- [x] Connect the metadata loader to the design registry without changing ICL
  override/top-selection semantics. Failed BSDL loads must not alter prior data.
- [ ] Only after metadata is verified, construct TAP/AccessLink models and check
  their instruction/interface/ActiveSignal behavior against the recorded oracle.

Use bounded linear scanning for BSDL source (16 MiB input, 4 MiB decoded-value
budget); do not expand unknown expressions or load external VHDL packages.
Missing package-defined values needed by the TAP must fail explicitly. New code
belongs in `src/pattern_bsdl.*` and tests in `tests/src/pattern_bsdl_test.cc`, with
Makefile/static/sanitizer/coverage integration before completion claims.

### Task 2 continuation: AccessLink source binding

- [x] Add `pattern_access_link.*` to bind standard links from the existing
  parsed ICL AST to loaded BSDL metadata. Retain instruction/interface order,
  ActiveSignal selections and source locations; do not introduce another parser.
- [x] Resolve direct-child interfaces through the existing design registry,
  including namespaces and unambiguous shorthand. Reject unknown instructions,
  missing/ambiguous interfaces, duplicate names, multiple selected chains and
  standard links outside the top. Generic-controller semantics remain unsupported.
- [x] Test the complete oracle fixture, keyword instruction names, comments,
  explicit/shorthand chains, errors, and 3,000 references to one shared module.
  Cache interfaces and chain counts by module type, preserving the original AST.
- [ ] Consume these bindings during TAP model construction. Binding alone does
  not connect ActiveSignals or scan/control ports, validate expanded interface
  widths/types, or generate a pattern; keep the corresponding acceptance gaps.

The binding result borrows the immutable Design and its shared AST/BSDL objects.
Design must outlive the result and must not be modified while bindings are used.

### Task 2 continuation: instruction-register component

- [x] Build an owning IR/ActiveSignal component from bound AccessLink
  metadata. Emit bounded in-memory ICL and reuse the shared parser, elaborator,
  logic evaluator and flat serializer instead of introducing another model tree.
- [x] Preserve BSDL instruction width/capture masks, alternative opcodes and
  shared ActiveSignals; avoid generated-name collisions and deep linear OR trees.
  Do not imply an initial reset or known register state.
- [x] Test exhaustive four-bit instruction states, unknown/masked bits, shared
  outputs, source failures, flat replay and 2,049 opcode alternatives.
- [x] Probe indexed ActiveSignals using isolated Tessent 2026.2 runs. Retain
  IEEE-legal syntax in the parser; record the measured vendor rejection and
  model indexed outputs natively through the shared range/parameter evaluator.
- [x] Test indexed widths/directions, parameter expressions, shared and sparse
  bit decoding, integer-boundary traversal, original diagnostic locations,
  temporary top lifetime and bounded duplicate/overlap work.
- [ ] Connect the component to the complete TAP FSM, reset behavior, data-register
  routing and child scan/control interfaces. Integrate indexed ActiveSignal
  outputs using normative semantics; vendor rejection is not a reason to remove
  valid IEEE grammar or reduce the final requirements.

`BuildTapDatapathModel` is an internal component builder, not the completed
AccessLink elaborator or generator. No CLI, installation or pattern-equivalence
completion follows from these component checks.


### Task 2 continuation: TAP data paths

- [x] Extend the shared-front-end component with explicit BSDL data registers,
  IR/DR ScanMux paths and instruction-chain return boundaries; retain unknown
  capture state where no source is defined.
- [x] Reuse register and ordered-chain identities across instruction aliases;
  reject conflicting exact/masked opcode routes with source context and bounded
  overlap work. Verify named-bit routing, known captures and flat replay.
- [x] Exercise 10,000 additional literal opcodes, masked-budget exhaustion,
  missing/inconsistent associations, generated-name collisions and shared paths.
- [x] Complete mandatory BSDL register mappings and unassigned binary-opcode
  BYPASS handling from IEEE 1149.1-2001 B.8.13.3(a) and 8.1.1(d). Keep private
  instructions and unsupported BSDL capture forms explicit remaining gaps.
- [x] Distinguish IDCODE/USERCODE capture values in the shared register, preserve
  unspecified custom capture state, and reject incompatible capture aliases.
- [x] Reproduce and fix the independent-review finding that AccessLink chains
  bypassed explicit standard register-name/width validation.
- [ ] Connect the datapath boundaries to the controller and elaborated child
  interfaces with their controls, clocks and resets.

The component is still not a complete TAP or a generator executable. Resource
limits and unsupported routes must not be converted into unreachability claims.

Final defaults/capture verification: complete `make check`, focused dynamic/static
tests, ASan/UBSan (leak detection disabled), formatting/naming, and independent
review passed. Logs: `build/tap-defaults-delivery-regression.log`,
`build/tap-defaults-delivery-sanitize.log`, `build/tap-defaults-style.log`.
The strict coverage run completed its suites but failed the unchanged gate at
8,016/8,345 lines (96.1%) and 13,167/21,502 raw branches (61.2%); see
`build/tap-defaults-delivery-coverage.log`. The corpus checkpoint `6dc6ff9` is
pushed; this subsequent model continuation remains uncommitted.

### Task 2 continuation: native TAP controller

- [x] Implement all 16 IEEE 1149.1 states with unknown initial state and
  conservative unknown TMS/TRST handling; retain the explicit no-initial-reset
  requirement.
- [x] Separate rising-edge capture/shift actions from falling-edge update,
  instruction-reset and TDO controls. Model asynchronous active-low TRST.
- [x] Test every transition and per-state action against explicit standard
  tables, five-clock reset convergence, reset hold/release, unknown inputs and
  invalid-input atomicity. Request independent controller review.
- [ ] Consume controller actions in separate instruction shift/latch state and
  child scan interfaces. `TapController` alone does not execute register changes
  or connect `TapDatapathModel` boundary nodes.

The temporal controller stays separate from the shared structural netlist; a
runtime must consume its returned actions and falling-edge output controls.
There is no new parser, AST or duplicate netlist. See the controller verification
entry in [model coverage](../../pattern-model-coverage.md).

Controller checkpoint verification: final complete `make check`, focused
dynamic/static tests, ASan/UBSan (leak detection disabled), Google style/naming
and independent review passed. Corrected gcov input collection includes inline
header code emitted by pattern-test translation units while excluding test
sources from totals. The unchanged strict coverage gate still fails at
8,066/8,395 lines (96.1%) and 13,204/21,546 raw branches (61.3%). Logs:
`build/tap-controller-delivery-regression.log`,
`build/tap-controller-final-sanitize.log`,
`build/tap-controller-delivery-style.log`,
`build/tap-controller-delivery-coverage.log`. This checkpoint was committed and
pushed as `863c658`; full runtime and child wiring remain open.

### Task 2 continuation: instruction-register execution

- [x] Consume the native controller events in an owning instruction-register
  state component, keeping shift storage and active instruction latch separate.
  Read capture/reset metadata from the existing BSDL model; do not initialize
  either stage until an observed event determines it.
- [x] Retain alternative/masked reset encodings as possible instruction values.
  Bound retained state and report resource limits separately from unreachability.
  Use circular shift storage for constant work per known shift edge; capture,
  update and uncertain-state joins may traverse the instruction width.
- [x] Test complete IR scans, capture/shift/update edge timing, reset selection,
  uncertain events, metadata validation, ownership and large repeated shifts.
  Run focused checks, independent review and final complete regression.
- [ ] Connect this state to instruction-dependent datapath evaluation and child
  scan/control interfaces. Standalone IR execution does not complete that wiring.

IR execution verification: complete `make check`, focused dynamic/static,
ASan/UBSan (leak checks disabled), Google style/naming and independent review
passed. Strict coverage remains below the unchanged gate at 96.1% lines / 61.4%
branches. Logs: `build/tap-instruction-regression.log`,
`build/tap-instruction-coverage.log`, `build/tap-instruction-delivery-sanitize.log`,
`build/tap-instruction-delivery-style.log`. This continuation is uncommitted
after checkpoint `863c658`.

### Task 2 continuation: AccessLink connection evidence

- [x] Probe explicit scan-input/control bindings, existing top-level TAP ports
  and two-interface order in isolated Tessent 2026.2 elaborations. Retain a
  reproducible stager under `tests/` and record outcomes in the reference ledger.
- [ ] Expand AccessLinks before ordinary signal resolution. Preserve shared
  parsed module definitions, qualified namespace resolution, parameter values
  and original source locations; do not overwrite unrelated ordered overrides.
- [ ] Reuse valid existing top TAP ports; reject wrong functions, existing TDO
  Source and explicit child scan-input bindings. Preserve explicit control
  bindings and wire ordered chains through instruction-dependent muxes.
- [ ] Connect native controller/IR state to child controls and DR execution,
  then verify canonical flat replay and generated-pattern equivalence.

The probe stager's focused safety tests and independent review passed; the final
combined `make check` also passed after integration into the regression recipe
(`build/tap-instruction-connections-final-regression.log`). The probe results
establish connection precedence for the next implementation, not completion of
native wiring or retargeting.

### Tasks 4–6 continuation: single-TAP transaction integration

- [x] Reuse the serial solver and native controller/event replay for IR/DR
  transactions, requiring explicit reset and retaining instruction state.
- [x] Constrain complete scan-interface events using the runtime's membership;
  reject disabled/shadow behavior outside this model without publishing output.
- [x] Expose repeatable `--bsdl ENTITY FILE`, protect BSDL input paths, preserve
  `iReset -sync`, and omit synthetic instruction writes from emitted PDL.
- [x] Check hand-derived serial bits, source/flat/generated replay, instruction
  switching, reset variants, assumptions, disabled controls and diagnostics.
- [x] Complete independent review, full regression, dynamic installation and
  installed-program replay. Strict coverage still fails at 95.1% lines and 60.3%
  raw branches; thresholds remain 100% and 90%.
- [ ] Complete multi-TAP/shared-chain/shadow and addressed/one-hot scheduling,
  merges/timing/resources, reset-only rewriting and full Tessent equivalence.

The current support boundary and verification evidence are in
[pattern-generation.md](../../pattern-generation.md). Earlier checkpoint notes
record historical limitations; they do not override this integration record or
remove any remaining requirement.

### Task 4 continuation: `iApply -together`

- [x] Enforce IEEE 1687-2014 §7.9.14(c): all queued writes share an update
  cycle, while prerequisite scans and separately scheduled reads remain legal.
- [x] Verify the common update in concrete replay and preserve its marker in
  generated PDL; retain source/call context on bounded failure.
- [x] Cover reset-held matching values, mutually exclusive paths, read-only
  groups, TAP selection, atomic failure and original/flat/generated replay.
- [x] Complete independent review, focused sanitizer/style checks and full
  dynamic/static/editor regression. Strict coverage remains unmet at 95.1%
  lines and 60.3% raw branches; no threshold changed.

This closes the queued-write same-cycle requirement for the currently supported
serial/single-TAP model. It does not complete iMerge, resource reservations,
timing, unsupported access topologies or the separate local-reset artifact.

### Task 4 continuation: literal TCK idle replay

- [x] Implement IEEE 1687-2014 §7.9.17 literal TCK counts using existing native
  single-TAP events, with explicit reset and no unapplied operations.
- [x] Retain full emitted counts while accelerating complete-state fixed points;
  bound nonconverging replay and preserve state on failure.
- [x] Track transient reset assertions through native propagation so idle
  cycles cannot resurrect stale controlling fill.
- [x] Verify stable/changing/periodic state, huge counts, error paths and both
  replay forms; complete independent review, full regression, sanitizer/style
  checks, installation and eleven installed CLI tests (`d274155`).
- [ ] Complete system-clock/absolute-time waits and the other outstanding
  scheduler, reset-rewrite and vendor-equivalence requirements above.

Strict coverage remains unmet: 95.2% lines and 60.3% raw branches against the
unchanged 100% / 90% gate. See the current generator guide for evidence.

### Task 4 continuation: enumerated read/write operands

- [x] Resolve `iRead`/`iWrite` named values through the original target's
  elaborated `RefEnum`, preserving instance parameterization and alias identity.
- [x] Reuse numeric width/X rules, leaving numeric-only commands unchanged;
  cover masks, slices, invalid names and both replay forms.
- [x] Complete independent review, full regression, sanitizer/style checks and
  twelve CLI tests against the dynamic installation. Strict coverage remains
  unmet at 95.2% lines / 60.4% raw branches; thresholds are unchanged.

This closes enum operands for the supported scan-register/alias transactions;
it does not complete the remaining access models, merging or reset-only output.

### Task 4 continuation: top-level pin state assumptions

- [x] Apply IEEE 1687-2014 §7.9.22 to supported top-level DataInPort/SelectPort
  snapshots without clock or reset events; share target validation and state
  assignment between serial and native TAP paths.
- [x] Preserve aliases, slices and unknowns; reject internal/output/controller
  targets before mutation and verify source/flat identity and generated replay.
- [x] Remove repeated full-vector reads in the per-bit assumption loop; compare
  three 65,536-bit samples before/after on the same host.
- [x] Complete independent review, full regression, sanitizer/style checks and
  thirteen installed-generator tests. Strict coverage remains unmet at
  95.2% lines / 60.4% raw branches; the original thresholds remain unchanged.

DataRegister state/execution and readback-history flags remain unfinished, as
do the previously listed access-model, merge and reset-rewrite requirements.

### Task 4 continuation: Alias `AccessTogether`

- [x] Resolve marked alias groups through the shared target resolver, preserving
  full nested groups when a PDL command names a slice.
- [x] Constrain each referenced register group to a single operation; keep
  unrelated groups separate and defer grouped read comparisons when needed.
- [x] Verify common accesses in concrete replay, retain debug-ICL markers and
  preserve generated-PDL replay through explicit common-update applies.
- [x] Reproduce and fix nested and overlapping-read edge cases; pass independent
  review, focused native/CLI tests, Google style checks and sanitizers.
- [x] Complete full regression, coverage accounting and installed CLI checks:
  full `make check` and fifteen installed CLI tests passed; strict coverage
  remains failed at 95.2% lines / 60.5% raw branches against unchanged 100%/90%
  thresholds. See the 2026-10-27 generator-guide verification record.

This completes a scoped alias feature, not the original generator requirements.
Parallel ports, DataRegisters and alias `iApplyEndState` need separate work.
