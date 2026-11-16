# Pattern generator model and connection audit

This is an implementation ledger, not a claim of complete generator coverage.
The executable now has a limited serial backend; see the
[current generator guide](pattern-generation.md) for its supported boundary.
All requirements in [requirements.md](requirements.md) and the
[approved design](superpowers/specs/2026-10-25-icl-pattern-generation-design.md)
remain mandatory. Parsing a construct does not establish that it can be retargeted.

The new elaboration layer consumes `icl::ParsedDocument`, its existing AST,
declaration index, and lexer tokens. It does not define a second ICL grammar.
PDL execution consumes `pdl::Document`; the editor tools never execute Tcl.

The existing formatter/LSP frontend already supports ScanInterface and Chain
syntax, and LSP regression tests cover ScanInterface definition lookup. The
generator additionally constructs elaborated interface models, resolves implicit
controls, and serializes/reimports those models for pattern generation. These
capabilities reuse the existing shared AST and declaration index.

## Normative and vendor evidence

- IEEE 1687-2014 §6.3.8: numeric bases, width, X padding, significant-bit truncation.
- §6.3.10–12: vector direction, concatenation, parameter references, arithmetic.
  The published arithmetic productions are right recursive at both precedence
  levels; formatting preserves the spelling and elaboration evaluates it.
- §6.4.7 and §6.5.4: instance inputs and parameter specialization.
- §6.4.8, §6.4.11–13, §6.5.2–3: registers, muxes, aliases and enumerations.
- §6.4.10: shared expression structure, operand widths, reductions and enum
  comparisons. The prose says equal-precedence operators associate left to right;
  the published grammar is right recursive and its precedence order differs from
  the prose's “decreasing order” list. The existing parser follows the grammar's
  precedence levels and the prose's left associativity. Tests use explicitly
  grouped equivalent forms where the printed examples are ambiguous.
- §6.7–8: implicit ICL and active control values.
- Tessent IJTAG User's Manual v2023.4, “How to Model Global Reset, Local Reset
  and Embedded TAPs”,
  `tshell_ijtag_user/topics/Concept_HowToModelGlobalResetLocalResetAndEmbeddedTaps_iddb5f80dd.html`:
  a reset input first inherits the unique parent ResetPort, otherwise the unique
  child ToResetPort, otherwise virtual Global Reset; ambiguity is an error.
  ToResetPort uses the corresponding rule in its own scope. Registers with
  ResetValue use this same scope-dependent association. Inferred ResetPort
  connections adjust for differing active polarities. TRST uses asynchronous
  Global Reset; TMS has no missing-source fallback. Explicit connections take
  precedence. Registers without ResetValue have no implied reset association.

## Current evidence and remaining work

This table describes the current implementation. The dated sections below
are historical checkpoints; statements such as “not implemented” there describe
that checkpoint, not the current CLI. Passing native tests does not establish
full model coverage or Tessent equivalence.

| Area | Current implementation/evidence | Remaining acceptance work |
|---|---|---|
| Shared frontends and ordered inputs | Shared ICL AST/PDL tree; Design/Runtime tests and CLI ordered overrides; reset cancels queued reads/writes across file and call boundaries with operand validation; observation-history iState flags are forwarded without hardware or queue mutation; getters evaluate fully supplied explicit history and applied reads invalidate their target bits | Complete runtime command coverage and broader real-pair generation |
| Elaboration and values | Namespaces, specialization, arithmetic, ranges, aliases, enum and compiled logic; hierarchical hardware references require child output ports, while aliases cannot bypass child-output boundaries and deep aliases remain PDL-only under §6.5.2(g), with its prose conflict documented; direct ScanOutPort register sources require the declared rightmost bit; explicit unsized-inversion rejection under §6.3.12(m), with conflicting-example/Tessent acceptance documented; source-located diagnostics; ordinary-data terminal-kind eligibility is checked after flat type restoration with bit-specific aliases and the two-input ScanMux ToIRSelect exception; checked Sources enforce local terminals/direct-child outputs and raw-alias ownership after origin restoration; generated TAP boundaries use explicit outputs | Remaining source scope/direction and contextual source-kind rules, broader alias-rule interpretation, and corpus specialization |
| Address/one-hot models | Contributor/domain/default/alias resolution; primary-, scan- and mixed-driven DataRegister writes/reads; nested OneHotScanGroup selection, indexed vector lanes feeding a scalar chain and canonical replay | Simultaneous vector scan-return lanes and broader model qualification |
| Native hardware state | Unknown initial state, reset propagation, separate shift/update storage, addressed DataRegister events, clock edges and event-free ScanRegister/DataRegister/top-level pin assumptions; pending default state overwrites overlapping write/compare values by resolved bit identity; no-event reads preserve storage and fill across primary, mixed and hidden-client paths; known write/reset history and mixed-storage acyclic/feedback snapshot settlement preserve transient reset effects | General gated/serial-dependent reset events and complete vendor builtin models |
| AccessLink/TAP | BSDL reader, IR/DR datapaths, FSM, child wiring and runtime metadata restored from flat ICL; explicit IR PDL reads/writes and ordered apply replay, with nine fresh SIB/reset directional certificates; common-phase multiple TAPs on equivalent root TCKs with per-IR metadata, aligned lanes, explicit reset connectivity and batched pin/clock snapshots | Independent TAP phases, shared serial storage, implicit multi-TAP reset ownership and remaining BSDL/private capture forms |
| ScanInterface controls | Runtime explicit/implicit membership; single-TAP phase-qualified visible/hidden capture, shift and update events with concrete replay; standalone explicit/inferred clients drive bound phase/clock pins; complete-path proof or bounded hidden-event planning, indexed lanes, primary-enable preparation and concrete clock sampling covered | Shared-chain, serial-derived controls and general independently gated events |
| Raw scans | Genuine opaque root-client lowering; static modeled root and accessible hierarchical child intervals, disjoint setup queues, binary held-selector/SIB routes and local stable proof; serial aliases/inversion, typed root controls, explicit reset, native CSU replay, TCK waits, default-chain spelling and partial-X controlling fill; bounded single-stream established ScanRegister reservations; ordered ordinary preparation before root raw replay | Reverse-order/child mixed preparation, closed-child raw access, opaque child, TAP/IR/parallel, raw merges, established port ownership and broader vendor qualification |
| Serial/mixed scheduler | Shared parallel/scan frames, primary observations, register/mux/logic paths, SIB and bounded feedback reset prerequisites, read obligations, register/primary/computed-port AccessTogether groups, forwarded/logic/mux/one-hot port writes and partial/X writes; ScanRegister/DataRegister/primary-input iApplyEndState defaults and final restoration; equivalent direct/inverted/forwarded observations and enabled alternatives on one path; simultaneous disjoint named client chains with aligned lanes/common events; independently selected clients (SelectPort or ShiftEn-only) and named-chain groups share hardware state, including reset-before-capture and transient reset history | Independent TAP phases, overlapping intermediate taps and broader access models |
| Idle cycles | Literal TCK counts on standalone clients or reset common-phase TAPs; per-edge state/reset/fill replay, reservation rollback, fixed-point acceleration and retained output count; one forward TCK vendor fixture; checked relative system-clock waits on clock-only routes including direct declared differential partners, ordered muxes, exact fixed-ratio scaling, versioned overrides, typed Period oscillator roots and root-relative replay | General implicit selection, inverted child clock phases, oscillator overrides without explicit source, bound differential-input consistency, absolute-time waits, general clock events and external time-dependent instrument behavior |
| Optimization | Dependency-guided and shared-address write batches; nested iMerge normalization with disjoint multi-apply ScanRegister/DataRegister read/write batching, including slices, aliases and X masks, plus connected/computed data/control-port accesses with bounded dependency ownership and serial fallback; independent tree/finite-state oracles, concrete replay and common-update constraints | Broader merge optimization, timing/resources, whole-program optimization and broader scale/vendor qualification; no global optimality claim |
| Resource ownership | Bit-level register/all-port alias/slice and instance-member takes; applied-write/iState protection, inherited ownership and scoped release; Tessent default/persistent takes within the input stream; protected dependency batches, per-clock replay, explicit-reset ownership clearing and atomic failures | Clock/scan assignment and frequency commands, pattern-set lifecycle, broader scale and vendor qualification |
| Flattened output | Shared netlist serializer, including empty DataRegister declarations; original/flat/generated-PDL tests, including common-phase multiple-TAP instructions | Full corpus and vendor-portable equivalence for all model kinds |
| CLI/build/install/safety | Five dynamic/static executables; staged outputs, input alias protection, BSDL loading; local source-archive/install checks and Rocky8 native dynamic/transferred-static qualification through473bc9e, including39 generator CLI suites/514 tests per build | Remaining command semantics and qualification of later changes |
| Local-reset-only artifact | Transitive ScanRegister/writable-DataRegister reset-control releases, grouped independent controls and feedback-component layers, ordered multi-file rewriting, whole main Tcl command spans, literal-site and post-argument fallback, unchanged procedures and actual rewritten-runtime validation; Tessent accepts whole/sliced argument-reset wrappers | Internal procedure reassertions, remaining caller-site transformations, serial-dependent feedback and broader vendor qualification |
| Tessent oracle | Seven legacy and sixty-three directional certificates for pin/vector, TAP, reservation, shadow storage, transitive-reset, end-state, scan-driven/mixed DataRegister, computed-port, control ownership, merged-observation, standalone client, simultaneous-chain, hidden-client, multiple-client, selected chain-group, ShiftEn-only, all-port membership, DataRegister state and data-controlled reset-mux and generated queued-state and DataRegister reset-release artifact and self-feedback reset-mux and standalone-idle cases; independently generated source schedules need not match | One-hot scan changes have local replay coverage only; merge certification covers the recorded two-stream fixture, and broader model/corpus equivalence remains unverified |
| Acceptance checks | Regression, sanitizers/style and independent review; latest local-reset and numeric-boundary evidence in the generator guide | Strict coverage remains below 100% lines/90% raw branches: fresh complete Task45 instrumented run96.2%/62.0% (14623/15208 lines,24524/39581 raw branches); full vendor equivalence is unverified |

The current integer evaluator reports an explicit resource error beyond signed
64-bit intermediate arithmetic; it does not wrap values. Based binary/hex values
can exceed machine-word size. Numeric values and individual object widths are
bounded at 1,048,576 bits. Elaboration bounds instance depth at 128 and object and
scope counts at 1,000,000 each. These are implementation limits, not IEEE limits.
Logic expression depth is bounded at 128. The total compiled operand storage
(including inferred widths and intermediate results) is bounded at 4,194,304 bits
per elaborated design; signal ranges are checked before expansion and dependency
references are deduplicated. These limits reject excessive work with a resource
error; they do not prove that a target cannot be configured.
No end-to-end performance or optimality claim has been established yet.

Reset dependency ordering now uses a deterministic ready queue, avoiding repeated
whole-register scans for reverse-declared chains. The ordering phase costs
`O((V+E) log V)` with ordered containers; reset-cone tracing and symbolic pattern
search are separate costs. At the supported 4096-register limit, the added
reverse-chain test measured 0.342 s for scheduler construction before this
change and 0.010–0.019 s over three runs afterward (g++ 16.2.1, `-O2`, Linux
6.18.33.2 WSL2). These are construction measurements, not an end-to-end speedup.
That checkpoint required rejection when the test closed the chain into a cycle.
Task39 replaces blanket cycle rejection with bounded feedback settlement and
tests construction of the same 4096-register cyclic graph.
Reproduce with `make pattern-scheduler-check BISON=/usr/bin/bison`; logs are
`build/reset-dependency-{baseline,ready-queue,ready-queue-repeat}.log`.

A repeatable foundation benchmark is `make pattern-netlist-benchmark
BISON=/usr/bin/bison`. It builds a 377,286-byte input with 5,000 specialized
instances and 15,002 nodes. Three runs using g++ 16.2.1 `-O2` on an AMD Ryzen 7 H
260, Linux 6.18.33.2 WSL2, took 54–69 ms to parse and 59–60 ms to elaborate;
whole-process wall time was 0.12–0.13 s and peak RSS 39,788–39,972 KiB. Other
regression/sanitizer jobs ran concurrently, so these are scale measurements, not
a controlled speedup comparison. Measure with `/usr/bin/time -f
'wall_seconds=%e peak_rss_kib=%M' build/pattern_netlist_benchmark`.

Generated build outputs remain ignored. All new regression programs are under
`tests/src/`; normative example inputs are identified in their comments.

The generator's Tcl runtime removes `after`, `vwait`, `update` and `interp` in
addition to using a safe interpreter. Native waits cannot reliably be interrupted
by a Tcl evaluation deadline, and child interpreters would bypass PDL command
instrumentation. Modeled timing belongs in PDL `iRunLoop`. This is not a promise
of memory isolation against hostile Tcl programs.

## Verification checkpoint (2026-10-25)

The focused design/expression/netlist/logic/runtime tests, static runtime test,
Google formatting/naming checks, and Clang ASan/UBSan checks passed. LeakSanitizer
was disabled because leak detection is unavailable under this environment's
tracing; this is not evidence of leak-free execution. Independent review found
invalid object operands and excessive repeated-operand expansion; both have
regression tests that failed before the fixes.

`make coverage BISON=/usr/bin/bison -j4` now includes all six generator
implementation files and their executable inline headers. The previous report's
fixed source list omitted these files despite building instrumented objects.
A regression in `tests/tests/test_coverage.py` verifies that an uncovered generator
line fails the gate. The corrected measurement is 5,878/6,135 handwritten lines
(95.8%) and 9,602/15,838 raw branches (60.6%). The required 100%/90% gate fails;
it has not been reduced. Raw branches include compiler exception/cleanup edges.

A 1,268-byte review input containing 64 comparisons against one 16,384-bit input
uses 47,500–47,512 KiB peak RSS and 0.07 s wall time across three optimized g++
runs after dependency deduplication. The earlier review measured 71,564 KiB for
that input before deduplication. This is a small-case memory observation under
concurrent regression activity, not an end-to-end throughput guarantee. The
maintained wide-expression test additionally checks rejection when the aggregate
operand limit is exceeded.

The final `make check BISON=/usr/bin/bison -j4` with the configured Honza reference
environment also passed after all code changes. It exercised 334/334 grammar
productions and 119/119 scanner rules, plus the existing formatter/LSP and
Vim/Neovim corpus suites. This is regression evidence for the current foundation;
end-to-end generation, flat-ICL replay and generated-PDL idempotency remain unrun
because their implementation is still outstanding.

## Flat serialization checkpoint

The internal `SerializeIcl` API emits instance-free ICL using collision-free
hexadecimal object names and the versioned `icl_pattern_origins_v1` module
attribute. Reimport restores original target paths, qualified module/procedure
bindings (including unused modules), specialized parameter values, scope
attributes, reset associations and virtual/missing connections. ICL parsing,
formatting and constant decoding reuse the existing shared frontend.

Internal input ports become forwarding `DataOutPort` declarations, because
ICL aliases cannot contain constant sources. Metadata restores their original
port kinds. These attributes are required to recover hierarchy-dependent reset
semantics; ordinary IEEE/Tessent parsers accepting the file does not establish
that they interpret this tool-specific metadata.

Negative tests reject non-signal reset references, duplicate/empty special
connections, contradictory polarity and misplaced reserved metadata. The
round-trip suite also covers partial/unconnected inputs, root and per-instance
parameters, namespaces and sequential procedure overrides. It runs in both
`make check` and `make check-dynamic`; the static check and coverage build include
it too. The existing 5,000-instance benchmark now checks canonical flat replay
and reports serialization/reimport time and flat input size.

Metadata is bounded at 16 MiB; ordinary constants retain their 1 MiB limit.
The metadata reader uses the same constant evaluator with an explicit larger
string limit, while leaving numeric bit limits unchanged. The initial scale
check exposed and reproduced the former 1 MiB metadata decoding mismatch.

These checks establish serialization foundations, not completed pattern
generation. The executable, complete model coverage, state scheduler, reset-only
rewrite and final generated-PDL idempotency remain mandatory unfinished work.

IEEE 1687-2014 §6.4.16 (printed pp. 130–133) supplies the typed ScanInterface
rules. The elaborator retains repeated Port statements, named Chain bodies,
per-chain attributes/default loads, and classifies client, host, client-TAP and
host-TAP interfaces. Unique shift/capture/update ports are associated implicitly
for the corresponding non-TAP interface direction. Invalid member kinds,
duplicate ports, missing SI/SO pairs, mixed chain/common scan data and ambiguous
interface types are rejected. Membership is validated after original port kinds
are restored from flat metadata. Unique-selectability and blackbox default-load
path-length proofs remain scheduler/model obligations.

Reset metadata is restored before hierarchy-dependent reset inference. This
avoids inventing a flat-module ambiguity for two separate original reset domains;
cycle checks still run after restoration. A register with ResetValue must retain
its reset association in metadata.

The serialization/interface checkpoint's strict coverage run reports
6,525/6,838 handwritten lines (95.4%) and 10,761/17,962 raw branches (59.9%).
`pattern_flatten.cc` itself reaches 463/512 lines (90.4%). The unchanged
100%/90% gate still fails; this is not a complete coverage result.
Focused dynamic/static checks, Google formatting/naming, and Clang ASan/UBSan
checks pass for this checkpoint. Leak detection remains disabled.

The extended benchmark emits 2,873,993 bytes of flat ICL from 5,000 specialized
instances (15,002 nodes), then reimports and serializes it again with exact byte
equality. Two timed optimized runs during regression activity took 0.65–0.66 s
wall time and 162,080–164,188 KiB peak RSS. These include parsing, elaboration,
serialization, reimport and a second serialization. Use the benchmark command
above to reproduce; these are scale observations, not a throughput guarantee.

The complete `make check BISON=/usr/bin/bison -j4` regression subsequently passed
with the established Honza reference environment after all serialization,
reset-domain and interface changes. It includes dynamic/static checks,
5,000-instance flat replay, 334/334 grammar productions, 119/119 scanner rules,
and full Vim/Neovim corpus checks (38,488 ICL and 728 PDL files each).
This does not complete the missing generator/scheduler/model requirements above.

## Reference policy and addressed-instance preparation

[Pattern generation references](pattern-retargeting-references.md) records the
IEEE/Tessent authority order, inspected Honza retargeting capabilities, and the
OneHotDataGroup Port-width behavior subsequently measured with Tessent 2026.2.

Elaboration preserves instance attributes separately from module attributes and
evaluates instance AddressValue in the enclosing scope. Flat origin metadata
retains those values using two optional trailing scope fields; the original
four-field records remain readable. The decoder rejects duplicate/invalid
attribute names, nonnumeric addresses, invalid field counts and instance
properties on the top module. No attribute override policy is inferred from
storing the two sets separately.

This is preparation for addressed models. It does not yet provide complete
OneHotDataGroup flattening, implicit address/data connections or runtime
configuration. Tests for the new fields and metadata replay are in
`tests/src/pattern_netlist_test.cc` and `tests/src/pattern_flatten_test.cc`.

The [Tessent differential harness](tessent-oracle.md) now preserves executable
width experiments and two complete existing network cases. Eleven vendor runs
matched their expected outcomes. This is reference evidence only: the missing
generator executable and local-reset rewrite still block end-to-end comparison.

The metadata changes passed focused dynamic/static tests, ASan/UBSan (leak
detection disabled) and the complete `make check` regression. After the final
braces-only style correction, clang-tidy, focused checks and a repeat complete
regression also passed, including both full editor corpora. Strict coverage for
this checkpoint measured 6,621/6,929 lines (95.6%) and 10,917/18,158 raw branches
(60.1%). The 100% line / 90% branch gate is still unmet; thresholds are unchanged.

## OneHotScanGroup elaboration

IEEE 1687-2014 §6.4.14 (printed pp. 127–128) defines the group as a distributed
scan multiplexer. Elaboration now records every Port contributor from the shared
AST, preserves concatenation/range direction and inversion, enforces equal
widths, and includes all contributor edges in the existing iterative bit-level
cycle check. Structural enabled-source information is propagated along data
paths through ports, aliases and scan muxes in dependency order. An enabled
ScanOutPort or another OneHotScanGroup must exist in each source signal's data
fan-in. Runtime choice of a usable path and the one-active-source condition
remain scheduler obligations; structural acceptance is not a configuration proof.

The enable information uses one byte per expanded graph bit and is allocated
only when scan groups exist. Group contributor storage is bounded at 4,194,304
bit references per elaborated design. The tests exercise a 3,000-group chain
without recursive graph traversal and reject compact inputs that exceed the
reference limit. Exhausting this budget is a resource error, not evidence that
hardware is unreachable.

The OneHotScanGroup checkpoint passed the complete `make check` regression
with the established reference environment after the final source/test edits,
including static/dynamic checks, 334/334 grammar productions, 119/119 scanner
rules and the full Vim/Neovim corpora. ASan/UBSan also passed with leak detection
disabled. The latest strict coverage measurement is 6,588/6,897 handwritten lines
(95.5%) and 10,862/18,096 raw branches (60.0%); the gate remains unmet. That
measurement preceded the final fixture-only corrections to hierarchical port
references; those corrected fixtures passed focused, sanitizer and full checks.

## OneHotDataGroup Port elaboration

IEEE 1687-2014 §6.4.15 (printed pp. 128–130) permits narrower Port contributors
and specifies explicit, forwarded and instance-address-based enabling. Port
contributors now reuse the one-hot scan netlist infrastructure with separate
scan/data enable flags. Original contributor widths, inversion and declaration
bit order are retained. `NetlistNode::OneHotSourceBit` maps an output offset to
its contributor bit or an unknown leading bit, following the measured Tessent
2026.2 Port behavior documented in [the oracle report](tessent-oracle.md).

Cycle checking uses the aligned bit dependencies, including enable dependencies,
so a leading unknown position cannot create a false feedback cycle. Structural
enable validation recognizes forwarded DataOutPorts, other data groups and
ports in an instance with AddressValue; it does not prove an enable can be driven
or that exactly one source will be active. Runtime selection remains scheduler
work. Flat serialization/reimport preserves Port-based groups and mixed source
widths. Both short and block forms of nested Instance/DataRegister contributors
produce an explicit unsupported-addressed-serialization error, rather than
silently disappearing. Their defaults, address evaluation and scheduling remain
unfinished.

Independent review identified a work-bound problem: a wide group with narrow
contributors could visit every contributor at every leading X position even
though stored references stayed below the allocation limit. Sources are now
stably ordered by descending width (source order is immaterial in §6.4.15), so
dependency traversal stops at the first wholly padded source. Traversal cost is
bounded by graph bits plus actual contributor references, apart from sorting,
dependency deduplication and node lookup. Equal-width scan contributor ordering
is unchanged.

The maintained benchmark adds a 100,109-byte input with a 1,048,576-bit group
and 10,000 scalar contributors. Before the fix it took 24,602.9 ms to elaborate;
the ten-second regression guard then reproduced the failure. After the fix,
two optimized runs took 29.1 and 27.8 ms for the same elaboration. The second
whole benchmark run, including the existing 5,000-instance flat-replay case,
took 0.76 seconds and 166,264 KiB peak RSS. Environment: g++ 16.2.1 `-O2`,
Linux 6.18.33.2 WSL2, with other verification jobs active. These measurements
describe this workload, not end-to-end generator throughput. Reproduce with
`make pattern-netlist-benchmark BISON=/usr/bin/bison` or
`/usr/bin/time -f 'wall_seconds=%e peak_rss_kib=%M' build/pattern_netlist_benchmark`.

Focused dynamic/static tests, Google clang-format/clang-tidy and Clang ASan/UBSan
passed. LeakSanitizer remained disabled because this environment does not support
its tracing requirements. Tests include the actual vendor-probed width fixture,
invalid enables and widths, descending/ascending padding cycles, mixed-width
flat replay, addressed-member rejection, a 3,000-group chain and contributor
allocation limits. `make coverage BISON=/usr/bin/bison -j2` ran the instrumented
tests and failed the unchanged strict gate: 6,657/6,965 handwritten lines (95.6%)
and 10,993/18,258 raw branches (60.2%), against required 100%/90%. The log is
`build/onehot-data-coverage.log`. This remains an acceptance gap; it is not
reported as a passing check.

After all implementation and review fixes, the complete regression exited 0:
`ICL_REF_DIR=/tmp/icl-reference-honza-20261021
PATH="$PWD/build/combined-audit/reference-venv/bin:$PATH"
make check BISON=/usr/bin/bison -j4` (log: `build/onehot-data-regression.log`).
It includes 334/334 grammar productions, 119/119 scanner rules, static/dynamic
checks, and both Vim and Neovim corpora with 38,489 ICL and 734 PDL files each.
The executable, scheduler, local-reset rewrite and complete model coverage are
still unfinished; this checkpoint completes only Port-based group foundations.

## Addressed-instance implicit data input connections

