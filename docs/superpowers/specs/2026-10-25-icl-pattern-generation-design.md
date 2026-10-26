# icl-pattern-generation: requirements and proposed design

Status: implementation started, 2026-10-25. The executable is not implemented.
This records the requested behavior and separates it from proposed engineering
choices. After the user's instruction to proceed, the implementation plan records
the working assumptions for the semantic choices below; they do not reduce the
original acceptance requirements.

## Purpose and scope

Add `icl-pattern-generation` to turn ordered ICL and PDL inputs into an explicit,
flattened configuration sequence, including required access-network setup.
Provide an additional local-reset-only rewrite and a re-readable flat ICL dump.
Reuse the existing language parsers and trees. ICL and PDL remain separate
languages; formatters and language servers must never execute user PDL.

This is an elaboration, execution and retargeting feature, not a formatter option.
The existing four tools and standalone Vim plugin retain their behavior.

Model boundary clarified by the user: model each component's observable ports,
function, state and connections to other components/ports. A ScanRegister is an
atomic behavioral component; do not expand its internal flip-flops or gate
wiring. This also applies to other primitives. Internal storage used to compute
behavior (such as a circular shift buffer) is not a structural netlist expansion.
Continue to preserve the actual ICL module hierarchy and inter-component nets.

## Requested command-line contract

| Option | Required behavior |
|---|---|
| `--icl <file>...` | Read files in supplied order. A later definition replaces an earlier definition of the same ICL module identity; warn on every replacement. |
| `--pdl <file>...` | Execute top-level commands in file and command order. Register procedure definitions when encountered; later replacements affect subsequent calls only. Warn on replacements, not on distinct module/namespace identities. |
| `--top_module <name>` | Select an explicit top. Otherwise use the sole module not instantiated by another module, with a warning identifying it. Multiple candidates are an error directing the user to this option. |
| `--output <path>` | Write flattened PDL here, or to `./out/<top_module>.pdl`. Report the resulting location. Inline procedures, merges and variables, and expose required TDR/SIB configuration order. |
| `--log <path>` | Mirror informational/diagnostic output to this file as well as stdout; without it, print to stdout only. |
| `--local_reset <path>` | Optionally produce a local-reset rewrite: preserve procedure definitions, modify only the main procedure/top-level sequence, and insert prerequisite reset releases before affected writes/calls. Recursively release prerequisite resets first; combine compatible releases under one apply. Do not insert SIB setup in this artifact. |
| `--debug` | Additionally write flat ICL to `<output-directory>/debug/<top_module>.icl`. With the default output, use `out/debug/<top_module>.icl`. |

Module identity includes its ICL namespace. Procedure identity must distinguish
ICL namespace, ICL module, PDL procedure namespace, and procedure name. The last
component is not sufficient, nor is the ICL namespace alone. This retains the
user's distinction between equally named procedures bound to different modules
and IEEE's additional `-iProcNameSpace` scope.

All ICL replacements are resolved before selecting/elaborating the top and
executing PDL. PDL definitions must not be globally preloaded: a call in the first
file cannot see a replacement that occurs in a later file. Replacement warnings
identify both source locations. A zero-root module graph is also an error; report
recursive instantiation or an empty design rather than selecting arbitrarily.

Proposed CLI details: accept repeated `--icl` and `--pdl` groups, preserving order
within each language; require at least one file of each kind; provide immediate
`--help`/`--version`; reject unknown/missing arguments. Resolve unqualified top
names only when unambiguous. Use a namespace-qualified basename when needed to
avoid collisions. Do not automatically call a procedure named `main`: the input
top-level calls define the entry sequence unless a different rule is agreed.

## Decisions still needed

1. **Output abstraction:** explicit register/SIB `iWrite`/`iApply` steps using
   original hierarchical targets (recommended from the supplied examples),
   top-level scan-interface `iScan` operations, or pin-level operations? These
   require different emitters and define different notions of a pattern.
2. **Hardware-dependent PDL:** recommended behavior is a source-located error if
   static generation needs live readback/status. Alternatives need supplied
   readback data or runtime branches; the latter contradicts fully flattened,
   variable-free output. Expected `iRead` data is not observed hardware data.
3. **Optimization objective and guarantee:** recommended objective follows
   Tessent: first scan loads/parallel I/O operations, then scan cycles, respecting
   semantic barriers. A guarantee of a global minimum requires a stated cost
   metric and a solver that proves optimality; a heuristic cannot claim it.
   Decide whether large cases may return a valid, explicitly unproven solution
   or must fail when the proof budget is exhausted.

