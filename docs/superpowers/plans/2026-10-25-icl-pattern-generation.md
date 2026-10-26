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

Implementation is in progress. Ordered ICL loading and PDL runtime foundations
are tested; typed elaboration, scheduling, CLI delivery and end-to-end round trips
remain unfinished. Actual commands and results are recorded in the ignored
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
- [ ] Complete implicit BSDL mappings and unspecified opcode handling from the
  governing standards/manuals; connect the datapath boundaries to the controller
  and elaborated child interfaces with their controls, clocks and resets.

The component is still not a complete TAP or a generator executable. Resource
limits and unsupported routes must not be converted into unreachability claims.