`Elaborator::ConnectAddressedData` reuses the declaration-ordered kind index
and shared `BitReference` connections. After explicit input bindings are
resolved, addressed instances with no explicit data binding consume the leading
parent logical data bits. A partial or whole binding to any child DataInPort
suppresses defaults for the complete child data bus. Missing or insufficient
parent data raises a source-located width error. Unaddressed instances retain
ordinary explicit-connection semantics. See [Tessent probe evidence](tessent-oracle.md#addressed-instance-data-defaults)
for source authority and measured behavior.

Mapping uses a cursor over parent ports, copying only the consumed references;
it does not expand the full parent bus per child. Work is proportional to
connected bits plus child ports (and kind-index lookups), bounded to 4,194,304
implicit data references per elaboration. The existing one-million-bit per-object
limit still applies. The source fixtures and `TestAddressedDataDefaults` cover
both vector directions, ordering different from alphabetical names, sibling and
nested scopes, partial/whole overrides, no default on ordinary instances,
insufficient/missing parent width, and aggregate allocation rejection.

This does not complete addressed-register semantics, address arithmetic or
read/write enable selection. Addressed group serialization still rejects
unsupported members explicitly, so no addressed flat-replay claim is made.

A representative scale check maps 5,000 one-bit addressed children from a
1,048,576-bit parent (`247,909` source bytes). Elaboration took 28.3 ms on its
first run and 48.6 ms while full regression, coverage and sanitizer jobs were
active; this is not a before/after speedup claim. The combined benchmark
(including existing 5,000-instance flat replay and sparse one-hot checks) took
1.10 s wall time / 166,248 KiB peak RSS with g++ 16.2.1 `-O2` on WSL2 Linux
6.18.33.2. Reproduce with `make pattern-netlist-benchmark BISON=/usr/bin/bison`
and `/usr/bin/time -f 'wall_seconds=%e peak_rss_kib=%M'
build/pattern_netlist_benchmark`.

Focused tests, Google style checks, independent review and ASan/UBSan checks
passed; LeakSanitizer was disabled as in the preceding checkpoints. All five
new maintained remote cases matched their expected outcomes, and both successful
PDL exports parsed with the shared trace helper. These are reference measurements,
not generator comparisons. Strict coverage ran and remains below the unchanged
100% line / 90% raw branch gate: **6,692/7,000 lines (95.6%)** and
**11,030/18,314 branches (60.2%)** (`build/address-defaults-coverage.log`).

The complete `make check BISON=/usr/bin/bison -j4` run with the reference
environment exited 0 after the final code/test edits
(`build/address-defaults-regression.log`). It covered 334/334 grammar productions,
119/119 scanner rules, static/dynamic checks, and both Vim and Neovim corpora
with 38,492 ICL and 735 PDL files each. The strict coverage gate above remains
separate and failed.

## Typed addressed contributors and structural flat replay

`OneHotSource` now distinguishes `Port`, `Instance` and `DataRegister` contributors.
It retains the original bit vector and either an instance scope ID or a register
node ID. The former `has_addressed_contributors` flag is removed: a nested member
now has real connectivity, participates in width/budget checks and contributes to
per-bit dependency traversal. Instance outputs concatenate in declaration order,
with each vector traversed left-to-right (IEEE 1687-2014 §6.4.6.7(f), p.89).
Narrow contributors retain their original widths and leading unknown positions;
see the addressed-contributor experiments in [the oracle report](tessent-oracle.md).

Addressed registers retain evaluated `AddressValue` constants. Their implicit
`WriteDataSource` is the rightmost part of their **own module's** logical data-in
bus (§6.4.9(r,v), p.121), including mixed vector directions and multiple ports.
This association is built after flat-origin restoration, so flattened physical
root ports cannot accidentally replace the original local bus. Existing shared
parser checks reject misplaced/mixed register access declarations; elaboration
also checks restored membership, widths, numeric address type and access type.
Address arithmetic, address-source selection and read/write-enable behavior
remain separate unfinished obligations.

Flat output embeds each direct register once in its group. An instance's output
contribution becomes a `Port` concatenation, with its original scope recorded in
optional version-1 `instance_contributors` metadata. Import verifies source
identity, parent scope, uniqueness, complete logicalDataOut width, bit order and
polarity. Metadata node rows use the existing name index for deterministic order:
width-sorting nested register declarations can change their transient node IDs.
Tests cover both standalone register and instance groups, mixed widths/types,
malformed origin records, and original PDL target binding after reimport.

This establishes **structural serialization/reimport**, not vendor-portable flat
hardware semantics or equivalent generated patterns. Our tool uses origin
metadata to retain implicit local buses and enable associations. An instance
contributor with no DataOutPorts still raises an explicit serialization error;
it is not silently dropped. The scheduler and control models must be completed
before testing the requested original-versus-flat generated-PDL equivalence.

Independent review exposed compact address-constant expansion: 100 scalar
registers containing `AddressValue 1048576'b0` occupied 100 MiB of retained
address data from just 4,638 source bytes (108,544 KiB measured RSS). Retained
instance and register address constants now share a 4,194,304-bit budget, checked
before retaining each value. Flat import applies the same combined limit to
register constants and restored scope addresses. Tests first demonstrated that
the oversized input was accepted, then verified rejection, including the mixed
instance/register path and metadata import. Signal/reference limits remain
independent; address values cannot evade them merely by using scalar registers.

The existing benchmark now resolves the previously omitted 5,000 instance
contributors of a scalar group. With a 1,048,576-bit parent data bus and 247,909
source bytes, elaboration measured 62.8 ms; the complete benchmark took 0.66 s
wall / 162,432 KiB peak RSS (g++ 16.2.1 `-O2`, WSL2 Linux 6.18.33.2). This performs
more work than the previous placeholder model, so it is not a speedup claim.
Use `make pattern-netlist-benchmark BISON=/usr/bin/bison` to reproduce.

Focused dynamic tests, the scale benchmark and Clang ASan/UBSan passed after the
review fix. Strict coverage again failed the unchanged 100%/90% gate:
**6,838/7,154 handwritten lines (95.6%)** and **11,263/18,658 raw branches
(60.4%)**, logged in `build/address-contributors-coverage.log`. This measurement
preceded a braces-only Google style correction; no behavioral logic changed in
that correction. Final clang-tidy passed. LeakSanitizer remains disabled due to
the environment limitation already recorded above.

After the final style correction, the complete regression exited 0:
`ICL_REF_DIR=/tmp/icl-reference-honza-20261021
PATH="$PWD/build/combined-audit/reference-venv/bin:$PATH"
make check BISON=/usr/bin/bison -j4`
(log: `build/address-contributors-final-regression.log`). It includes static and
dynamic checks, 334/334 grammar productions, 119/119 scanner rules, and both
Vim/Neovim corpora with 38,493 ICL and 737 PDL files each. The final ASan/UBSan
rerun also exited 0 (`build/address-contributors-reviewed-sanitizers.log`). The
separate strict coverage gate remains failed as reported above.

## Address/control requirement resolution

`src/pattern/pattern_address.{h,cc}` consumes the existing immutable `Netlist` and returns
an addressed DataRegister's effective address signals, computed numeric address,
and read or write enable source. It does not execute PDL, schedule operations,
initialize registers, or prove global address uniqueness. The netlist must
outlive the resolver and must not be changed while it is in use.

The implementation follows IEEE 1687-2014 §6.4.6.24–26, §6.4.7(s–u), and
§6.4.9(l–n,r,s,u). Port order is declaration order and vector order is left to
right. Implicit addressed ancestors share one cached bus; instance offsets and
the register AddressValue are summed with arbitrary-width binary arithmetic.
An explicit scan-register source establishes a new domain. Explicit complete
parent-bus bindings preserve the parent's domain. Read and write enables resolve
independently, and the requested control must share primary-input provenance or
one ScanRegister with the address. Missing, constant, inverted, logic-driven or
mixed-register sources fail with the target's source location.

Numeric leading zeros do not inflate a computed address. A value that actually
exceeds the effective bus width is rejected rather than silently truncated.
This intentionally differs from the measured Tessent overflow behavior described
in [the oracle notes](tessent-oracle.md#address-domain-probes-2026-10-25).
ReadEnPort's automatic deactivation, write data/state constraints, conflicts,
global uniqueness and shortest-pattern scheduling remain later obligations.

`tests/src/pattern_address_test.cc` covers explicit/implicit connections,
scan-driven banks, nested 72-bit sums, mixed vector directions and declaration
order, significant-width checks, repeated failure/cache recovery, illegal source
provenance, canonical flat replay and 2,000 siblings sharing a 4,096-bit bus.
Port indexing is O(nodes); inherited domain storage is shared rather than copied
per descendant. Cached signal references plus offset bits have a 4,194,304-bit
budget. The focused suite measured 0.03 s wall and 21,264 KiB peak RSS on the
existing WSL2/g++ optimized build; this is a new workload, not a speedup claim.
Reproduce with `/usr/bin/time -f 'wall_seconds=%e peak_rss_kib=%M'
build/pattern_address_test` after `make pattern-address-check`.

Independent review found and reproduced three issues before delivery: disabled
DataOutPort forwarding was treated as a direct wire; aliases could discard an
ancestor's address offset; and unrelated million-bit parent buses were allocated
again for each tiny scan-driven bank. Resolution now rejects gated forwarding,
stops at the parent-domain identity through aliases, and compares cached widths
before streaming parent references. The alias test requires 4 + 2 + 1 = 7 both
before and after flat replay, with explicit and implicit parent bindings.

The regression adds 10,000 independently scan-driven banks under an unrelated
1,048,576-bit parent address. The old implementation exceeded its 10-second test
guard; the corrected **whole** expanded suite, including 2,000 shared-bus siblings,
ran in 0.58 s wall / 114,436 KiB peak RSS with g++ `-O2`. The 0.03 s measurement
above was the earlier, smaller suite and is not a comparable baseline. Tests also
exercise oversized logical buses and repeated failures at the retained-offset
budget. Gated control constraints, reordered/partial parent logical-address
bindings and global configuration feasibility are not implemented by this
resolver; unsupported paths fail explicitly. Scheduling remains unfinished.

Final strict coverage after the added primary-input and reordered-mapping checks:
**7,025/7,347 handwritten lines (95.6%)**, **11,541/19,060 raw branches
(60.6%)**. `pattern_address.cc` reaches **187/193 lines (96.9%)** and
**273/402 branches (67.9%)**. The unchanged 100%/90% gate still fails;
`build/address-last-tests-coverage.log` records the failure. ASan/UBSan passes,
with leak detection disabled as previously documented. Google format/tidy and
focused dynamic/static checks pass. The post-review complete regression passed
(`build/address-final-regression.log`); a subsequent complete run includes the
last two test-only additions and is recorded separately below when finished.

The final complete `make check` run after all address source/test changes exited
0 (`build/address-delivery-regression.log`). It includes 334/334 grammar
productions, 119/119 scanner rules, static/dynamic tests, and both Vim and Neovim
corpora: 38,499 ICL / 738 PDL files each. Final ASan/UBSan after the last tests also
exited 0 (`build/address-last-tests-sanitizers.log`). Strict coverage remains
failed as reported above. This verifies the delivered foundation changes, not
the unfinished pattern-generation executable.

## BSDL metadata foundation

`pattern_bsdl.*` extracts the selected entity's static instruction width,
opcodes (including alternatives and masks), instruction capture, optional
IDCODE/boundary length, register associations and scalar TAP port roles.
VHDL identifiers use case-insensitive lookup; instruction names inside attribute
strings retain their spelling. The lexer is independent of ICL and PDL.
Integer constants and concatenated string constants are supported; required
unresolved or unsupported expressions produce source-located errors.

This is not a complete VHDL/BSDL conformance checker or an executable TAP model.
Other attributes retain their original expression and origin. In particular,
`TAP_SCAN_CLOCK` is retained as opaque metadata: the future hardware/timing model
must validate it before use. Instruction legality, electrical pin relationships,
package-defined values and generic AccessLink semantics are not inferred.

`Design::AddBsdl` parses before insertion. Entity lookup is case-insensitive in a
separate registry, without changing ICL top selection or module overrides.
Duplicate BSDL entities are rejected with both locations; malformed or duplicate
loads preserve prior metadata. This intentionally does not introduce a BSDL
override policy absent from the requested ICL/PDL override requirements.

`tests/src/pattern_bsdl_test.cc` uses the maintained Tessent-accepted 1,409-byte
AccessLink companion, static expressions, comments/quotes, malformed inputs,
every truncated source prefix, single-byte deletions, truncated embedded tables,
constant cycles/expansion and a 10,000-port input. Resource limits are 16 MiB
source, 1,048,576 tokens, nesting 128, and separate 4 MiB budgets for decoded
strings and retained attribute/register-association payloads. These payload
budgets are not total RSS limits: token vectors and maps have additional costs.
Port and metadata maps use logarithmic lookup; lexical scanning is linear in
input size, and expression expansion is budgeted.

Tests reproduced and fixed acceptance of constrained `bit(...)` TAP pins and
memory amplification through multi-target attributes and register associations.
Independent review's 82,990-byte input retained 64 MiB of repeated register
names before the fix; copies now charge the budget before insertion. The whole
expanded focused suite measured 0.11 s wall / 71,196 KiB peak RSS on WSL2
6.18.33.2, g++ 16.2.1, `-O2`. Reproduce with
`/usr/bin/time -f 'wall_seconds=%e peak_rss_kib=%M' build/pattern_bsdl_test`.
This is a new workload, not a comparative throughput claim.

After the additional duplicate-declaration and numeric-token tests, the BSDL
reader reaches **404/406 lines (99.5%)**, **760/1,068 raw branches (71.2%)**.
The two unexecuted lines are defensive end-of-declaration and unclosed-string
guards behind the lexer sentinel and prior balanced-statement validation.
Overall handwritten coverage is **7,447/7,771 lines (95.8%)** and
**12,324/20,164 branches (61.1%)**. The unchanged 100%/90% gate fails
(`build/bsdl-final-coverage.log`); neither focused nor complete regression
success should be interpreted as satisfying that coverage gate.
Google formatting/tidy and focused dynamic/static tests pass. ASan/UBSan passes
with leak detection disabled (`build/bsdl-final-verification-sanitize.log`).
The final complete `make check` run after all source/test changes exited 0
(`build/bsdl-final-regression.log`), including 334/334 grammar productions,
119/119 scanner rules, dynamic/static checks and Vim/Neovim corpora of
38,500 ICL / 739 PDL files each. BSDL metadata loading is verified at this
checkpoint; TAP construction, scheduling, CLI/local-reset output and actual
generator-versus-Tessent equivalence remain incomplete.

## AccessLink source binding

`BindAccessLinks` in `pattern_access_link.*` binds the selected top module's
standard AccessLinks to the existing BSDL registry and child module ASTs.
It preserves instruction order, each instruction's ordered scan-interface list,
ActiveSignal selections and source locations. Named interfaces and unique
instance-only shorthand use the existing namespace-aware instance resolver.
Unknown instructions, missing/ambiguous interfaces, duplicate instance/interface
names, multi-chain selections and non-top standard links produce errors.
Generic links fail explicitly because their controller semantics are unspecified.
This follows IEEE 1687-2014 §6.4.17(a,b,e,f), printed pp.133–135.

This API is a source-binding prerequisite for the unfinished TAP builder.
It does not wire the TAP, synthesize ActiveSignals, evaluate signal ranges, or
replace the elaborator's ScanInterface port/type validation. It does not imply
that AccessLink retargeting or flat serialization is implemented. Its result
borrows Design's existing immutable AST and BSDL objects; Design must outlive
the result without replacement or mutation.

The hierarchy walk visits each reachable module type once. Interface definitions
and chain counts are indexed once per referenced type, then looked up for each
reference. No per-instruction copy of BSDL or module ASTs is made. The maintained
test uses 3,000 references and 3,000 unrelated ports: binding measured 2.2 ms;
the complete focused suite measured 0.02 s wall / 17,316 KiB peak RSS on the same
WSL2/g++ `-O2` environment described above. Reproduce with
`/usr/bin/time -f 'wall_seconds=%e peak_rss_kib=%M' build/pattern_access_link_test`.
This is a workload measurement, not a comparative speedup claim.

`tests/src/pattern_access_link_test.cc` covers the complete oracle fixture,
ordering/shorthand/namespaces, both standard-link spellings, comments, one named
chain, an ordinary hierarchy without BSDL and source-attributed rejection paths.
Independent review found a valid instruction named `BSDLEntity` was mistaken
for the declaration keyword. The failing regression now passes because dispatch
distinguishes AST statements from instruction blocks.

Post-review dynamic/static, Google formatting/tidy and ASan/UBSan checks pass
(leak detection disabled). `pattern_access_link.cc` reaches **142/143 lines
(99.3%)**, **181/288 raw branches (62.8%)**. The unexecuted gcov line is the
closing brace of the text-construction helper; the entry, loop and return are
covered. Overall handwritten coverage is **7,589/7,914 lines (95.9%)** and
**12,505/20,452 branches (61.1%)**. The unchanged strict gate still fails
(`build/access-link-reviewed-coverage.log`); thresholds and exclusions were not
relaxed.
The complete post-review `make check` run exited 0
(`build/access-link-reviewed-regression.log`), including dynamic/static checks,
334/334 grammar productions, 119/119 scanner rules and both editor corpora
(38,500 ICL / 739 PDL files each). This verifies the source-binding foundation;
TAP construction and end-to-end generator comparison remain unfinished.

## Instruction-register and ActiveSignal component

`BuildTapDatapathModel` in `pattern_tap.*` contains the owning standalone
instruction-register component built from a bound AccessLink. It generates in-memory
ICL and passes it through the existing `Design`, `Elaborate`, shared AST and
logic compiler. BSDL width and capture bits become a ScanRegister; each selected
instruction has one shared decoder, including alternative/masked opcodes.
ActiveSignals reference that decoder or an OR of the relevant decoders per bit.
Indexed ranges, including parameter expressions, are evaluated in one batch
through temporary port declarations using the shared ICL frontend and
elaborator. No separate range grammar or parameter evaluator is introduced.
Generated names avoid all user-visible output names. OR expressions are balanced
so a large opcode list does not exhaust the shared expression-depth limit.

The result owns its parsed documents and can outlive the input binding/Design.
It records the IR, scan-input and decoder node IDs. It is **not a complete TAP**:
FSM behavior, resets, clock handling, external/child connections and retargeting
remain unimplemented. The data-register extension is described below. No reset value or initial
hardware state is inferred by this component. Indexed ActiveSignals are modeled
natively even though the isolated Tessent 2026.2 probes reject their syntax.
Those vendor outcomes remain expected errors, not equivalence passes.

IEEE 1687-2014 §6.4.17(g), printed p.135, supplies instruction-controlled
ActiveSignal behavior. The inspected Tessent Shell Reference Manual v2023.4
AccessLink topic describes high/low instruction decoding; the existing 2026.2
oracle fixture supplies IR `0011` and capture `0001`. Tests independently check
all 16 four-bit instruction states, shared outputs, multiple masked opcodes,
unknown inputs/capture masks, internal-name collisions and canonical flat replay.
Range tests cover ascending/descending selections, nonzero single-bit indices,
parameter dependencies, overlapping selections, sparse inactive bits, invalid
scalar/vector mixtures and maximum integer indices. A merged vector follows the
first selection's direction (descending for a singleton); numeric bit identity is
preserved, and bits absent from every instruction are inactive. Errors in range
evaluation identify the original ActiveSignal source location.
These checks establish component logic, not full generated-pattern equivalence.

At the scalar-only checkpoint the 2,049-opcode-alternative case measured about
10–14 ms including input parsing, binding, model construction and evaluation.
That earlier suite measured
0.01 s wall / 10,180 KiB peak RSS on WSL2/g++ `-O2`; reproduce with
`/usr/bin/time -f 'wall_seconds=%e peak_rss_kib=%M' build/pattern_tap_test`.
The generated text has a 16 MiB cap. This is not a peak-memory cap: temporary
balanced expressions, ASTs and elaborated bit references require additional
memory and remain subject to the shared frontend's own limits.

Indexed expansion additionally caps aggregate output width and range-bit visits
at 1,048,576 each. Exact duplicate selections within one instruction are
deduplicated before bit traversal; distinct overlapping spans still consume the
visit budget. This prevents small input text from causing unbounded repeated
vector scans. Shared multi-instruction OR expressions are emitted once per
distinct decoder set. The 10,000-repeat / 65,536-bit case is maintained in
`tests/src/pattern_tap_test.cc`. Binding canonicalizes the selected top through
the Design registry, so a caller's temporary module copy cannot dangle.
The optimized focused runs measured 0.095–0.119 s for this repeated-range case.
A whole-suite measurement during concurrent regression was 0.19 s wall and
58,224 KiB peak RSS (WSL2, g++ `-O2`); see
`build/tap-vector-measurement.log`. This is a scale check, not an isolated
before/after speedup benchmark.

The preceding scalar-only checkpoint passed focused dynamic/static, Google style
and ASan/UBSan checks (leak detection disabled). Its strict coverage failed at
**7,675/8,001 handwritten lines (95.9%)**, **12,638/20,678 raw branches (61.1%)**; the new component reaches
**85/87 lines (97.7%)**, **132/226 branches (58.4%)**. Thresholds remain unchanged
(`build/tap-instruction-final-coverage.log`). The first complete regression found
duplicated instrument modules in the new probe fixtures; removing those copies
and reusing the original definition makes the corpus audit pass without an
allowlist exception. The post-correction complete `make check` exited 0
(`build/tap-instruction-final-regression.log`), including dynamic/static checks,
334/334 grammar productions, 119/119 scanner rules, and Vim/Neovim corpora of
38,503 ICL / 739 PDL files each. This does not complete the TAP or generator.

The indexed-component checkpoint passes focused dynamic/static tests, Google
formatting/naming checks, and ASan/UBSan with leak detection disabled. Independent
review confirmed the range-work bound and ownership correction. The unchanged
strict coverage gate still fails: **7,802/8,130 handwritten lines (96.0%)** and
**12,790/20,924 raw branches (61.1%)**. `pattern_tap.cc` reaches **210/215 lines
(97.7%)**, **279/468 branches (59.6%)**; defensive malformed-binding/text-limit
paths remain uncovered. Evidence: `build/tap-vector-delivery-coverage.log`,
`build/tap-vector-delivery-sanitize.log`, and
`build/tap-vector-delivery-tidy.log`. The final post-edit complete `make check` exited 0
(`build/tap-vector-delivery-regression.log`), including dynamic/static checks,
334/334 grammar productions, 119/119 scanner rules, and Vim/Neovim tests of
38,503 ICL and 739 PDL files each. Full TAP wiring, state/control behavior,
scheduling and the generator executable remain unfinished.


## TAP data-register datapath (2026-10-26)

`BuildTapDatapathModel` now extends the same generated/shared-AST netlist with
an IR/DR scan mux, scan output, and BSDL-declared data registers. `ir_select`
is a generic `DataInPort` boundary for a future controller; IEEE ICL defines
`ToIRSelectPort` for controller outputs, not an `IRSelectPort` input keyword.
Each selected ordered AccessLink interface chain has a scan-return input.
Instructions referencing the same ordered instance/interface names share that
boundary; distinct orders remain distinct. These boundaries are not connected
to child module ports yet.

The model uses explicit `REGISTER_ACCESS` associations for instructions without
an AccessLink scan chain:

- `BYPASS`: one bit, CaptureSource zero.
- `DEVICE_ID`: 32 bits, CaptureSource from `IDCODE_REGISTER`.
- `BOUNDARY`: width from `BOUNDARY_LENGTH`; no invented capture value.
- Other register names: require an explicit width; retain unknown capture
  behavior instead of inferring hardware functionality.

Shared register names produce one register and must agree on width. An
ActiveSignal-only instruction can select its declared register. Missing
associations/widths and inconsistent standard-register widths fail with both
AccessLink and BSDL source context. Aggregate emitted data-register width is
bounded at 1,048,576 bits. Fresh names prevent collisions with ActiveSignals.
No reset or initialized instruction/data state is assumed.

The modeled structure follows the *AccessLink* example in the Tessent Shell
Reference Manual v2023.4 (local
`htmldocs/tshell_ref/topics/Command_Accesslink_id84260660.html`), also visible in
the retained Tessent 2026.2 expansion
`build/access-link-no-reset-evidence/expanded.icl`. IEEE 1687-2014 §6.4.17(f),
printed p.135, defines instruction-controlled chain ordering; §4.2.2, pp.12–15,
describes separate instruction/data paths and their controller timing. This
checkpoint implements datapath topology, not that timing or a complete BSDL
conformance checker. No new successful vendor pattern comparison is claimed.

Exact opcode aliases selecting the same route are accepted. Overlapping exact
or masked opcodes selecting different routes are rejected rather than assigning
an arbitrary priority. Exact codes are indexed; mask checks have a 16,777,216
work-unit budget, including pair visits and compared bits. Budget exhaustion
reports a resource limit, not a proof that the network is unreachable. Tests
exercise both conflict orders, shared chains/registers, naming collisions,
10,000 additional exact opcodes, and exhaustion with 6,000 distinct masks.
The focused dynamic/static runs measured approximately 15–19 ms for the
10,000-additional-opcode case, including input parsing, binding and elaboration
(WSL2, g++ `-O2`); this is a scale check, not a before/after benchmark.

The earlier component flat-replay tests now include the data registers and muxes.
Tests also check literal register-selection expectations and capture constants;
idempotency alone is not treated as proof of correct wiring. At that checkpoint,
remaining gaps included implicit BSDL register associations, unlisted instruction bit patterns,
full TAP FSM/reset/clock behavior, child-port wiring and semantic equivalence of
differently named interfaces. A later scheduler must not mistake an unmodeled
route for proven disconnection. End-to-end generation remains unfinished.

Final verification after all source/test changes: complete `make check` exited 0
(`build/tap-datapath-regression.log`), including static/dynamic checks, 334/334
ICL grammar productions, 119/119 scanner rules, and Vim/Neovim corpora of 38,503
ICL and 739 PDL files each. ASan/UBSan passed with leak detection disabled
(`build/tap-datapath-sanitize.log`); Google formatting/naming and diff checks
passed. Strict coverage still fails at **7,934/8,262 lines (96.0%)** and
**12,988/21,234 raw branches (61.2%)**. The datapath source reaches **342/347 lines
(98.6%)**, **476/778 branches (61.2%)**
(`build/tap-datapath-coverage.log`). Thresholds remain unchanged; no generator,
full TAP, or vendor-equivalence completion is implied.

## BSDL implied associations and unassigned opcodes (2026-10-26)

The continuation after corpus checkpoint `6dc6ff9` adds the mandatory register
associations for BYPASS, CLAMP and HIGHZ (BYPASS); EXTEST, INTEST, PRELOAD and
SAMPLE (BOUNDARY); and IDCODE and USERCODE (DEVICE_ID). They may be omitted from
`REGISTER_ACCESS`. Explicit standard register names and widths are checked even
when an AccessLink instruction supplies a child scan-chain route. Nonstandard
instructions without a chain still require an explicit register association.

`USERCODE_REGISTER` is read through the existing bounded BSDL bit-pattern reader.
IDCODE and USERCODE share one DEVICE_ID register but use an instruction-selected
32-bit `DataMux` for their different captures. An unspecified custom instruction's
capture remains unknown; it does not inherit IDCODE's value. Overlapping opcodes
with different capture descriptions are rejected; merging partly unknown
capture descriptions is not implemented. The current reader supports one
32-bit pattern per IDCODE/USERCODE attribute; alternative pattern lists and
per-instruction BSDL `CAPTURES` clauses remain explicitly unsupported.

Unassigned binary opcodes select the shared one-bit BYPASS register. A balanced
OR of distinct opcode comparisons recognizes literal and masked assignments;
an outer `ScanMux` selects the declared route or BYPASS. This avoids enumerating
an exponential complement of the instruction space. Unknown instruction bits
remain unknown when the selected route cannot be determined. No reset or known
initial register state is introduced. A missing BYPASS register is an explicit
model error, not an invented route.

The governing source is **IEEE Std 1149.1-2001 (R2008)**, *Test Access Port and
Boundary-Scan Architecture*, local Zotero storage `RLZEUHNS`:

- Annex B.8.13.3(a), printed157/PDF165: mandatory associations and validation
  of redundant explicit descriptions.
- §8.14.1(b–c), printed58/PDF66: USERCODE selects DEVICE_ID with its own capture.
- §8.1.1(d), printed36/PDF44: otherwise unused binary codes behave as BYPASS.

Tests cover omitted associations, contradictory names/widths, AccessLink paths
that previously bypassed validation, all 16 values of a four-bit instruction,
masked and unknown bits, equal/conflicting capture aliases, unspecified custom
captures, missing metadata, and flat replay of the capture mux. Earlier width
tests now use custom instruction names instead of assigning EXTEST/SAMPLE to
nonstandard registers. Existing 10,000-opcode and mask-budget tests remain.

Resource limits are unchanged: generated ICL is bounded at 16 MiB, capture/route
overlap checks each have a 16,777,216-work-unit budget, and decoded BSDL values
remain bounded. Coverage decoding is proportional to retained opcode text and
uses a set to remove duplicates; it does not scan all possible instruction values.
The added decoder costs extra AST/elaboration work; no speedup is claimed.
On WSL2 with g++ `-O2`, a native focused-suite run measured 0.31 s wall and
61,052 KiB peak RSS; the 10,000-additional-opcode case took 0.085 s. Command:
`/usr/bin/time -f 'wall_seconds=%e peak_rss_kib=%M' build/pattern_tap_test`.
Evidence: `build/tap-defaults-measurement.log`. This run overlapped the editor
regression and is a scale observation, not an isolated performance comparison.

This remains a datapath component. Full controller state/timing/reset behavior,
child interface wiring, private-instruction modeling, pattern scheduling and
end-to-end generator/vendor equivalence remain unfinished.

The final complete `make check` passed after the last test change
(`build/tap-defaults-delivery-regression.log`). It includes static/dynamic
component checks, 2,180 ICL conformance cases, 212 PDL block cases, 137 vendor PDL
streams and both editor corpus runs (38,627 ICL / 802 PDL files per editor).
Grammar/scanner reachability remains 334/334 and 119/119. ASan/UBSan passed with
leak detection disabled (`build/tap-defaults-delivery-sanitize.log`); Google
formatting/naming checks passed (`build/tap-defaults-style.log`). Independent
review confirmed the explicit-association validation fix and reported no
remaining findings.

The final instrumented suites also passed, but `make coverage` exited 2 at the
unchanged strict 100% line / 90% raw-branch gate: **8,016/8,345 lines (96.1%)**,
**13,167/21,502 branches (61.2%)**. `pattern_tap.cc` reaches **420/425 lines
(98.8%)**, **632/1,032 branches (61.2%)**; defensive text/range-validation paths
remain uncovered. Evidence: `build/tap-defaults-delivery-coverage.log`. No
thresholds were relaxed. The continuation remains uncommitted after the pushed
corpus checkpoint; the full generator remains unfinished.

## Native TAP controller (2026-10-26)

`TapController` in `src/pattern/pattern_tap.h/.cc` now models all 16 controller states.
Its 16-bit set starts with every state possible; explicit TRST assertion or five
high TMS rising edges establishes Test-Logic-Reset. Inputs accept normalized
`0`, `1`, and `x`; invalid characters throw before state mutation. Unknown TMS
unions possible transitions; unknown TRST also retains the reset possibility.
This is conservative digital modeling, not analog or metastability simulation.

Rising edges return capture/shift actions using the state before transition.
Falling edges return update actions, reset the instruction in Test-Logic-Reset,
and change TDO enable/register selection. TRST assertion returns an asynchronous
instruction-reset action and holds the controller in reset, without inventing
an observed power-up reset. Actions are `0`, `1`, or `x` and apply only to the
reported event; callers must issue one event per actual clock edge. TDO selection
is undefined (`x`) outside Shift-IR/Shift-DR, where the driver is disabled after
the falling edge. Assertion of TRST does not change those TDO controls early.

The reference is IEEE Std 1149.1-2001 (R2008), Figure 6-1 and §§6.1.2, 6.2.1,
6.3.1, 7.2.1: rising-edge state changes/capture/shift, Table 6-2 TDO timing,
asynchronous TRST, and falling-edge instruction update/reset. Tests contain an
independently stated transition/action table for every state, all `0/1/x` TMS
transitions, edge timing through both IR/DR scans, five-clock convergence,
unknown reset, asynchronous reset with enabled TDO, and malformed-input atomicity.
Transition work and memory are bounded by 16 states, independent of netlist size.

The coverage manifest includes `pattern_tap.h` now that it contains executable
state-mask/accessor functions. Generated parser/scanner code remains excluded;
the authored-code thresholds stay at 100% lines and 90% raw branches.
The gcov input list now also includes the pattern-test translation units:
inline header definitions can be linked from those units instead of a library
object. Before this correction, the report showed zero hits for `TapStateBit`
despite 2,063 calls recorded in the test unit. A Makefile regression check covers
the report inputs. Test source files themselves remain excluded from the totals.

The controller is a separate temporal component. Instruction shift/latch data,
reset opcode selection, register execution, and child interface wiring are still
unfinished; controller tests do not establish generated-pattern or Tessent
equivalence. It does not yet drive the datapath's `ir_select` boundary. Keeping
temporal state separate avoids mutating the shared structural AST/netlist.

The final complete `make check` passed after all source/test changes
(`build/tap-controller-delivery-regression.log`): dynamic/static components,
2,180 ICL conformance cases, 212 PDL blocks, 137 vendor PDL streams, all 334/334
grammar and 119/119 scanner rules, and both Vim/Neovim runs with 38,627 ICL and
802 PDL files each. Focused ASan/UBSan passed with leak detection disabled
(`build/tap-controller-final-sanitize.log`). Google formatting/naming checks
passed (`build/tap-controller-delivery-style.log`). Independent reviews found
no remaining controller or coverage-reporting issues.

The corrected instrumented suites passed, but the unchanged strict coverage
gate failed: **8,066/8,395 lines (96.1%)**, **13,204/21,546 raw branches (61.3%)**.
`pattern_tap.cc` reaches **464/469 lines (98.9%)**, **674/1,076 branches (62.6%)**;
its five missed lines are existing defensive datapath validation paths.
`pattern_tap.h` reaches **5/5 lines (100%)**. Evidence:
`build/tap-controller-delivery-coverage.log`. This is a verified component
checkpoint, not completion of the generator or its coverage acceptance gate.

## TAP instruction-register execution (2026-10-26)

After pushed checkpoint `863c658`, `TapInstructionRegister` consumes the native
controller's IR actions. It owns capture/reset metadata extracted by the existing
BSDL reader and keeps circular shift storage separate from active instruction
codes. Both stages start unknown. Capture/shift occur on rising TCK; only falling
Update-IR changes the active instruction during a scan. Reset chooses IDCODE
when the BSDL describes its register, otherwise BYPASS. A declared IDCODE without
`IDCODE_REGISTER` is an error, matching the datapath builder's missing-metadata
handling. Capture metadata must have the required `01` least-significant bits.

Reset preserves every alternative/masked encoding described for the selected
instruction, with deterministic ordering and duplicate removal. It does not
invent a particular encoding or collapse disjoint alternatives into one bitwise
pattern. Unknown update/reset events retain possible old/new values. A wholly
unknown code subsumes all narrower alternatives, avoiding false resource errors
on an already unknown instruction. Uncertain shifts conservatively join bit
values; correlations between controller state and data may be lost. These joins
must not be used to prove unreachability or claim optimal patterns.

For a BSDL TAP without the optional TRST pin, the reset input is tied inactive
without initializing the controller or instruction. Five TMS-high clocks can
then establish reset and normal IR scans work. Attempts to assert or make an
absent TRST pin unknown fail. With TRST present, explicit assertion resets the
instruction asynchronously. Undefined shift-stage knowledge is discarded;
construction and reset do not invent shift-register contents.

Width and retained encoding text are bounded at 1,048,576 bits/characters, with
at most 65,536 distinct alternatives. Raw reset-code examination also has a
1,048,576-character work limit, including duplicates. Runtime union overflow
reports the BSDL origin and preserves controller/register state, including the
stored reset input. Invalid pin values also fail before mutation.

Known-state shifts use circular storage and take constant work per clock edge,
independent of instruction width. Capture, update, snapshots and uncertain
shifts traverse the width; alternative unions use a bounded ordered set. Tests
exercise 262,151 cycles at width 65,537 and verify bit order after wraparound.
The focused suite records the timing; this is a scale check, not a comparison
against a previous implementation. See `build/tap-instruction-measurement.log`
for native g++ `-O2` wall time and peak RSS on this WSL2 host.

Tests also cover full serial IR loads, update-edge timing, reset alternatives,
unknown TMS/TRST, uncertain updates, metadata ownership/validation, absent TRST,
runtime budget rollback, duplicate-work and alternative-count limits. A decoder
integration test feeds the active instruction codes through the existing shared
AST/netlist logic evaluator. No new parser, AST, or ICL/PDL token set is added.

This component executes IR events and returns DR actions for the future runtime.
`ShiftOut()` exposes the shift stage's internal serial bit; it is not a modeled
physical TDO pin. Live instruction-dependent datapath evaluation, DR execution,
falling-edge TDO sampling, child-interface wiring and full pattern generation
remain unfinished.

Final complete `make check` passed (`build/tap-instruction-regression.log`),
including dynamic/static checks, 2,180 ICL conformance cases, 212 PDL block cases,
137 vendor PDL streams, all 334/334 grammar and 119/119 scanner rules, and both
editor runs (38,627 ICL / 802 PDL files each). ASan/UBSan passed with leak checks
disabled (`build/tap-instruction-delivery-sanitize.log`); Google style/naming and
independent review passed. The native focused suite took 0.40 s and 63,744 KiB
peak RSS; its 65,537-bit / 262,151-cycle check took 0.0111 s while broader checks
were running. No isolated speedup comparison is claimed.

The instrumented suites passed but the strict gate remains unmet:
**8,186/8,517 lines (96.1%)**, **13,373/21,772 raw branches (61.4%)**.
`pattern_tap.cc`: **582/589 lines (98.8%)**, **843/1,302 branches (64.7%)**;
`pattern_tap.h`: **7/7 lines (100%)**. Misses include the earlier defensive
datapath validation paths and defensive shift reinitialization. Evidence:
`build/tap-instruction-coverage.log`. Thresholds remain unchanged.

After adding the isolated AccessLink connection-probe stager and its safety
tests, the final combined complete regression also passed:
`build/tap-instruction-connections-final-regression.log`. The eight remote
elaboration outcomes and their limitations are recorded in
[the reference ledger](pattern-retargeting-references.md#accesslink-connection-precedence-probes).
No C++ behavior changed after the coverage/sanitizer runs above.

## AccessLink external connection checks (2026-10-26)

`BindAccessLinks` now checks BSDL-named top-level TAP declarations and rejects
conflicting port kinds, duplicate declarations, and an existing TAP output
`Source`. A selected child scan input cannot have an explicit instance
`InputPort` binding, including indexed bindings and interfaces using one named
`Chain`. Diagnostics retain the conflicting source location and AccessLink
context. Explicit select/capture/shift/update bindings remain accepted, as
observed in the Tessent probes recorded in the reference ledger.

The binder reuses the shared declaration index and AST, without rewriting the
input. Enum items, comments and attributes do not become port declarations or
connections. Optional BSDL roles reserve names only when present. Instance
bindings are indexed once, including instances in `OneHotDataGroup`; each
module's interface scan-input set is cached. Unselected instances of the same
module do not acquire the selected instance's conflicts. No primitive internal
connections or additional parser are introduced.

Dynamic/static focused tests, Google formatting/naming checks and independent
review passed. ASan/UBSan passed with leak detection disabled. A native focused
run with `/usr/bin/time -f 'wall_seconds=%e peak_rss_kib=%M'
build/pattern_access_link_test` took 0.03 s and 17,296 KiB peak RSS; binding 3,000
references to a shared module containing 3,000 unrelated ports took 0.0028 s.
This overlapped broader regression and is a scale observation, not a speedup
claim.

This completes connection conflict preflight only. Synthesizing external TAP
and child serial/control connections, integrating live DR execution, scheduling,
local-reset rewriting, the generator CLI and vendor pattern equivalence remain
unfinished.

The final complete `make check` passed after all code and test changes:
`build/access-link-preflight-regression.log`. It includes dynamic/static
component checks, 2,180 ICL conformance cases, 212 PDL block cases, 137 vendor
PDL streams, 334/334 grammar and 119/119 scanner rules, and Vim/Neovim corpus
runs of 38,627 ICL and 802 PDL files each. Focused sanitizer and style evidence:
`build/access-link-preflight-final-sanitize.log` and
`build/access-link-preflight-final-style.log`.

Instrumented suites passed; `make coverage` exited 2 at the unchanged strict
100% line / 90% raw-branch gate: **8,264/8,595 lines (96.1%)** and
**13,505/21,984 branches (61.4%)**. AccessLink binding reaches **221/222 lines
(99.5%)** and **315/504 branches (62.5%)**; the uncovered line is the closing
brace of the existing `Text` helper (compiler-emitted cleanup). Raw branch
totals include exception/cleanup edges. Evidence:
`build/access-link-preflight-coverage.log`. This is a failed coverage gate,
not a waiver or a claim of complete generator support.

## Behavioral model insertion (2026-10-26)

The shared `Elaborate` overload accepts parsed leaf modules keyed by top-level
standard AccessLink name. It replaces those opaque declarations with instance
scopes before resolving ordinary connections. The existing BSDL-derived TAP
datapath can therefore provide `tap.long_setup` to the original child-instance
binding without editing or reparsing the user's modules. ScanRegisters remain
atomic behavioral components; their flip-flop/gate internals are not expanded.

Model definitions retain shared ownership of their parsed ASTs, including after
the source registry and temporary model objects are destroyed. The same
parameter resolver, declaration cache, connection checker, reset inference and
serializer handle original and inserted modules. Later source-file module
overrides, namespaces, original diagnostic locations and parent parameter
bindings remain intact. Source modules and replacement warnings are not
rewritten; normal automatic-top-selection warnings still apply.

Unknown replacement names, conflicting module identities, malformed descriptors,
non-leaf models and models containing AccessLinks fail explicitly. Existing
validation also rejects flat-origin metadata on an inserted non-root module.
Module-level name checks include instances/registers inside OneHotDataGroup and
exclude scoped enum/Chain names. A focused regression exposed a grouped-instance
collision in the first implementation; it failed before the fix and passed
afterward.

Tests exercise parameterized/namespaced source and later-definition overrides,
model lifetime, identity collisions, 3,000 instances sharing one model AST, and
insertion of the actual BSDL-derived datapath followed by byte-stable flat
serialization/reparse. The original ordinary `Elaborate` entry point retains its
behavior. Independent review found no additional substantive issue.

This is an insertion mechanism for already-bound behavioral models, not an
AccessLink conformance check or complete TAP expansion. The caller must first
bind/validate the AccessLink. Top-level TAP port synthesis, child serial/control
connections, live DR execution and retargeting remain unfinished. Unconnected
model boundary inputs are still unconnected, and no successful pattern or
vendor-equivalence result is inferred from this structural test.

Focused dynamic netlist and dynamic/static TAP checks passed, as did Google
formatting/naming and ASan/UBSan (leak detection disabled). With g++ `-O2` on
the current WSL2 host, 3,000 shared-model insertions took 0.013 s; the complete
focused netlist test binary took 0.41 s and 117,720 KiB peak RSS. Command:
`/usr/bin/time -f 'wall_seconds=%e peak_rss_kib=%M' build/pattern_netlist_test`.
Evidence: `build/access-link-insertion-measurement.log`. This run overlapped
broader regression; no speedup comparison is claimed.

The complete regression passed after the final collision fix and test updates
(`build/access-link-insertion-regression.log`), including static/dynamic
components, 2,180 ICL conformance cases, 212 PDL block cases, 137 vendor PDL
streams, 334/334 grammar rules and 119/119 scanner rules. Both Vim and Neovim
passed their 38,627-ICL/802-PDL corpus runs. Final focused sanitizer and style
logs are `build/access-link-insertion-final-sanitize.log` and
`build/access-link-insertion-final-style.log`.

The instrumented suites passed, but the unchanged strict 100% line / 90%
raw-branch gate failed: **8,321/8,652 lines (96.2%)**, **13,614/22,134 branches
(61.5%)**. `pattern_netlist.cc` reaches **1,053/1,094 lines (96.3%)** and
**1,572/2,440 branches (64.4%)**. Evidence:
`build/access-link-insertion-coverage.log`; `make coverage` exited 2. No coverage
threshold was relaxed, and this checkpoint does not complete Task 2 or the
generator.

## TAP pins and serial-network assembly (2026-10-26)

`ElaborateTapNetwork` in `src/pattern/pattern_tap_network.cc` binds AccessLinks, builds
the existing behavioral datapaths, inserts their shared ASTs, and connects the
serial topology. Missing BSDL-named TAP pins are synthesized before ordinary
connection resolution and reset/TMS inference. Correct existing scalar pins,
including indexed one-bit declarations, are retained. Conflicting types,
widths, declarations or output drivers fail explicitly; absent optional TRST is
not synthesized.

Each instruction connects its listed IJTAG client interfaces in source order.
Typed ScanMux nodes select a child SI's predecessor by full instruction opcode;
identical sources share deduplicated codes. The last child's SO feeds the TAP
return for that instruction. Endpoint indices and unspecified bits of wider
ports survive flat serialization/reparse. Generated module/mux names avoid user
definitions, scopes, nodes and parameters. Two independent TAPs retain separate
models and pins; multiple TAPs cannot silently overwrite one child input.

The existing bit-level cycle/group-fan-in checker is shared by ordinary
elaboration and the final connected network. A regression closes a feedback
path only when the new serial connections are applied and checks its rejection.
Models and original source documents retain shared ownership, and failed
assembly does not rewrite the source registry. Model internals remain opaque
behavioral primitives.

Aggregate generated model text is limited to 16 MiB. Routing code text, selector
bits and allocated destination bits share a 4,194,304-unit budget; generated
objects retain the elaborator's 1,000,000-object cap. Exhaustion is reported as
a resource limit, not as proof that a target is unreachable. A 600-interface,
4,096-bit instruction test reaches the route limit. No-link designs use ordinary
elaboration without a second graph check.

Tests cover generated/reused pins, optional TRST, named chains, instruction
ordering, exact opcodes, indexed SI/SO and TAP pins, explicit controls, two
independent TAPs, conflicts, unsupported interface kinds, new feedback, model
name collisions, flat idempotency and 3,000 interfaces. Independent review found
no actionable issue. Focused dynamic/static, Google style/naming and ASan/UBSan
(leak detection disabled) passed. The full focused TAP binary measured 0.67 s
and 104,880 KiB peak RSS; the 3,000-interface assembly took 0.179 s with g++ `-O2`
on WSL2. Command: `/usr/bin/time -f 'wall_seconds=%e peak_rss_kib=%M'
build/pattern_tap_test`; evidence: `build/tap-network-measurement.log`. This
overlapped broader regression and is not a speedup claim.

This assembles serial topology only. Other interface kinds fail with an explicit
implementation-limit diagnostic. Capture/shift/update/reset/control event
wiring, live DR execution and retargeting remain unfinished. Structural flat
replay does not yet restore `TapNetwork::taps` runtime node associations. The
retained Tessent 2026.2 elaboration probes inform the topology tests; no new
vendor run or generated-pattern equivalence is claimed.

The complete regression passed after the final source/test changes:
`build/tap-network-regression.log`. This includes static/dynamic components,
2,180 ICL conformance cases, 212 PDL block cases, 137 vendor PDL streams, all
334 grammar rules and 119 scanner rules, and both Vim/Neovim runs of 38,627 ICL
and 802 PDL files. Final focused/sanitizer/style evidence is in
`build/tap-network-delivery-focused.log`,
`build/tap-network-delivery-sanitize.log` (leak checks disabled), and
`build/tap-network-final-style.log`.

Instrumented suites passed, but `make coverage` exited 2 at the unchanged strict
100% line / 90% raw-branch gate: **8,505/8,843 lines (96.2%)** and
**13,867/22,548 branches (61.5%)**. The new serial assembler reaches **153/160
lines (95.6%)** and **189/326 branches (58.0%)**. Uncovered paths include the
aggregate model-byte cap, final object-count guard, conflicting cross-TAP port
roles, and compiler-emitted cleanup lines. The exercised route-expansion budget
is distinct from those guards. Evidence: `build/tap-network-coverage.log`.
Thresholds remain unchanged; this is not a full-generator completion claim.

## Controller events and executable flat replay (continuation)

`Netlist::taps` now retains the BSDL input and typed IR/DR, decoder, serial-return,
pin and controller-boundary associations. Flat restoration reparses the retained
BSDL with the existing bounded reader and validates node identities, widths,
scopes, capture masks and external serial connections. Existing client-interface
bindings take precedence over generated capture/shift/update, select and clock
connections. Indexed control ports retain their selected bit indices.

`NetworkState` owns unknown-initial register shift/update state and external
input values. Its iterative, cached combinational evaluator uses the existing
compiled logic and connection graph. `TapExecution` applies observed pin edges
to the native controller and those register models. Registers are behavioral
units, with no internal gate or flip-flop expansion.

The regression checks rising-edge capture/shift, falling-edge update, simultaneous
old-state serial sampling, reset without Select qualification, held-reset shifting,
pre-edge reset qualification of updates, immediate propagation of decoded IR
resets, post-update shift uncertainty, explicit clock suppression, bypassed but
explicitly enabled registers, and alternative interfaces sharing a register.
Select conditions are ANDed within an interface; effective events are combined
across alternative interfaces. Original and flat networks run the same complete
IR/DR scan sequences and compare state and TDO after every cycle.

Independent review identified three ordering/interface defects; each has a
failing-before/fixed-after regression in `tests/src/pattern_state_test.cc`.
A separate typed-netlist test catches double application of reference inversion
when replaying manually constructed logic through flat ICL.

For 1,000 eight-bit register interfaces, stopping membership traversal at each
interface's SI reduced observed construction time from 0.223 s to 0.0019–0.0026 s
on the same WSL/g++ `-O2` fixture. These are construction measurements, not
whole-pattern throughput claims. Evidence is in ignored `build/tap-runtime-scale-*`
and `build/tap-alternative-interface-green.log`.

This is observed-event replay. One-hot state execution, remaining interface
kinds, inverted child-clock phases, complete implicit-control propagation and
multi-TAP virtual-reset ownership still need work. Scheduling, the CLI and the
local-reset artifact are not delivered by these tests. The initial full runtime
regression passed (`build/tap-runtime-regression.log`); the later typed-inversion
fix and symbolic additions require final integrated regression again.


## Integrated serial generator checkpoint (2026-10-26)

The executable now connects ordered ICL/PDL loading, procedure expansion,
stateful scheduling, concrete scan replay, canonical PDL/ICL emission and safe
file publication. Both dynamic and static targets exist. This is a material
integration step, not completion of the whole retargeter.

The bounded serial backend separates unknown hardware state, controlling/fill
state and read expectations. It searches scan horizons and then minimizes shift
length; a separate finite-state enumerator checks four nested-SIB initial
configurations. Per-transaction serial replay verifies register updates against
the solver model. Read expectations are emitted at the first active capture,
including all-X reads; a requested expected value is not assumed to be actual
hardware input data.

Regression additions in `tests/src/pattern_scheduler_test.cc` and
`tests/src/test_icl_pattern_generation_main.py` cover SIB/reset prerequisites,
reset release edge ordering, stale post-reset fill, partial/masked writes,
repeated writes, read/capture independence, original/flat replay and PDL
idempotency. The CLI tests also cover ordered procedure replacements, path
alias rejection, staging failure preservation and help/version behavior.

Independent review found implicit-client controls being ignored and a
child-bound `iReset` being converted to a global reset. Shared effective-interface
discovery and a runtime scope check fix these, with failing-then-passing tests.
IEEE 7.9.9(a,e) also confirms that an explicit reset resets controlling state
while unresettable hardware retains its logic state, and that `-sync` has no
additional effect when the target has no TMSPort.

Outstanding: TAP transaction planning, nondefault interface events, shared
multichain paths, one-hot/addressed access, feedback reset solving, full PDL
commands/merging/timing/resource behavior, local-reset-only source rewriting,
full-corpus generation and Tessent-generated pattern equivalence. Unsupported
cases remain errors; none is counted as a successful generated case.

## Single-TAP scheduler integration (2026-10-26)

The executable now consumes the existing native TAP model. A symbolic frame
chooses IR or DR, reuses the runtime's interface membership, constrains complete
capture/shift/update events, and replays the result through `TapExecution`.
Unknown initial TAP state requires explicit `iReset`; no reset is inserted.
The CLI accepts `--bsdl ENTITY FILE`, protects BSDL inputs against output aliases,
and preserves `iReset -sync`. Debug ICL carries BSDL metadata for standalone
flat replay. Generated PDL omits internal instruction-register writes.

Focused tests cover hand-derived serial bits, retained and switched instructions,
TRST/synchronous/no-TRST reset, inherited ResetPort behavior, state assumptions,
disabled clock/select/shift/update, source/call-stack errors, failed-state
preservation, and original/flat/generated output equality. These checks do not
establish support for arbitrary shadow events, multiple TAPs or the remaining
retargeting requirements.


## Fresh authored-code coverage (2026-11-14)

The complete instrumented suite includes Tasks40–45 and completes its tests,
but the strict gate fails:14,623/15,208 lines (96.2%) and24,524/39,581 raw branches
(62.0%), against100%/90%. Evidence: build/alias-boundary-full-coverage.log/.exit(2),
with raw reports in .worktrees/coverage/2026-11-14-alias-boundary. Earlier reports
were preserved. Grammar-rule coverage remains a separate measure.

Of15,057 untaken raw branches,10,690 are marked '(throw)' by gcov. This diagnostic
count does not exclude exception edges or weaken the required threshold. Meaningful
uncovered behavior and compiler-emitted edges still need separate investigation.
No full-coverage or complete-retargeter claim is made.

## Common-edge TAP control replay (2026-11-14)

All TAP actions are sampled before control publication, and all control outputs
are published before effective register controls are evaluated. Six source/flat
native cases establish name-order invariance for forwarded capture, shift and
update. One joint state clock and existing instruction/reset settlement remain.
Full dynamic/static and leak-enabled native/CLI checks pass; see
[the verification record](superpowers/plans/2026-11-14-tap-edge-snapshot.md).
This establishes concrete replay only. Multiple-TAP scheduling, shared serial
ownership and implicit multiple-TAP reset semantics remain unfinished. The
Task45 strict coverage result and473bc9e Rocky8 qualification predate this fix.

## Common-phase multiple-TAP scheduling (2026-11-14)

Shared-TCK TAPs now use one joint solver/state and aligned independent serial
lanes, with per-IR captures and explicit reset connectivity. Pin snapshots batch
all reset assertions and per-cycle TMS/TDI levels. Unrequested lanes remain in
physical replay; synthetic BSDL storage is not emitted as a PDL target.
Per-frame scan end-state constraints fix early override expiration in both
single- and multiple-TAP generated applies. Native/canonical tests and independent
review cover these changes, including a name-order-dependent gated-reset failure.

Full integration passes:40 dynamic/static suites,51 directional/seven legacy
goldens,2523 ICL334/119 rules and editor6. Focused leak-enabled functional checks
pass; instrumented16/32-TAP scale cases hit the unchanged solver budget and are
retained as failures. Optimized scale cases pass with explicit IR setup for32.
See [the exact limits and evidence](pattern-generation.md#common-phase-multiple-taps-2026-11-14).
Distinct equivalent clock roots, independent TAP phases, shared serial storage
and implicit multiple-TAP reset ownership remain required. Vendor loading
failures provide no multiple-TAP certificate; strict coverage and Rocky8
qualification predate this implementation.

## System-clock override snapshots (2026-11-14)

Per-bit version histories retain previously checked durations until another iClock;
current reservations use current routes. Omitted source/factors return to immutable
ICL defaults. Prospective cycles fail atomically, and non-clock fanout remains
unsupported. Native snapshot/reservation/rollback tests and30 CLI methods pass;
4096-stage/1024-wait alternating-version benchmark takes4.32s optimized and21.4063s
with leak checks. Independent review, naming/style and selected native/CLI
ASan/UBSan checks pass. Full integration passes in
build/clock-override-final-full.log/.exit0 (1050.603s):41 dynamic/static suites,
51 directional/seven legacy goldens,2523 ICL334/119 rules and editor6 in182.629s;
five optional ANTLR tests skipped. See
[the guide](pattern-generation.md#system-clock-override-snapshots-2026-11-14)
for exact evidence and measured Tessent2026.2 omitted-default/no-option differences.
Oscillators, absolute time, general events, strict coverage and later platform
qualification remain open.

## Embedded oscillator relative waits (2026-11-14)

Typed Period ToClockPorts supply distinct hierarchical/bit-specific roots; positive
descendants scale once. Thirty-seven CLI methods, native held-bit/state/flat tests
and independent source review pass.

Final integration passes: build/oscillator-final-full.log/.exit0,999.946s wall,
41 dynamic/static suites,51 directional/seven legacy goldens,2523 ICL334/119
rules, editor6 in176.843s; five optional ANTLR tests skipped. Leak-enabled native
scheduler/flatten/netlist and main35/clock37/client18/reset12 checks pass. Naming,
formatting and independent source review pass. Tessent2026.2 source/normalized
exports and our output have identical child.osc7/10 waits; fetched input hashes
match. Evidence: build/oscillator-vendor-probe/comparison.json. Strict coverage
and Rocky8 qualification remain open.

## Explicit oscillator source overrides (2026-11-14)

Period targets accept explicit replacement sources with old-version duration and
current-version reservation checks. Forty-one clock methods and native source/flat
bit-isolation, rollback and state checks pass. Omitted-source oscillator overrides
remain open.

Full integration passes in build/oscillator-override-final-full.log/.exit0,
1086.789s wall:41 dynamic/static suites,51 directional/seven legacy goldens,
2523 ICL334/119 rules and editor6 in181.938s; five optional ANTLR tests skipped.
Leak-enabled native scheduler/flatten/netlist and main35/clock41/client18/reset12
checks pass. Naming, formatting and independent source review pass. The exact
Tessent2026.2 probe trace matches our normalized output; no formal certificate,
strict coverage or platform refresh is claimed.

## Clock source terminal eligibility (2026-11-14)

Positive clock/data/constant unions are checked across ordinary clock consumers
with per-bit alias memoization. Differential references and explicit child input
bindings survive flat replay with validated metadata and restored alias restrictions.
Main38/native/flat checks and corrected independent source review pass. Direction,
contextual DataMux/control rules and other source families remain required.

Full integration passes in build/clock-source-final-full.log/.exit0,1103.136s
wall:41 dynamic/static suites,51 directional/seven legacy goldens,2523 ICL334/119
rules and editor6 in181.921s; five optional ANTLR tests skipped. Leak-enabled
native scheduler/flatten/netlist and main38/clock41/client18/reset12 checks pass.
Formatting, naming and corrected independent source review pass. Strict coverage,
new vendor certification and later platform qualification remain open.

## Child TCK input source kinds (2026-11-14)

Restored TCK bindings use a separate terminal family from general clocks while
retaining ordinary data/constants. Explicit ToTCKPort Source extension coverage
keeps native/client gated-clock assertions without validation exceptions.

Confirmed full integration passes in build/tck-source-confirmed-full.log/.exit0,
964.274s wall:41 dynamic/static suites,51 directional/seven legacy goldens,2523
ICL334/119 rules and editor6 in182.718s; five optional ANTLR tests skipped.
Leak-enabled native scheduler/flatten/netlist and main39/clock41/client18/reset12
checks pass in build/tck-source-confirmed-sanitizer-*. Original full exit2 and
scheduler sanitizer exit1 are preserved. Source/fixture reviews, naming and style
pass; no kind exemption or solver-budget change was introduced. Strict coverage,
new vendor certification and later platform qualification remain open.

## Reset source terminal eligibility (2026-11-14)

Reset source families and virtual global reset are checked without generated
exceptions; separate TRST acceptance is preserved pending the stated normative
ambiguity. DataMux forcing and broader reset models remain open.

Confirmed full integration passes in build/reset-source-confirmed-full.log/.exit0,
955.735s wall:41 dynamic/static suites,51 directional/seven legacy goldens,2523
ICL334/119 rules and editor6 in182.967s; five optional ANTLR tests skipped.
Leak-enabled native scheduler/flatten/netlist and main40/clock41/client18/reset12
checks pass in build/reset-source-confirmed-sanitizer-*. First full exit2 and
scheduler sanitizer exit1 remain recorded. Corrected source/fixture reviews,
naming/style and focused checks pass. No frontend exemption is introduced.
Strict coverage, formal vendor certification and platform refresh remain open.

## Scan controls and TMS/TRST source eligibility (2026-11-14)

Matching capture/shift/update/TMS/TRST families, ordinary data/constants and
bit-specific aliases are checked after origin restoration. TRST admits inferred
asynchronous global reset. Ordinary data in TMS/TRST Source contexts now requires
a direct DataMux driver; see the direct-driver section below. Unique scalar
reset/TMS/TRST sources broadcast across
grouped destinations; ambiguous vector candidates are rejected and explicit vector
mapping is unchanged. A4M inferred-control-bit preflight bounds allocation.
Main43/native/flat/TAP/client/reset checks and independent review pass. Vendor
scalar/grouped and rejected-vector evidence establishes loading compatibility,
not functional control equivalence; see the guide and focused plans.

Full combined integration passes in build/control-source-final-full.log/.exit0,
1094.986s wall:41 dynamic/static suites,51 directional/seven legacy goldens,
2523 ICL334/119 rules, editor6 in185.044s; five optional ANTLR tests skipped.
Leak-enabled native scheduler/flatten/netlist and main43/clock41/client18/reset12
checks pass in build/control-source-final-sanitizer-*. Final source/test hashes
match the reviewed Task59 checkpoint. Naming/style and independent source/resource
reviews pass. Strict coverage, formal vendor certification and platform refresh
remain open.

## Scan source eligibility and alias-hidden taps (2026-11-14)

Concat-scan consumers accept scan/data/constants; bare ScanInSource accepts only
scan terminals, rightmost register bits and constants. Both register bit classes
retain ordinary-data eligibility. Per-bit cached aliases expose invalid middle
taps without traversing legitimate mux/port terminals. Explicit/implicit rightmost,
ascending/descending and source/flat cases pass. Main46/native/state/TAP/client/
scheduler checks and independent review pass. Vendor19 loading-only cases match
submitted hashes; their seven accepted controls and12 source-specific rejections
are not a functional equivalence certificate. Direction/context and broader
scheduling/event requirements remain open.

Full combined integration passes in build/scan-sources-final-full.log/.exit0,
1121.542s wall:41 dynamic/static suites,51 directional/seven legacy goldens,
2523 ICL334/119 rules, editor6 in198.149s; five optional ANTLR tests skipped.
Leak-enabled native scheduler/flatten/netlist/state/TAP and main46/clock41/client18/
reset12 checks pass in build/scan-sources-final-sanitizer-*. Final three source/test
hashes match the reviewed Task61 checkpoint. Naming/style and independent source
reviews pass. Strict coverage, formal vendor certification and platform refresh
remain open.

## Addressed-port source terminal eligibility (2026-11-14)

Matching Address/WriteEn/ReadEn families, data ports and both ScanRegister bit
classes are checked after restoration. Other data primitives/constants/controls
are rejected; cached binding holes remain distinct. Existing explicit/inherited
and top-level DataInPort domains pass. The lowered resolver logic/cache-retry case
retains its assertions. Vendor35 matched-input loading cases distinguish eligible
terminals from enabled-output context failures; no operation/equivalence claim.
Contextual direction and complete elaboration-wide domain validation remain open.

Full integration passes in build/address-port-source-final-full.log/.exit0,
1094.305s wall:41 dynamic/static suites,51 directional/seven legacy goldens,
2523 ICL334/119 rules, editor6 in180.025s; five optional ANTLR tests skipped.
Leak-enabled native scheduler/flatten/netlist/state/TAP/address and main47/clock41/
client18/reset12 pass in build/address-port-source-final-sanitizer-*. Four final
source/test hashes match the reviewed Task62 checkpoint. Naming/style and
independent source/fixture review pass. Strict coverage, formal vendor certification
and platform refresh remain open.

## DataMux impossible terminal kinds (2026-11-14)

Choice inputs now enforce the data/five-conditional-control permitted superset;
selectors remain ordinary data. This is not full context/structural/forcing
validation. Main48/native/state/TAP/client/clock and reviewed fixture checks pass.
The wide655360-reference case measures0.137–0.167s source validation with equal
baseline/replay output hashes. Context qualification preserves unused outputs and
records standard/vendor mixed-fanout differences without an equivalence claim.
Matching use and aggregate mux allocation/dependency-work bounds remain required.

Confirmed full integration passes in build/data-mux-source-union-confirmed-full.log/
.exit0,1061.125s wall:41 dynamic/static suites,51 directional/seven legacy goldens,
2523 ICL334/119 rules, editor6 in187.584s; five optional ANTLR tests skipped.
Selected leak-enabled native scheduler/flatten/netlist/state/TAP/address and
main48/clock41/client18/reset12 pass in build/data-mux-source-union-final-sanitizer-*.
The five selected source/test inputs hash-match that snapshot; the sixth corrected
multi-TAP CLI fixture passes optimized22, including source/flat/generated replay.
No instrumented multi-TAP scale claim is made. Initial full exit2 and test-only
metadata assumptions remain recorded. Naming/style and independent product/fixture
reviews pass. Strict coverage, formal vendor certification and platform refresh
remain open.

## Conditional DataMux matching use (2026-11-14)

Observed special-control choice bits require matching typed control use, including
reset associations and TRST-to-reset compatibility. Aliases/mux data chains carry
per-bit context; other consumers only mark observation. Unused bits stay valid.
Main49/native original/flat/mixed-fanout/boundary tests pass. Wide/deep overhead is
measured with equal baseline/replay output hashes. This does not establish mux
structure, polarity, forcing, operation or full vendor compatibility.

Full integration passes in build/data-mux-control-use-final-full.log/.exit0,
1228.810s wall: dynamic/static suites,51 directional/seven legacy goldens,
2523 ICL334/119 rules and editor6 in281.764s. Five optional ANTLR tests remain
skipped. Leak-enabled native scheduler/flatten/netlist/state/TAP/address and
main49/clock41/client18/reset12 pass in build/data-mux-control-use-final-sanitizer-*.
All three source/test SHA-256 values match the independently reviewed checkpoint
and measured scale source. Naming/style and independent review pass. Strict
coverage, full vendor certification and refreshed platform checks remain open.

## Mux expansion and selector dependencies (2026-11-14)

Shared4M selector/source/code expansion accounting covers all mux kinds,
alternatives, instances and final generated TAP routes. Preflight covers unsized
padding; selector codes must be numeric. Exact combined source/flat boundary and
atomic CLI rejection tests pass. Shared selector vertices preserve scalar cycles,
valid cross-bit DAGs, register cuts and enabled-data validation. Wide selector
measurements preserve output hashes while reducing repeated graph traversal;
ordinary/control scale remains measured. Shared target-width preflight also bounds
unsized padding in other resolver callers. Necessary primary performance fixes
coalesce symbolic vector runs and bound unsigned objective chunks, preserving
value/knownness, scan state, fill order, reservations and replay. Legal1024/4096-bit
writes,262-bit preference and4097-bit atomic rejection are checked.

Full final integration passes in build/mux-resource-primary-confirmed-full.log/
.exit0,1641.867s wall with make-j1check: dynamic/static generator suites,51 directional
goldens,2523 ICL334/119 rules, LSP62 and editor6 in216.032s. Five optional ANTLR
tests are skipped. Leak-enabled native symbolic/netlist/flatten/state/TAP/address/
scheduler and main50/clock41/client18/reset12 pass in
build/mux-resource-primary-final-sanitizer-*. All8 source/test hashes match the
reviewed checkpoint. Naming/style and independent review pass. Initial solver
failures remain recorded. Strict coverage, full vendor certification and refreshed
platform qualification remain open.

## CaptureEn/UpdateEn DataMux structure (2026-11-14)

Actual CE/UE provenance through mux data inputs and aliases triggers per-bit
validation: one distinct corresponding control path, with literal-zero forcing
inputs. Canonical inversion-aware aliases, repeated references/zeros, identity
muxes, chains and independent vector bits retain source/flat coverage. Unused
actual-control gates are checked. Ordinary-data-only mux interpretation remains
explicitly separate, as do output polarity and operational sensitization.

Corrected34 and boundary18 Tessent2026.2 loading probes have12 and6 accepted
controls; fetched model/runner hashes match. Initial36 cases were shared-fixture
setup failures and supply no semantic evidence. Duplicate-control-row and
ordinary-data-only consumer differences are documented standard-first choices,
not vendor equivalence. Matched-output scale measurements and memory are in
build/capture-update-mux-scale/comparison.json. Naming/style, independent review,
native7 leak-enabled ASan/UBSan and main51/clock41/client18/reset12 pass.

Full integration passes in build/capture-update-mux-final-full.log/.exit0,
1774.796s wall with make-j1check: dynamic/static generator suites,51 directional
goldens,2523 ICL334/119 rules, LSP62 and editor6 in255.589s. Five optional ANTLR
tests are skipped. Native7 leak-enabled ASan/UBSan and main51/clock41/client18/
reset12 pass. All four final source/test hashes match the reviewed checkpoint.
Strict coverage, full vendor certification and refreshed platform checks remain open.

## Direct reset/TMS/TRST data drivers and known binary muxes (2026-11-14)

All six input/To* Source contexts require DataMux drivers for ordinary-data bits.
Canonical aliases remain transparent; ordinary ports/storage/logic/groups do not.
Native direct/aliased producers, mixed vectors, legal original/flat muxes and
main52 artifact-preservation cases cover the rule. Typed controls, constants,
virtual reset and holes remain eligible.18 source-specific vendor ICL65 cases,
qualified typed/mux controls and corrected constant cases supply loading evidence
with fetched hashes. Full shape, direction, forcing/polarity and operation remain
open; no functional vendor certificate is inferred.

Connected fixtures retain release/order/reservation/replay checks with real
reset paths. Exact arbitrary feedback/history circuits have explicit lowered
native coverage, including self CaptureSource and per-failed-Apply storage
rollback; their former source/flat/CLI artifact dimensions are not claimed.
No-TRST standalone global assertion is distinguished from TRST-specific selection.

Fully known scalar binary muxes now use direct Select in either row order,
with width guards and shared knowledge masks. Unknown/overlapping/incomplete/
alternative/wider cases preserve consensus; shared Enable/held/shift paths stay
unchanged. Variable-selector,243 ternary vector, malformed-width RED/GREEN and
512/4096 closed-form proofs pass. Final27 interleaved32-register source/flat/
generated cases pass for implicit/physical/reversed roots with matching hashes.
Diagnostics retain65 reset passes/34 objectives, halve named constraints and
measure73/141ms checks in the physical case. No deadlines or assertions weakened.
Deep4096 selector0.026–0.031s construction and data0.132–0.139s construction are
recorded; old4096 selector timed out after60s without a completed measurement. Loading overhead
has separate wide/deep/ordinary matched-output measurements. Naming/style and
independent reviews pass. Native7 and main52/clock41/client18/reset12 leak-enabled
ASan/UBSan pass. Final full serial integration passes in
build/direct-control-driver-final-full.log/.exit0,1808.878s, with41 dynamic/static
generator suites,51 directional goldens,2523 ICL334/119 rules, LSP62 and editor6
in220.236s. Optional ANTLR5 skipped; all29 reviewed source/test hashes match.
Strict coverage, full vendor and platform qualification remain open.

## Reset/TMS/TRST mux shape (2026-11-14)

Per consumed bit, require a corresponding control path and at most two canonical
mux inputs. Alias/inverted-literal identity, mixed vectors, nested control gates,
ordinary-data forcing muxes and unused gates have native/CLI boundaries. Identity,
duplicate-row and two-control cases retain the documented semantic interpretation;
72 matched vendor loading cases distinguish it from vendor special-row limits.
No operation or broad conformance is inferred. Current original/flat/generated and
atomic pattern/reset/debug checks pass;512/4096 shared control chains retain flat
identity. All60 interleaved wide/deep/ordinary loading runs pass with matched
hashes. No API/metadata/deadline or dense-array addition is made; context storage
is reused after earlier checks.

Local native7/main53/clock41/client18/reset12/multiple-TAP22/feedback10, naming/style
and independent review pass. Root native7 and selected main53/clock41/client18/
reset12 leak checks pass. Final serial regression passes in
build/reset-mux-structure-final-full.log/.exit0,1618.418s:41 dynamic/static generator
suites,51 directional goldens,2523 ICL334/119 rules, LSP62 and editor6 in211.728s.
Optional ANTLR5 skipped; all3 source/test hashes match. Rocky8 evidence is
an earlier candidate snapshot with native7/dynamic41 individual exits0 and a
completed archive, despite transport124; no final platform qualification claim.
Full directional/forcing/polarity/operational and remaining runtime/TAP/lifecycle,
strict coverage, full vendor and refreshed platform requirements remain open.

## Effective typed-control inversion (2026-11-14)

Matching typed TCK/TMS/TRST/CaptureEn/ShiftEn/UpdateEn Sources reject odd canonical
inversion. Direct/child output terminals, per-bit mixtures and aliases are covered;
even cancellation retains the documented semantic compatibility interpretation.
Data/constant, Reset/Scan/clock contexts, virtual active-low TRST and the existing
ToTCK extension retain their boundaries. No broader syntax, direction, mux/output
polarity or operation claim is made.64 matched-hash loading probes distinguish
vendor extensions/restrictions; no PDL ran.44 main negatives protect all3 artifacts;
two existing clock-phase fixtures retain their backend assertions via extensions.

Local native7/main54/clock41/client18/reset12/multiple-TAP22/feedback10, naming/style
and independent review pass.48 loading comparisons preserve output/flat hashes.
Required native7/main54/clock41/client18/reset12 leak checks pass. Full serial
integration passes in build/typed-control-inversion-final-full.log/.exit0,
1381.252s,41 dynamic/static suites,51 directional goldens,2523 ICL334/119 rules,
LSP62 and editor6 in173.740s. Optional ANTLR5 skipped; all5 hashes match.
Rocky8 individual native7/dynamic41 and archive pass; transport124 and test-format
snapshot differences remain explicit. Strict coverage, full vendor certification,
remaining functionality and final platform qualification remain open.

## Explicit reset-port polarity (2026-11-15)

IEEE1687 6.4.6.13(a,b) dictates an omitted ResetPort polarity from its explicit
canonical typed Reset/TRST source, including alias inversion. Uniform typed bits
dictate one declared-port polarity; contradictory typed bits reject. Untyped/mux/
literal/hole bits do not independently dictate polarity. Without a typed source
the default stays high. ToResetPort retains its independent high default; direct
reset mismatches reject. Implied connections keep automatic inversion. Register
reset references use the resolved polarity; flat flags are validated, including
fixed-low internal TRST semantics. Native hierarchy, aliases/cycles, vectors,
implicit conversion and altered metadata plus main56 reset/write/read source/
flat/generated replay and three-artifact rejection checks cover this boundary.

Connected native7/main56/clock41/client18/reset12/multiple-TAP22/feedback10,
format/naming and independent review pass. The review-driven TRST metadata fix
has RED/GREEN evidence. All48 interleaved loading runs preserve pattern/flat hashes
against525f1b6, including65536 reset bits,4096 aliases,4096 child scopes and655360
ordinary references.82 vendor inputs/runners match:37load/31ICL64 polarity failures/
14 existing grammar failures; vendor high default differs from primary inheritance.
The manually fetched archive/individual exits are authoritative; the original
transport124 and wrapper marker parsing failure remain recorded. No PDL proof.

Final native7 and main56/clock41/client18/reset12 leak-enabled ASan/UBSan pass.
Full serial make-j1check passes in build/reset-polarity-final-full.log/.exit0,
1339.731s:41 dynamic and41 static generator suites,51 directional goldens,2523 ICL
334/119 rules, LSP62 and editor6 in181.382s. Optional ANTLR5 skipped. All3 final
production/test hashes match the reviewed snapshot. Additional current-source
Rocky8 build/native/main56 and fetched archive pass with matched hashes and
transport0; full platform/static/install qualification remains separate. Mux
polarity/forcing/operation, remaining runtime/TAP/lifecycle, strict coverage and
full vendor acceptance remain open.

## Possible Reset/TRST mux path polarity (2026-11-15)

Consumed ResetPort/ToResetPort mux bits require a possible typed Reset/TRST path
with the consumer's resolved polarity; TRSTPort/ToTRSTPort require a separate
possible typed TRST-low path. A Reset forcing input cannot satisfy a TRST path.
Aliases and mux-edge/output inversions compose; branches union possible polarities.
The existing two-control forcing interpretation is retained, so not every typed
ancestor must match. This is structural possibility, not associated-reset ownership,
selector reachability, normal-operation sensitization or TRST assertion proof.
Mux ancestry does not dictate omitted ResetPort polarity; another direct typed
bit in a grouped declaration may still dictate the group under the prior rule.

The existing bounded iterative origins array is widened to uint16_t, with separate
polarity pairs and unknown512. It adds at most16MiB at the16M-bit graph bound,
without another array or graph traversal. Family-only CE/UE/shape checks retain
their masks; one deferred polarity message preserves structural diagnostics.
Native parity tables, nested cancellation, shared/mixed consumers, typed forcing,
parent/child TRST producers and implicit virtual TRST plus main58 artifact checks
cover this boundary. Connected native7/main58/clock41/client18/reset12/multiple-
TAP22/feedback10 pass. Final child native/style and independent combined review pass. All72 valid
interleaved loading comparisons preserve output/flat hashes versus5714c7c; full
serial integration and required leak checks now pass.

All88 vendor input/runner hashes match across40 Reset,40 TRST and8 legal child-
ToTRST loading probes:36load/36ICL64 polarity failures/8same-module ToTRST source-
context ICL65/130 failures/8mixed-special-input ICL115 failures. Source-context and
special-input differences remain distinct from polarity; no PDL equivalence or
full control operational conformance is inferred.

Joint262144 Reset+262144 TRST loading takes0.617–0.667s, max171424KiB versus
baseline171704KiB; no memory-saving claim is made from allocator variation. Wide
262144 Reset source0.616–0.717s versus0.567–0.617s;4096 shared source0.215–0.265s;
deep/ordinary runs and exact commands/hashes/time/RSS are retained in
build/reset-mux-path-polarity-loading-scale/comparison-valid.json. Empty PDL/debug
I/O measures whole loading, not isolated guard cost or a speedup. Both invalid
joint-fixture pilots remain excluded and recorded; the corrected12 joint runs
plus60 earlier valid runs form the72-row comparison. No product code changed.

Final combined native7/main58/clock41/client18/reset12 leak-enabled ASan/UBSan
passes, aggregate build/reset-mux-path-polarity-final-sanitizer-all.exit0. Full
serial make-j1check passes in build/reset-mux-path-polarity-final-full.log/.exit0,
1341.862s:41 dynamic and41 static generator suites,51 directional goldens,2523 ICL
334/119 rules, LSP62 and editor6 in175.000s. Optional ANTLR5 skipped. All3 final
production/test hashes match the reviewed snapshot. Earlier remote Reset-only
native7/dynamic41 results have matched source hashes and a completed fetched
archive; they do not qualify the later combined TRST source. Full vendor, strict
coverage, source direction, forcing/suppression/selector operation and remaining
runtime/TAP/lifecycle/platform requirements remain mandatory.

## Possible capture/update/TMS identity paths (2026-11-15)

Consumed CaptureEn/ToCaptureEn, UpdateEn/ToUpdateEn and TMS/ToTMS mux bits with
matching typed ancestry require a possible non-inverted typed path. This derives
a necessary transparent-normal-path condition from6.4.6.4/.5/.9/.10/.15/.16;
it is not an explicit Reset-like polarity clause. Data expressions allow inversion,
so net alias/edge/output cancellation remains valid. Passing identity does not
prove zero suppression, associated ownership, selector reachability or normal/
TRST operation. All-data enable consumers, unused muxes and ordinary terminal
boundaries retain their prior scope. The canonical inverse-arm shape fixtures
retain all rows and alias orders, with output cancellation preserving the identity
and original/flat checks; suppression is explicitly unproved.

Three more polarity pairs fit the existing uint16_t storage; unknown32768 remains
disjoint from valid flags. CE/UE-only networks reach the shared consumer check,
and only the old shape families seed Reset/TMS/TRST work. No new array, traversal
or public metadata is added. Parent/child producer and input/To* parity matrices,
nested cancellation, vector/shared negatives and main59 artifact checks cover
this boundary. Remote native/main59 and independent code/test review pass;
local connected/scale/style also pass; required full/leak integration now passes.

All72 matched vendor loading models pass, including odd paths. This differs from
the derived identity condition and is not a PDL/suppression/operation certificate.
Remote72 interleaved source/flat comparisons preserve output/flat hashes against
the frozen combined72/73 binary; wide CE-only262144 bits take0.569–0.620s with
103732KiB peak in both variants. Other wide/shared/deep/ordinary/reset baselines
and exact commands/source/binary/input hashes/time/RSS are retained under
build/control-mux-identity-scale-transfer/evidence/build/control-mux-identity-
loading-scale. All72 final local comparisons also pass with matching source/flat hashes.

Local72 valid comparisons also preserve bd99b48 output/flat hashes: three65536-bit
families0.265–0.266s,82564KiB versus82668KiB;CE-only262144 bits0.315–0.366s,
103380KiB versus103420KiB;4096 shared gates0.215–0.266s. Deep/ordinary/reset cases,
commands/environment/compiler/hashes/time/RSS are retained in
build/control-mux-identity-loading-scale/comparison.json. These are whole empty-PDL
CLI/debug runs, not isolated guard time or speedup claims. Local connected native7/
main59/clock41/client18/reset12/multiple-TAP22/feedback10 and style/naming pass.

Final native7/main59/clock41/client18/reset12 leak-enabled ASan/UBSan passes,
aggregate build/control-mux-identity-final-sanitizer-all.exit0. Full serial
make-j1check passes in build/control-mux-identity-final-full.log/.exit0,1345.807s:
41 dynamic and41 static generator suites,51 directional goldens,2523 ICL334/119
rules, LSP62 and editor6 in174.686s. Optional ANTLR5 skipped. All3 production/test
hashes match the reviewed snapshot. Current-source remote native7/dynamic41 and
fetched archive also pass with matched hashes; full platform/static/install
qualification remains separate. Strict coverage, source direction, full gating/
selector operation, remaining runtime/TAP/lifecycle and vendor certification
remain mandatory.


### Control-output reference scope (2026-11-15)

Covered Reset/TCK/CaptureEn/ShiftEn/UpdateEn/TMS/TRST hardware Sources and
conditional control DataMux choices reject a canonical To* terminal declared in
the reference expression's own scope (6.3.12). Input bindings use their parent
scope; output Sources and mux choices use their own scope. Resolution stops at
ports and runs after flat-origin restoration, so aliases and serialized bindings
cannot bypass the boundary. This reuses the terminal cache without another array,
pass or public metadata. PDL-only aliases, semantic reset associations, ordinary
data/clock direction, ToIRSelect and the existing lowered ToTCK Source extension
retain their prior scope. Broader direction and ancestor restrictions remain open.

Native family/direct/alias/consumer/mux matrices, mixed vectors in both orders,
partial and original/flat positives, altered-flat rejection and main60 atomic
pattern/reset/debug checks cover this boundary. Older positives now expose their
To* producers through child instances. ClockForward fixtures retain the existing
lowered extension and all clock-phase/unknown-gate/artifact assertions. Local
connected native7/main60/clock41/client18/reset12/multiple-TAP22/feedback10 pass;
the first native scheduler failure was another own-output fixture, repaired and
rerun0 in115.012s. Independent code/test review and additional scheduler review
report no findings. Remote GREEN2 native7/main60/client18/multiple-TAP22 pass on
the earlier test snapshot. Final current-source remote native7 and official
dynamic41 pass; all6 source/test hashes match the submitted reviewed snapshot,
and the completed evidence archive was fetched successfully.

All99 Tessent2026.2 models/runner hashes match:46 child-output hardware cases and
7 alias-only cases load;46 own-output hardware references fail ICL130. This is
loading evidence, not PDL equivalence. The first remote RED accepted invalid own
outputs, followed by the guard's rejection. Vendor evidence remains under
build/control-output-scope-vendor-probe/comparison.json.

All72 serial interleaved source/flat loading comparisons against86e3da1 preserve
output/flat hashes. Seven8192-bit families take0.08–0.13s (baseline0.07–0.11s);
262144-bit child Reset mux0.51–0.57s (0.51–0.55s),128948 versus128996KiB peak.
4096 aliases0.05–0.12s,4096 consumers0.06–0.09s,CE-only262144 bits0.31–0.34s and
ordinary655360 references0.10–0.15s. Exact inputs, environment, compiler, commands,
hashes, wall/RSS are retained in build/control-output-scope-loading-scale-serial/
comparison.json. These are whole empty-PDL/debug CLI runs; no isolated guard cost
or speedup is claimed. The earlier overlapping measurement remains archived and
is excluded from timing claims. Final style/naming and required leak-enabled
ASan/UBSan native7/main60/clock41/client18/reset12 checks pass; aggregate
build/control-output-scope-final-sanitizer-all.exit0. Full serial integration
passes against6 source/test files matching the reviewed snapshot. Strict coverage, complete gating/selector rules,
remaining runtime/TAP/lifecycle requirements and full platform/vendor qualification
remain mandatory.


Final Task75 verification: required leak-enabled ASan/UBSan native7/main60/clock41/
client18/reset12 all0. Full serial make-j1check0 in1340.802s, recorded in
build/control-output-scope-final-full.log/.exit/.wall:41 dynamic and41 static
suites,51 directional goldens,2523 ICL files with334/119 grammar/scanner rules,
LSP62 and editor6 in175.279s. Optional ANTLR5 skipped. All6 final source/test
hashes match the independently reviewed snapshot. Current-source remote native7/
dynamic41 all0, with actual6 hashes matching and completed archive fetched0.
These results qualify this source-scope increment; the remaining model, strict
coverage, vendor and full platform/static/install requirements stay open.


### Ordinary-output reference scope (2026-11-15)

The connected source validator applies the local-input/instance-output boundary
of6.3.12(c–e) to eligible DataOut,ScanOut,ToSelect,ToClock and ToTCK terminals,
including ordinary data fallbacks of typed Sources. Canonical aliases stop at
ports, and original scopes are restored before comparison. Source bindings use
the input's parent scope; other hardware expressions use the consumer scope.
One shared accepted-terminal check retains wrong-kind, strict scan/rightmost,
inversion and direct-DataMux diagnostic precedence. Constants, holes, virtual
sources, PDL-only aliases, semantic reset associations and the two-input ScanMux
ToIRSelect exception remain unchanged. DifferentialInvOf direction remains under
its separate conversion interpretation; unchecked lowered ToTCK Source stays the
existing extension. No ancestor/depth constraint or public metadata is added.

Native direct/alias/consumer/choice matrices, mixed vectors in both orders,
partial bindings, altered-flat rejection, own alias-only and differential-pair
replay cover the boundary. Main61 preserves pattern/reset/debug atomic rejection.
Older positives use child producers; enabled-data forwarding retains the9-case
concrete/Z3 truth table, flat contributor names/slices and actual per-bit feedback.
The3000-group/65wide-source and shared8/8192/zero-cache budget assertions remain.
Independent implementation and each affected-fixture review report no findings.
Local first connected main61/clock41/client18/reset12/multi22/feedback10 pass;
all native suites pass after necessary fixture repairs. Product canonical-terminal
reuse avoids duplicate alias-map lookup on the measured hot path; its connected
rerun passes, while final integration is pending. It adds no array, cache or graph pass.

All86 vendor input/runner hashes match.13 initial scan fixtures had unrelated
ICL71 missing-interface failures and remain separate; qualified follow-up shows
6 child scan cases load (2 mux rows warnICL68),6 own cases failICL130, and a scan
alias-only model loads. Across valid ordinary/scan/OneHot scope probes,32 child
hardware cases load,32 own cases failICL130 and5 alias-only cases load. Own scan
Source rows also haveICL103. Specific6.4.15(d) restricts enabled contributor kinds,
without overriding6.3.12 hierarchy; own OneHot direct/alias probes both fail,
child equivalents both load, so no exemption is added. The same-module positive
differential output pair loads; inverted or hierarchical associations failICL59.
This is loading evidence, not PDL equivalence or full clock/ScanInterface proof.

First valid serial84 comparisons preserve output/flat hashes versus1c28b14.
Four8192-bit output families take0.05–0.09s; scalar scan0.02–0.04s;262144 Reset
mux0.53–0.60s;4096 aliases0.05–0.12s;4096 shared outputs0.06–0.10s;CE-only262144
bits0.30–0.33s. Ordinary655360 references0.16–0.18s versus0.12–0.15s baseline
motivated terminal reuse. A retained pre-reuse binary/source supports the final
interleaved comparison. Failed wide-scan/mixed-interface/scalar-index fixture
pilots are retained outside valid timing claims. Wide scan-return scheduling
remains an explicit requirement. Whole empty-PDL/debug CLI runs measure loading,
not isolated guard time or general optimality. Final comparison, style, leak and
full serial integration evidence are pending. Remaining source/alias rules,
complete gating/selector operation, runtime/TAP/lifecycle, strict coverage and
full vendor/platform/static/install qualification remain mandatory.


Final canonical-reuse connected native7/main61/clock41/client18/reset12/multi22/
feedback10 all0 (scheduler114.355s). All126 interleaved source/flat comparisons
among1c28b14, retained pre-reuse76 and final76 preserve output/flat hashes. Four
8192-bit families0.05–0.10s; scalar scan0.02–0.03s;262144 Reset mux0.52–0.55s;
4096 aliases0.06–0.12s;4096 consumers0.06–0.09s;CE-only2621440.31–0.33s.
Ordinary655360 references0.12–0.16s versus pre-reuse0.15–0.19s and baseline
0.12–0.15s; before/final peak66716KiB, baseline66832KiB (no savings claim).
Environment/compiler, exact commands/input/binary/source hashes, phase timing and
RSS are retained in build/ordinary-output-scope-loading-scale-terminal-reuse/
comparison.json. These are whole CLI source/flat runs, not isolated guard time.
The earlier84 valid runs and failed fixture pilots remain separate and retained.
Independent canonical-reuse review is clean. Required leak/full integration and
current-source remote qualification remain pending.


Final shared-output fixture qualification: prereuse remote native7 and40/41 CLI
suites pass with actual8 hashes matching; shared-output gating/vector fixture
used an own ScanOut Source. Reviewed child Forwarder repair retains Rootinner as
an enabled observation, gated/inverted a[3:2], b and all original/flat/generated
replay assertions. Normal9tests0 in1.418s; leak-enabled shared9tests0 in3.145s.
All required leak ASan/UBSan native7/main61/clock41/client18/reset12/shared9 pass;
aggregate build/ordinary-output-scope-final-sanitizer-complete.exit0. All7 changed
source/test files match independent review snapshots. Final current-source9-file
remote native7/all41 runs separately. Full serial integration is starting.


First full integration stopped2 after76.961s at a further old own-output sparse
OneHot benchmark fixture. Logs/exit/wall are retained as build/ordinary-output-
scope-first-full-fixture-failure.*. Reviewed Producer child repair retains the
1048576-bit group,10000 scalar contributors,5000-instance hierarchy/flat replay,
wide parent bus mapping and unchanged10s script deadline. Focused build/benchmark/
format all0; the complete optimized benchmark takes0.839s (sparse elaboration
30.093ms). Production and prior7 changed source/test hashes are unchanged; no
sanitizer rerun is needed for this independent benchmark-only fixture. Full serial
integration restarts after this fix; no full-suite success is claimed yet.


Final Task76 verification: full serial make-j1check0 in1318.863s, recorded in
build/ordinary-output-scope-final-full.log/.exit/.wall, after the separately
retained first benchmark-fixture failure.41 dynamic and41 static generator
suites,51 directional goldens,2523 ICL files with334/119 grammar/scanner rules,
LSP62 and editor6 in174.320s pass. Optional ANTLR5 skipped. Required leak-enabled
ASan/UBSan native7/main61/clock41/client18/reset12/shared9 pass; aggregate
build/ordinary-output-scope-final-sanitizer-complete.exit0. All8 changed source/
test hashes match their independent review snapshots. Current generation code
and native/CLI fixtures pass remote native7/all41, with actual9 submitted hashes
matching and completed archive fetched0; the extra benchmark fixture is locally
qualified0 under its unchanged10s deadline. No full platform/install or vendor
PDL equivalence claim is made. Broader restored-scope, clock association, gating/
selector, runtime/TAP/lifecycle, strict coverage and final vendor/platform/static/
install requirements remain mandatory.


### Restored hardware source scope and generated TAP outputs (2026-11-15)

Accepted hardware references are checked after restoring original scopes: local
non-output terminals or outputs of a direct child instance. Input Source bindings
use the parent expression scope; other checked roles use the consumer scope.
Cross-scope raw Alias references also fail, even if the canonical terminal is a
legal child output. Alias declarations remain subject to their existing separate
rules. Kind, inversion, rightmost-register-bit and direct-DataMux diagnostics keep
precedence. Constants, virtuals and holes retain handling; semantic reset fields,
interface membership, DifferentialInvOf, ToIRSelect and the lowered ToTCK Source
extension keep their existing contracts. This does not complete their remaining
source, alias or operational rules.

Two current76 counterexamples are recorded: altered flat Source to child input
or grandchild output loaded0 despite the declared scope. Original deep hardware
syntax was already rejected by the parser; this closes reconstructed-model scope,
not a new parser gap. Native matrices cover10 families, raw nonlocal aliases,
parent bindings, deep/internal sources, partial and both mixed vector lanes;
main62 preserves atomic pattern/reset/debug rejection. The unsupported literal-
Alias RED fixture was removed before implementation; Alias grammar excludes it.
Corrected RED native/CLI/TAP failures demonstrate missing scope and old TAP wiring.

Generated TAP sources now use root TDI for initial routes, a shared vector DataOut
for IR selectors, typed phase outputs and ToSelect for selection logic. Existing
Connect calls retain multi-TAP ownership checks while Add records exact edges.
Finalize checks the recorded old source and appends one cached output per source
after all original nodes. Raw runtime IDs/names, bit shape and inversion remain;
new bits/objects spend the existing budgets. No metadata provenance/name exemption,
new version or blanket graph pass is introduced. Single/multi-TAP assertions and
original/flat execution pass, including wrapper append order and IR reuse.

Local connected native7/main62/clock41/client18/reset12/multi22/feedback10 pass;
independent code/test review finds no consequential issue. The benchmark preflight
passes0 in0.897s under its unchanged10s deadline. Style/naming passes after two
brace-only fixes. All84 paired source/flat loading comparisons againstd891771
preserve output/flat hashes: four8192-bit families0.04–0.08s; scalar scan0.02–
0.04s;262144 Reset mux0.52–0.57s;4096 aliases0.06–0.12s;4096 shared outputs
0.06–0.08s;CE-only2621440.31–0.33s;ordinary655360 references0.13–0.16s versus
0.13–0.17s baseline. Exact commands/environment/compiler/hashes/time/RSS are in
build/restored-source-scope-loading-scale/comparison.json. These measure whole
CLI source/flat loading, not isolated guard cost or a memory-saving claim.

Three interleaved native TAP comparisons retain all tests:3000-interface network
construction0.164–0.183s versus0.144–0.158s baseline; whole TAP test processes
1.12–1.17s versus1.12–1.15s, peak122620 versus122720KiB (allocator variation).
Extra boundary ports have a measured construction cost; no speedup is claimed.
Evidence is build/restored-source-scope-tap-scale/comparison.json. New generated
TAP flat files replay correctly. Older native TAP dumps with direct private
references must be regenerated from original ICL/BSDL: baseline76 emits the
sample0 and new code rejects its private core.select source2, recorded under
build/restored-source-scope-legacy-flat. No legacy migration is added.

Required leak/full serial integration and broader remote qualification are pending.
The remote behavior snapshot precedes the two brace-only style fixes. Complete
source-special-context/alias rules, gating/selector behavior, runtime/TAP/lifecycle,
strict coverage and final vendor/platform/static/install qualification remain
mandatory. This is not full IEEE or vendor PDL equivalence certification.


Final leak-enabled ASan/UBSan native7/main62/clock41/client18/reset12 all0,
aggregate build/restored-source-scope-final-sanitizer-all.exit0. All5 changed
source/test hashes match the reviewed integration snapshot. Broad remote native7/
all41 pass with actual5 hashes matching the submitted pre-brace behavior snapshot;
final brace-formatted source-matched flatten/TAP/main62 also all0 with actual5
hashes matching. Both completed evidence archives fetched0. Full serial integration starts
now; no full-suite pass is claimed until its aggregate completes.


Final Task77 full serial make-j1check0 in1338.899s, recorded in
build/restored-source-scope-final-full.log/.exit/.wall:41 dynamic and41 static
generator suites,51 directional goldens,2523 ICL files/334grammar119scanner rules,
LSP62 and editor6 in174.509s pass. Optional ANTLR5 skipped. Required leak-enabled
ASan/UBSan native7/main62/clock41/client18/reset12 all0. All5 changed source/test
hashes match review. Broad pre-brace remote native7/all41 and final source-matched
flatten/TAP/main62 pass with actual5 hashes matching each submitted snapshot;
completed archives fetched0. Older TAP dumps with direct private references need
regeneration from original ICL/BSDL. Remaining special port-source/association,
alias, gating/selector, runtime/TAP/lifecycle, strict coverage and complete vendor/
platform/static/install requirements stay mandatory; this is not full conformance.


### OneHotScan contributor scope (2026-11-15)

OneHotScanGroup Port contributors now use the shared scan/data source-kind and
restored hardware-scope checks. Local inputs, registers, muxes and groups remain
valid; output ports must belong to a direct child. Direct, alias-hidden and
inverted own outputs are rejected. Deep restored outputs and hierarchical aliases
cannot bypass that boundary. Unused aliases for PDL remain valid. Enabled fan-in,
width and cycle validation retain their earlier diagnostics; middle-register
concat-scan behavior is unchanged.

Concrete and symbolic one-hot enable resolution now follows bound ScanInPort
Sources across module boundaries. The original enabled outputs still determine
selection; missing bindings remain unsupported and existing traversal budgets
apply. This repairs a runtime gap exposed by making a nested scan fixture legal.
The fixture retains all27 enable/unknown combinations and independent held/shift
storage checks. No DataInPort traversal or other source-family expansion is added.

Tessent2026.2 loads all8 legal child cases and rejects all8 own-output cases with
ICL130; consumed own cases also report ICL103. The unused cases isolate scope.
Actual16 model hashes are in build/special-source-context-vendor-probe; vendor
loading does not establish pattern equivalence. ToIRSelect owner controls,
differential association and the remaining event/reset work are separate tasks.


Verified checkpoint: full serial make check exits0 in1345.595s, including41
dynamic and41 static generator suites,51 directional goldens,2523-file ICL
corpus (334/334 grammar and119/119 scanner rules),LSP62 and editor6 in175.874s.
Five optional ANTLR tests are skipped. Required leak-enabled native7/main63/
clock41/client18/reset12 all pass. Final remote native7/all41 CLI suites pass
with all5 changed source/test hashes matched. Exact evidence is under
build/onehot-scan-source-scope-final-{full,sanitizer-*} and
build/onehot-scan-source-scope-final-remote/evidence. Loading measurements cover
72 source/flat comparisons with equal output hashes; no speedup is claimed.


### Controller owners and differential clock inputs (2026-11-15)

A module declaring ToIRSelectPort must declare exactly one TMSPort and at most
one TRSTPort, as required by IEEE1687 6.4.6.23(a,b). Counts are declarations,
including indexed/vector ports; separate interface, implicit-binding and runtime
ambiguity checks still apply. Parent, child and sibling declarations do not
satisfy the owner. Validation uses restored scopes and the existing port index.
Unused selectors are subject to the same rule. Generated BSDL selectors retain
their separate data-port model.

ClockPort DifferentialInvOf associations now require a positive ClockPort in
the same module, following6.4.6.19(b). Aliases, slices, concatenations and inversion
syntax retain canonical per-bit validation. Multiple clock pairs and unrelated
clock declarations may coexist. The negative input's parent Source binding is
independent: constants, inversion and partially unspecified bindings keep their
existing behavior. Restored parent/sibling associations cannot bypass ownership.
ToClockPort conversion associations retain their separate6.4.6.20 behavior.

Tessent2026.2 owner probes reject missing TMS but accept extra TMS/TRST declarations;
this tool follows the stricter explicit declaration-count clauses. Differential
probes accept direct same-kind pairs but reject tested aliases, whereas the
standard clock-signal grammar permits aliases. These are documented dialect
boundaries, not vendor pattern-equivalence claims. Inputs/hashes/logs are under
build/special-source-context-vendor-probe. General output associations, lowered
ToTCK handling, control forcing and broader clock/reset events remain separate.


Verified checkpoint: required leak-enabled native7/main65/clock41/client18/reset12
all pass. Full serial make check exits0 in1335.271s:41 dynamic and41 static
generator suites,51 directional goldens,2523 ICL files covering334/334 grammar
and119/119 scanner rules,LSP62 and editor6 in174.677s; five optional ANTLR tests
are skipped. Broad remote native7/all41 CLI suites pass with4 source/test hashes
matched. Loading comparisons60 all pass with equal PDL/flat hashes. Exact evidence:
build/source-context-associations-{connected,loading-scale},
build/source-context-associations-final-{full,sanitizer-*}, and
build/source-context-associations-connected-remote/evidence. Remaining output
association, lowered ToTCK, forcing and event requirements are not waived.


### Lowered test-clock sources and IR selector boundaries (2026-11-15)

Explicit Source properties on the project’s lowered ToTCKPort extension now use
the existing clock/data terminal validator. ClockPort, ClockMux, TCK, ordinary
data, constants, aliases and inversions retain their behavior. Unsupported typed
control/scan sources are rejected; output references require direct-child ports.
Restored child inputs, deep outputs and nonlocal aliases cannot bypass scope.
IEEE1687 6.4.6.18 defines no Source property; this extension is not represented
as vendor-standard syntax. Source-less ToTCK owner/equivalence remains separate.

ToIRSelectPort retains its special local ScanMux-selector permission. Nonlocal
producers must be direct-child outputs, and raw aliases must belong to the
expression scope. Restored parent/sibling/grandchild references and child aliases
are rejected. The existing one-bit/two-input selector rule and PDL-only alias
behavior remain intact. This checks wiring boundaries; controller association
and active-chain behavior under6.4.6.23(c) require separate qualification.


Verified checkpoint: required leak-enabled native7/main67/clock41/client18/reset12
all pass. Full serial make check exits0 in1338.568s:41 dynamic and41 static
generator suites,51 directional goldens,2523 ICL files (334/334 grammar and
119/119 scanner rules),LSP62 and editor6 in174.998s; five optional ANTLR tests
are skipped. Broad remote native7/all41 suites pass with4 snapshot source/test
hashes checked, including the unchanged clock fixture. Loading60 comparisons
pass with equal PDL/flat hashes. Evidence: build/lowered-source-boundaries-
{connected,loading-scale,final-full,final-sanitizer-*} and the final connected
remote archive. Source-less test-clock inference remains the next task.


### Source-less test-clock forwarding (2026-11-15)

A source-less ToTCKPort now requires a TCKPort in its owning module and gains
real same-polarity Source connections. Scalar and indexed-scalar inputs supply
all output bits. Multiple scalar definitions are equivalent under IEEE1687
6.4.6.17(a); a local scalar is used as a representative, including mixed scalar/
vector owners. Vector-only input declarations receive an unsupported implicit
mapping diagnostic; explicit lowered Sources retain their existing mappings.
Parent, child and sibling declarations cannot replace the local owner requirement.
New and legacy source-less flat dumps reconstruct connections from restored port
types/scopes. Inferred edges receive the existing cycle and source checks.

One shared4M allowance bounds newly inferred Reset/TRST/TMS/TCK references before
allocation. Existing explicit bindings remain under their existing limits.
Concrete and symbolic parent/child tests cover0/1/X transitions and legacy replay.
Symbolic repeated references now use compact bitvector repetition while retaining
slice ordering, masks, inversion and held/shift views. The matched20s benchmark
previously timed out during65536 bits; all six cases now complete, with the two
65536-bit cases at0.780/1.137s under unchanged3s proof limits. This is the measured
symbolic case, not a global generation-speed claim.

Tessent2026.2 probes34 model/runner pairs: all load, but only unique scalar/
indexed inputs expose fan-ins to every output bit. Missing owners also load;
this tool follows the stricter explicit IEEE6.4.6.18(a) rule. Multiple-scalar/
mixed representative inference is standards-derived; no ordinal vector mapping
or vendor waveform equivalence is claimed. Query data and exact input hashes are
in build/totck-association-vendor-probe. Flat export records inferred connections
using the project's existing explicit-Source extension. Root TAP scalar limits
and broader clock/reset scheduling requirements remain separate.


Verified checkpoint: required leak-enabled build/native7/main68/clock41/client18/reset12 all pass. Full serial make check exits0 in1346.653s:41 dynamic and41 static generator suites,51 directional goldens,2523 ICL files with334/334 grammar and119/119 scanner rules,LSP62 and editor6 in174.913s; five optional ANTLR tests remain skipped. Final corrected remote build/native7/all41CLI/aggregate all pass, with9 source/test hashes matched. Reviewed source/test8 remain unchanged. Loading60 comparisons and the bounded repeated-source benchmark are recorded above. Evidence: build/implied-totck-final-{full,sanitizer-*}, build/implied-totck-final-remote/evidence, build/implied-totck-loading-scale and build/implied-totck-repeat-{red,green}-remote. Remaining clock/reset/forcing/scheduler/vendor/coverage requirements are not waived.


### Symbolic literal and sparse one-hot runtime (2026-11-15)

Literal/reset runs now build one symbolic value per contiguous run. Physical
slices and repeated references retain their existing behavior. Single-bit reads
and run reads share the interpretation of 0/1/X, unspecified bits, inversion,
and synchronous/asynchronous Global Reset.

One-hot evaluation uses actual contributor payloads and adds unknown leading
lanes at the result boundary. Symbolic evaluation accumulates narrower payloads
first; concrete evaluation still validates every active payload. No selection,
multiple selections, and unknown enables retain their unknown-result behavior.
Tests include all 27 enable combinations for mixed 1/3/7-bit contributors,
inverted sources, opposite vector directions, unsorted runtime descriptors,
empty-contributor rejection, and existing held/shift OneHotScan behavior.

The matched sparse benchmark previously timed out at 65,536 declared bits and
512 scalar contributors under a 20-second command bound. The changed benchmark
completed in 0.05 seconds at 42,112 KiB peak RSS; that symbolic case took
0.00851 seconds. Its proof limit remains three seconds. These figures describe
the recorded isolated workload, not arbitrary solver problems.

Literal construction also avoids Z3 4.13.3's cache of every power of two through
the requested exponent. It packs at most 64 bits at a time and folds each word
to a numeral before adding the next. Word-window tests independently verify
order and known masks at partial-word and odd-word boundaries. Growing numeral
copies remain a cost; arithmetic work is not claimed to be linear.

The original eight-case literal profile needed about 4.3 GiB after run batching.
A 512 MiB virtual-memory check failed at 262,144 bits. Word construction passes
the same bound: eight cases finish in 0.71 seconds at 49,436 KiB. An extended
ten-case profile through 1,048,576 bits finishes in 8.67 seconds at 64,072 KiB;
the two largest cases take 3.95 seconds each. Both retain the 20-second command
and three-second proof limits. This qualifies the measured inputs; the 4M-bit
allocation limit does not establish a latency guarantee at that maximum.

Independent reviews of all three changes are clean. Final connected, immutable
sanitizer/full and source-matched remote integration results are recorded next.
Existing clock/reset/event, resource, vendor and strict coverage requirements
remain open.


Final source-matched remote integration:60 statuses; build/native7/all41CLI/aggregate and all3 changed literal/onehot benchmark repetitions pass. Only3 intentionally failing bounded-memory baseline repetitions return1 (outofmemory). Actual10 final and10 baseline source/test hashes match manifests. Identical final benchmark program uses512MiB VM/20s command/3s proof; baseline fails at262144 in1.12s/457980–458192KiB, changed ten-case profile through1M completes13.46–13.50s/65008–65148KiB on the remote host; largest pair6.080–6.104s. Onehot three runs .09s/42128KiB,65536/512 symbolic .02847–.02861s. Host/compiler/binary hashes and commands are retained; these remote figures are distinct from local measurements. All required local leak-enabled build/native7/main68/clock41/client18/reset12 pass; full serial check is still running with source/test3 unchanged. Evidence: build/symbolic-runtime-final-remote/evidence, build/symbolic-runtime-final-sanitizer-* and build/symbolic-runtime-final-full.*.


Verified Tasks85–87 checkpoint: required leak-enabled build/native7/main68/clock41/client18/reset12 all0. Full serial make check exits0 in1327.585s:41 dynamic and41 static generator suites,51 directional goldens,2523 ICL files with334/334 grammar and119/119 scanner rules,LSP62 and editor6 in177.462s. Five optional ANTLR tests remain skipped. Final remote native7/all41 and changed benchmark repetitions pass, with10 final and10 baseline source/test hashes verified; the3 deliberate memory-baseline failures are preserved. Reviewed source/test3 remain unchanged throughout. Commit this eight-file snapshot without push. ScopeTask88 and capture/updateTask89 remain isolated; broader clock/reset/events, resources, vendor, strict coverage and final platform requirements remain mandatory.


### Differential output association ownership (2026-11-15)

ToClockPort DifferentialInvOf now checks raw-alias ownership and terminal scope
after flat origins are restored. Local input/data/clock terminals and direct-child
outputs retain their existing clock/data eligibility. Restored child inputs,
parent/sibling terminals, deep outputs, nonlocal aliases, and unrelated local
output kinds cannot bypass those boundaries.

The existing same-owner positive ToClockPort partner remains valid, including
the supported vector, slice, alias and inversion forms. This preserves the
conversion context and the direct local-pair Tessent loading evidence; it does
not apply ClockPort input-pair restrictions to output conversion. Existing
vendor alias/cross-kind differences remain documented. Differential clock
execution remains outside the qualified SystemClocks routes.

Native restored-scope and original/flat positive cases pass. All 69 main CLI
tests and 41 clock tests pass, including atomic preservation of pattern, local
reset and debug outputs. Independent review is clean. Original multilevel
spelling retains the parser's existing Alias-only diagnostic; flattened deep
references now receive the missing ownership check.

All 60 remote source/flat measurements pass with equal PDL and canonical flat
hashes. Baseline versus changed tool times: 65,536-bit local pair 0.04 seconds in
both; 4,096 aliases and 4,096 consumers 0.19–0.23 seconds in both; 3,000 owners
0.22–0.37 versus 0.22–0.36; 3,000 direct-child pairs 0.24–0.35 versus 0.25–0.34;
720,896 ordinary data bits 0.11–0.12 versus 0.12 seconds. Input, binary/source
hashes, environment, commands and memory measurements are retained in
build/clock-output-scope-loading-remote/evidence. These are loading comparisons,
not a generation-speed claim. Broad connected and final immutable integration
evidence follows after the current root checkpoint is verified.


### Capture selector observation readiness (2026-11-15)

A direct ScanRegister read or routed data observation with a declared
CaptureSource now requires the effective capture event. The existing event model
includes clock edges, selection and enable gating. Pending reads can wait while
a prerequisite scan writes their selector; presence on a scan path alone does
not complete the observation.

Write-only capture suppression and held reads without CaptureSource remain
supported. Dedicated instruction-register handling and direct primary
observations retain their existing paths. Expected read data remains a
comparison: requesting 0101 against a modeled constant1010 still produces a
pattern, with capture enabled.

The reproduced failure previously emitted cap_enable0 while comparing the
uncaptured source. Corrected tests establish ownership before assigning known
reserved values. Direct and routed capture cases, a scan-driven prerequisite,
update gating, suppression and atomic artifact failures now pass. All24 client
methods and the connected native7/main69/clock41/reset12/multi22/feedback10 checks
pass; independent review and format/naming checks are clean.

Tessent2026.2 forward certificates pass unchanged ordered-apply comparison for
capture_mux_selector and update_mux_selector. All19 actual staged file hashes
match. Reverse capture exports but its round-trip groups differ; reverse update
fails the vendor's default suppression-assignment policy. Those attempts are
retained and neither reverse direction is certified. The two forward standards
bring the offline directional total to53; all53 pass with the current candidate.

Gated reads of16 four-bit registers take1.36–1.46 seconds from original/flat
inputs and0.60 seconds for generated replay, with115368KiB peak RSS. At32
registers, original/flat take13.47–13.72 seconds and generated replay5.86 seconds,
with512236KiB peak RSS. These are supported measured cases near the bounded
event planner's practical limit, not a broad scalability claim. Larger event
histories and remaining optimization requirements stay open.

Evidence: build/control-mux-operation-{baseline,baseline2,baseline3,green}-remote;
build/control-mux-prerequisite-red-remote; WT/build/control-mux-final-client.*;
build/control-mux-operation-vendor/evidence; the two maintained forward
certificates; WT/build/control-mux-capture-scale/comparison.json. Source and
binary hashes, commands, environment, wall time and RSS are retained.

Final combined88/89 remote integration passes50 statuses: build/native7/all41CLI/aggregate all0, with actual12source/test hashes matching the submitted immutable snapshot. Required local leak-enabled build/native7/main69/clock41/client24/reset12 all0. Full serial regression remains running; no production/test/case/certificate changes during these final gates. Evidence: build/clock-output-control-final-remote/evidence and build/clock-output-control-final-{sanitizer-*,full.*}.

Verified Tasks88/89 checkpoint: full serial make check exits0 in1381.145s,
41 dynamic and41 static generator suites,53 directional goldens,2525 ICL files
with334/334 grammar and119/119 scanner rules,LSP62 and editor6 in174.760s.
Five optional ANTLR tests remain skipped. Required leak-enabled build/native7/
main69/clock41/client24/reset12 all0. Final remote50 statuses all0 and actual12
source/test hashes match. All26 production/test/case/certificate files remain
unchanged from independent review through final gates. Commit this31-file
checkpoint without push; Task90 remains isolated. Broader model, vendor, strict
coverage and final platform requirements remain mandatory.


### Stable-control read scheduling (2026-11-15)

Pure reads on standalone clients can use the existing regular scan model when
all control, enable and route dependencies remain stable within each phase.
Storage-derived and serial-input-derived controls, TAPs and physical reset paths
retain the full event model. Direct deasserted virtual resets can qualify. A
bounded isolated attempt publishes its complete result only after successful
concrete replay; failed attempts fall back with live scheduling state intact.
Mixed writes retain their existing path, including generated PDL with explicit
primary-input preparation. No public option, dependency or solver-limit change.

The maintained32-register capture-gated read test checks automatic enable
selection, each comparison and identical original/flat/generated PDL. Original
and flat pure reads have a five-second command budget; generated mixed-write
replay retains the normal correctness timeout. ReservedCE0 held reads without
CaptureSource also exercise a failed regular attempt followed by successful
full-event scheduling. Reset-bearing and no-reset variants remain covered.

On the same remote host, all26 interleaved commands preserve identical PDL and
canonical flat hashes for each input size.16 four-bit registers take1.58–1.62s
with the previous solver and0.14s with the candidate.32 take15.43–15.59s and
0.26s respectively; peak RSS falls from512256KiB to80496KiB.64 candidate
registers take0.60–0.61s,98328KiB peak; no64-register baseline was measured.
Three candidate source/flat repetitions and three baseline source repetitions
are recorded; baseline flat is checked once per size. These results qualify
stable-control pure reads, not arbitrary event histories or global optimality.

The exact initial32-register baseline times out at5.02s; final focused25client
methods pass with actual source/test hashes verified. Independent design and
source reviews are clean. Earlier guard-exclusion and generated-timeout pilots
remain recorded. Evidence: build/regular-event-read-{red,test-red,test-red2,
green,green2,green3,comparison}-remote/evidence. The comparison report records
commands, environment/compiler, input/source/binary hashes, wall time and RSS.
Connected, style, offline and final immutable integration checks follow.

Local connected native7/main69/clock41/client25/reset12/multipleTAP22/feedback10 all0; format/naming0 and53offline directional certificates0. Independent final reviewclean. Integrate thisfivefile snapshot onto41a2fa0 and freeze production/tests for required leak-enabled sanitizer/full gates. Final12hashremote verification remains in progress.

Final Task90 remote50 statuses all0 with actual12 source/test hashes matching frozen submission. Required leak-enabled build/native7/main69/clock41/client25/reset12 all0. Root source/test2 unchanged; full serial check now running in build/regular-event-read-final-full.*.

Verified Task90 checkpoint: required leak-enabled build/native7/main69/clock41/
client25/reset12 all0. Full serial make check exits0 in1345.397s,41 dynamic and
41 static generator suites,53 directional goldens,2525ICL334/119rules,LSP62 and
editor6 in181.414s. Five optional ANTLR tests remain skipped. Remote50 statuses
all0 with actual12 source/test hashes matching. Source/test2 stayed immutable
through final gates. Connected, style/naming,53offline certificates, independent
review and26 matched performance commands pass. Branch cleanup removed44 already
integrated task references and preserved116 draft files; all old draft paths and
plans exist in master. The developer guide now requires completing each task by
integration and deleting its merged branch. Commit thissixfile snapshot on the
Task90 branch, fast-forward master and delete that branch; restore only Task91
future files onto its own active branch. No push. Broader event/clock/resource,
vendor, strict coverage and final platform requirements remain mandatory.


### Differential clock duration routes (2026-11-15)

Relative iClock/-sck waits can trace a declared negative ClockPort at the root
or a negative ToClockPort to its positive partner. Direct hierarchical targets,
indexed vectors, local output aliases, clock muxes, ratios and oscillator roots
retain original/flat/generated/reset-only replay. This traces cycle counts on
clock-only routes; it does not simulate phases or add hardware clock events.

Generic ICL Source references to negative clock terminals retain the existing
6.3.12(e) restriction. Explicit inverted expression edges remain unsupported.
A negative input with both an external binding and a local differential partner
still needs a consistency decision and receives a specific bound-input error;
its unused association does not by itself make a positive clock-only root unsafe.
Differential override targets retain their existing diagnostic.

Association edges participate in the shared route, unsafe-fanout, reservation
and cycle walks. Holding either half forbids a nonzero wait on either half;
zero waits remain valid. An introduced override cycle through a negative partner
fails before publishing its version, including after an earlier checked clock.
Native source/flat tests also verify unchanged hardware state and propagation
from a lowered non-clock consumer through the differential association.

All47 clock CLI methods, connected native7/main69/client25/reset12/multipleTAP22/
feedback10, formatting/naming and53 offline directional standards pass. Independent
design and final source reviews are clean.24 interleaved scale commands preserve
identical PDL and canonical flat hashes per case: ordinary4096-stage source/flat
baseline0.27–0.38s and candidate0.27–0.38s; new4096-stage differential paths
0.34–0.53s,123196KiB peak;65536-bit paired inputs0.10s,72720KiB peak. Each input
has1024 checked waits and three source/flat repetitions. New paths have no
supported old functional baseline; no speedup claim is made for them.

Tessent2026.2 rejects negative ports as direct iClock targets, a recorded difference
from this valid-port interpretation of7.9.15. The normalized positive-clock
sequence exports successfully after declaring both physical inputs as clocks
and their inverse equivalence. All3 ordered wait commands match; Tessent drops
one redundant repeated iClock root. This is a direct probe, not a strict
certificate. Actual5 submitted/6 staged pilot hashes and3 qualified hashes match;
source rejection and both diagnosed physical-setup failures are preserved.

Evidence: build/differential-duration-{red,red2,green,green2,scale}-remote/evidence;
build/differential-duration-vendor{-qualified,-qualified2,}-probe; maintained
fixtures and probe runner. Commands, input/source/binary hashes, environment,
wall time and RSS are retained. Final immutable integration checks follow.

Final help rebuild0, format0 and naming0. First main naming invocation omitted build macro ICL_FORMAT_VERSION; failed pilot retained, corrected invocation0 with no production fix. Core clock source/performance provenance and final3GREEN2source/tests verified. Local connected/native/main69/clock47/client25/reset12/multipleTAP22/feedback10,53offline,24matched scale commands and independent review pass. Integrate11reviewed files onto6702646 and freeze8non-document files for required leak/full checks. Final19hashremote native7/all41 running. Commit on task branch, fast-forward master and delete completed branch after verification.

Required Task91 leak-enabled build/native7/main69/clock47/client25/reset12 all0. Root8non-document files remain hash-identical to the reviewed snapshot; full serial regression is running in build/differential-duration-final-full.*. Final19hashremote verification remains pending.

Final Task91 remote50 statuses all0: build/native7/all41CLI/aggregate, with actual19source/test/fixture hashes matching the frozen submission. Required local leakall0; fullserial stillrunning. No production/test/fixture changes to Root8frozen files.

Verified Task91 checkpoint: full serial make check0 in1418.426s,41 dynamic and
41 static generator suites,53 directional goldens,2526ICL334/119rules,LSP62 and
editor6 in202.606s. OptionalANTLR5 remain skipped. Required leak-enabled build/
native7/main69/clock47/client25/reset12 all0. Final remote50 statuses all0 and
actual19source/test/fixture hashes match. All8non-document files stayed immutable
through the final gates. Connected, style/naming,53offline,24matched scale,
independent review and qualified normalized vendor direct probe pass; vendor
negative-target rejection and redundant-check export difference remain recorded.
Commit this11file snapshot on its task branch, fast-forward master and delete
that completed branch. Preserve only Task92future files and its new docsection
on the next active branch. No push. Remaining phase/event/absolute-time/resource,
vendor, strict coverage and final platform requirements remain mandatory.


### Inverted clock duration routes (2026-11-15)

Explicit inversion on an otherwise supported clock-only route preserves its
relative cycle count. Direct, aliased, nested, vector, mux and oscillator paths
can normalize to the same frequency root; switching only polarity does not
require a new iClock for a frequency-only wait. Ratios, rounding and old checked
override versions retain their existing behavior. This does not model phases,
clock events or absolute time.

Only inversion-based rejection is removed. Literal/nonperiodic sources still
fail, even for zero waits. Non-clock consumers remain unsafe through inverted
aliases, and known held inputs/outputs still forbid nonzero waits. Frontend
terminal restrictions and bound differential-input errors retain their paths.
Original/flat/generated/reset-only replay and native state checks cover these
boundaries, including an inverted ICL default under old/new overrides.

All51 remote clock methods and the native scheduler pass, with actual3source/
test hashes matching. Independent design/source review is clean.24 interleaved
source/flat scale commands preserve identical PDL and canonical flat hashes:
ordinary4096 stages baseline0.25–0.38s and candidate0.25–0.36s; new4096-stage
inverted paths0.33–0.48s,121604KiB peak;65536 inverted bits0.18–0.20s,93008KiB
peak. Each input has1024 checked waits and three source/flat repetitions. The
new paths have no supported old functional baseline; no speedup is claimed.

Tessent2026.2 accepts both original and normalized inverse-path probes using
the existing clock runner and declared physical inverse inputs. Both exports are
byte-identical, with all five ordered root waits7,8,7,5,8 retained across nested
ratios and old/new override checks. Vendor export removes redundant repeated
iClock checks; this direct probe is not a strict directional certificate.
All5submitted and6actual staged hashes match.

Evidence: build/inverted-duration-{red,green,scale}-remote/evidence and
build/inverted-duration-vendor-probe/comparison.json. Source/binary/input hashes,
commands, environment, wall time and RSS are retained. Connected/style/offline
and immutable final integration checks follow after Task91 is verified.

Advanced onto verified0bf0fb3 after Task91full0(1418.426s/editor202.606s), leakall0/finalremote50all0actual19. Saved15draftfiles/full.diff/hashes/stashes under/tmp/inverted-duration-task92-before-advance-files; Task91committedonbranch, fast-forwardmaster, completedbranchdeleted. Restoredonly7Task92futurefiles andits new docsections, preserving finalTask91docs; onlymaster+activeTask92branch remain. Localconnected/native7/main69/clock51/client25/reset12/multipleTAP22/feedback10, format/naming and53offlineall0. Freeze9reviewedfiles (6non-document) for finalleak/full; final22hashremote currently running.

Task92 required leak-enabled build/native7/main69/clock51/client25/reset12 all0; finalremote50 statuses all0 with actual22source/test/fixture hashes matching. Root6non-document files remain hash-identical to the reviewed snapshot. Fullserial verification is running in build/inverted-duration-final-full.*; commit/fast-forward/delete only after it passes.

Verified Task92 checkpoint: fullserial make check0 in1346.839s,41dynamic and
41static generator suites,53directional goldens,2527ICL334/119rules,LSP62 and
editor6 in184.735s. OptionalANTLR5 remain skipped. Required leak-enabled build/
native7/main69/clock51/client25/reset12 all0. Finalremote50 statuses all0, actual22
source/test/fixture hashes match;6non-document files stayed immutable. Connected,
style/naming,53offline,24matched scale and independent review pass. Original and
normalized vendor exports0 andbyte-identical withfiveorderedwaits matching;
redundanticlockchecksremoved, no strictdirectionalcertificateclaimed. Commit9files
on taskbranch, fast-forwardmaster anddeletecompletedbranch. Preserve only the
new opaque-root fixtures on the next active branch. No push. Broader scan/clock/
phase/reset/resource, fullvendor, strictcoverage andfinalplatform work remains.

## Opaque root iScan subset (Task93)

The generator lowers raw scans for one explicit root client with scalar,
unconditional, directly driven pins and an unsourced ScanOutPort. Its
DefaultLoadValue must be numeric, explicitly sized, and between1 and1048576bits;
explicit sizing is a supported-subset restriction. Each supplied scan validates
its interface, length, options and data even if overwritten or reset before Apply.
Binary/hex grouped data uses the shared PDL grammar. SI defaults to zero and SO
to X; shorter data pads, longer explicitly based payloads fail without truncation.
Lengths may be decimal, binary or hex. Canonical output uses binary payloads.

The latest queued scan is emitted once at its explicit Apply. Reset discards
pending scans. Empty applies add no scan and are omitted consistently with ordinary
scheduling. Expected SO stays comparison data and is never treated as live readback.
The subset does not automatically repeat stable/default payloads; it preserves
explicit -stable declarations without claiming general blackbox retargeting.
Source, flat-model, generated and unchanged local-reset PDL replay are checked.

Modeled storage, hierarchy, TAP/IR, parallel chains, computed or enabled interface
pins, reservations, merges, clock/state/override operations and mixed modeled
accesses remain explicit unsupported combinations on this path. The separate
visible-register fixture is a vendor reference probe, not supported by this subset.
Tessent2026.2 accepts that fixture but rejects the genuinely opaque unsourced path
at Apply and warns that -stable is ignored. No opaque vendor certificate is claimed.

Verified Task93: fullserial make check0 in1320.089s;42dynamic and42static
generator suites,53directional goldens,2529ICL files with334/119grammar/scanner
rules,LSP62 andeditor6 in186.379s. OptionalANTLR5 remain skipped. Required
leak-enabled build/native7/main69/opaque12/clock51/client25/reset12 all0.
Finalremote51statusesall0 withactual17source/test/fixture hashes matching;
supplementalcoverage7tests0 andactualsubmittedhashesmatch. Firstfullrun2 at
1021.167s caught the missing coverage source entry; tests/coverage.py now measures
pattern_opaque.cc, with no exclusion or waiver. Reviewed production/test/fixture
bytes stayed immutable. Sixlocal scale commands0 at65536/1048576bits; maximum
.07-.08s/~38636KiB, repeatedoutputhashesequal. Opaquevendorpathfailsconfiguration
whilemodeledreferenceexports0; -stableignored byTessent, no opaquevendorcertificate.
Commit21files on taskbranch, fast-forwardmaster, deletecompletedbranch andpreserve
the nextmodeled-scan draft. Broader retargeted/parallel/TAP scan, phases/reset/
resources/vendor/strictcoverage/finalplatform requirements remain open. No push.

## Modeled root scans (Task94)

Raw iScan also supports a complete static, non-inverted modeled chain behind one
explicit root client. Scalar controls and resets must be direct root pins (or
released virtual global resets), with an unconditional SO and every ScanRegister
on the path. An explicit iReset is required before applying a modeled raw scan.
The supplied length must equal the full chain width, up to1048576bits and the
existing4096-register structural limit. Hierarchy, TAP, mux/alias routes, internal
reset cones, conditional SO and end-state directives remain outside this subset.

Native capture, update and idle handling preserve register and pin state. A
qualified complete shift uses one native bookkeeping edge and replaces fully
overwritten shift vectors; it does not perform one vector-wide operation per bit.
The shared bulk primitive validates every load, control and inactive reset before
mutation, and is tested against literal per-bit replay with complete snapshots
and event histories. The scheduler adopts its transaction copy only on success.
Known input bits update controlling fill even when other bits are X. Expected SO
is never assumed to be hardware readback. Empty applies with no end-state work
return before event-proof construction, using the same existing Solve condition.

Raw and ordinary register read/write accesses may occur on successive Applies.
They cannot share one pending Apply. Reservations, merges, state, clock and
interface override operations remain explicitly rejected in a modeled raw-scan
program for this increment. Existing local-reset rewriting preserves source order.
The standard-library maps introduce logarithmic lookups in register count; work
scales with payload and model size, without quadratic wide-register shifting.

The mixed vendor probe alternates raw scans and ordinary reads/writes. Tessent2026.2
accepts original and generated forms and exports byte-identical PDL; actual7
submitted and10prepared hashes match. It still ignores -stable and exports a
synthetic -chain default, so this is recorded as a probe, not a strict directional
round-trip certificate. Broader retargeting, parallel/TAP and ownership semantics
remain required follow-up work.

Verified Task94: fullserial make check0 in1411.913s;43dynamic and43static
generator suites,53directional goldens,2530ICL files with334/119grammar/scanner
rules,LSP62 andeditor6 in184.300s. OptionalANTLR5 remain skipped. Required
leak-enabled build/native7/main69/modeled9/opaque12/clock51/client25/reset12 all0.
Finalremote52statusesall0 andactual29source/test/fixture hashes match. Local15
connectedchecks, formatting/naming and53offline certificates pass. Fifteen scale
commands0:65536bits .05s/~51548KiB;1048576bits .30-.36s/~103940KiB;4096registers
.16-.23s/~86300KiB, canonicalreplayhashesequal. Vendororiginal/normalizedboth0 and
byte-identical exports, actual7submitted+10preparedhashesmatch; -stableignored
and synthetic-chain policy prevents a strictcertificate claim.
The million-bit maintained RED exposed unnecessary event proof before Solve's
existing empty/no-end-goal return. Moving that condition after fill update and
before NeedsScanEvents passed GREEN and focused independent equivalence review;
no reset limit was widened and no transaction is skipped. Native bulk shifting
retains update shadows, final SI, event behavior and partial-X controlling fill.
Commit21files on taskbranch, fast-forwardmaster, deletecompletedbranch andpreserve
only the next static-alias draft. General retargeting/parallel/TAP scans, phase/
reset/resource/clock/pattern lifecycle, broader vendor, strictcoverage andfinal
platform requirements remain open. No push.

## Static raw-scan aliases and inversion (Task95)

The modeled root path now accepts pure aliases and inversion on serial links.
Selected alias bits are traced to their actual terminal, retaining the rightmost
register-bit and complete-chain requirements. The iterative walk checks bit keys,
register uniqueness and the terminal SI index. Direct root control/reset and all
other static-path restrictions remain unchanged.

Each register load uses total SI-to-SO parity XOR its register-to-SO parity.
Known bits invert when that parity is odd; X stays unknown. Physical iScan SI/SO
payloads and the final SI pin retain their original values. SO-only inversion
changes no register load; double inversions cancel. Native phases and controlling
fill use actual register values. Source, flat-model, generated and unchanged
local-reset replay are tested, with literal per-bit native comparisons across
all input/bridge/output inversion combinations and alias forms.

The asymmetric two-register vendor probe passes for original and generated PDL,
with byte-identical exports and actual6submitted/10prepared hashes matching.
The wrong-alias-tap atomic test passes. Fifteen scale commands pass: the ordinary
million-bit path retains its previous canonical output and .30-.36s timing range;
inverted wide aliases take .35-.43s with peak130452KiB;4095alias depth takes
.06-.10s with peak64976KiB. Replay hashes match per scenario. These measurements
qualify the added paths; no unsupported old-path speedup is claimed. Tessent still
ignores -stable and uses its synthetic default-chain spelling, so strict round-trip
certification remains separate. Hierarchy, muxes, TAP/IR, parallelism, ownership
and the remaining general event/lifecycle work are still required.

Verified Task95: fullserial make check0 in1427.583s;43dynamic and43static
generator suites,53directional goldens,2531ICL files with334/119grammar/scanner
rules,LSP62 andeditor6 in184.278s. OptionalANTLR5 remain skipped. Required
leak-enabled build/native7/main69/modeled14/opaque12/clock51/client25/reset12 all0.
Finalremote52statusesall0 andactual33source/test/fixture hashes match. Local15
connectedchecks, formatting/naming and53offline certificates pass. Independent
review foundno consequential issue. Native per-bit replay validates16polarity/
alias combinations inoriginal/flat, actualregister/pinvalues andpartial-X fill.
Fifteen scalecommands0: ordinary1M .30-.36s matchesTask94canonicalhash andtiming;
inversewide1M .35-.43s/~130452KiB;4095aliasdepth .06-.10s/~64976KiB, canonical
replayhashesmatch. Vendororiginal/normalizedboth0 withbyte-identicalexports,
actual6submitted+10preparedhashesmatch, wrongalias-tapatomic0. Tessent-stable/
synthetic-chain discrepancies remain, no strictcertificate claim.
Commit12files on taskbranch, fast-forwardmaster anddeletecompletedbranch. Preserve
only the next hierarchy draft. The old116-file preserved-draft audit found45current
byte matches,61masterhistory matches and10olderstatus-onlyplandrafts; no unique
unfinishedbehavior, archiveintact at/tmp/icl-branch-cleanup-20261115/content-review.json.
General retargeting/parallel/TAP, phase/reset/resource/clock/lifecycle, broader
vendor, strictcoverage andfinalplatform requirements remain open. No push.

## Hierarchical static root scans (Task96)

A full root iScan may traverse nested static modeled instances. The target stays
an explicit root client, and every ScanRegister must lie on its complete serial
path. Pure aliases and scalar scan/control/reset forwarding preserve their actual
terminal identities and inversion. Child controls and TCK must trace to the
corresponding physical root pins; undriven native fallback controls and auto-driven
child clocks do not qualify. Reset checks include elaborator-inferred parent
connections and active-low normalization, followed by the native inactive check.

The existing transactional capture/bulk-shift/update/idle path is unchanged.
Native literal per-bit comparisons cover source and flat forms, bound pin values
and later partial-X controlling fill. CLI checks cover nested and repeated
parameterized leaves, implicit local interfaces/reset forwarding, procedures,
serial inversion, active-low reset and forwarded TCK. Invalid bindings remain
atomic failures. Child-interface scans, partial-chain insertion, muxes, TAP/IR,
parallel chains and resource ownership remain separate requirements.

Independent final source review found no concrete defect. Source-matched native
state/scheduler, nineteen modeled CLI cases and twelve opaque CLI cases pass.
Tessent2026.2 accepts original and generated hierarchical PDL with byte-identical
exports. Its ignored -stable option and synthetic default-chain spelling still
prevent a strict directional certificate claim. Required final gates and scale
measurements are recorded below when complete.

Scale evidence: fifteen commands pass on the source-matched remote candidate.
The1048576-bit leaf takes .26-.33s with peak104796KiB;1024 repeated8-bit leaves
take .19-.32s with peak99296KiB;97 levels take .05-.11s with peak64580KiB.
Source repetitions, flat input and generated-PDL replay have identical canonical
output hashes within each scenario. These qualify newly supported shapes; no
old unsupported-path speedup is claimed. Vendoractual6submitted/10preparedhashes
match, original/normalized exits0 and byte-identical exports. Local15 connected
checks, formatting/naming and53offline directional goldens pass.

Verified Task96: required leak-enabled build/native7/main69/modeled19/opaque12/clock51/client25/reset12 all0. Full serial make check passes in 1364.230889235041s. Final remote52 statuses all0 and actual37 source/test/fixture hashes match. Local15 connected checks, formatting/naming and53 offline directional goldens pass. Independent final review found no concrete defect. Native literal replay checks original/flat nested register and pin state plus partial-X controlling fill. Fifteen source-matched scale commands pass:1M leaf .26-.33s/~104796KiB,1024 leaves .19-.32s/~99296KiB,97 levels .05-.11s/~64580KiB; canonical replay hashes match per scenario. Vendor original/normalized exits0, byte-identical exports and actual6 submitted/10 prepared hashes match. Existing Tessent stable/default-chain differences prevent a strict certificate claim. Commit11 files, fast-forward master, delete completed task branch and preserve only the next child-scan draft. General child/opaque/mux/TAP/parallel/resource/event/reset/clock/lifecycle, strict coverage and final vendor/platform requirements remain open. No push.

## Static child-interface scans (Task97)

Raw scans may target explicit child clients whose complete static paths are
contiguous intervals of the qualified root chain. Logical length/SI/SO are local
to that interface. At Apply the generator inserts the child payload into a full
root scan, retaining surrounding controlling values. Separate SI and SO inversion
conversions preserve actual register loads and expected comparison placement.
Expected SO never supplies scan-in values or live hardware observations.

Setup is queued per interface: a later setup replaces that target only; disjoint
child requests share one native root transaction. Overlapping targets fail before
replay and identify the conflict. Reset clears the queue, and malformed discarded
setups still reject. Mixed ordinary/raw pending accesses remain unsupported.
Compact cached intervals avoid per-interface register-vector copies. Existing
1Mbit/4096register/depth128 limits apply, with total queued payload1Mbit and
aggregate profile traversal16M steps. Original local-reset source order is retained.

Native literal full-root shifts cover unequal surrounding widths, eight SI/child-
SO/root-SO polarity combinations in source/flat forms, exact SI/SO placement,
bound pin state and rollback. Twenty-four modeled CLI cases cover multiple queued
children, overwrite, overlap, procedures/prefixes, partial X and replay. Independent
review found no blocking defect or required coverage gap.

Tessent2026.2 rejects internal child iScan in this ordinary pattern mode, while
accepting generated full-root output. A separately calculated physical root
reference passes and exports byte-identically to generated child lowering.
Actual9submitted/10prepared/4physical-reference hashes match. This is a reference
probe, not a strict child-interface directional certificate. Twenty source-matched
scale commands pass with equal replay hashes: root/child1M .31-.37s, peak106092KiB;
1024queued children .19-.37s, peak99604KiB;97levels .06-.12s, peak64064KiB.
No unsupported-path speedup is claimed. Broader opaque/mux/TAP/parallel/resource,
phase/reset/clock/lifecycle, strict coverage and final vendor/platform work remain.

Verified Task97: required leak-enabled build/native7/main69/modeled24/opaque12/clock51/client25/reset12 all0. Full serial make check passes in 1346.48929021199s. Final remote52 statuses all0 and actual41 source/test/fixture hashes match the corrected immutable snapshot. Local15 connected checks, formatting/naming and53 fresh offline directional goldens pass. Independent review found no blocking defect. Native literal shifts check unequal widths and eight SI/child-SO/root-SO polarity combinations in original/flat, exact comparison offsets, register/pin state and rollback. Twenty scale commands pass with matched hashes: root/child1M .31-.37s/~106092KiB,1024queued children .19-.37s/~99604KiB,97levels .06-.12s/~64064KiB. Tessent rejects original internal-child iScan in this mode (exit1 retained), accepts generated root output0; a separate hand-calculated physical root reference also passes0 with byte-identical exports. Actual9 submitted/10 prepared/4 physical-reference hashes match; no strict child directional certificate claim. Commit13 files, fast-forward master, delete completed branch and preserve the next raw-idle draft. Initial fullserial failed corpuscheck because the new fixture duplicated Leaf; renamed only the new fixture/native helper to ChildLeaf, corpus audit0 and affected leak checks plus fullserial rerun0, no policy exception. General mixed/opaque/mux/TAP/parallel/resource/event/reset/clock/lifecycle, strict coverage and final vendor/platform requirements remain open. No push.

## TCK idle cycles in modeled raw programs (Task98)

Qualified modeled root and child raw scans may be separated by iRunLoop TCK
waits. The existing runtime queue validation and native idle engine are reused;
no clock/event or topology model is added. Unapplied raw setup still fails before
any wait, including zero cycles. Count validation, transactional state/fill replay
and complete-state fixed-point acceleration retain their existing limits.
Opaque raw programs, system-clock/time extensions and independent TAP phases
remain outside this increment.

Native tests compare seven literal idle cycles after raw shifts for all eight
SI/child-SO/root-SO polarity combinations in source and flat forms. Twenty-seven
modeled CLI cases include exact0/one/many/UINT64MAX output counts, child/implicit
controls, partial-X fill followed by masked writes, and atomic pending/unknown/
overflow/wrong-clock/time/opaque failures. Independent review found no blocker.
The production delta only admits iRunLoop through the modeled-raw whitelist.

Tessent2026.2 accepts original/generated positive7/3cycle programs with
byte-identical exports, actual9submitted/10preparedhashes match. It rejects zero
TCK waits in both forms; that negative evidence and local zero-count tests are
retained. Existing ignored-stable/default-chain differences remain separate from
strict certification. Twenty maximal-count scale commands pass with matching
source/flat/generated hashes:1M root/child .27-.33s peak108172KiB;1024children
.19-.37s peak102420KiB;97levels .06-.13s peak64020KiB. No speedup is claimed.
Required final gates and integration evidence follow when complete.

Verified Task98: required leak-enabled build/native7/main69/modeled27/opaque12/clock51/client25/reset12 all0. Full serial make check passes in 1369.6011084119673s; editor6 in182.739s. Final remote52 statuses all0 and actual45 source/test/fixture hashes match. Local15 connected checks, early corpus audit, formatting/naming and53 fresh offline directional goldens pass. Independent final review found no blocking defect or missing coverage. The whitelist-only product change reuses native idle/fixed-point behavior; native literal edges cover eight polarity combinations in source/flat, CLI checks exact counts through UINT64MAX, implicit controls, partial-X fill and atomic invalid cases. Twenty maximal-wait scale commands pass with equal replay hashes: root/child1M .27-.33s/~108172KiB,1024children .19-.37s/~102420KiB,97levels .06-.13s/~64020KiB. Positive-wait vendor original/normalized0 with byte-identical exports, actual9submitted/10preparedhashes match. Tessent zero-count rejection and ignored-stable/default-chain limits remain recorded; no strict certificate claim for this stable fixture. Copied build inputs received fresh mtimes before required builds. Commit11 files, fast-forward master, delete merged branch and preserve the default-chain draft and genuine certificates for its later verified integration. Remaining general models/events/ownership/lifecycle, strict coverage and final vendor/platform requirements remain open. No push.

## Default-chain compatibility and strict raw certificates (Task99)

For already qualified single unnamed clients, the shared raw normalizer accepts
Tessent's exact -chain default spelling and emits the existing canonical form.
This is a documented vendor extension; other chain names, named/parallel contexts
and IR/TAP support are unchanged. Length, payload, X padding, stable flags,
validation-before-discard and native transactional replay retain their checks.

The static comparison tool reuses that pure normalizer only after qualifying an
explicit root non-TAP single unnamed client. It preserves every raw event, SI/SO,
stable flag, order and Apply boundary. Malformed eligible operations fail; legacy
unsupported contexts remain verbatim. Both payload widths count toward its
262144-bit aggregate expansion limit. One normalization may allocate within the
shared1Mbit bound before charging; the aggregate limit is not a peak-memory cap.
Source Tcl is never executed by the comparison tool. Its Makefile target links
the existing helper object directly; no dependency or solver model is added.

The unique RawRoundtrip fixture, without stable, passed genuine Tessent2026.2
FORWARD and REVERSE run/strict record/strict check, using the unchanged certificate
harness and policy. Source/artifact/vendor-console hashes were reviewed, and all
vendor-added reads and writes remain present. Two schema3 snapshots are registered;
all55 directional goldens pass with the rebuilt local tools. This qualifies that
specific modeled-root fixture, not broader raw portability or genuinely opaque,
child, IR/TAP or parallel certification. Existing stable/zero-wait vendor limits
remain recorded. Independent final review found no blocking defect or required
coverage gap. Local16 connected checks, corpus audit, formatting/naming and
original/flat/generated/local-reset default spelling checks pass. Required final
gates and integration evidence follow when complete.

Ten source-matched1Mbit parsing commands pass: plain .30-.36s peak106200KiB, default .30-.37s peak106132KiB. All source repetitions, flat and generated forms share the same canonical hash across both spellings. This measures compatibility overhead, with no speedup claim.

Verified Task99: leak-enabled build/native7/main69/modeled29/opaque13/clock51/client25/reset12/comparison16/all55offline certificates all0. Full serial make check passes in 1474.1217212839983s. Final remote54 statuses all0 and actual68 source/test/certificate hashes match. Local16 connected checks, corpus audit, style/naming and55 fresh offline certificates pass. Independent source/certificate review found no blocker or gap and verified21forward/18reverse hash/content comparisons. Genuine Tessent2026.2 FORWARD and REVERSE run/strict record/strict check0 for the unique no-stable RawRoundtrip fixture, unchanged harness/policy, all exported reads/writes and comparison bits retained. Two schema3 snapshots registered. Exact synthetic-default-chain compatibility is documented as a vendor extension for qualified unnamed clients; no new TAP/parallel model. Ten1Mbit scale commands0: plain .30-.36s/~106200KiB, default .30-.37s/~106132KiB, canonical hashes equal across spellings/replay forms. The comparator charges both payloads to its262144-bit aggregate budget and does not execute Tcl. Added only its necessary existing-helper link dependency; pre-existing braces fixed mechanically. Commit29 files, fast-forward master, delete merged branch and preserve next mux design/work. Broader raw topology/ownership/events/reset/clock/lifecycle, strict coverage and final platform/vendor requirements remain open. No push.

### Gated binary root raw routes (Task100)

A root modeled iScan may traverse scalar binary ScanMux routes selected by known
held ScanRegister update data, constants or pure data forwarding. Descendant
SelectPort gating may use the same held data; capture/shift/update/TCK/reset
connections must retain direct root forwarding. Explicit reset is required before
setup, and the supplied length must match the current active route. Child raw
targets through dynamic routing remain rejected.

The route and inversion parity are fixed for each CSU. Native controls must
activate every register on that route and produce no capture, shift or update
events for bypassed storage. Opening a SIB preserves the previously disabled
child; closing updates the previously enabled child on the common update edge.
A -stable scan must retain the same ordered route, parity and eligible controls,
even when another possible route has the same width. Failed checks preserve held
and shift state, pins, events and controlling fill. Partial-X data retains the
existing controlling-fill rules. Serial-valued or computed selectors, DataRegister,
TAP/IR, parallel chains and arbitrary clock/reset logic remain outside this model.

Maintained tests cover source/flat/generated/local-reset replay, literal per-bit
native equivalence, opening/closing/reopening, inactive storage/reset rejection,
ordinary writes before raw setup, failed stable rollback and partial-X fill,
equal-width route changes, held aliases and unknown selection. Native scheduler
and modeled35 checks pass locally; final immutable gates and vendor/performance
evidence remain pending.

Independent review found no semantic blocker and identified repeated native phase
reads through shared forwarding. The bounded fix supplies the existing native
control algebra with qualified terminal identities and one held-register copy per
phase; primary pin reads stay live across the edge. Ordinary and TAP callers keep
their default reader. A maintained native equivalence test exercises the hook.
Remote baseline1025 registers: wide1040384-bit selector/1M active bits1.04–1.05s,
peak144036KiB;512 aliases shared across1024 children2.31–2.33s, peak78416KiB.
Equivalent post-fix measurements and remaining final gates are pending.

The review fix passes the maintained native hook-equivalence check, complete native
scheduler suite and modeled35 tests locally and remotely (actual26 submitted
hashes match). Formatting/naming pass. Initial16 connected checks and55 offline
certificates also pass; final broad gates cover the optional-reader change.
All25 post-fix scale commands pass across three source repetitions, flat and
generated forms, with equal canonical hashes. Equivalent source measurements:
wide shared selector1.04–1.05s before versus .77s after (peak147380KiB after);
shared512 aliases across1024 children2.31–2.33s before versus .38s after
(peak78916KiB after). Wide gated1Mbit payload .31–.32s, static1Mbit baseline
.36s,128 open/write/close repetitions .05s. Peak168188KiB covers the larger
flattened wide-selector form. Raw output hashes match the pre-fix baseline.
A first flat benchmark command incorrectly reused --debug, colliding with its
input; the corrected25-command run passed without production changes.
Genuine Tessent2026.2 accepted the stable fixture in both directions. Strict
reverse run/record/check pass; strict forward record rejects a dropped -stable
flag and vendor-added selector comparison. That evidence and checker policy are
preserved. A separate no-stable source explicitly compares the selector and child
value; its genuine two-direction qualification remains pending.

The separate explicit-comparison gated_sib_raw_roundtrip fixture passes genuine
Tessent2026.2 FORWARD and REVERSE run/strict record/strict check, all0 using the
unchanged harness and policy. Both schema3 snapshots are registered; all source,
artifact and submitted-input hashes were checked before import. The original
stable fixture and its rejected forward certificate evidence remain separate.
This qualifies the stated no-stable observation contract, not general vendor
stable preservation or child/IR/TAP/parallel portability.

Verified Task100: leak-enabled native7/main69/modeled35/opaque13/clocks51/client25/reset12/trace16/all57offline certificates all0. Full serial make check passes in 1487.5529853680055s. Final remote54 statuses all0, actual88 source/test/certificate inputs match. Independent review found no semantic blocker and one Important shared-control evaluation cost; bounded optional reader fix has maintained RED missing-hook compile, GREEN native equivalence/literal source-flat tests and complete suites. Equivalent wide-selector source1.04-1.05s before/.77s after; shared512aliases/1024children2.31-2.33s before/.38s after; canonical hashes unchanged. All25 scale commands pass across source repetitions/flat/generated, gated1Mbit .30-.33s and128 open/write/close repetitions .05s; peak168188KiB includes wide flattened selector form. Stable fixture accepted by Tessent but strict forward mismatch retained (dropped stable and added selector read); reverse0. Separate no-stable explicit-comparison fixture passes genuine Tessent2026.2 BOTH directions run/strict record/strict check0, unchanged policy, two schema3 certificates imported with all hashes checked. Source tree audit/style and initial16 connected checks passed; final broader gates cover the reader fix. Cached held-data selection, full-map inactive phase proof, pre-update eligibility and separate-copy stable proof preserve all state/fill rollback. Native helper hook is the necessary scope ruling beyond the initial scheduler-only file list; no separate state engine. Commit28 files, fast-forward master and delete merged branch, preserve next accessible-child work. Closed-child preparation, broader raw/ownership/events/reset/clock/lifecycle, strict coverage and final platform/vendor requirements remain open. No push.

### Accessible child raw scans inside a gated route (Task101)

An unnamed child with a static local chain may be scanned when its complete
register interval is already part of the known active root route. The scheduler
reuses the interval/parity proof and emits one physical root CSU for disjoint
queued children. It preserves surrounding controlling fill, translates SI and SO
parity separately, validates discarded setup and rejects overlapping targets.
Closed/incomplete children and children whose own local chain contains a mux remain
atomic errors; this increment does not insert preparatory opening scans.

Current dynamic route/slice caches live only until Reset, Idle, Assume, ordinary
Apply or successful raw adoption. Copies/speculative schedulers start cold.
Immutable qualification and static-child caches persist. A changed route recomputes
child positions, including equal-width switches and offsets shifted by another
SIB. Setup/queue validation reuses one current profile rather than rebuilding it
for each child.

Any latest stable claim on a dynamic child requires the full physical root route
and native phase eligibility to remain stable in this bounded increment. That
proof is separate from the emitted -stable flag, which remains the conjunction
of latest flags. Mixed stable/nonstable siblings are accepted when the physical
route stays unchanged. Local-only stability while another sibling changes the
root route remains future work. Existing static child behavior is unchanged.

Maintained native reference tests and modeled43 pass locally. They cover physical
root equivalence, expected SO, partial-X fill, sibling coalescing/latest setup,
mutation/copy lifetime, changed offsets, equal-width switches and all8 SI/child-SO/
root-SO inversion combinations, with source/flat/generated/local-reset replay.
The unique ActiveRawLeaf fixture preserves child SO and final child/selector
comparisons. Independent final review and connected/vendor/scale/final gates are
pending. Broader raw topology/ownership, closed-child preparation, opaque/IR/TAP/
parallel behavior, strict coverage and final release qualification remain open.

Task101 evidence: initial16 connected checks/corpus audit and57 offline
certificates pass; corpus2550 files, zero duplicate-file groups,26 reviewed
module groups. Independent review finds no production defect. Its two test gaps
were strengthened: warm ordinary Apply/copy cache states and independent expected
SO polarity. Fault injection removing Apply invalidation and SO parity produces
the exact native warm-cache failure and4 SO assertions; exact reviewed source
7b0b0a6c is restored, native scheduler and modeled43 pass. No mutation ships.
All26 source-matched scale commands pass:1M active bits via child .29–.30s
(source),1024 queued children with512 shared aliases .37–.38s,97-level child
.10s,128 reopen/write/close repetitions .04s. Peaks107052KiB overall. Three
Task100 physical-reference repetitions .24–.27s wide and .20s queued-network
provide equivalent output hashes; this measures retargeting overhead, not speedup.
Source/flat/generated forms share the same canonical hashes.
Genuine Tessent2026.2 forward run/strict record/strict check0 using unchanged
harness/policy; one schema3 certificate imported after source/artifact/input/
console hash checks. Reverse original source1 reports that child.local is an
internal ScanInterface and iScan is unsupported in this tested context; retained
evidence is not a reverse certificate. All child SO and final child/selector
comparisons remain present. Existing and new forward snapshots total58; final
immutable sanitizer/fullserial/remote/style/offline gates remain pending.

Verified Task101: leak-enabled native7/main69/modeled43/opaque13/clocks51/client25/reset12/trace16/all58offline certificates all0. Full serial make check passes in 1496.6201768030296s. Final remote54 statuses all0 and actual99 source/test/fixture/certificate inputs match. Independent source review found no production defect; two required test-strength refinements are fixed with mutation RED (missing warm-Apply invalidation and SO parity), exact production-byte restoration and GREEN native/model43 suites. Warm-copy isolation, changed offsets, same-width switches and8 inversion/expected-SO combinations are maintained. Existing interval/parity/root-CSU logic reused with constant-sized cached child slices, by-value return and audited dynamic cache lifetime. Any latest stable claim on a dynamic route still requires whole physical-root stability in this bounded increment; OR proof is separate from AND emitted flag. Local-only stable scope and inner child muxes remain next work, with no waiver. All26 scale commands0:1M active bits via child .28-.31s,1024 queued children/512shared aliases .20-.38s,97-level child .04-.10s,128 reopen/write/close .04s;107052KiB peak, hashes equal to equivalent Task100 physical reference runs and across source/flat/generated forms. Source audit2550 files/zero duplicate groups/26reviewed module groups;16 connected/style/naming/offline pass. Genuine Tessent2026.2 FORWARD run/strict record/strict check0 for unique ActiveRawLeaf source; schema3 certificate imported with source/artifact/input/console hashes checked, all child SO and final child/selector comparisons retained. Reverse original source1 explicitly rejects internal ScanInterface in tested context; no reverse certificate claimed and checker policy unchanged. Commit20 files, fast-forward master, delete merged branch and preserve next dynamic-child work. Closed-child preparation, broader raw/ownership/events/reset/clock/lifecycle, strict coverage and final platform/vendor requirements remain open. No push.

### Binary routes inside an accessible child (Task102)

A modeled child raw scan can follow the current qualified binary mux/SIB path
inside that child. For example, after its ancestor is open, a one-bit child scan
may open an inner leaf and the next child scan uses nine bits. One physical root
CSU retains surrounding controlling fill and independently translated SO masks.
The current root profile records selected mux choices; cached child segments refer
to contiguous spans of that profile rather than copying register vectors.

Stable claims apply to the named interface. The implementation proves local
ordered registers, SI/SO parity, accessibility and native post-route eligibility;
this is a deliberately stronger modeled criterion than IEEE1687-2014 §7.9.12's
minimum repeatable-length rule. A child may remain stable while an unrelated
sibling route changes. The physical root output then omits -stable and retains
the completed scan's original length/data. Explicit root stable claims still
require the whole root geometry to remain stable. Mixed source flags never gain
a physical stable flag. A child path change is rejected even when another sibling
compensates so the total root width stays equal.

Pre-update controls determine opening/closing effects. Post proof derives a fresh
profile from trial state, compares compact local spans using a private traversal
budget, and checks the post-route active set on another state copy. Failed proof
never enters live caches or adopts data/fill/events. Wholly nonstable scans retain
existing behavior when their post route becomes unknown. Closed ancestors still
need separate access preparation; no new TAP/parallel/reset/clock/selector model.

Maintained nested literal native reference and modeled47 tests pass remotely
(actual30 submitted hashes match). Tests include child1/9/9/1 open/load/close/
reopen, retained leaf data, partial-X retry after failed stable, explicit SO masks,
all8 inversion cases, unknown selection, root-vs-local scope and compensated-width
failure. Existing static child/root/opaque tests pass. Style-only braces are being
normalized; connected58offline/corpus, scale, vendor and final gates remain pending.

Task102 evidence: source-matched remote native/state/opaque13/modeled47 pass after
formatting. Local16 connected checks, corpus2551 files/zero duplicate groups/26
reviewed module groups and58 offline certificates pass before new vendor data.
Independent final review found no production defect and one Important regression
gap. Equal-count/equal-total-parity ID-swap and same-ID suffix-parity changes with
compensating inversions now have negative and legal-repeat tests. Disabling only
the comparison loop produces2 failures; exact reviewed source31b31ef6 is restored
and all48 CLI methods pass. A first mutation harness formatting anchor failed
before any source write; corrected brace anchor supplied the intended mutation.
No mutation or checker exception ships; no re-review loop.
All26 scale commands pass. Source repetitions:1M active bits .32s,1024 queued
dynamic children/2049 registers/512 shared aliases .85–.86s,97-level incremental
opening .48–.49s,128 nested open/write/close cycles .06s. Peak165192KiB includes
the flattened2049-register form. Equivalent Task100 physical references .27s
wide/.46–.47s many; hashes match between references and child/source/flat/generated
forms. Measurements describe overhead, not a speedup claim.
Genuine Tessent2026.2 FORWARD run/strict record/strict check0 for BOTH nested
dynamic_child_raw and local-stability projection fixtures. The second case keeps
a child stable while the physical root grows8->16, preserving explicit SO masks
and final child/sibling observations. Two schema3 forward snapshots imported with
all hashes checked; total directional offline snapshots60. Both reverse original
source runs1 explicitly reject internal ScanInterfaces in this tested context;
no reverse certificate is claimed. Policy/harness remain unchanged. Final style/
60offline/leak/fullserial/remote gates and integration remain pending.

Verified Task102: leak-enabled native7/main69/modeled48/opaque13/clocks51/client25/reset12/trace16/all60offline certificates all0. Full serial make check passes in 1400.9289951249957s. Final remote54 statuses all0 and actual119 source/test/fixture/certificate inputs match. Independent final source review found no production defect; its Important equal-count ID/suffix-parity test gap is fixed. Disabling only the per-register comparison produces2 failures; exact31b31ef6 production bytes restored and all48 CLI methods pass. Local stable proof compares ordered register IDs and parity, with immutable pre/post segment spans and bounded shared proof work; full native eligibility retains pre-update versus post-proof active sets. AND emitted stable is removed when local geometry stays stable but the physical root changes. All26 scale commands0:1M active bits .32s source/.33s flat/.31s generated;1024 dynamic children/2049 registers/512shared aliases .85-.86s source/.90s flat/.45s generated;97levels .48-.49s source,128reopens .06s. Peak165192KiB. Hashes equal across forms and equivalent Task100 physical references (.27s wide/.46-.47s many); no speedup claimed. Source audit2552 files/zero duplicate groups/26reviewed module groups;16 connected/style/naming/offline pass. Genuine Tessent2026.2 FORWARD run/strict record/strict check0 for both dynamic_child_raw and dynamic_child_local_stability, with two schema3 certificates and all source/artifact/input/console hashes checked. Explicit SO and final reads retained, including local-stable root8->16 projection. Both reverse originals1 reject internal ScanInterfaces in tested context; no reverse certificate claimed and checker unchanged. Commit29 files, fast-forward master, delete merged branch and preserve Task103 reservations work. Closed-child preparation, broader raw/ownership/events/reset/clock/lifecycle, strict coverage and final platform/vendor requirements remain open. No push.

### Resource reservations around modeled raw scans (Task103)

Modeled single-stream iScan now accepts existing iTake/iRelease/iState commands.
An applied write or controlling iState after a take establishes protected bits;
an unwritten take remains free. IEEE1687-2014 §7.9.19(c,d) distinguishes this
retargeting rule from ownership across merged calls; raw iMerge remains guarded.
Explicit raw SI may replace already protected bits in addressed ScanRegisters.
Automatic surrounding fill must preserve protected values at each native phase.
Unknown SI on an established protected bit rejects before hardware/fill adoption;
expected SO and history-only iState do not establish controlling knowledge.

Latest requests contribute a pre-CSU register footprint, revalidated at Apply
after any intervening controlling iState. Newly enabled post-route registers are
absent from that footprint. The existing ResourceReservations recorder refreshes
only already protected bits after success; takes, call return, persistent scope,
release and explicit reset retain their existing behavior. The default native
API remains compatible and skips footprint construction without a preservation
map or requested output. Failure leaves hardware, fill and caller footprint
unchanged; valid pre-state geometry may be cached during normalization.
Established port/non-ScanRegister reservations reject explicitly in this bounded
raw implementation. This does not add opaque, closed-child, IR/TAP/parallel/raw
merge, clock-port ownership or new reset/clock support.

Initial native and56 CLI methods pass; source/flat/generated/local-reset forms,
partial aliases, surrounding fill, X rollback, unwritten takes, scopes and
pending state are covered. Added same-length pending child-route regression and
invalid-mask native assertions; connected/final/style/scale/vendor gates pending.

Independent Task103 review identified quadratic full-register copies in the shared
recorder reader. Maintained1M setup+explicit-refresh regression fails at15.032s
before the fix. Scheduler Value now reuses the native SignalEvaluator::Bits through
ReadBits, keeping one evaluator and its existing memoization for the whole target.
The benchmark includes explicit-wide assignment as well as surrounding protection.
No additional cache engine or weakened check is introduced; final gates pending.

Task103 qualification: all20 local connected groups and60 prior offline certificates
pass after the performance fix; all14 remote groups pass with79 submitted inputs
verified. Both local and Rocky8 remote44-command scale runs pass, with all physical
packets equal across source/plain/flat/generated and equivalent Task102 no-owner
reference (only controlling iState metadata excluded from the packet hash). Remote
1M active bits: full protected surrounding .64-.65s source/.67s flat; explicit-wide
refresh .74-.76s source/.78s flat; partial32-bit take .47-.48s;1024 child requests
with512 shared aliases .44s. Peak195112KiB, including flat form. Current no-owner
baselines .30-.35s wide/.25s many; measurements describe reservation overhead.
Exact commands/environment/hash/memory records are in benchmark results.json.

Genuine Tessent2026.2 FORWARD run/strict record/strict check0 for
raw_scan_reservations. All source/artifact/job-input/console hashes verified before
importing one schema3 snapshot, directional offline total61. Initial forward run
added pre-write comparisons; the fixture now explicitly retains those observations
(0 thenA5) along with raw SO8 and finalA7. No comparison or checker rule removed.
Original REVERSE still1: vendor enforces previous iWrite bit3=1 after rawA5 and
rejects the next partial write. No reverse certificate or full vendor equivalence
is claimed. IEEE1687 §7.9.12(e) defines explicit SI controlling state like iWrite;
the implementation refreshes already-protected author-assigned storage, while
§7.9.19(d) prohibits software changes. Tessent v2023.4 iTake/iScan manual topics
and actual2026.2 behavior are recorded separately from that normative reading.

Final immutable leak/fullserial/remote gates and integration remain pending.

Verified Task103: leak-enabled native7/main69/modeled58/resources35/data9/queued15/observation20/opaque13/clocks51/client25/reset12/trace16/all61offline certificates all0. Full serial make check passes in 1464.4951526590157s. Final remote54 statuses0 and actual133 inputs match;13 source-matched GREEN2 tests are explicitly reused, with79 input hashes verified and golden registry checked freshly. Local20 connected/style/audit2553 files/zero duplicate/26reviewed groups0. Independent final review found one Important quadratic recorder read:1M setup/explicit-refresh regression RED at15.032s then all58 CLI GREEN22.440s; native ReadBits reuses existing SignalEvaluator::Bits and its memoization instead of whole-register copies per bit. No other findings or deferred minors. Both platforms44 scale commands0, exact commands/environment/memory/hashes archived. Remote1M full surrounding .64-.65s, explicitwide .74-.76s (.78s flat), partial32 .47-.48s,1024children/512aliases .44s; peak195112KiB. No-owner references .30-.35s/.25s; every physical packet/Apply hash matches after excluding only controlling iState metadata. Genuine Tessent2026.2 FORWARD run/strict record/strict check0 with all source/artifact/input/console hashes verified and one schema3 snapshot imported. Original REVERSE1 retains old iWrite bit3 after rawA5 and rejects later partial write; no reverse certificate or full equivalence claimed. Added author pre-write read0/A5 to retain vendor observations alongside SO8/finalA7; policy unchanged. Single-stream unwritten takes stay free; only previously protected explicit pre-CSU ScanRegister bits refresh, all surrounding values checked through native phases. Established port/non-ScanRegister ownership remains rejected; valid pre-state cache memoization allowed on failure, hardware/fill/caller footprint unchanged. Rulings: reuse immutable source evidence, review an uncommitted package before final commit, explicit SI controlling-state interpretation per IEEE7.9.12(e)/7.9.19(d) with vendor limitation recorded. No allocation-failure transaction expansion without concrete requirement. Commit26 files, fast-forward master, delete completed branch, preserve future mixed-root preparation work. Broader raw topology/ownership/events/reset/clock/lifecycle, strict coverage and final platform/vendor requirements remain open. No push.

### Ordinary preparation followed by a root raw scan (Task104)

An ordinary iWrite/iRead queue may precede a qualified root-client iScan in one
source iApply. Existing ordinary planning executes once, records newly established
reservations and renders its concrete preparation steps. Raw replay then checks
the resulting route length and uses refreshed ownership state. For example,
iWrite open1 followed by root iScan9 in one Apply can open a bypassed eight-bit
leaf, then load that nine-bit route. Generated PDL keeps both physical CSUs.

RawTemplate caches immutable qualification; RawChain caches current selected
geometry. Deferred setup validates root identity, graph/port/control/reset roles,
options, numeric bounds and payloads, with selected geometry/reset readiness
checked at Apply after preparation. Pure raw setup stays strict. Reset can cancel
a valid deferred queue. Writes/reads after a raw setup, child preparation and mixed
-together remain rejected in this increment. No new solver or reset/clock model.
Native raw failure preserves separately successful preparation; generator failure
preserves pattern/local-reset/debug artifacts. Diagnostic --log records the error.

Native strict/deferred/cache/copy/rollback tests and65 modeled CLI methods pass;
all20 local connected groups,14 remote groups/84 input hashes, style, corpus2554
files/zero duplicates/26reviewed module groups and63 offline certificates pass.
Independent final review found no Critical/Important/Minor issue. Genuine
Tessent2026.2 BOTH directions run/strict record/strict check0, two schema3 snapshots
imported with source/artifact/input/console hashes verified. Explicit pre-write
read0, rawSO1 and final leafA5/open1 comparisons retained; checker policy unchanged.

Scale acceptance distinguishes the inherited ordinary solver from raw replay.
Both platforms33 supported-workload commands0, complete generated bytes equal to
Task103 explicit-boundary references:16,384active bits,1024 queued writes to the
same SIB, and128 prepare/raw-close groups. Remote source repetitions3.31-3.34s,
.05s and.47s respectively; peak75464KiB. Local16K2.14s/queued.03-.07s/repeat.28-.30s,
peak75436KiB. These measurements show boundary handling adds no demonstrated
scaling change; no speedup is claimed. Queued1024 is not1024 distinct instruments.

The broader ordinary-planner performance requirement remains open: exact1M active
bits exceeds existing reset-work accounting (storage plus model nodes);1,047,552
bits reaches existing scan-solver limits;65,536bits timed out at45s in BOTH saved
reference and candidate, and1024 distinct gated preparations hit the existing
scan-event resource bound in BOTH. Rejections/timeouts and original model/PDL/
commands remain archived under mixed-root-raw-scale{,2}/probes/limits evidence.
Raw-only1M support remains verified by the maintained Task103 regression. The
upgrade trigger is workloads exceeding the characterized ordinary-planning range
or requiring those distinct preparations; address that planner separately, without
waiving its guards or claiming full project scale acceptance here.

Final immutable leak/fullserial/remote gates and integration remain pending.

Verified Task104: leak-enabled native7/main69/modeled65/resources35/data9/queued15/observation20/opaque13/clocks51/client25/reset12/trace16/all63offline certificates all0. Full serial make check passes in 1460.4493277180009s. Final remote54 statuses0 and actual151 inputs match;13 unchanged functional groups reused from GREEN2 with84 inputs verified, separately measured benchmark/changed golden registry excluded explicitly. Local20 connected/style/audit2554 files/zero duplicate/26reviewed groups0. Independent final review no Critical/Important/Minor. Exact desired reservation refresh order, cold-copy/cache invalidation, strict pure normalization, deferred root validation, pending state/owner return/reset cancel and publication checks pass. Pattern/reset/debug publication is atomic; diagnostic log records errors by existing contract. Both platforms33 supported scale commands0 with complete output byte hashes equal to Task103 explicit-Apply references:16K active3.31-3.34s remote source,1024 writes to SAME SIB .05s,128 prepare/raw-close groups .47s; peak75464KiB. Local2.14s/.03-.07s/.28-.30s. Maximum1M/near1M ordinary budget rejections and65K45s timeouts/1024 DISTINCT-SIB event-budget failures occur in both saved reference and candidate; retained artifacts and exact commands, no guard raised or broader optimization waiver. Raw-only1M remains verified; next native ordinary optimization addresses this real inherited bottleneck. Genuine Tessent2026.2 BOTH directions run/strict record/strict check0 with all source/artifact/input/console hashes verified; two schema3 snapshots imported and every pre-write/raw/final comparison retained. One ordinary preparation queue precedes root raw replay in a source Apply; generated output keeps physical preparation and final raw CSU. No duplicate solver or state engine, immutable RawTemplate factored from current RawChain. Reverse-order/child/mixed-together/opaque/TAP/IR/parallel/merge and broader reset/clock/port ownership stay explicitly unsupported. Commit28 files, fast-forward master, delete completed branch and preserve native ordinary optimization work. Strict coverage/final platform/full vendor and remaining functionality acceptance stay open. No push.

Qualified ordinary root accesses now reuse native CSU replay before symbolic
frame expansion when all required ScanRegisters are already active and all
physical loads are concrete. Existing topology/reset/clock qualification and
exact partial/aliased reservation masks apply. Expected scan-out data remains
observation metadata; unknown fill, inactive targets, ports, TAP, data registers,
end-state and unsupported grouping retain the existing solver. No resource
budget was raised. Million-bit visible preparation and1024 distinct visible SIB
writes are covered by maintained source/flat/generated replay and native
literal-clock checks; measured performance and final gates are recorded in the
2026-11-16 native ordinary scans plan. Broader solver/clock/reset coverage and
strict project acceptance remain open.

Verified Task105: frozen26 files and scheduler9c8f3f2698c151a8cd9501912878382985e3908d5c7b3a4ddaf57d9cf7ce54e6. Leak-enabled native7/main69/modeled68/resources35/data9/queued15/observation20/opaque13/clocks51/client25/reset12/trace16/all65offline certificates all0. Full serial make check passes; elapsed seconds recorded in build/native-ordinary-final-full.wall. Final remote54 fresh statuses all0 and169 original input hashes match. After immutable full completion, only the benchmark helper was corrected to give reference/candidate matching --debug options and a default scenario-provider callback for reuse; unchanged other-module AST and168 unchanged functional inputs proved. Original26 snapshot retained; corrected helper qualified by fresh local40/remote40 commands and independent delta review, no runtime product change. No functional reuse after necessary style extraction. Final local/remote40 matched-option scale3 commands each all0, candidate source/flat/generated complete artifact hashes match; equivalent16K and128-repeat ordinary reference bytes match. Maximum1M visible preparation,1024distinct SIB/512alias preparation and1Mpartial32 now pass without raised guards; their final raw packet hashes match manually authored raw references because legacy large ordinary programs reject. Independent literal1M retained-state and eight unequal-width/ascending/inverted SI-SO alias clocks prove hardware and exact observation/reservation masks. Native attempt reuses RawChain/ReplayScan, exact DesiredPreserved and lazy top-level event dispatch; internal fixed event calls unchanged, no duplicate solver/evaluator. Unknown fill/inactive/data/TAP/end-state/group/port cases retain symbolic fallback. Existing access-group validation/context/order extracted unchanged to satisfy style limit, independently reviewed; clang-format/tidy pass. Initial connected20 and GREEN2 fourteen groups all0; current post-style sanitizer/full/remote gates run fresh. Corpus2555 files, zero duplicate-file groups,26 reviewed repeated-module groups. Genuine Tessent2026.2 BOTH directions run/strict record/check0, all source/artifact/jobinput/console hashes verified before two schema3 snapshots imported; current source reproduces both. First forward probe failed due retained open1 observation; final source explicitly retains that observation on every later apply, without deleting comparisons or normalizing trace evidence. Independent final and necessary style-delta reviews no Critical/Important/Minor, declined-to-judge lists empty. Commit26, fast-forward master and delete completed branch; preserve root raw queue-order drafts. Raw-order/child/opaque/IR/TAP/parallel/merge and broader reset/clock/ownership/strict coverage/final release acceptance stay open. No push.

Root modeled iScan requests may now appear before or between ordinary iWrite/
iRead commands in one source Apply group. All ordinary accesses prepare the
route before the final raw CSU, regardless of source order. Each queue keeps its
existing last-value rules. Current root route/length/reset-readiness checks run
at Apply; reset can cancel pending execution, while malformed operands/options
and immutable-model qualification still fail immediately. Public native strict
normalization stays available; mixed child preparation and together remain
explicitly unsupported. Maintained permutations, call/owner/override/cancellation
and scope checks, canonical-byte scale comparisons and strict vendor snapshots
are recorded in the2026-11-16 root raw queue-order plan.

Verified Task106: root raw setup may precede or interleave ordinary access setup; existing ordinary preparation executes before final raw CSU. Every queued root defers current geometry/reset readiness to Apply; immutable qualification/options/payload bounds stay early, reset cancels execution and strict native default remains. Only16-line source front-end change, no scheduler/evaluator/reset/clock/API change. Root predicate retains child mixed rejection in both orders. Maintained RED5 methods/8 intended failures; GREEN2 all15 statuses0, main69/modeled72, actual174 inputs verified;12 unchanged functional groups reused with explicit169-input native-engine ancestry. Sole old expectation updated to accept canceled no-reset raw and separately reject executed no-reset raw; no product fix after independent review. Independent final review no Critical/Important/Minor; only actual gate/performance/vendor evidence was declined and verified by executor. Local20 connected0 with native7 source-identical Task105 full checks reused;13 fresh CLI/trace groups. Corpus2556/zero duplicate files/26 reviewed repeated-module groups0. Format/tidy with required version macro0. All67offline0. Genuine Tessent2026.2 BOTH directions actual run/strict record/check0, all source/artifact/jobinput/console hashes verified before importing two schema3 snapshots, local replay0. Local40 and remote matched-option40 scale commands0 with complete output hashes equal canonical Task105 explicit queue order, covering1M physical bits,1024distinct/512aliases,128 repeats and1M partial32 held state. Remote .25s1M,.73-.74s distinct1024,.03s128repeats,.49-.50s partial32; same-option canonical times equivalent. Earlier unequal-option evidence retained and superseded. Final leak-enabled sanitizer/native7/main69/modeled72/owner/data/state/clock/reset/trace/all67offline0; full serial make check0 (elapsed in build/root-raw-queue-order-final-full.wall). Final remote54 all0, actual187 hashes match frozen source;14 unchanged functional groups reused from source-matched GREEN2 with174 input verification/explicit benchmark+registry exclusions and inherited reuse records retained, fresh offline covers registry, separately measured benchmarks/main/engine hashes verified. Commit25, fast-forward master and delete branch; preserve mixed-child preparation drafts and recovery artifacts. A pre-existing nested symbolic-fallback route-cache issue discovered during Task107 is corrected and independently reviewed in that dependent task; its producer changes remain outside this frozen front-end snapshot. No push. Mixed child/opaque/IR/TAP/parallel/merge and broader reset/clock/ownership/strict coverage/release acceptance stay open.