The following are proposed interpretations to review with the design:

- `--local_reset` names an additional output file, not an input dependency map
  or an alternative to `--output`.
- With no explicit main-procedure selector, only top-level commands may be
  rewritten; all `iProc` definitions are preserved byte-for-byte. If a called
  procedure internally reasserts reset and later accesses the reset target,
  a top-level preamble cannot repair it. Report this limitation, rather than
  silently editing the procedure or emitting an incorrect rewrite.
- Top-level Tcl may be expanded for statically determined executions to insert
  invocation-specific prerequisites. Preserve the order of procedure-definition
  events and their original definition text. This is not permission to expand
  hardware-dependent branches using invented values.
- Multiple independent reset releases use multiple legal `iWrite target value`
  commands before one `iApply`. A single `iWrite` can cover them only through a
  suitable existing alias. The example's missing values are schematic: release
  values must be solved from the actual reset logic and polarity.
- Flat ICL contains versioned tool-specific attributes preserving original
  instance/module identity and object mappings. This permits original PDL to
  bind after structural hierarchy removal. These attributes are data, never
  executable scripts; compatibility with other consumers must be documented.

## Existing code and reuse

Confirmed user direction: all existing ICL/PDL-related code is available for
reuse, especially the parsers. Reuse each language's existing AST/CST wherever
possible, extending shared representations and APIs when necessary. Inspect
existing helpers before introducing replacements. Keep ICL and PDL trees
distinct; sharing with the generator does not mean sharing tokens between
languages. Preserve formatter/LSP behavior and verify shared changes through
their existing regression suites.

CodeGraph inspection and current source show:

- `src/icl_ast.h` owns the shared ICL AST. Many statements/headers preserve text;
  this is not a typed executable netlist.
- `src/icl_semantics.h` supplies `ParsedDocument`, declarations and name uses.
  `SemanticWorkspace` is an editor index with ambiguity handling, not an ordered
  overriding module registry or an elaborator.
- `src/pdl_document.h` supplies a separate Tcl-backed command/word tree and
  source ranges. `src/pdl_document.cc` deliberately uses Tcl parsing without
  evaluating source. Its workspace procedure index represents source lookup,
  not time-dependent procedure registration.

Reuse parsing, source ownership, literal/expression representations, diagnostics
and rendering where appropriate. Add typed information to the existing ICL tree
as needed; do not build a second regex-based ICL parser. Expose read-only access
to PDL commands for the runtime. Keep execution in a separate library linked
only by the new executable. Do not change editor duplicate-resolution semantics
to implement the generator's last-definition-wins behavior.

## Proposed processing pipeline

```text
ordered ICL -> shared ICL parse -> module registry -> typed elaboration
                                               -> hierarchy/default connections
                                               -> flat netlist + origin map
ordered PDL -> shared PDL parse -> sequential runtime + procedure registry
                                               -> attributed operation stream
netlist + operations -> state/constraint model -> scheduler -> canonical PDL
                     -> local-reset analysis -> source-preserving rewrite
netlist + origin map -> canonical, re-readable flat ICL
```

### Netlist representation

Use owned design/module/instance objects, stable node and net IDs, and explicit
port-bit connectivity. Give behavioral model families a common interface where
they need distinct evaluation/constraint logic; use derived register, mux, logic,
clock/reset and access-link models. Keep connectivity/state data separate from
algorithm visitors. Do not model ICL merely as Verilog gates or reproduce a
general Verilog simulator.

Elaborate parameter values, widths, bit order, slices, concatenations, enums,
aliases and signal expressions per instance. Preserve unknown values, constant
sources, polarity, capture/shift/update behavior, defaults, access interfaces,
resource reservations and source origins. Reject unresolved modules/parameters,
illegal connections and nonterminating elaboration with an instance chain.

Resolve implied connections while hierarchy is available, before flattening.
Use a documented per-port/per-model rule table from Tessent, with explicit
connections taking precedence and ambiguity diagnosed. One generic
"connect to parent by name" rule is insufficient. A model library must record
each implemented IEEE primitive/vendor model, document/version, constraints and
tests. An unsupported model affecting a target must fail explicitly.

IEEE 1687 is the first authority. If it leaves behavior implicit, consult the
applicable Tessent IJTAG manual, ATPG manual, user guide and command/reference
guide, recording release and topic and identifying vendor-specific behavior.
If these sources still do not resolve a material ambiguity, ask the user before
implementing that behavior and continue independent work. Inspect
Honza255/icl_parser for actual retargeting/pattern-generation capabilities and
use relevant algorithms and tests as implementation references, not normative
rules.

Flatten by visiting instantiated objects and remapping endpoints, not repeatedly
copying every module's already flattened descendants. Maintain a collision-free,
deterministic mapping between original hierarchical paths and legal flat ICL
identifiers. The serialized mapping must retain procedure binding, instance
context, parameter values and externally visible aliases needed by original PDL.
Validate mapping schema, sizes, references and uniqueness on import.

Per-register reset association also depends on hierarchy: a single flat module
cannot in general express several original implicit reset domains through one
ordinary module-level ResetPort rule. Serialize the elaborated per-object reset
connection and polarity in the versioned metadata as well. Re-import restores
the already resolved connection rather than inferring a different flat-module
default. Such metadata is tool-specific; ordinary IEEE/Tessent consumers must
not be claimed to reproduce its semantics merely because they parse its syntax.

### PDL runtime and operation stream

Evaluate commands sequentially with registered PDL semantics and Tcl level-1
evaluation only in the generator. A constrained Tcl interpreter is preferable
to a new partial Tcl implementation; filesystem/process/network facilities are
not needed to generate patterns and must not be implicitly enabled. The exact
supported Tcl command policy and execution limits belong in the implementation
plan. Never treat arbitrary Tcl square-bracket substitutions as vector syntax;
reuse the existing PDL-aware parsing distinctions.

Each operation retains source file/range, original command, invocation stack,
procedure-definition version, instance context and command ordinal. Argument
defaults, variable substitution, prefixes, namespaces and nested calls resolve
before emission. A running call retains its chosen definition even if subsequent
execution registers a new version.

Keep controlling, expected and logic states distinct. Preserve IEEE last-write
semantics within each apply group, partial writes and X masks. Preserve read
obligations even for all-X expected values. Model reset and clock commands,
resource ownership, timing and merge nesting. Do not delete a repeated write
merely because its value matches: its apply may perform a required capture,
update or side effect.

Proposed initial-state contract: logic state starts unknown, as specified in
IEEE 1687 section 7.3.3. `ResetValue` is not evidence that a reset has occurred.
Apply input `iReset` and `iState` according to their distinct semantics; never
insert an unsolicited global reset to obtain a convenient starting state. A
generated configuration must work under its stated initial-state assumptions,
or diagnose the missing prerequisite. Test unknown state and explicit reset
separately, including registers without reset values.

### Scheduling and local reset

Derive access prerequisites from netlist connectivity and modeled state, not
signal/module naming conventions. Plan SIB/mux selection, reset releases and
target writes with explicit applied-state transitions. A control bit updated in
one scan cannot be assumed active during that same scan's shifting phase.

For local resets, derive a Boolean release condition, including inversion and
multiple controlling bits. A dependency DAG is sufficient only for simple
acyclic cases. Cycles and feedback require state reasoning; distinguish a
provably unreachable state from unsupported behavior and solver exhaustion.
Shared apply groups require compatible values and simultaneous accessibility.
Preserve intentionally requested resets and their effects on other registers.

Expand `iMerge` into dependent operation streams and optimize compatible work
jointly; do not emit a separate merge artifact. Preserve required apply/clock/
read/reset/resource barriers. IEEE requires an empty apply queue on merge entry;
optimizing surrounding work does not authorize moving pending writes across
that boundary. Retain the terminal state and observable behavior of every stream.

Use exhaustive shortest-path search on small reference models as an independent
optimality oracle. Choose the production solver after measuring representative
models. Report valid solution, proven unsatisfiable, unsupported semantics and
resource-limit exhaustion separately. Bounded search failure is not proof of
disconnection. No claim of globally shortest output without a proof.

### Diagnostics and artifact publication

Errors identify the top-level call, procedure and command ordinal, offending
write/read, resolved target and causal connectivity/state chain, with source
locations for both PDL and ICL. Distinguish a missing target, disconnected path,
blocked selection, asserted reset, conflicting requirements and unsupported
model. Warnings do not convert these failures into success.

Parse/execute in memory and stage output files before publication. Reject output,
debug and log paths aliasing inputs or each other, including symlink/hardlink
aliases. Do not truncate an input or publish a successful-looking partial PDL
on failure. Logs may retain failure diagnostics. Describe multi-file publication
failure accurately; a sequence of renames is not an atomic transaction.

Emit deterministic names, ordering, literals and whitespace. Exclude timestamps,
absolute input paths and changing run diagnostics from canonical output bytes.
Retain diagnostic source maps separately from canonical serialization metadata.

## Acceptance tests

### Tessent differential oracle (2026-10-25 addition)

Use the lianghuaxiao account and the existing flows under
`/data/workspaces/lianghuaxiao/test/tessent_ijtag` as references. Run new
experiments in isolated directories created for this project; preserve the
existing cases. Establish narrower OneHotDataGroup behavior experimentally
before choosing padding/alignment semantics.

Inventory every ICL/PDL file in `tests/`, retaining an explicit disposition for
syntax-negative tests, unpaired fragments, complete runnable cases and missing
dependencies. A parser input is not automatically a complete executable network.
Maintain source-attributed complete case manifests and emit Tessent scripts
using `write_patterns -pdl` in `patterns -ijtag` analysis mode. Record the actual
Tessent release, input hashes, command, exit status, tool errors and output files.

For differential execution, our generator reads the original source PDL and
produces both flattened PDL and the `--local_reset` rewrite. Tessent must read
that rewrite, including preserved procedures, rather than the source PDL or our
fully flattened output. Never substitute a hand-written reset fix and claim the
generator supplied it. Cases without a generator executable or reset rewrite
are blocked comparisons, even when a standalone Tessent experiment succeeds.

PDL output wrappers, comments and spelling may differ between generators.
Compare canonical executable operations using the shared PDL frontend; report
structural differences separately from proved behavioral mismatches. Different
valid schedules require state/target and cost checks, not an unconditional
byte-equality assertion. Keep vendor-generated patterns and logs ignored.

All new test scripts belong under `tests/`, mirroring their source paths.
Classify fixtures by IEEE, Tessent, existing repository/corpus or self-constructed
origin. Keep generated output/build data ignored; maintained expected files are
reviewed specifications and are never refreshed by ordinary regression.

Before implementation, inventory every IEEE primitive/PDL command and the
Tessent default-connection/model rules against concrete complete corpus pairs.
That matrix defines the coverage obligation, not just the currently convenient
fixtures. Unsupported-model diagnostics are required failure behavior during
development; they do not count as implementing that model or justify claiming
the complete feature is delivered. Any proposed release subset must be explicit
and agreed, rather than silently replacing the requested scope.

| Area | Required tests |
|---|---|
| CLI and I/O | Repeated file groups/order, spaces in paths, missing arguments/files, zero/multiple tops, namespace ambiguity, all default paths, logging, debug output, aliasing and write failures. |
| Overrides | Module replacements with namespace distinction; calls before/after procedure replacement; distinct modules/PDL namespaces; argument-dependent/nested calls; warning locations. |
| Elaboration | Repeated parameterized instances, reversed/vector slices, concatenations, enums/aliases, cross-instance sources, recursion, unresolved references, name collisions. |
| Default connections | Each documented rule and precedence, zero/one/multiple sources, polarity conversion, reset/TRST/TMS distinctions, registers with and without ResetValue. |
| Hardware semantics | Serial and nested SIBs, shared/mutually exclusive routes, mux/logic controls, direct parallel access, capture/update behavior, partial writes, X handling, unknown initial states. |
| Local reset | Independent and chained releases, active-high/low logic, multiple controls, feedback, shared applies, reassertion, internal call barriers and byte-preserved procedure definitions. |
| PDL execution | Level-0 rules, level-1 constants/loops/conditionals, defaults, namespaces, prefixes, nested calls/merges, reservations, timing, readback policy, bounded recursion and evaluation. |
| Optimization | Independently calculated small-model minimums, resource conflicts, combined merge scheduling, preserved barriers/side effects, stable tie-breaking, explicit budget outcomes. |
| Failures | Exact source/call-chain assertions, disconnected/blocked targets, unsupported models, no successful partial output. |
| Real corpus | Select complete ICL/PDL pairs with known tops/entries/expectations; inventory exclusions. Parser-only fragments are not automatically executable patterns. |

For generation `G`, flat-ICL serialization `F`, source ICL `I`, source PDL `P`:

1. Produce `J = F(I)` and `Q = G(I, P)`.
2. Require `F(J) == J` and `G(J, P) == Q` byte-for-byte.
3. Require `G(I, Q) == Q` byte-for-byte.
4. Replay generated operations in an independent small-model simulator and
   compare intended register states, read obligations, resets and ordering.
5. Compare maintained expected artifacts and exercise dynamic/static builds.

The origin map is essential for step 2 with unchanged, module-bound source PDL.
Do not weaken that check by silently rewriting the source PDL. Stable output is
necessary but cannot establish semantic correctness on its own.

Run focused new suites, then complete existing regression for both language
pairs and Vim/Neovim integration. Measure handwritten coverage without lowering
the existing thresholds. The previously recorded coverage gap remains a gap.

## Performance and integration

No throughput or memory target was specified. Proposed measurement set: small
oracle cases, representative complete vendor networks, repeated module instances,
deep SIB/reset chains and wide merge groups. Record node/bit/edge counts,
operations, search states, elapsed time and peak RSS.

Target near-linear parsing/elaboration in input plus expanded netlist size;
index symbols and adjacency once. General configuration scheduling may require
exponential search, so bound it and report the distinction between impossibility
and resource limits. Avoid per-register subprocesses and repeated whole-design
scans. Do not add a solver dependency before evaluating the required model and
the project's dynamic/static deployment constraints.

Implementation reaches the shared typed AST additions, generator runtime/model/
scheduler/emitter, CLI, Makefile dynamic/static targets, install/uninstall and
source packaging, tests and user/developer documentation. Keep Tcl execution
out of ICL-only binaries and all editor tools. The README must not advertise the
fifth executable as available until build/install/regression checks pass.

## Reference evidence inspected

- IEEE 1687-2014, clauses 7.3.4 (printed p.146), 7.9.4–5 (pp.159–160),
  7.9.11 (pp.166–167), 7.9.18 (pp.176–177), and 8.3–8.4.1 (p.183): ordered
  model updates, module/procedure namespaces, write syntax, merge barriers and
  hardware readback. Read from the supplied PDF under
  `/home/banana/win11/Zotero/storage/X9436TPS/`; PDF pages are printed pages +14.
- Tessent IJTAG User's Manual v2023.4: *How to Build an ICL Netlist*,
  *How to Model Global Reset, Local Reset and Embedded TAPs*, and *iApply
  Retargeting in the IJTAG Engine*. Local HTML topics are under
  `/home/banana/win11/Desktop/tessent_2023_doc/htmldocs/tshell_ijtag_user/topics/`.
  The reset topic specifies hierarchy-sensitive implicit sources and polarity
  handling; the retargeting topic describes operation-count then scan-cycle
  optimization and acknowledges partitioning may miss the optimum. This is not
  yet a complete audit of every default connection or built-in model.
- [Icarus Verilog netlist documentation](https://github.com/steveicarus/iverilog/blob/master/Documentation/developer/guide/ivl/netlist.rst),
  retrieved 2026-10-25 from the moving master URL: separation of parsed form,
  elaboration and netlist; typed nodes/nets, pin links, connectivity nexus and
  expression widths. Adopt the separation and explicit connectivity concepts,
  not Verilog execution semantics. Pin the source revision before implementation
  relies on additional details.
- Verilator internals at `63685a23525c7022e17c0b4e5fea6226c42b0306`, available in
  `build/verilator-review/docs/internals.rst`: link, parameter elaboration and
  per-instance scope phases. See the existing
  [reference review](../../verilator-testing-practices.md).
- Local `/home/banana/cpp_workspace/hcstest/verilog-parser/`: inspected
  `verilator-parser/VerilogNetlist.h`, `VerilogNetlist.cpp::flattenHierNet`,
  `V3LinkCells.cpp` and `GenPrimitiveNetlist.h`. Hierarchical/flat representations,
  indexed module maps and explicit port remapping are useful examples. Its
  whole-netlist copying and Verilog/standard-cell assumptions are not adopted
  automatically. No source from these projects has been copied or modified.

## Completion status

The ordered ICL source registry, constant evaluation, initial typed elaboration,
and sequential PDL runtime have focused regression tests. The elaborator retains
the existing AST and handles specialized widths, bit connections, enum reset
values, mux connectivity and initial hierarchy-sensitive reset inference. This
does not complete model coverage or retargeting. No executable, solver, canonical
flat serializer or installation is delivered yet. The
[implementation plan](../plans/2026-10-25-icl-pattern-generation.md) tracks all
remaining stages and preserves the complete requested scope.

An independent design-only builder check identified initial state, mandatory
model coverage and top-level Tcl preservation as additional planning gaps. The
proposals above address them explicitly. That check did not validate an
implementation or establish conformance.
