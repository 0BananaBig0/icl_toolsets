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

Task153 adds qualified explicit callback-only DataRegister procedure execution
and source-preserving reset replay. All43 post-fix gates pass, including63x1048
tests per generator,14 native/14 sanitizer suites,24 quality checks and packaging.
Task159 supports actual modeled ordinary endpoint observations in explicit
write callbacks while keeping abstract callback storage unknown. Invoked
read-callback histories, physical callbacks and automatic downstream/mixed/merged
groups remain open; see [write-callback observations](pattern-generation.md#actual-endpoint-observations-in-write-callbacks)
and the [callback guide](pattern-generation.md#task153-explicit-dataregister-callback-execution).

Task154's conservative primary-state index improves the measured1,024-target
callback source workload from26.21s to2.61s with exact parent/flat/generated
pattern matches. Ten focused methods and native state/copy checks pass; all46
qualification gates,64x1058 per generator,14 native/14 sanitizer suites,
six quality checks and packaging pass. The single Important review finding is
fixed; one Minor is deferred. Broader item10 is pending.

| Area | Current implementation/evidence | Remaining acceptance work |
|---|---|---|
| Shared frontends and ordered inputs | Shared ICL AST/PDL tree; Design/Runtime tests and CLI ordered overrides; reset cancels queued reads/writes across file and call boundaries with operand validation; observation-history iState flags are forwarded without hardware or queue mutation; getters preserve never-read/sparse unknown bits and explicit history; opt-in PIO and serial-register reads, including bound TAPs, supply selected-frame observations and exact failure counts; default applied observations remain unavailable and initial/no-cared-comparison status is known zero | Complete runtime command coverage and broader real-pair generation |
| Elaboration and values | Namespaces, specialization, arithmetic, ranges, aliases, enum and compiled logic; typed contiguous vector-port functions retain per-function attributes/enums/properties, selected access eligibility, child bindings and associated-flat identities (Task142 reviewed; two blockers fixed and post-fix qualification passes); hierarchical hardware references require child output ports, while aliases cannot bypass child-output boundaries and deep aliases remain PDL-only under §6.5.2(g), with its prose conflict documented; direct ScanOutPort register sources require the declared rightmost bit; explicit unsized-inversion rejection under §6.3.12(m), with conflicting-example/Tessent acceptance documented; source-located diagnostics; ordinary-data terminal-kind eligibility is checked after flat type restoration with bit-specific aliases and the two-input ScanMux ToIRSelect exception; checked Sources enforce local terminals/direct-child outputs and raw-alias ownership after origin restoration; generated TAP boundaries use explicit outputs | Remaining source scope/direction and contextual source-kind rules, broader alias-rule interpretation, and corpus specialization |
| Address/one-hot models | Contributor/domain/default/alias resolution; primary-, scan- and mixed-driven DataRegister writes/reads; nested OneHotScanGroup selection, indexed vector lanes feeding a scalar chain, inferred simultaneous disjoint interface-free vector returns and canonical replay | Overlapping returns, independently controlled lanes and broader model qualification |
| Native hardware state | Unknown initial state, reset propagation, separate shift/update storage, addressed DataRegister events, clock edges and event-free ScanRegister/DataRegister/top-level pin assumptions; pending default state overwrites overlapping write/compare values by resolved bit identity; no-event reads preserve storage and fill across primary, mixed and hidden-client paths; known write/reset history and mixed-storage reset snapshot settlement preserve supported transient reset effects; Task144 adds bounded simultaneous DataRegister feedback from real old state, known/unknown convergence, self-disable history and scoped intermediate ownership checks in native and primary/mixed replay; all three Important review findings are fixed, including unknown mixed history, pending-read deltas and implicit OneHot controls, with14 replay methods and full post-fix qualification | Automatic downstream/mixed/merged DataRegister callbacks and invoked read-callback histories/physical output (Task153 adds explicit callback-only execution and reset-source replay; Task159 supports ordinary endpoint observations within write procedures), general gated/serial-dependent reset events (including changing external resets during data feedback) and complete vendor builtin models |
| AccessLink/TAP | BSDL reader, IR/DR datapaths, FSM, child wiring and runtime metadata restored from flat ICL; explicit IR PDL reads/writes and ordered apply replay, with nine fresh SIB/reset directional certificates; common-phase multiple TAPs on equivalent root TCKs with per-IR metadata, aligned lanes, explicit reset connectivity and batched pin/clock snapshots | Independent TAP phases, shared serial storage, implicit multi-TAP reset ownership and remaining corpus/reference qualification |
| ScanInterface controls | Runtime explicit/implicit membership; single-TAP phase-qualified visible/hidden capture, shift and update events with concrete replay; standalone explicit/inferred clients drive bound phase/clock pins; complete-path proof or bounded hidden-event planning, indexed lanes, primary-enable preparation and concrete clock sampling covered | Shared-chain, serial-derived controls and general independently gated events |
| Raw scans | Genuine opaque root-client lowering; one fixed declared-width opaque child on a complete scalar root route with private unknown capture and prior stable/default preparation; static modeled root and accessible hierarchical child intervals, disjoint setup queues, binary held-selector/SIB routes and local stable proof; serial aliases/inversion, typed root controls, explicit reset, native CSU replay, TCK waits, default-chain spelling and partial-X controlling fill; bounded single-stream ScanRegister reservations and explicit root port effects with retained locks; ordinary preparation before root or child raw replay in either source order and automatic parent setup for known modeled child-local routes; IR and complete ordered instrument/builtin DR on one bound scalar root TAP, including literal X input on canonical phase/reset profiles | Broader opaque-child topology, shared/independent parallel and multi-TAP raw scans, raw merges, Tessent2026.2 literal-X export, TAP established port ownership and broader vendor qualification; compatible modeled-child port locks now use actual edge/final checks |
| Serial/mixed scheduler | Shared parallel/scan frames, primary observations, register/mux/logic paths, SIB and bounded feedback reset prerequisites, read obligations, register/primary/computed-port AccessTogether groups, forwarded/logic/mux/one-hot port writes and partial/X writes; ScanRegister/DataRegister/primary-input iApplyEndState defaults, per-emitted-apply primary representation and final restoration; equivalent direct/inverted/forwarded observations and enabled alternatives on one path; simultaneous disjoint named client chains with aligned lanes/common events; independently selected clients (SelectPort or ShiftEn-only) and named-chain groups share hardware state, including reset-before-capture and transient reset history | Independent TAP phases, overlapping intermediate taps and broader access models |
| Idle cycles | Literal TCK counts on standalone clients or reset common-phase TAPs; per-edge state/reset/fill replay, reservation rollback, fixed-point acceleration and retained output count; one forward TCK vendor fixture; checked relative system-clock waits on clock-only routes including direct declared differential partners, ordered muxes, exact fixed-ratio scaling, versioned overrides, typed Period oscillator roots, explicitly bound differential inputs with a common modeled phase origin and root-relative replay; named secondary sources on negative clock inputs with six matching source/normalized vendor exports; typed ToClockPort encodings for internal clock inputs with legacy loading and four fresh mapped direct/alias flat traces; metadata-gated physical clock names for native flat iClock/waits and four raw export replays; minimum -time waits with exact literal preservation on models without storage, opaque hardware, clock-driven data or forwarded test clocks | General implicit selection, inverted child clock phases, general intrinsic oscillator restoration/frequency, general vendor-flat output-source networks, general physical/native naming collisions, raw data/override names and negative vendor iClock targets, general bound differential phase relationships, general stateful absolute-time waits, general clock events and external time-dependent instrument behavior |
| Optimization | Dependency-guided and shared-address write batches; nested iMerge normalization with disjoint multi-apply ScanRegister/DataRegister read/write batching, including slices, aliases and X masks, plus connected/computed data/control-port accesses with bounded dependency ownership and serial fallback; independent tree/finite-state oracles, concrete replay and common-update constraints | Broader merge optimization, timing/resources, whole-program optimization and broader scale/vendor qualification; no global optimality claim |
| Resource ownership | Bit-level register/all-port alias/slice and instance-member takes; applied-write/iState protection, inherited ownership and scoped release; Tessent default/persistent takes within the input stream; protected dependency batches, per-clock replay, explicit-reset ownership clearing and atomic failures | Clock/scan assignment and frequency commands, pattern-set lifecycle, broader scale and vendor qualification |
| Flattened output | Shared netlist serializer, including empty DataRegister declarations; original/flat/generated-PDL tests, including common-phase multiple-TAP instructions | Full corpus and vendor-portable equivalence for all model kinds |
| CLI/build/install/safety | Five dynamic/static executables; staged outputs, input alias protection, BSDL loading; Task137 full normal/SAN, Rocky51x820 dynamic/static and archive-build/install checks; eight editor products unchanged; four archived physical-export/model replays qualified with --physical_pdl | Remaining command semantics; changed products require fresh qualification |
| Local-reset-only artifact | Transitive ScanRegister/writable-DataRegister reset-control releases, grouped independent controls and feedback-component layers, ordered multi-file rewriting, whole main Tcl command spans, literal-site/post-argument fallbacks, evaluated-argument and recorded-invocation dispatch at repeated write/call sites; unchanged procedures, no-op strict replay and actual rewritten-runtime validation. Task143 verifies identical-argument/state-dependent caller releases after one review/fix pass, with strict joined-source/level/cleanup attestation. Tessent accepts earlier whole/sliced argument-reset wrappers | Required unchanged-body diagnostics for unrepairable internal reassertions, serial-dependent feedback and broader vendor qualification |
| Tessent oracle | Seven legacy and seventy-seven directional certificates for pin/vector, TAP, reservation, shadow storage, transitive-reset, end-state, scan-driven/mixed DataRegister, computed-port, control ownership, merged-observation, standalone client, simultaneous-chain, hidden-client, multiple-client, selected chain-group, ShiftEn-only, all-port membership, DataRegister state and data-controlled reset-mux and generated queued-state and DataRegister reset-release artifact and self-feedback reset-mux and standalone-idle cases; independently generated source schedules need not match | One-hot scan changes have local replay coverage only; merge certification covers the recorded two-stream fixture, and broader model/corpus equivalence remains unverified |
| Acceptance checks | Regression, sanitizers/style, independent review and fixed zero-op/caught-error/actual-ownership reset regressions | Strict coverage below100% lines/90% raw branches: Task137 96.68%/62.97% (17589/18193 lines,29784/47295 raw branches); item1 closure excluded by current user instruction; full vendor equivalence unverified |

Current task status is maintained in [items2–15](remaining-work.md).
Task137 also qualifies physical reset-masked scan loads on one bound TAP with
scan-only storage, including actual fill after repeated reset and bypassed
partial-X updates. Other physical-mode topologies/operations remain outside
that completed increment. All six reference adapter methods, including the
five skipped in Task137's full run, execute successfully in the Task138
pinned-reference follow-up. Dated records below retain their original results.

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

Mixed modeled child raw preparation reuses ordinary routing and full-root native
CSUs; current child length/contiguity is validated after ordinary preparation,
with child-only authored footprints and unchanged protected surrounding state.
Both source orders and disjoint batches are supported, including leaf-scoped
procedures and nested symbolic preparation. Current-route cache invalidation
after successful ordinary Apply fixes the inherited nested fallback issue.
Pure inaccessible-child execution, opaque/IR/TAP/parallel/merge and broader
reset/clock behavior remain open. Forward strict Tessent qualification passed;
reverse is unavailable in the tested vendor internal-interface context.

Verified Task107: ordinary preparation now supports qualified modeled child raw clients in either source order and disjoint batches. Existing immutable client/pin checks factored into RawClientPorts; deferred mode checks root/child metadata and operands, exact current route/length remains strict at Apply. Default strict native normalization unchanged; bool parameter rename preserves signature. Removed root-only source guard without introducing a solver/evaluator/clock/reset engine. Child-only raw authored footprint preserves surrounding/parent SIB protection, exact ownership refresh, owner return and local stable/overlap proofs. Initial RED native/5CLI14failures; first GREEN exposed stale canceled-geometry expectations at plain/default-chain plus inherited native root-only expectation, replaced with positive canceled cases and actual Apply/type rejection. Dedicated cache RED reproduced successful nested goals with stale warm Root1 versus cold Root10. Public ordinary Apply now invalidates dynamic route/segment caches after success as well as before; failed native attempt's valid pre-state cache remains on failure. New native hot/cold nested Root10/stable child9 and assignment/failure/retry/all-endpoint tests pass. Final and necessary delta reviews no unresolved Critical/Important/Minor; runtime evidence declined by reviewer and verified by executor. GREEN2 all15 fresh statuses0 and actual192 inputs verified; native scheduler0/main69/modeled77 in48.263s remote and34.027s local. Local20 connected0 (scheduler/modeled source-identical standalone checks reused), style/tidy0, corpus2557/zero duplicates/26 reviewed repeated-module groups0. Local32/remote32 matched-option scale commands0, complete candidate source/flat/generated hashes equal saved Task106 explicit-Apply references:1M route .25s remote,1024 child interfaces/512 sharedaliases .73-.74s,128 reopen/close .04s,1Mpartial32 held bits .52-.53s; memory/timing/commands and artifacts retained. Genuine Tessent2026.2 FORWARD run/strict record/check0, all source/jobinputs/artifacts/console hashes verified, local replay0,7 schema3 snapshot files imported. REVERSE vendor1 because child.local is internal ScanInterface unsupported in tested context unless inferred active for ISTG; leaf-scoped procedure also rejected. Original console retained, no reverse certificate/trace waiver. All68offline0. Frozen22 leak-enabled SAN/native7/main69/modeled77/resources/data/state/clock/reset/trace/68offline all0, fullserial make check0 (elapsed build/mixed-child-raw-final-full.wall). Final remote54 all0, actual199 hashes match;14 source-matched functional groups reused with192 input verification/explicit registry+README exclusions, fresh offline covers registry. Commit22, fast-forward master and delete completed branch; preserve raw TAP IR work/recovery artifacts. No push. Pure inaccessible-child execution without ordinary preparation, opaque/IR/TAP/parallel/merge and broader reset/clock/ownership/strict coverage/release acceptance remain open.

Verified Task108: known-binary iScan -ir on a direct scalar root client-TAP bound to one BSDL TAP, explicit or inferred unnamed. Shared numeric/default-chain parsing retains leading-ir canonical PDL; source queue offsets account for IR target/payload. Reuses ordinary IR Apply on a copied scheduler, adopts full state and publishes assigned IR only after exact one-CSU/endpoints/serial/mask/load-write-read proof. Unknown SI, raw DR, multi-TAP/parallel/child-TAP and raw merge stay unsupported. Native literal LSB-first/final-exit/N+6-cycle/all-endpoint/idle/state/transaction checks, CLI7 source/flat/generated/reset/owner/partial/queue/cancel/call/scope groups and trace17 pass. Intended native/CLI RED precedes implementation; separate trace numeric/default-chain RED observed. Initial single-lane preflight assertion and private-field compile issue corrected; no unresolved product failure. Independent final review no Critical/Important/Minor; pending runtime gates declined and verified by executor. Format/tidy0. Full serial make check0 in1435.091s, editor6 in195.861s, corpus2558/zero duplicate files/26 reviewed repeated-module groups; full checkpoint included68 directional certificates. New registry and15 genuine snapshot files then checked freshly:70 offline locally and leak-enabled SAN, oracle/golden/roundtrip harness checks all0. Original22 SAN gates all0; initial additional offline attempt under ptrace failed for LeakSanitizer environment, approved untraced rerun passes. Final remote60 statuses0, actual3903 input hashes match;53 functional/performance checks reused from all0 GREEN56 with3888 exact unchanged runtime/test/benchmark inputs and explicit registry exclusion, new certificates/registry fresh. Remote extra harness checks initially lacked formatter/inventory executables; archived failures retained, built prerequisites and reran affected checks0 without source edits. Genuine Tessent2026.2 BOTH directions run/strict record/check0; all source/job-input/artifact/console hashes verified before importing15 schema3 files. Local and remote32 matched-option scale commands each:30 successes with complete source/flat/generated byte identity and canonical whole-IR ordinary equivalence,2 expected inherited4096 scan-event rejections, no guard increase. Remote4bit .05s,32bit .09-.11s,256bit2.68-2.71s,1024nodes .54s,128repeats3.32s; copy/proof overhead measured, no speedup or1M IR promise. Original frozen17 inputs unchanged throughout full/SAN/remote checks; later changes only documentation and separately verified certificates/registry. Commit34, fast-forward master and delete completed branch, retain recoverable worktree/backups/stashes. No push. Pure inaccessible child, opaque child, raw DR/TAP/parallel/multi-TAP/merge, broader reset/clock/ownership, strict coverage and final release acceptance remain open.

Verified Task109: known-binary raw DR on one active full instrument ScanRegister on one bound scalar root TAP. Shared qualification/normalization/private TAP request and exact-one-CSU transaction proof with raw IR; read-only ActiveDataRegisters reuses hardware selection. IR/data queue keys are separate; ordinary preparation runs first in either source order. Builtin DRs remain explicitly unsupported because ordinary synthetic PDL targets are forbidden; next task addresses them through an internal-only exact-ID allowance. Native/CLI intended RED and independent trace spelling RED precede implementation. Literal TMS100/LSB-first/final-exit/N+5 clocks, all endpoints/state/idle and assigned/failure/retry checks pass. Fixture capture was made explicit after an uncontrolled-capture test mismatch. Independent review found one Important stable post-route disconnect; new native rollback and CLI constant/alternate/inverted post-input regressions RED then GREEN. Existing ActivePath now optionally requires bound TDI and compatible parity, plus exact post register/width; default callers unchanged. No new walker/FSM/solver. Final native/DR8/IR7/trace17/style checks pass. Fresh23 leak-enabled SAN gates all0; full serial make check0 in1664.5059401610051 seconds on original frozen21 inputs. Local64 and corrected remote64 matched-option DR/IR scale commands each60 successes and4 expected inherited4096 resource rejections; complete source/flat/generated bytes and canonical prior ordinary outputs match. Remote DR8 .07s,32 .08s,2561.24s,1024nodes1.00s,128repeats3.26s; no speedup or1M TAP promise. Initial remote57 statuses0 retained; after review fix, fresh GREEN2 all58 statuses0 and actual3910 inputs match. Genuine vendor initial reverse0, forward strict mismatch because vendor adds initial IR capture0001. Strengthened only the source fixture with explicit iRead tap.INSTRUCTION1; original frozen source remained immutable through full completion. Recertified BOTH Tessent2026.2 directions run/strict record/check0 with all source/job-input/artifact/console hashes verified,15 schema3 files imported. Fixture-only amendment qualified by fresh optimized/static/SAN DR8, all72 SAN/optimized offline snapshots and harness tests0; original snapshot and amendment manifest retained. Final remote61 statuses0 and3925 actual hashes match;54 source-matched functional/performance checks reused with explicit registry/fixture exclusions, affected DR8/new metadata fresh. Commit37, fast-forward master and delete completed branch, preserve3 builtin-task drafts/recovery artifacts. No push. Pure inaccessible/opaque child, builtin/multiple-register/multi-TAP/parallel/rawmerge, broader reset/clock/ownership, strict coverage and final release acceptance remain open.

Verified Task110: raw IDCODE/BYPASS reuse the existing one-data-CSU planner and proof on one bound scalar root TAP. A private call-local optional exact register ID flows through Apply/Solve/FilterTapData; normal public calls stay nullopt, public synthetic iWrite/iRead remain forbidden and no permission is stored/copied. Internal proof accesses never appear in generated PDL. No new solver/walker/FSM/public API. Native/CLI RED reaches prior unsupported builtin boundary. GREEN native independent IDCODE32/BYPASS1 captures, LSB-first/N+5 clocks/all endpoints/idle/owned IR and assigned/copy/failure checks0; CLI4, DR8 and IR7 all0. Independent review found no production defect; Important test gap addressed with valid-width failed privileged apply (incompatible protected IR), reserved-context proof, all-state/assigned rollback, original/copied BOTH-node public rejection and successful retry. Existing feature RED retained; no fabricated behavior fix. Style0. Local and remote88 benchmark commands each84 successes/4 expected inherited4096 guard rejections, source/flat/generated bytes agree. Builtin24 candidate-only measurements explicitly lack a prior comparable CLI baseline because prior CLI rejects them; no speedup claim. Remote IDCODE32 .09s/BYPASS1 .08s,1024model nodes .57/1.01s,128repeats6.37/3.37s; existing IR/DR matched-option ordinary equivalence retained. Full serial make check0 in1669.6238344970043 seconds on original frozen17, fresh24 leak-enabled SAN gates0 and GREEN remote60 statuses0 with actual3932 inputs verified. Initial vendor reverse strict0, forward strict1 only because Tessent drops -stable flags; original diff/archive retained. Vendor-only PDL fixture omits flags, while native/CLI/bench inline sources still test stable. BOTH Tessent2026.2 portable fixture directions run/strict record/check0, source/job-input/artifact/console hashes verified before15 schema3 files imported. Original freeze immutable through full; fixture-only amendment qualified freshly on optimized/static/SAN builtin4, all74 offline snapshots/harness checks0. Final remote63 statuses0, actual3947 hashes match;56 exact source-matched functional/performance groups reused with explicit registry/fixture exclusions and affected builtin4/metadata fresh. Commit33/ff master/delete branch, preserve2 multi-register design drafts/recovery artifacts; no push. Multi-register/parallel/multi-TAP/opaque child/rawmerge and broader clock/reset/ownership/strict coverage/final release acceptance remain open.

Verified Task111: full known ordered raw TAP data paths span unequal modeled registers and serial aliases/inversion. One shared TraceSerialPath preserves ordinary Replay enable behavior while raw qualification requires known enables, complete register boundaries and bound TDI. Per-register SI uses total XOR prefix parity; SO uses prefix parity with X masks preserved. One copied ordinary Apply must prove exact single data CSU/maps/endpoints/serial streams before full state adoption; assigned set remains pre-CSU path. Stable permits parity changes at unchanged complete length and rejects disconnected/alternate input. Builtin exact-ID privilege and public synthetic restrictions unchanged. Native/CLI intended RED precedes implementation; independent literal16 parity/index cases and dynamic length/protected-side-effect/failure/retry/assigned tests pass, CLI4 including16 parity subcases, DR8/builtin4/IR7. Independent review no Critical/Important/Minor; runtime judgments verified by executor. Format/tidy0. Original frozen16 unchanged through full serial make check0 in1501.9609203200089s/editor6 in170.949s; corpus2561, zero duplicate-file groups,26 reviewed repeated-module groups, five optional ANTLR tests skipped. Fresh25 leak-enabled SAN gates0. Local/remote132 benchmark commands each126 successes and6 expected4096 event-work guard rejections, exact source/flat/generated bytes and equivalent ordinary canonical outputs agree. Remote64x1 .85s/16x16=256bits4.06s/1024unrelated1.03s/128repeats4.4s/512aliases1.4s; no budget increase. Combined64x4 ordinary reference exceeded120s and candidate probe15s; original failures retained and explicit inherited fragmented-wide limit stays open for next native TAP optimization task. Initial vendor reverse strict0, forward run0/record1 only retained a4/b22 observations in partial-write group; trace diff preserved. Fixture explicitly retains those reads; comparator/runtime unchanged. BOTH Tessent2026.2 directions recertified run/strict record/check0, all source/job-input/artifact/console hashes verified before15 schema3 files imported. Fixture-only amendment after immutable full qualified freshly on dynamic/static/SAN CLI4, all76 offline certificates (optimized/SAN) and oracle23/golden17/roundtrip7 harness tests0. Final remote65 statuses0,3969 actual hashes match;58 source-matched functional/performance checks reused with explicit registry/fixture exclusions and affected CLI/metadata fresh. Original16 and qualified32 snapshots/recovery evidence retained. Integrate into master and delete completed task branch, preserving only next performance design/plan; no push. Pure inaccessible/opaque child, parallel/multi-TAP/rawmerge, broader event/reset/clock/ownership, strict coverage and final release acceptance remain open.

Selected known instrument TAP data now has a conservative concrete candidate
using shared Cycle/ReplayTapStep, existing path/parity maps and held-IR selection
proof. Joint planning remains the fallback. Final native TAP performance and
qualification evidence is recorded below when gates complete; broader event,
clock/reset, retargeting, coverage and final-release acceptance stays open.

Verified Task112: selected known instrument TAP data uses one concrete candidate before joint event planning. Existing ordered path/parity maps and shared Cycle/ReplayTapStep preserve exact capture/final-exit/update/idle timing; the optional sample occurs after pin Drive before rising edge. Canonical phase/clock forwarding plus held-IR-only SymbolicNetwork proof qualify stable selection, with all other terminals unknown. Public validation/groups/synthetic target restrictions precede unchanged dispatch. Full execution/state/fill remain on copies until actual load updates and every reserved edge pass; unsupported controls, resets, unknown fill or observations fall back. Existing unplanned non-reset fill stays unchanged even for unlogged off-path update-to-X, while actual complete hardware state is adopted; independent reviewer found no observable divergence. No new FSM/solver/walker/dependency/RTL or raised limit. Saved Task111 native64x4 source RED times out15s locally and remotely. Dedicated CLI5 includes source/flat/generated and equivalent ordinary packet plus exact partial history, physical masks, emitted differing comparisons, disabled-capture rejection and direct-reset fallback. Real all-X disabled-capture gap was RED then fixed by canonical control qualification. Existing native16 literal parity/index/all-state tests reused and freshly pass; connected multiDR4/DR8/builtin4/IR7 and format/tidy0. Final independent review no Critical/Important/Minor findings; execution judgments verified by executor. Local/remote152 measurements each146 successes+6 unchanged4096 event guards; complete artifact hashes checked. New20 are candidate-only because previous fragmented-wide planner times out; no fictitious completing-wide baseline/speedup ratio. Other132 retain matched prior ordinary comparisons. Local/remote single64x4 .42/.64s,16x16 .12/.18s,512aliases .71/1.13s,1024unrelated .29/.53s,128wide packets10.84/26.51s; peak native remote262360KiB, no15s whole128-packet or1M TAP claim. Frozen9 unchanged through full serial make check0 in1478.5179252219968s/editor6 in176.745s,26 fresh leak-enabled SAN gates0,76 offline certificates0, corpus2561/zero duplicate files/26 reviewed repeated-module groups and334/334 parser+119/119 scanner rules. Five optional ANTLR integration tests skipped. Remote64 gates0 and3971 actual input hashes match. Fresh BOTH Tessent2026.2 directions actual run/strict record/check0; source/jobinput/artifact/console hashes and current binary fingerprint verified, fresh source/trace/artifact hashes identical to existing certificates, so no duplicate snapshot rewrite. Original9 snapshot/recovery evidence retained; final changes only verified documentation. Integrate directly into master/delete finished branch and preserve future automatic-child design/plan; no push. Closed/opaque child, parallel/multi-TAP/rawmerge and broader events/reset/clock/ownership, strict coverage and final release acceptance remain open.

### Automatic modeled-child raw preparation

A source child iScan may start behind a reset-closed parent. Existing ordinary
configuration planning now emits the parent prefix before the final physical
root packet. Child-local geometry must be known; compatible children share one
configuration solve. The final route is checked after preparation, including
contiguity, length, overlap, stable claims and ownership. Parent setup preserves
protected values and does not count as authored child data.

Native NormalizeScan and ApplyScan/ApplyScans stay strict by default. A trailing
preparations output opts into setup and receives every actual prefix step.
Hardware, controlling fill and both caller outputs publish together only after
raw replay succeeds. Unsupported or ambiguous local routes retain bounded
failure. Existing SI/X, clocks/reset and resource rules remain in force.

Task113 qualification: full check0 (1538.405s),27 leak-enabled SAN gates0,
remote62 gates0 with3976 original input hashes, and88 artifact-verified benchmark
commands on each platform. Fixture-only amendment passes corpus2562 (zero
duplicate-file groups),77 optimized/SAN offline certificates, the focused static
certificate and harness checks; final remote6 gates0/3985 input hashes match.
Fresh strict forward certification uses an unknown-capture parent reusing the
existing leaf. The original self-capture vendor run adds one setup comparison
and remains unqualified; both reverse runs reject the internal interface. See
[automatic preparation evidence](pattern-generation.md#automatic-modeled-child-raw-preparation).
Strict coverage and the remaining acceptance gaps above are unchanged.

### Literal unknown raw TAP input

The qualified single-root TAP path reuses ordinary validation on a discarded
binary proof and adopts only literal ReplayTapStep/Cycle state. Unknown SI
remains unknown in hardware/output, with known-only controlling-fill refresh.
Per-edge ownership, actual end-state/stable validation and atomic outputs remain.
All-binary dispatch and existing alternative/event-work guards are unchanged.
Noncanonical selection, reset and clock histories remain outside the X profile.

Task114 qualification: full check0 (1564.5019128859858s),28 leak-enabled SAN gates0,
all7 local gates0 and52 artifact-verified measurements per platform. Remote60
source-matched gates are reused with exactly2 corrected test scripts;4 fresh
correction gates0 and3991 current input hashes match. All77 existing directional
certificates pass. Fresh Tessent2026.2 IR/DR forward/reverse all reject SI X,
with source/job-input/console provenance verified; no new certificate or full
Tessent equivalence claim. Strict coverage and remaining acceptance are open.

### Native ordinary instruction planning

A qualified known single-root pure IR goal reuses shared serial tracing and TAP
replay before joint solving. Data interfaces must forward native phase/clock
roles; data selection may be inactive duringIR. Existing validation, fallback,
actual-state/fill ownership and work guards remain. Source-family typing and
update-storage selectors remain unchanged. An unknown-initial primary end-state
replay can add an initialization on generated replay; the saved Task114 binary
reproduces this inherited issue. It remains an acceptance gap, independent of
this native performance path (which is disabled for end-state directives).

Task115 verified native ordinary IR replay: pure-IR goals reuse RawTapPath,
NativeTapControls and actual shared ReplayTapStep/Cycle, preserving outer
validation and all default data behavior. IR qualifies every modeled data
interface phase/clock but does not require active data selection. Actual final
known instruction load and writable replay eligibility replace unavailable IR
ControlEvents; existing data event checks remain. Complete trial state, execution
and known-only fill publish only after preservation/observation checks. No new
engine, public API, dependency, RTL or raised guard. Same-value, read-only,
partial/unknown recovery, actual N+6 cycles/public endpoints, protected automatic
rollback/retry and existing unsupported fallback cases pass. All30 functional
replay artifacts equal saved Task114 output. True memory RED preceded product
changes. Independent final review has no Critical/Important/Minor findings.

Both platforms98 artifact-verified measurements pass:42 matched IR commands,
4 candidate-only dense128,32 literal-X and20 known-data. Local64x4 IR source
252568-252840KiB/.44-.46s becomes48328-48500KiB/.05-.06s;512aliases
261560-261696KiB/.64-.70s becomes49924-49984KiB/.06-.07s. Local256-bit IR
259748-259812KiB/1.87-1.91s becomes48712-49088KiB/.09-.11s. Dense64x4
128 IR groups candidate .47s/49328KiB local and .94s/49528KiB remote;
its saved reference exceeds120s (8 groups also exceed15s), retained as unavailable,
not a completing baseline or speedup ratio. Matched128 uses the completing
one-register control. Runtime guards remain unchanged. No1M IR/global optimum
or15s whole-program claim; wide literal128 still21.87s local/51.38s remote.

Fresh full serial make check0 in1855.3154613880324s, editor6 in194.849s,29 leak-enabled
ASan/UBSan gates0,13 local qualification gates0 and66 remote gates0. All3994
actual remote inputs match current source. Five optional ANTLR integration tests
are skipped. Fresh Tessent2026.2 binary IR and multiple-DR forward/reverse run,
strict record and strict check all0; source/job-input/artifact/console hashes
verify, and traces/artifacts equal existing certificates. All77 directional
certificates pass; no duplicate certificate/registry entry. Literal-X vendor
rejection from Task114 remains. Original11 runtime freeze remains unchanged;
only verified documentation follows it. Merge into master/delete task branch,
no push. Inherited unknown-initial primary end-state replay extra initialization
reproduces on saved Task114 and remains open; initialized primary guard passes.
Other opaque/parallel/multi-TAP/rawmerge, event/reset/clock/ownership, strict
coverage and final release acceptance remain open.

Primary end-state replay now requires declared default bits to be known at each
emitted apply boundary in both the joint scan and primary-only solvers. The
existing override emitter retains known nondefault values on each write-emitting
apply. This
addresses the Task115 inherited extra-initialization case without requiring a
user-supplied iState. Unrelated unknown bits, temporary override lifetime,
concrete reset/control replay and reservations retain their existing rules.
Shared-address multiple-register and IR source/flat/generated replay regressions
cover both affected paths. A comparison-only frame must either match its defaults or include a proved
input event that can carry explicit overrides. No physical drive is inserted
after replay. Qualification is recorded below.

Task116 qualification: true native-IR, shared-address primary-only and
comparison-only replay REDs precede their fixes. Declared primary default bits
must be known on emitted frames; a nondefault value additionally requires a
proved input event so the existing override emitter can express it. No drive is
added after replay, no fake iState, no changed override lifetime or raised guard.
Final review found no Critical/confirmed Important initially; its declined
comparison-only boundary was reproduced and re-graded Important, then fixed in
one RED/GREEN pass. Documentation's override scope was corrected. All15 end-state
tests, expanded known/unknown/default0/1 TAP replay, indexed/inverted masks,
unrelated unknown-port and existing reset/reservation cases pass. Fresh full
serial make check0 in1820.7168697050074s, editor6 in189.933s,32 leak-enabled SAN gates0,
17 local qualification gates0,66 remote gates0 and3994 current input hashes.
Both platforms98 measured native-profile artifacts equal Task115 byte-for-byte;
no new speedup claim. All77 existing directional certificates pass on the current
candidate. Four Task115 genuine binary IR/multiple-DR vendor proofs are reused
with unchanged fixture source/input/artifact provenance and current offline
checks; no new vendor fixture or certificate, no new literal-X equivalence.
Seven-file runtime freeze remains unchanged; verified documentation only follows.
The inherited unknown-initial primary extra-initialization gap recorded in
Task115 is resolved for these supported joint/primary cases. Other opaque,
parallel/multi-TAP/rawmerge, event/reset/clock/ownership, strict coverage and
final release acceptance remain open. Integrate master/delete branch; no push.

### Rocky Linux 8 refresh (2026-11-17)

Source-archive qualification now covers implementation through `f018bb2`.
All 4,203 shipped tracked files match that clean checkout; only `.gitignore`
is excluded. The six additional files are the generated parser/scanner sources,
headers, location header and `.prebuilt` marker. The source archive SHA256 is
`d3bd6f48b720ff18ed03e099d1d0cf8c724311e75219fa8ae05d5e526cae2e9a`; the five-binary static archive SHA256 is
`461682b1cfa823006415af8c8891fe5cf83978671a29d3c49c391bc54eb3a2f6`. Both transferred archive hashes pass verification.

The unchanged archive builds on Rocky Linux 8.10 with Bison/Flex disabled and
no static Tcl archive. Existing pinned Make/GCC/Z3 dependencies are reused
read-only. Exact toolchain versions and dynamic library resolution are in
`build/qualification-f018bb2/remote-evidence/environment.log` and
`dynamic-libraries.log`; all three Tcl-linked dynamic tools use system Tcl8.6.
All 16 remote gates pass, including native tests, both five-tool installations,
PDL/LSP/pattern/reset replay, both installed ICL LSP runs (62 checks each), and
manifest uninstall that preserves unrelated sentinels. Every transferred static
ELF lacks an interpreter and its installed hash matches the local archive.
Each remote generator variant passes all 51 suites and 776 tests. Static tools
are transferred local builds; this is not a remote static source build.

Fresh local dynamic/static installations also pass all 11 gates, with matching
binary fingerprints and sentinel-preserving uninstall. All 3,994 source/test
input hashes remain identical to Task116. Its full local regression
(1820.716870s; editor six tests189.933s), 32 leak-enabled sanitizer gates and
77 directional certificates are reused explicitly. No product code, dependency,
user installation or editor configuration changes. Exact commands, archive
membership/content, fingerprints and raw remote results remain under
`build/qualification-f018bb2/`. This refresh does not claim a remote full
`make check`, a new Tessent certificate, complete model support or the still
unmet 100% line/90% raw branch coverage gate. No push.

### Authored-code coverage refresh (2026-11-17)

The complete current instrumented run at `94bc043` measures 17,300/17,961
lines (96.3%) and 29,171/46,677 raw branches (62.5%). The unchanged 100% line /
90% raw branch gate fails: Make exits2 only after the final strict report.
All prescribed instrumented checks completed, including 58 Python unittest
groups (938 cases; the generator subset is51 groups/776 cases),1916 regression
cases and2180 conformance cases. Native checks also passed. The regression
log's heading “FAIL: invalid ICL must be rejected” labels expected negative
inputs; its result is PASS=1916/FAIL=0. This is not a passing strict gate.

The fresh build uses GCC/gcov16.2.1, `-g -O0 -fprofile-arcs -ftest-coverage`,
serial suites and at most2 compiler jobs. Wall time is1303.821085s. All54
required authored-source reports are present, and all3994 source/test input
hashes still match the qualified implementation. Old profiles were not mixed.
Raw totals retain never-executed, compiler exception and cleanup edges. Of17506
untaken raw edges,12237 are compiler-marked throw edges and5269 are other edges;
these annotations do not remove anything from the denominator or prove edges
unreachable. No filtered score, weakened threshold or new failure machinery.

Persistent evidence is `build/current-coverage-audit-94bc043/`: full log,
command/environment/input hashes,19 instrumented binary fingerprints, all raw
reports, per-file analysis and artifact hashes. No product or test source
changed. Task116 normal full regression/SAN32 and Task117 five-tool platform
results are reused only for the identical implementation. Coverage measurement
completion does not establish complete model support or vendor equivalence.

Concrete next test targets are existing generator argument/input failures
(missing files/values, unknown options, unreadable text), staged-output API
misuse and publication failure (repeat Prepare, Publish without a stage,
destination replaced by a directory), and numeric/target API validation
(malformed tokens/ranges and oversized values). These are real behavior and
output-preservation checks, using existing test runners. Larger remaining line
gaps are scheduler189, flattening57, PDL document50, netlist49, generator48 and
runtime46. Closing braces, initializer attribution, internal contract guards
and throw/cleanup edges need individual source review; a percentage alone is
not a reason to delete validation or invent behavior. The strict coverage
requirement and broader model/vendor work remain open. No push.

### CLI and staged-output coverage (2026-11-17)

Two new cases in the existing generator CLI suite cover argument/input errors
and preservation of generated, reset and debug artifacts. NUL cases use valid
ICL/PDL plus NUL and require the input-reading diagnostic; a parser error cannot
satisfy that check. Existing native tests now exercise staged publication without
Prepare, repeated Prepare, successful publication, abandoned stages and real
rename failure with directory/sentinel preservation. Native checks increase
from 60 to 78. The existing unit target adds its missing file_output.h header
prerequisite; a make what-if check changes from no rebuild to a scheduled compile.
There are no production source, product recipe, flag or external dependency changes.

The corrected final snapshot passes all 71 generator cases in local dynamic,
static and leak-enabled ASan/UBSan runs and both Rocky variants; native normal,
SAN and Rocky checks pass 78/0. Format/tidy pass for the changed C++ sections.
An unchanged legacy whole-file tidy finding remains outside this change.
One independent final review found the NUL fixture weakness and broad empty-list
diagnostics; one correction pass fixes both before final qualification.

The complete fresh instrumented recipe takes 1239.545330s with
GCC/gcov16.2.1, -g -O0 -fprofile-arcs -ftest-coverage, serial suites and at most
two compiler jobs. All 58 Python groups/940 cases pass, including 51 generator
groups/778 cases, plus native checks, 1916 regression and 2180 conformance cases.
All 11 selected previously-zero source guards execute. The final strict gate
still fails: 17,311/17,961 lines (96.4%) and 29,201/46,677 raw branches (62.6%), against unchanged 100%/90% thresholds. Make exits 2
only at that final report. No raw edges are filtered or waived. Missing raw
edges include 12,235 compiler-marked throw edges and
5,241 other edges; these labels do not prove unreachability.

Persistent evidence is build/cli-output-coverage-20261117/: complete log, 54
authored-source reports plus raw library reports/profiles, command/environment,
3994 input hashes, 19 instrumented binary fingerprints, focused and remote
results, review summary, analysis and artifact hashes. Only Makefile and the two
existing test inputs differ from Task118; all product source and ten local and
remote binary fingerprints remain unchanged. The initial pre-review full run
is preserved separately and is not the final qualification snapshot.

Task116 normal full/SAN32 and Task117 platform gates are reused for identical
product bytes, with the changed groups freshly qualified here. No fresh Tessent
run or performance benchmark is claimed for this test-only change. The strict
coverage requirement and broader model/vendor gaps remain open. No push.

### Public operand API coverage (2026-11-17)

The existing native expression and symbolic runners now exercise public numeric
and target validation directly. Malformed token spans/literals/operators and
target ranges/indices require the intended runtime-error diagnostic. Valid
parameter arithmetic, target declaration/alias ordering and exact supported
numeric width/text boundaries remain accepted. The largest text is 1MiB+3
characters for a just-over-limit rejection; valid payloads are at most one
megabit. Cases run inside the two existing native processes with bounded linear
work and a few MiB transient data; no product speedup is claimed.

Small caller-created public IR copies exercise oversized targets, aliases with
missing/cyclic sources and an empty enum target. These cases establish API
validation, not acceptance of those models through the ICL parser. Product
source, recipes, targets, flags and external dependencies are unchanged.

Both native runners pass normal, leak-enabled ASan/UBSan, instrumented and Rocky
checks. Changed-section format/tidy and strict C++17 test-source syntax checks
pass. A new structured-binding capture was replaced with a C++17 pair reference;
an unchanged production scheduler capture warning stays outside this test-only
scope. The first focused command retained old binaries because COV_DIR was
omitted; its guard assertions correctly failed. The corrected Make target
selection rebuilds both test runners and hits all 18 designated guards. Initial
logs/reports and Task119 profiles/raw reports/old native binaries are preserved.

The complete unchanged coverage recipe clears profiles and uses GCC/gcov16.2.1,
-g -O0 -fprofile-arcs -ftest-coverage, serial suites and at most two compiler
jobs. It reuses 17 identical instrumented binaries and rebuilds the two changed
native runners. Wall time is 1344.132797s. All 58 Python groups/940
cases pass, including 51 generator groups/778 cases, plus native checks, 1916
regression and 2180 conformance cases. The strict gate remains failed:
17,331/17,961 lines (96.5%) and 29,262/46,677 raw branches (62.7%), against unchanged 100%/90% thresholds. Make exits 2 only at the
final report. All raw exception/cleanup edges stay included: missing edges
include 12,232 compiler-marked throw edges and
5,183 other edges. No unreachability proof or waiver.

Evidence is build/public-operand-coverage-20261117/: full log, 54 authored-source
reports plus raw library reports/profiles, command/environment, 3994 input
hashes, 19 instrumented binary fingerprints, focused/remote data, analysis and
artifact hashes. Only the two native test inputs differ from Task119. All ten
local and ten remote product binaries remain unchanged. Task119 CLI71/native78,
Task116 full/SAN32 and Task117 platform gates are reused only for unchanged
bytes, with the changed native groups freshly qualified here. No fresh Tessent
run or product benchmark is claimed. One independent final review found no Critical, Important or Minor findings. The scoped test task is qualified for
integration; strict coverage and broader model/vendor work remain open. No push.

### Scheduler C++17 portability (2026-11-17)

The raw-scan overlap callback now captures an ordinary const reference to the
same map key. Its previous structured-binding capture required a C++20 extension
despite Makefile's C++17 setting. The first interval-checking loop uses only the
key; the unused operation binding is removed. The map outlives the synchronous
callback, and diagnostics, ordering and the later processing loop are unchanged.
No copy, allocation or new control flow; interval work remains O(C log C) time
and O(C) space for C clients. There is no product speedup claim.

The strict Clang C++17 probe changes from exit1 to exit0; changed-section format
and tidy pass. Existing automatic-child, multiple-interface and chain-group
suites pass39 cases per local dynamic/static variant, plus a leak-enabled
overlap test. Rocky GCC15 syntax/build/identity/client checks pass7 gates and39
cases. All3994 input hashes match Root/worktree/remote, with only scheduler
source changed from Task120. All ten local and ten remote product binaries are
byte-for-byte identical to their preserved originals after rebuilding.

Evidence is build/scheduler-cpp17-20261117/: compiler RED/GREEN, local/remote
logs/exits, source/input/product hashes, original generators/source and artifact
hashes. Exact unchanged artifacts support reuse of Task120/119/116/117 runtime
and platform checks. No fresh vendor run or product performance benchmark.
Task120 remains the last complete pre-edit raw coverage measurement at
17331/17961lines (96.5%) and29262/46677branches (62.7%). The declaration edit
shifts source attribution; no post-edit gcov metric is claimed or inferred from
those reports. Strict100%lines/90%rawbranches and broader model/vendor work
remain open. One independent final review found no Critical, Important or Minor findings. The portability fix is qualified for integration. No push.

### Oscillator omitted-source audit (2026-11-17)

Seven isolated Tessent2026.2 probes distinguish a factor-only override from a
subsequent clock check. They reuse the unchanged explicit-oscillator driver and
model: child.osc has Period10ns, and root is configured at100ns. The context is
patterns -ijtag / analysis with an open pattern set. All21 fetched input hashes
match the submitted files.

| Pattern | Vendor result | Observed stage or exported waits |
| --- | --- | --- |
| Existing explicit-source control | exit0 | child.osc2, child.osc2, root1 |
| Multiplier2, then fresh iClock | exit1 | iClock fails with undriven child.osc source |
| Divider3, then fresh iClock | exit1 | iClock fails with undriven child.osc source |
| Explicit source/divider3, then multiplier2 and fresh iClock | exit1 | final iClock fails with undriven child.osc source |
| Explicit source, then optionless override | exit1 | iClockOverride syntax rejection |
| Multiplier2, retain previous clock check | exit0 | child.osc4, child.osc4 |
| Explicit source/divider3, then multiplier2, retain previous check | exit0 | child.osc4, root12, root12 |

The override itself is accepted in the factor-only cases. A previous checked
clock remains usable in the two cached-wait patterns; those patterns at the audit revision
failed in our tool at the earlier explicit-source guard (exit2). This is an
identified compatibility gap, not a completed clock feature. The guard stays
unchanged in this documentation-only audit; a separate implementation must
preserve the cached duration without inventing an intrinsic frequency or
self-referencing clock source. Fresh-check failures and optionless rejection
must remain distinct from the successful cached-wait paths.

IEEE1687-2014 clauses7.9.15b and7.9.16c-e (printed174-175/PDF188-189) define
oscillator roots and optional override fields/defaults. Vendor failure does not
prove an IEEE prohibition. The v2023.4 Shell Reference Manual iClockOverride
topic (Command_Iclockoverride_id87a56b58.html) describes different omitted-field
behavior; it is not evidence for measured2026.2 behavior. IEEE-only intrinsic
restoration and general oscillator override semantics remain unqualified.

Evidence is build/oscillator-omission-probe-cee5dcf/ and its snapshot-probes/:
original inputs, raw logs/exits/exports, native gap reproductions, reference
hashes and audit facts. Negative runs have no export. The existing native
oscillator-target rejection method passes dynamic/static/leak-enabled SAN;
that verifies the present restriction and does not close the cached-wait gap.
All3994 runtime inputs and20 product fingerprints remain unchanged. No formal
certificate, broader equivalence, post-edit coverage or performance claim.
One independent final review found no Critical, Important or Minor findings. The audit recorded the cached-wait gap; the following implementation closes that bounded gap. Strict coverage and
broader model/vendor requirements remain open. No push.

### Oscillator cached waits (2026-11-17)

Factor-only overrides of a Period ToClockPort now preserve waits using an
earlier iClock check, including a check that previously selected an external
source and ratio. This closes the two native gaps measured in the omission
audit. The latest omitted ICL source may be missing; a fresh iClock still fails
on that inactive route, matching the measured Tessent2026.2 patterns. Optionless
Period overrides stay rejected, and unconnected non-Period targets still need
a source. No intrinsic frequency, self-reference, absolute-time or event model.

The change reuses existing versioned definitions and checked routes, with no
new API/header/cache/state representation. Native source/flat cases prove saved
ratios survive a failed fresh check, invalid source/factor updates publish no
partial change, an explicit source recovers, other vector bits stay independent,
and original period metadata/hardware remain unchanged. Two new CLI methods
cover multiplier/divider variants through five replay forms; existing fresh
failure checks preserve all previous output artifacts. Native --clock-snapshots
selects existing clock tests for focused sanitizers; default execution retains
the full tests. Actual CLI/native RED failures precede the two-guard repair.

Dynamic/static clock suites each pass53 methods; leak-enabled ASan/UBSan passes
the native snapshot selector, all53 clock and71 main CLI methods. Changed C++
format/tidy and strict C++17 syntax pass. The4096-stage/1024-wait and65536-bit
oscillator workloads remain within the existing45s limit; measurements are in
the clock logs. History remains one record per override, with the existing ordered-map route
lookups and binary search within each bit's history. No product speedup claim.

Complete normal integration passes in 1871.732309s. The fresh
complete raw coverage recipe takes 1389.920953s, with all58
Python groups/942 cases (51 generator groups/780 cases), native checks,1916
regression and2180 conformance cases passing. Strict coverage still fails:
17,333/17,963 lines (96.5%) and 29,260/46,667 raw branches (62.7%), against unchanged100%/90% thresholds. Make exits2 only at the
final report. All54 authored reports and raw compiler exception/cleanup edges
are retained; untaken edges include 12,230 compiler-marked
throw edges and 5,177 other edges. No filtering or waiver.
Optional ANTLR checks in normal integration remain skipped when unavailable.

Rocky qualifies the current3994 inputs, updated native tests and both generators:
51 suites/780 cases pass per dynamic/transferred-static variant. Dynamic builds
from current source; the transferred static generator matches the Root hash and
has no ELF interpreter. Only the two generator products change; other eight
product bytes remain unchanged on each platform. This is generator qualification,
not a fresh five-tool install/source-archive refresh.

Both new normalized programs pass fresh Tessent2026.2 retargeting with the
unchanged driver, and vendor exports replay successfully through the native
tool. Their matched wait traces are child4/child4 and child4/root12/root12.
These scoped probes add no formal directional certificate or general oscillator
equivalence claim. Evidence is build/oscillator-cached-20261117/: normal/SAN and
raw coverage logs/profiles,54 reports,3994 input hashes,19 instrumented binary
fingerprints and product fingerprints, remote and vendor data, comparison and artifact hashes. Only clock
source and two existing tests differ from the qualified audit runtime inputs.
Independent final review found no Critical, Important, or Minor issues in the scoped cached-wait repair. The scoped cached-wait feature is qualified for integration. Strict and broader model/
vendor requirements remain open. No push.

### Cached-wait package qualification (2026-11-17)

Source-archive and five-tool qualification now cover clean master `2b5eec1`,
including the cached oscillator-wait repair. All 4,217 shipped tracked files
match that checkout; only `.gitignore` is excluded. Six generated parser/scanner
sources/headers/location/`.prebuilt` extras are recorded separately. Source
archive SHA256 `7cfbde0b39dbec6b0b7b97efd2c6d2a947685dadffbb7ae0855babfaa5b44ebb` and five-binary static archive
SHA256 `e8c794849779a0b352c6e1d6cfee96623ab3ac45b62091872b26c07f1eff41df` are verified before extraction on Rocky Linux 8.10.

The fresh archive builds five dynamic tools with Bison/Flex disabled and static
Tcl unavailable, reusing pinned Make/GCC/Z3 dependencies read-only. All 16 remote
gates pass: native netlist/runtime/flatten targets, installed versions and PDL/pattern/reset replay,
installed ICL LSP 62 checks per variant, dynamic libraries, transferred-static ELF
checks and sentinel-preserving manifest uninstall. Both generator variants pass
all 51 suites/780 cases. All five transferred static executables match the local
hashes and lack ELF interpreters; these are local builds, not remote static
source builds. Environment and exact system Tcl 8.6 resolutions are preserved. Initial
extraction/build clock-skew warnings (~21s) are retained; all objects were built
in the fresh extraction and all build/test gates exited 0.
All 11 fresh local temporary installation gates pass for the same ten products.

All 3,994 runtime input hashes match Task 123 exactly. Its complete normal
integration passes (1871.732309s, including six editor tests), focused leak-enabled
ASan/UBSan native/clock (53)/main (71) checks and fresh raw coverage are reused by
hash. Optional ANTLR checks remained skipped when unavailable.
Coverage remains FAILED at 17,333/17,963 lines (96.5%) and 29,260/46,667 raw
branches (62.7%), against 100%/90%; no waiver or new coverage result is claimed.
No product code, test, dependency, user installation or editor config changes.
Raw commands/statuses, archive membership, hashes and remote/local evidence are
in `build/qualification-2b5eec1/`; Task 123 evidence remains immutable in
`build/oscillator-cached-20261117/`. This does not claim a full remote make check,
a new vendor certificate, or complete model support. No push.
Independent final review found no Critical, Important, or Minor issues in this package qualification refresh. The current package qualification record is ready for integration.

### Raw port ownership audit (2026-11-17)

A documentation-only audit at `26ce069` measures established port ownership
around root raw scans. IEEE1687-2014 sections 7.9.12 and 7.9.19 (printed 168–169,
178–179) describe full network control by iScan and the distinction between
explicit PDL assignments and retargeter changes to taken resources. The local
Tessent Shell Reference Manual v2023.4 iTake topic idd8a68c5f also documents
establishment by iState; its iScan topic idb66e0799 describes scan loads after
ordinary retargeting and modeled state simulation. These older references are
kept separate from the following measured Tessent 2026.2 results.

Ten isolated programs reuse the unchanged qualified ReservedRawTop driver and
model, adding only scalar DataOutPort hold (constant 1) and held (value[0]). The
initial model with an extra DataInPort and wide DataOutPort was rejected by the
native raw-profile guard before reservations; those attempts are preserved and
are not counted as ownership results. The corrected control passes both tools.

| Corrected program | Tessent 2026.2 | Current native dynamic/static |
|---|---|---|
| Baseline reset and eight-bit raw load | Export succeeds | Both pass |
| Taken select established at 1 or 0, then raw load | Both exports succeed | Both reject at final Apply: ScanRegister-storage-only reservation guard |
| Taken constant output established by applied write, then raw load | Export succeeds | Same raw reservation guard |
| Taken observed value[0] established at 0; raw 0x5a or 0xa5 | Both exports succeed | Same raw reservation guard |
| Taken ScanInPort si, then iWrite si 0, before raw 0 or transient-changing raw data | Both fail at setup line 5: invalid iWrite target | Both fail at that write; raw reservation behavior is not reached |
| Taken inactive select, successful raw load, then ordinary iWrite value 1 | Fails later Apply at line 9, reporting iTake sel value 0 | Rejects earlier raw Apply at line 7 |
| Taken observed output, successful raw 0xa5, then ordinary iWrite value[1] 1 | Fails later Apply at line 10, reporting iTake held value “-” | Rejects earlier raw Apply at line 8 |

There are six vendor successes and four stage-specific failures. Five vendor
successes expose native source-acceptance differences at the raw guard. The two
follow-ups show that acceptance of a raw operation does not establish release
of the earlier reservation: later ordinary retargeting still reports its iTake
constraint. The held-output diagnostic does not specify a binary locked value;
this audit does not infer that value. In particular, accepting a raw operation
must not be generalized into automatic release or refresh of every port lock.
The original native unsupported test remains an implementation boundary, not an
IEEE prohibition. This audit does not choose a general port-ownership policy.

All 30 submitted/fetched input hashes match. Vendor calls run serially with 120s
timeouts and complete/error anchors. Initial SSH launch timed out locally after
30s; the existing run was inspected and fetched without relaunching it. Actual
probe exits, not transport status, determine results. All six vendor exports
replay through both current native variants after an explicit call to their
exported procedure. This is replay evidence, not a new directional certificate
or a full ordered-group equivalence claim. Native failed runs retain their
previous result artifact. Exact programs, logs, stages, hashes and exports are
in `build/raw-port-ownership-probe-26ce069/`; first-model evidence is adjacent.

All 3,994 runtime inputs and ten local product fingerprints remain unchanged.
Task 123 full integration/SAN and Task 124 five-tool platform evidence are reused
by hash; no new full regression, coverage or performance measurement is claimed.
Strict coverage remains unmet at 96.5% lines and 62.7% raw branches against 100%/90%.
General port/raw ownership, broader model/vendor equivalence and other recorded
acceptance work remain open. No product/test/Makefile edits or push.
Independent final review found no Critical, Important, or Minor issues in this documentation-only ownership audit. The bounded raw port ownership audit is ready for integration.

### Direct root raw port ownership (2026-11-17)

Direct requests for the qualified standalone root client now accept the five
port-reservation source cases measured in the preceding audit. The explicit
root CSU may change ports in that network; their established reservations keep
their earlier values and still constrain later ordinary retargeting. For example,
a taken select assumed 0 may be driven by the root raw scan, while a subsequent
ordinary write still fails until its ownership constraint is explicitly changed
or released. This separates explicit iScan effects from retargeter preparation
under IEEE1687-2014 section7.9.12 and the bounded Tessent 2026.2 observations.
No general child/TAP ownership policy is implied.

One private flag identifies the requested root client after interval validation.
ReplayScan validates every original reservation mask before omitting port IDs
from its held-state checks for that root operation. ScanRegister protections,
unknown-bit rejection and original reservation values remain intact; assignment
footprints still contain only explicitly addressed pre-route registers. Child
requests retain the storage-only guard, including a measured child interval
covering exactly all eight root storage bits. Invalid masks and failed later
retargeting preserve state and caller footprints; explicit release recovers.
No public API/header/cache/dependency or per-bit replay loop is added.

Existing CLI/native tests first fail at the old storage guard, then pass with
the bounded change. Original/flat/generated/reset replay covers five vendor
positive source gaps, inverted output aliases, retained later constraints,
release/reset/explicit rebase and child rejection. Full modeled-root suites pass
80 methods per dynamic/static variant. Leak-enabled ASan/UBSan passes the focused
native reservation function, all 80 modeled-root and 71 main methods. The native
function also covers source/flat owner history, register-only assignments,
invalid port mask validation before filtering and rollback. Its new test-only
--raw-reservations selector reuses the default suite function. Changed C++
format/tidy and strict C++17 checks all pass.

Complete normal integration passes in 1533.315018s,
including six editor tests in 174.471s. The existing raw-reservation benchmark
passes 88 rows across dynamic/static: 1,048,576 active bits and up to 1,025 registers,
wide/partial ownership and 1,024 protected masks, three-run old plain references,
current plain/reserved forms, flat and generated replay. Physical packets match
within every scenario. Maximum measured process time/RSS is 0.50s/203,020KiB for
dynamic and 0.48s/201,732KiB for static. Per-row commands, sizes, platform and binary
hashes are retained; no speedup claim. Filtering adds at most one ordered-map
erase per protected port; the existing full-shift acceleration is retained.

The fresh complete raw coverage recipe takes 1118.165168s.
All 58 Python groups/945 cases (51 generator groups/783 cases), native checks,
1916 regression and 2180 conformance cases pass. Strict coverage still fails:
17,336/17,966 lines (96.5%) and
29,268/46,679 raw branches (62.7%), against unchanged 100%/90%
thresholds. Make exits 2 at the final report. All 54 authored reports and raw
compiler exception/cleanup edges remain included; untaken edges comprise
12,233 throw and5,178 other edges. No filtering or waiver.
Optional ANTLR checks remain skipped when unavailable.

Rocky qualification passes all seven gates, including current native checks and
51 suites/783 cases per dynamic/transferred-static generator variant. All 3,994
submitted source/test hashes match the remote files. Only the two generator
product bytes change; the other eight products remain unchanged on each platform.
The transferred static generator matches the local fingerprint and lacks an ELF
interpreter. This is current generator qualification; five-tool installation and
source-archive qualification from Task 124 predate this implementation change.

All five normalized programs retarget successfully in fresh Tessent 2026.2 runs
using the unchanged driver; exports replay through the current native tool and
all raw payload traces match the preceding audit. After the review correction,
all 20 submitted program/driver/model hashes remain identical, so the first
Tessent results are reused and every export is replayed through the corrected
native binary. Canonical replay bytes match
the two select cases. The constant-output export removes an empty pre-scan
Apply; both observed-output exports add an ordinary pre-scan capture comparison.
Those differences are retained, so full ordered-group equality and a new formal
directional certificate are not claimed. Existing broader vendor/model gaps
remain open. Evidence is `build/root-raw-port-20261117/`: frozen inputs, raw
coverage/profiles, local/remote/vendor logs, 88 benchmark rows, equal-span child
checks and fingerprints. Only scheduler source and two existing tests change
among the 3,994 runtime inputs. No push.
The independent review found one Important missed caller: TryNativeScan passed
its desired-map pointer into the newly inserted boolean parameter. A one-line
caller correction now passes false and the exact desired map. A 65,536-bit
ordinary explicit update of a taken register bit timed out 15s on the reviewed
candidate and passed five forms in 0.393004s after the correction; the pre-feature
reference passed in 0.400905s. The new maintained wide regression runs in every
current 80-method suite. All source-dependent qualification above was repeated
after this fix; first-candidate evidence is retained before-review. No second
independent review was requested. Independent final review found one Important missed caller; the executor resolved it in one fix pass with RED/GREEN evidence and refreshed source-dependent qualification. No unresolved Critical or Important issues remain. The scoped root raw port feature is qualified for integration.

### Current package identity and installation (2026-11-17)

The clean source archive now covers `f75edb8`, including the root raw port
ownership repair and the review correction that preserves ordinary reserved-
write native scheduling. All 4,223 shipped tracked files match that commit;
`.gitignore` is the sole exclusion and six generated parser/scanner/marker
extras are identified. Source archive SHA256 `c7191ce2f8edd687fb678627d99de141361f200610d0946fe6761af467a2923b`;
five-product static archive SHA256 `6e065010e4776a45192dd07dd2089c1a77ac0e98ce6f20df725a5e5e3a7346dc`.

Fresh temporary installations pass 11 local and 14 remote gates. Both dynamic
and transferred-static installations exercise all five versions, PDL tools,
original/flat/generated/reset generator replay, installed ICL LSP 62 checks per
variant, and manifest uninstall preserving unrelated sentinels. Remote library
checks resolve the three Tcl-linked dynamic products to system Tcl 8.6; all five
static products have no ELF interpreter and match the local bundle fingerprints.
No user installation, dependency, editor configuration or product code changes.

All 4,013 non-documentation archive files, including generated parser/scanner
and `.prebuilt`, match the current qualified remote source tree. All 3,994
runtime inputs and ten local/ten remote product fingerprints match Task 126.
This exact identity reuses its current build/native checks and 51 generator
suites/783 cases per dynamic/transferred-static variant. Source-archive content
identity and fresh installations are checked here; the remote checkout is not
rebuilt from this new archive and the full CLI suites are not rerun here.

Task 126 complete normal 1533.315018s (six editor checks 174.471s), leak-enabled
SAN native/80 modeled-root/71 main, 88 matched performance rows and fresh raw
coverage are reused unchanged. Strict coverage remains FAILED at 17,336/17,966
lines (96.5%) and 29,268/46,679 raw branches (62.7%), against unchanged 100%/90%.
Optional ANTLR checks remain skipped. No new vendor certificate or broad model/
vendor completeness claim. Exact archive contents, 4,013 remote file hashes,
products, commands and installation logs/statuses are in `build/package-f75edb8/`;
underlying runtime evidence remains `build/root-raw-port-20261117/`. No push.
Independent final review found no Critical, Important, or Minor issues in this current-package record. The current package qualification record is ready for integration.

### Bound differential clock inputs (2026-11-18)

Relative system-clock waits now support explicitly connected differential child
inputs when their modeled routes prove complementary. For example:

```icl
Module Pair {
  ClockPort positive;
  ClockPort negative { DifferentialInvOf positive; }
}
Module Top {
  ClockPort root;
  Instance pair Of Pair {
    InputPort positive = root;
    InputPort negative = ~root;
  }
}
```

`iClock pair.negative; iRunLoop 7 -sck pair.negative` emits a seven-cycle wait on
`root`. Aliases, bit permutations, selected ClockMux paths, shared frequency
converters and oscillator roots use the same proof. Both the declared positive
reference and the physical negative binding are traced. Same-phase bindings,
unrelated roots, incompatible ratios and distinct conversion origins are
rejected; matching frequency alone does not prove complementarity. A non-unit
frequency conversion starts a new modeled phase origin. Unit-ratio forwarding
retains that origin and tracks explicit inversion. This checks the ICL clock
model; it does not establish physical waveform timing or CDC signoff.

IEEE1687-2014 section6.4.6.19, printed102–103, defines the differential link;
section6.4.6.20, printed103–104, permits clock conversion across hierarchy.
Section6.3.12(e), printed71, excludes already-negative declarations as
clock_signal source terminals, so these connections use legal positive sources
with explicit inversion. Existing source-kind checks remain intact.

The versioned route cache now records modeled phase origin and inversion parity.
An iterative dependency walk validates both branches before caching a bound
input. It avoids recursion and repeated shared-route walks; costs remain
O((V+E)log V) time and O(V+E) storage per invalidated cache version. Checked
historical overrides, applied selector revalidation, failed-check recovery,
held-clock restrictions and hardware-state isolation remain covered.

Dynamic/static clock suites pass59 methods each; leak-enabled ASan/UBSan passes
all59 clock methods,71 main methods and the focused native snapshot/binding
checks. Changed C++ formatting, clang-tidy and strict C++17 checks all pass.
The complete normal integration passes in1829.958s, including1916 regression
cases,2180 conformance cases, all334 parser and119 scanner rules over2549 files,
and six editor tests in201.434s. Optional ANTLR checks remain skipped.

A4096-pair/4096-stage shared graph (615278 input bytes),1024 waits and five replay
forms completes in1.598264s dynamic/1.524562s static, with peak process RSS125500/
125424KiB. Existing4096-stage/65536-bit clock fixtures remain within their45s
subprocess limit. Commands, environment, binary hashes and baseline measurements
are retained; no speedup or global performance claim is made.

Fresh coverage takes1357.151s. All58 Python groups/951 cases pass, including
51 generator groups/789 cases, native checks,1916 regression and2180 conformance
cases. The strict gate still fails with Make exit2:17368/17997 lines(96.5%) and
29316/46739 raw branches(62.7%), against unchanged100%/90% thresholds. All54 authored
reports and19 instrumented binaries remain included; missing edges comprise
12244 compiler-marked throw edges and5179 other edges. No filtering or waiver.

Rocky Linux qualification passes all seven gates: current GCC15 native build,
focused/full native suites,51 groups/789 methods per dynamic/transferred-static
generator, and static ELF verification. All3994 submitted runtime hashes match.
All14 fresh remote installation gates pass for five commands per variant,
libraries, LSP checks and uninstall sentinel preservation. The two generator
products change remotely; the other eight remain byte-identical. Locally the
unchanged editor sources are rebuilt with GCC16.2.1-3 rather than the baseline
16.2.1-2; all ten current local products have fresh fingerprints and full
integration evidence. Generated parser/scanner sources remain identical.

The clean272827f source archive ships4227 tracked files plus six generated
parser/scanner/prebuilt files. Its fresh local dynamic build runs with Bison/Flex
and static Tcl unavailable and passes installed five-tool smoke checks. All12
local package gates pass, including dynamic/static installation, installed PDL/
pattern/reset/LSP checks, static ELF checks and sentinel-preserving uninstall.
The4013 non-doc archive files match the current qualified Rocky source tree;
current remote compile/runtime results and14 installation gates are reused by
that exact source/product identity. This is not a fresh remote archive rebuild.
Documentation-only revisions retain the same qualified non-doc payload.

Independent final review found no Critical, Important or Minor issues and
approved this scoped native change. The reviewer inspected supplied current
source/test/contract snapshots and selected a focused native check through a
read-only proxy because its tool host was unavailable. Broader evidence is
verified by the executor; coverage and vendor portability remain open.
No fix pass or second review was needed.

Evidence: `build/bound-differential-20261118/`. General clock events, independent
phase relationships without a common modeled origin, time-dependent instrument
behavior and full vendor equivalence remain outside the qualified boundary.

Tessent2026.2 rejects all three native inverted-positive binding fixtures during
`set_current_design` with ICL65-1, before executing PDL. Its diagnostic requires
a negative input to use a declared secondary ClockPort/ToClockPort source (or a
constant). The literal IEEE6.3.12(e)/6.4.7(h) clock_signal rules instead exclude
already-negative source declarations; the native source validator currently
rejects that vendor binding form. These are explicit contextual compatibility
gaps. The three runs supply no directional certificate or vendor export replay;
no Tessent qualification is claimed for this native feature. Inputs, hashes,
stage-specific diagnostics and exit1 statuses are retained in the vendor evidence.


## Task129 secondary clock source qualification (2026-11-18)

See [the current behavior, complete evidence and limitations](pattern-generation.md#secondary-clock-input-bindings-2026-11-18). Named secondary parent inputs/direct-child outputs are admitted only on negative ClockPort Sources; other contexts and scope guards remain unchanged. Native source/flat metadata and63 clock methods pass with full normal, leak SAN, current Rocky51x793 suites per variant and archive/install qualification. Six fresh vendor source/normalized ordered traces match and twelve export replays pass; three vendor flattened reads fail ICL59-1, and the negative-iClock control fails after elaboration. No formal registry certificate is added;77 directional/seven legacy certificates remain unchanged. Strict100/90 coverage still fails at96.5%/62.7% (17376/18005 lines,29350/46789 raw branches), without filtering or waiver. Broader event, source/contextual, vendor-flat and release acceptance remains open.

Independent Task129 whole-branch review at986a1db approves the scoped merge with no Critical/Important findings. The reviewer selected dynamic63/static63/native-netlist checks, all passing through a disclosed read-only Root proxy because its tool host was unavailable. Broad artifact reproduction remains executor-owned; the recorded verifier checks hashes, counts and statuses. One optional wording minor is deferred:4013 denotes non-documentation files, although the guide currently calls them bytes. No runtime fix pass or second review was needed. Broader release and vendor acceptance remain open.


## Task130 typed flat clock qualification (2026-11-18)

See [the current representation, migration, measurement and limits](pattern-generation.md#typed-flat-clock-declarations-2026-11-18). Internal clock inputs use typed output declarations and preserve original kinds/scopes/physical bindings through version1 metadata; both old encodings load, while original clock inputs reject output-only frequency/Period fields. Native,64 clock methods per dynamic/static/SAN variant, full normal, Rocky51x794 per variant and source archive/install qualification pass. Four fresh vendor flat traces match after origin-name mapping and8 mapped native replays pass;8 raw-name native replays fail2 and both divider elaborations fail ICL130-2. Six older source/normalized directions are reused only by exact input/native-output identity, without new formal certificates. Strict100/90 coverage still fails at96.5%/62.7% (17386/18015 lines,29399/46869 raw branches), without filtering or waiver. General output-source flattening, physical/native name lookup, clock events and full release acceptance remain open. The Task129 file-count wording minor is clarified in the guide.

Independent Task130 whole-branch review at3720dc9 approves the scoped merge with no Critical/Important/Minor findings. The reviewer selected the whole native flatten runner, passing through a disclosed read-only Root proxy because its tool host was unavailable. Root owns broad source/product/archive/vendor status authentication; the verifier checks the recorded hashes and counts. No fix pass or second review was needed. General vendor flattening, raw-name lookup, clock events, strict coverage and full release acceptance remain open.


## Physical clock names in flat input (2026-11-18)

Task131 lets iClock and iRunLoop -sck use canonical physical clock names
from the associated native flat ICL. Original declaration names take priority.
Otherwise, a lowercase n followed by pairs of hexadecimal digits may name an
original ClockPort or ToClockPort when valid flat-origin metadata is present.
Existing scalar/vector suffix checks still apply. The metadata-presence check
is cached per immutable netlist; decoding costs O(name length + log nodes).

Generated PDL uses original names and replays against original or flat input.
Raw physical names and their reset artifacts require the associated flat model.
Original models without flat metadata, aliases, data/resource commands and
iClockOverride target/source spellings keep their existing lookup behavior.
Literal original names that collide with physical spellings win; this is not
general raw-name mode. Prefix and procedure-call qualification is unchanged.

Four unchanged Task130 vendor exports now replay directly on their flat models;
their complete ordered traces match the native original-name result. All eight
origin-mapped replays still pass. The four raw replays on original models still
reject unknown physical targets. Vendor evidence is reused only after exact
identity checks of the physical models/programs and current native outputs;
no new vendor execution or formal certificate is claimed. The two divider flat
models still fail vendor set_current_design with ICL130-2. General flat output
networks, negative vendor clock targets, clock events and absolute time remain
open. Evidence: build/flat-clock-targets-20261118/.

Qualification for Task131 passes all nine changed-C++ format/tidy/strict17
gates, native clock snapshots and68 clock methods per dynamic/static variant.
Leak-enabled ASan/UBSan passes native clock/netlist/flatten checks,68 clock
methods and71 main CLI methods. All3994 runtime input hashes remain frozen.
Only the two generator products change; the other eight products are unchanged.

Full serial normal integration passes in1663.067s,
including1916 regression and2180 conformance cases,334 parser/119 scanner rules
across2549 ICL files, and six editor tests. Five optional ANTLR checks are skipped.
Fresh instrumented testing takes1252.948s; all58 Python
groups/960 cases pass, including51 generator groups/798 cases. The unchanged
strict coverage gate still fails: 17417/18046 lines
(96.5%) and29457/46955 raw
branches(62.7%). All54 authored reports and19
instrumented products remain included, with12282 missing throw
edges and5216 other missing edges. No threshold waiver or
exception-edge filtering is applied.

The physical-name fixture has4096 pairs,4096 shared mux stages,541574 source
bytes,1024 waits and five replay forms. With metadata deliberately placed last,
it completes in1.537519s dynamic/1.475700s static; peak RSS122008/122292KiB.
The parent rejects raw physical targets, so this is no completing raw-name
baseline or speedup claim. Matched existing4096-stage/65536-bit clock workloads
pass on parent/current variants; matched parent/current5000-instance netlist benchmarks also pass. Commands, environment and wall/RSS logs are saved.

All eight current Rocky compiler/native/dynamic/static/ELF gates pass,
including51 groups/798 cases per generator variant. All3994 submitted source
hashes match. All12 local archive/install and14 remote install gates pass for
five commands per variant, LSP behavior, static ELF and uninstall sentinel
preservation. The e5ee52e archive ships4233 tracked files plus six generated/
prebuilt files. Its local dynamic build runs with Bison/Flex disabled. All4013
non-documentation files are byte-identical to the current qualified remote
source; current compiler/suite evidence is reused by identity, without claiming
a fresh remote archive rebuild. Later doc-only archive refreshes preserve the
qualified runtime bytes and original provenance. Full release acceptance,
strict coverage and the documented broader model/vendor gaps remain open.

Independent Task131 whole-branch review at aac20fd approves the scoped merge with no Critical/Important/Minor findings. The reviewer inspected connected lookup, restoration, copy and emission paths and selected native clock snapshots plus68 dynamic clock methods, all passing through a disclosed read-only Root relay because its tool host was unavailable. Root owns broad artifact/status authentication; the verifier checks recorded hashes and counts. No fix pass or second review was needed. General raw naming, vendor models, clock events, strict coverage and full release acceptance remain open.


## Absolute-time waits without clock-driven state (2026-11-18)

Task132 preserves an exact validated iRunLoop -time value when the model has
no register storage, TAPs, opaque interfaces or AccessLink nodes, and no clock/
TCK source feeding unsupported data/control or test-clock output logic.
Forwarded test clocks are also excluded: a data input bound to a child TCK
can conceal a fixed clock level and requires additional timing/role modeling.
Pure ports, combinational data, aliases, muxes and groups without storage may
therefore retain absolute delays. General stateful time execution remains
unsupported and reports the need for absolute-period and clock-event models.

IEEE1687-2014 7.7 printed154/PDF168 defines time values;7.9.17 printed175-176/
PDF189-190 specifies a separate -time alternative, minimum elapsed time and
free-running clocks. It does not provide combined -time/-sck syntax.
Native output retains the exact shared-frontend-validated text, including units,
exponents and zero, rather than guessing a tester period or converting through
floating point. Runtime revalidates values after Tcl substitutions. Existing
pending-read/write rejection and source/reset rewrite bookkeeping apply.

Eligibility is cached only for the immutable graph. Each call checks preserved
ClockPort/ToClockPort/TCKPort levels, including minimum-zero waits, since an
external tester need not implement a minimum-zero delay as zero cycles.
Direct clock-port iState/iWrite commands retain their existing unsupported
status; native API tests exercise the held-clock input contract. Clock-route
cache invalidation preserves checked-clock versions and original-spelled
overrides. No metadata version, option, new runtime dependency, clock assignment
or device event model is introduced.

A fresh Tessent2026.2 control accepts and exports a1.25us wait unchanged on a
clock-only model. It uses the documented patterns -ijtag/analysis pattern-set
flow and configured free-running clocks. This is one control, without a broad
vendor certificate or a claim about stateful/opaque timing. Evidence and current
qualification records are retained under build/invariant-time-waits-20261118/.

Qualification for Task132 passes all12 changed-C++ format/tidy/strict17 gates,
native clock/time snapshots and71 clock methods per dynamic/static variant.
Leak-enabled ASan/UBSan passes native clock/netlist/flatten checks,71 clock
methods and71 main CLI methods. All3994 runtime inputs remain frozen; the
non-runtime README is updated separately in the qualified remote source.

Full serial normal integration passes in1639.195s,
including1916 regression cases,2180 conformance cases,334 parser/119 scanner
rules over2549 ICL files and six editor checks. Five optional ANTLR checks are
skipped. Fresh instrumented testing takes1252.714s: all58
Python groups/963 cases pass, including51 generator groups/801 cases.
The unchanged strict100/90 gate still fails at17452/18083
lines(96.5%) and29536/47075
raw branches(62.7%). All54 authored reports and19
instrumented products remain included;12309 throw edges and
5230 other edges remain missing. No threshold waiver or filtering.

The new workload has4096 pairs,4096 shared mux stages,541574 source bytes,
1024 absolute-time waits and five replay forms. It completes in
1.512163s dynamic/1.472742s static,
peak RSS122004/122216KiB.
The parent rejects this workload; no completing baseline or speedup is claimed.
Matched existing4096-stage/65536-bit routes and parent/current5000-instance
netlist benchmarks pass. Commands, environment, input hashes and wall/RSS
records are retained.

One fresh Tessent2026.2 clock-only control exports1.25us unchanged. Its three
retrieved input hashes match submissions, the complete ordered trace matches
native input/output, and both original/flat native export replays pass.
This is one control without a formal registry certificate, broader vendor
equivalence or physical timing/event signoff.

All eight current Rocky compiler/native/dynamic/static/ELF gates pass,
including51 generator groups/801 cases per variant and3994 matching runtime
hashes. All12 local archive/install and14 remote install gates pass for five
commands per variant, LSP behavior, static ELF and uninstall sentinels.
Only the two generator products change; the other eight remain unchanged.
The 8c97e90 archive ships4235 tracked files plus six generated/prebuilt
files. Its local dynamic build runs with Bison/Flex disabled. All4013
non-documentation archive files, including the updated README, match current
remote source; current runtime compilation/suite evidence is reused by identity,
without claiming a fresh remote archive rebuild. Later doc-only refreshes keep
qualified runtime bytes and original archive provenance. General stateful time,
events, strict coverage and full release acceptance remain open.

Independent Task132 whole-branch review at bb941f0 approves the scoped merge with no Critical/Important/Minor findings. The reviewer inspected connected clock safety, physical resource holds, substituted-time validation, generation bookkeeping and opaque lowering. Fresh selected native clock checks and71 dynamic clock tests pass. Direct reviewer tooling was unavailable; source and checks used a disclosed Root read-only relay. All seven declined subjects have executor rulings, including independent artifact authentication, broader reproduction, physical timing, excluded features, vendor generality, release acceptance and the unrelated global tool host. No production fix was requested. Closing documentation changes preserve the3994 qualified runtime inputs; strict coverage and broader timing/release acceptance remain open.

## Native clock API contract coverage (2026-11-19)

Task133 adds tests for existing behavior; production files and all ten product
executables are unchanged from Task132. Native callers now exercise empty or
incomplete override arguments, duplicate/unknown options after valid parsed
fields, and a cycle through an inactive clock-mux alternative. Rejected calls
retain both saved and freshly checked first-root/divide2 durations; a subsequent
valid second-root/divide3 override preserves the old snapshot and yields nine
root cycles after a new check. Tests also check unrelated combinational logic
and named-chain client pins in original and flat models, unchanged hardware,
and the existing opaque-hardware exclusion for minimum-time waits.

Task132's eight zero-hit clock lines73/80/104/125/387/392/393/394 are now covered.
The complete fresh report measures **384/384 authored clock lines**. This is a
file-level result, not full project acceptance. Existing-behavior coverage was
the baseline gap; no failing product behavior or production fix is claimed.
An initial fixture violated the local-output Source rule and was corrected to
use a legal sibling-instance output, preserving language validation.

All three changed-C++ format/tidy/strict-C++17 checks pass. Focused native checks
pass; the full native scheduler passes leak-enabled ASan/UBSan in 318.304s.
Fresh full make check passes in 1665.605s, including dynamic/static generator
suites,1916 regression cases,2180 conformance cases,334 parser/119 scanner rules
across2549 ICL files and editor checks. Five optional ANTLR tests remain skipped.

Fresh raw coverage completes all58 Python groups/963 cases, including51
generator groups/801 cases, in 1347.667s. The unchanged100% line/90% raw branch
gate still fails with Make exit2: **17460/18083 lines
(96.6%) and 29556/47075 raw branches (62.8%)**.
All54 authored reports and19 instrumented binaries remain in the evidence.
Untaken edges include 12309 compiler-marked throw edges and
5210 other edges; none are excluded or waived.

Current Rocky build, strict C++17, focused-native and full-native checks pass,
with the aggregate gate0. All3993 other runtime inputs and all ten local and
remote product fingerprints match Task132. The refreshed source archive ships
4237 tracked files plus six generated entries; its4013 non-documentation files
match the current remote source. Task132's broader CLI/SAN, vendor and local/
remote archive-build/install evidence is reused only for unchanged inputs and
products. This task does not claim fresh remote CLI suites, vendor execution,
archive rebuild qualification or installation runs. Only the source archive is
refreshed and checked against every tracked shipped file and generated entry.

Evidence: build/native-clock-contracts-20261119/. Attempt1 preserves the focused
coverage command that retained an old runner because COV_DIR was missing, and
the remote script quoting failure before qualification. Both were corrected;
the final focused run rebuilt the native runner and all eight target lines hit.
Strict project coverage, broader timing/model support and release acceptance
remain open.

Independent Task133 whole-branch review at4daf6ba approves the scoped merge with no Critical/Important findings. The reviewer inspected the changed tests, connected clock/safety/state code, guide changes and evidence through a disclosed Root read-only relay because its tool host was unavailable. The requested fresh native clock check passes. Two optional minor improvements are deferred: identify negative-test arguments/actual exceptions in failure messages, and seed a known SelectPort value for an additional preservation assertion. All declined subjects have executor rulings. No production or test correction was required; closing documentation preserves the qualified3994 inputs and unchanged products. Strict coverage and broader release acceptance remain open.

## Runtime API contract coverage (2026-11-19)

Task134 tests existing public Runtime contracts without changing production
files or the ten product executables. A missing top module reports its intended
error. Structurally malformed later input is rejected before publishing that
file's operations; the previous note remains and the failed runtime cannot
resume or finish. A pending write inside a procedure retains the iCall frame
when Finish diagnoses the unapplied queue. This boundary concerns structural
parse failures; runtime semantic errors can retain partial internal operations
in the failed runtime and do not acquire a rollback guarantee.

A valid two-level instance path works without elaboration for a procedure with
no ICL parameters. Parameter-dependent calls require real elaboration before
the body executes. With elaboration, VALUE=4'b10x1 is passed unchanged and emits
`iNote 0b10x1`. Missing-module binding is tested once; missing-instance paths
take the intended error paths through both constructors. Selecting then clearing iUseProcNameSpace emits alternate
and default notes with no override warning. Expectations use literal values;
no private callback invocation, fabricated IR, new live readback or simultaneous
vector scan-return support is introduced.

All16 previously zero-hit runtime lines49/88-91/125/551/569/573/627/661/
703-704/734-735/742 now execute. The fresh full report measures
607/636 runtime lines; clock coverage remains384/384.
Coverage misses were the baseline gap, not a claimed product-failure RED.
The first parse fixture used an invalid option, which is checked during level1
execution. It was replaced with an unclosed body; the existing EOF diagnostic
is invalid.pdl:3: missing close brace. No product correction was required.

Changed-file format/tidy/strict-C++17 checks pass. The full changed runtime
runner passes leak-enabled ASan/UBSan in 0.357s. Full normal make check
passes in 1525.108s, including51 generator suites/801 cases per dynamic/
static variant,1916 regression and2180 conformance cases,334 parser/119 scanner
rules across2549 ICL files and six editor checks. Five optional ANTLR tests
remain skipped.

Fresh raw coverage completes58 Python groups/963 cases in 1178.198s. The
unchanged100% line/90% raw-branch gate still fails with Make exit2:
**17477/18083 lines(96.6%) and
29603/47075 raw branches(62.9%)**.
All54 authored reports and19 instrumented products remain included. Untaken
edges comprise12308 compiler-marked throw and
5164 other edges; none are filtered or waived.

Fresh Rocky build/C++17/native checks and the aggregate gate pass. All3993 other
runtime inputs and all ten local/remote product fingerprints match Task133.
The refreshed archive ships4239 tracked files plus six generated entries, with
all4013 non-documentation files matching current remote source. Broader remote
CLI, other SAN, vendor and archive-build/install evidence is retained only for
unchanged sources/products; these suites are not freshly rerun here. Source-
archive refresh checks all tracked shipped bytes and generated entries.

Evidence: build/runtime-api-contracts-20261119/. Strict project coverage,
broader runtime/model support and full release acceptance remain unfinished.

Independent Task134 whole-branch review at44e96ef approves the scoped merge. The reviewer directly inspected source, coverage counts and source/product fingerprints and independently reran all24 native runtime tests successfully. The executor corrected a qualification overstatement: missing-module binding ran once, while missing-instance paths ran through both constructors; the observed note is now spelled accurately. The optional missing command-name assertions in two success cases are deferred. All declined subjects have executor rulings. Closing edits affect documentation only; all3994 qualified runtime inputs and ten product fingerprints remain unchanged. Strict coverage and broader release/model acceptance remain open.

## Independent vector scan returns (2026-11-19)

Task135 extends the existing simultaneous-chain scheduler to interface-free
networks with disjoint root outputs, including vector OneHotScanGroup returns.
Every possible static route from each output must reach one distinct ScanInPort
bit. Existing ownership validation rejects any shared graph vertex, storage or
input. Ambiguous endpoints and disconnected alternatives remain unsupported,
even when currently inactive. Explicit interfaces and TAPs retain their existing
supported chain/event models; this inference does not model independent phases.
Equivalent observations of a single path retain their earlier behavior.

The endpoint pass is O(V+E) time and O(V) storage. Existing ordered root maps and
ownership lookups add O(R log R) for R roots. The existing 4096-register and
65536-vertex bounds remain. Native tests assert unequal 4/2-bit lane streams,
leading padding and expected-bit masks, unused input bits, copied state and
reservation rollback against literal expectations. Ten one-hot CLI methods cover
both branches, inversions, ascending/offset indices, separate output ports,
original/flat/generated/reset replay, an equivalent explicit-chain control and
failure preservation of PDL/reset/debug artifacts. Fixed unknown and contending
enables remain rejected. Unknown writable primary bits can instead be resolved
by the existing planner; this task does not change primary-X semantics.

Local GCC16.2.1/C++17/Z3 4.13.3 dynamic/static runs of 8 lanes x 8 branches
(64 four-bit registers, three replay forms) took 0.549/
0.574s wall time, including Python/process/replay overhead.
The parent rejects this undeclared-lane model, so there is no completing baseline
or measured speedup claim. This does not measure the whole supported graph cap.
See the /usr/bin/time -v logs for RSS and environment evidence.

Both changed C++ files passed changed-line format, clang-tidy and strict C++17
checks. Full native scheduling passed; its fresh address/undefined/leak sanitizer
run took 322.775s. The one-hot, named-chain and main CLI suites
also passed with leak detection enabled. The full normal check passed in
1746.520s: 1916 regressions, 2180 conformance cases and the existing
parser/scanner, file and editor suites; five ANTLR tests remain environment skips.
Rocky Linux GCC15.2.1/Tcl8.6.8 passed native/strict17 checks and all51 generator
suites (804 cases) for both its dynamic generator and the exact local static
binary. Source archive build and local/remote install/replay/LSP/uninstall checks
passed freshly. Eight editor products retain their previous byte identities;
only the two generator products changed. No fresh vendor control or certificate
is claimed for these inferred lanes.

Fresh unfiltered coverage ran58 Python suites/966 cases successfully and retained
54 authored source reports and19 instrumented executables. The raw strict gate
still exits2: 17499/18105 lines (96.65%) and
29628/47103 branches (62.90%), with
12313 missing throw and 5162 other
missing edges. No reports, compiler exception edges or thresholds were removed.
The required100% line/90% raw branch acceptance and broader retargeter tasks
remain unfinished. Detailed commands, hashes, timings, review and rulings are in
`build/vector-scan-returns-20261119/`.

## Reset caller-site argument dispatch (2026-11-19)

Task136 finishes a concrete reset-only caller gap: the same literal main
write/call site can need different releases for different evaluated arguments.
The first three source-rewrite candidates retain their behavior. A fourth
candidate records the evaluated arguments, builds a literal exact Tcl switch
inside the existing anonymous apply wrapper, releases that invocation's reset
prerequisites, and executes the original command with uplevel in caller scope.
No procedure body, persistent helper/global counter, SIB setup, dependency or
solver/runtime budget is added. The complete rewritten stream is executed and
checked before publication. Identical signatures requiring different preambles
remain a precise atomic diagnostic rather than an arbitrary release choice.

Two Important final-review findings were fixed in one pass: observed no-op and
caught-error calls now retain their empty signatures and original results/errors;
the actual rewritten operations are strictly scheduled before publication,
including their reservation lifetimes. The native and maintained regressions
were observed RED before the fixes and GREEN afterward.

The original contract expressly preserves iProc bytes and diagnoses an internal
reassertion that no caller preamble can repair. That negative behavior remains
covered. The11 historical reset implementation plans have no unchecked tasks;
this new caller-site work is recorded separately. Broader serial-dependent
feedback/event ownership and physical export replay are still unqualified
models, not hidden by that historical-plan inventory.

The21 argument-reset methods include nine new methods covering opposite reset
domains at one write/call site, qualified/default procedure names, literal and
Unicode arguments, prefixes/vectors, once-only substitutions, file/definition
order, idempotent rewrite and original/flat/generated/reset replay. Rejected
identical-signature, pending-queue, reservation and immutable-procedure cases
preserve preexisting output/reset/debug artifacts. The native runtime assertion
pins the actual evaluated vector/value metadata independently.

Measured local dynamic/static scale checks cover64 and256 calls, each with five
replay forms; complete commands, source/rewrite byte sizes, wall time and RSS
are in local/scale-*.log. Their whole-test wall times are
8.135/7.913s, including both input
sizes, Python and process overhead. Rocky CLI logs retain matching input sizes
and measured times. There is no completing parent baseline for this formerly
rejected source, and no speedup or structural-cap claim. Evaluated-argument
metadata uses memory proportional to executed argument bytes; the signature map
and emitted switch use space proportional to distinct signatures/release text.
The C++ map is built during generation, without per-invocation dictionary
rebuilding, subprocesses or whole-netlist scans in the emitted wrapper.

Twelve format/tidy/strict17 gates pass. Full native/runtime and leak-enabled
reset/source/main CLI suites pass; fresh normal0 took
1649.142s and fresh raw coverage took
1271.275s. Rocky11 exit records and the complete51x813
functional suites per binary pass. Fresh local archive build and local/remote
install/replay/LSP/uninstall checks pass with exact input/product identity.
The pre-review standalone macro-less syntax failure is archived. Its corrected
command initially reused unchanged functional evidence; after the review fixes,
all required scoped suites were rerun against the final source hashes.

Tessent2026.2 accepts the generated conditional wrapper and a manually ordered
reference. Their complete exported result.pdl files are byte-identical. The
unmodified source fails because the vendor requires a separate local-reset
apply boundary; the original failure is retained without increasing limits.
The post-fix wrapper matches the qualified vendor input bytes exactly, so those
vendor executions are reused by input identity. Four fresh post-fix native
source/flat export replays return2 on a generated
physical filler write: control0 asserts b's reset while the export reloads b=3
and compares its pre-update3. Native source goals require both requested writes
to hold and reject that masked payload. This broader physical-pattern import
limitation is preserved explicitly; source validation was not weakened and no
new directional certificate is added. See vendor/comparison.json and all logs.

Raw coverage remains 17546/18150 lines(96.67%) and
29689/47181 branches(62.93%), with
12328 missing throw and 5164 other
missing edges. The100/90 strict gate staysFAILED; no reports/edges/thresholds
were filtered. Evidence, failed attempts, hashes, review and rulings are under
`build/reset-caller-dispatch-20261119/`.


## Task140: audited BSDL model/default forms

The bounded metadata reader and TAP model support case-insensitive BSDL
instruction/register identities, repeated opcode descriptions, implicit allones
BYPASS storage/reset encoding, private instructions with omitted associations,
per-instruction CAPTURES, shared register widths and alternative IR/IDCODE/USERCODE
capture lists. ICL signal identities retain their original case. An undefined
private data route stays unknown and rejects raw access; it never becomes an
unassigned-code BYPASS. Explicit supplied private AccessLink routes work.

Alternative lists retain only common guaranteed bits; cross-bit correlations and
selection of a particular device variant are not modeled. Shared opcode captures
instead combine compatible care constraints and reject opposite cared bits.
Partially overlapping opcode regions retain their own unknown bits. Existing
constant/nonoverlapping mux paths remain; grouped decoders avoid repeated opcode
expansion per bit and per register. Original/flat/generated/reset replays and
atomic rejection cover these forms. The1024-register capture model builds in
about0.03s; the10000-opcode model remains qualified. Single measurements establish
no speedup or general throughput promise.

Authority: IEEE1149.1-2001(R2008) B.5.1/B.6.1; B.8.11–13 (printed153–158,
PDF161–166), with reset7.2.1e/f. B.8.13.3c explicitly permits later omission of
a known custom register width; the printed BNF still spells brackets as required.
This implementation follows the semantic permission and records that conflict.
The reader is not a full VHDL/BSDL conformance checker or boundary-cell simulator.

Initial qualification passed all13 native pattern suites,51 generator suites/830
cases per dynamic/static variant, native BSDL/binding/TAP/state/scheduler/runtime
sanitizers, focused12/main71 sanitizer checks,24 style/C++17 gates and source
distribution checks. The post-review qualification record below supersedes it.
The final maintained CLI fixtures also place private/optional-register attributes
in the standard order, verified by focused dynamic/static/SAN and package checks.

Fresh Tessent2026.2 setup/analysis/export probes use the original complete audit
models with corrected attribute order. Six complete with exit0: baseline,
CAPTURES, unused private instruction, IDCODE alternatives, implicit BYPASS, and
compatible shared capture masks. Three exit1: repeated instruction descriptions
(only-one-name rule in that tool), IR capture lists and USERCODE capture lists
(parser comma rejection). These standard-first native forms do not therefore
claim Tessent compatibility in that release. Earlier out-of-order pilot failures
are not counted as valid-form vendor evidence. Source/input/log hashes, commands
and complete exports are retained under build/bsdl-model-defaults-20261120/.
This is bounded evidence, not general model or hardware equivalence.
Item11's broader corpus/reference acceptance and items2–12 remain separately
tracked; excluded strict coverage item1 is unchanged.

The single review fix pass adds seven CLI methods (19 total). It rejects
contradictory three-way opcode overlaps in either ordering, case-equivalent
AccessLink blocks, private RUNBIST and inconsistent/first-omitted custom widths
even behind explicit ICL routes. It restores legacy mixed-case flat semantic
keys while preserving literal node paths and rejecting canonical duplicates.
Physical BYPASS captures remain zero for custom, BYPASS and unassigned opcodes.
A proven no-IDCODE reset BYPASS selection survives alternative encodings while
the IR value remains uncertain (for example1xxx); no encoding is fabricated.

Capture validation borrows per-group masks instead of copying a wide capture
per opcode. On the same65536-bit/4096-alias probe, the initial45e4510 model used
269744KiB RSS/0.20s and failed under a160MiB address-space cap; the corrected
model used19672KiB/0.04s and passed that cap. These single runs demonstrate this
resource acceptance case, not a general speedup.

All six successful vendor exports additionally pass48 local original/flat/
generated replays with nonempty apply sequences. Six fresh Tessent runs consume
the native normalized exports; all succeed. Existing ordered-apply trace
comparison matches all12 forward/reverse results, including the vendor's
explicit IR capture comparisons. These evidence comparisons are under
vendor-replay-qualified/, vendor-native-forward/ and
vendor-directional-comparison.json; they do not add source-controlled golden
certificates or establish general vendor/physical equivalence.

Post-review qualification passes all13 native suites,51 generator suites/837
cases per dynamic/static variant, six native leak-enabled ASan/UBSan suites,
focused19/main71 sanitizer methods,36 changed-hunk format/tidy/C++17 gates and
source-distribution build/install/uninstall checks. The wide-alias probe also
passes the160MiB address-space cap after the complete matrix. Exact18-step
commands, exits, timings and qualified input hashes are retained in review-fix/.
The single final review found eight Important issues, all fixed with RED→GREEN
regressions in one pass; it found no Critical or Minor issues. No re-review or
excluded coverage run was performed.

## Task141: bounded runtime, clock-name and ownership audit fixes

Associated flat iClockOverride target/source mapping, sparse never-read history,
provably zero initial/no-compare status and compatible modeled-child port locks
are covered by Task141. Applied reads still require actual observations for
getters; cared comparisons do not manufacture status. Child conflicts retain
intermediate serial-state invariance/per-edge checks, rollback and scope ownership.
Effective queue care and static validation preserve honest status/history. Two
nonblocking API/diagnostic limitations remain recorded in the qualification. See
[Task141 qualification](pattern-generation.md#task141-flat-clock-overrides-sparse-history-and-child-port-locks).
The broader time/event, observation-provider, interface/chain, clock assignment
and lifecycle requirements remain in items5–7.

The subsequent finite corpus navigation probe records261 source versions/
161 ordered module groups,71 load-only successes and50 nonempty original/flat/
generated write replays. Three further current models load with explicitly
recorded same-directory companion snapshots. These are exploratory accounting,
not maintained acceptance certificates. It also confirms a valid-source defect:
separate disjoint vector input/output port declarations are rejected as duplicate
objects, despite IEEE1687-2014 6.4.1g/h (printed80/PDF94) and successful2026.2
read/write exports. Sliced instance InputPort bindings already work; no change
to those bindings is implied. Exact cases and identities remain in
build/completion-source-probes-20261120/segmented-ports/. This finding remains
item11 work, separate from Task141's frozen production changes.

## Task142: disjoint vector-port source repair

Real indexed port functions now share a checked interval view without merging
their types/properties or adding synthetic hardware. Whole/sliced PDL, aliases,
child bindings, flat identities and clock/TAP callers are covered by17 focused
methods per variant.39 changed-code quality gates pass. Nine fresh normalized
Tessent2026.2 runs and72 nonempty native forms match18 directional canonical
comparisons. Eleven corpus cases pass this source boundary and encounter the
existing interface-event integration guard; whole corpus acceptance remains open.

Full13 native suites,52x869 dynamic/static generator cases, eight native
leak-enabled sanitizer suites and port17/clock72/history30/child14/BSDL20/main71
sanitizer methods pass.39 code-quality gates and source packaging pass.
Independent review found no Critical/Minor and two Important blockers. One
verified pass fixes doubly indexed native endpoints and legal child defaults
in flat export, preserving exact values and source bindings, including single
port controls and strict metadata rejection. No unresolved blocker remains.
See [the implementation and bounded evidence](pattern-generation.md#task142-typed-vector-port-functions).
The earlier Task141 corpus finding above is historical; current status is in
[remaining work](remaining-work.md).

## Task145: callback declaration model

Parameterized ReadCallBack/WriteCallBack names and typed literal/data/register
arguments survive canonical flat ICL replay with original instance scopes.
Seven focused CLI methods and native value assertions cover the declaration model,
including strings that contain placeholder or Tcl syntax and atomic invalid
name/type rejection. Matched512/4096-register measurements are recorded in
[the implementation evidence](pattern-generation.md#task145-typed-dataregister-callback-declarations).
Callback procedure execution and sourced-value access remain item4; declaration
replay does not establish either capability. Shared evaluator limits remain.

Task145 post-guard qualification passes all 13 native/all 13 native sanitizer
suites, 54x900 dynamic/static generator cases, callback-model 7/main 71 sanitizer
methods, 21 code-quality gates and source packaging. Frozen identities cover 3998
source/test/build inputs and 29 products. The two callback types have explicit
2026.2 read_icl rejection evidence; no callback vendor equivalence is claimed.

Independent Task145 review reports0Critical/0Important/0Minor and approves the
increment after18 reviewer-designed probes. Tool execution was proxied through
the author because the reviewer's host was unavailable; the reviewer made the
independent judgment from relayed source and results. No independent gate rerun
or usability-reader claim is made. Existing iWrite trailing-option value loss is
separately reproduced on parent/current and fixed by Task146 below.

## Task146: optioned write values

Validated iWrite auto/off/on hints preserve their supplied payload at the shared
Runtime boundary, including aliases, enum values, masks and merged calls.
Original evaluated source arguments remain intact. Invalid widths/enums reject
atomically even when reset later cancels the queue. Existing explicit-load
planning remains; no skip-load optimizer or global option command is added.

All33 qualification steps pass:13 native/13 native sanitizer suites,55x906
dynamic/static generator tests, six focused and71 main sanitizer methods, six
quality gates and source packaging. Frozen checks verify3999 source/test/build
inputs and29 products. Four successful Tessent2026.2 cases and16 matching native
forms give bounded changed-value PIO agreement. The54-process scale record
compares matched16/128-apply controls without a speedup claim. See
[the implementation evidence](pattern-generation.md#task146-optioned-iwrite-payloads).
Broader item10 scheduling and item4 callback execution remain open.

## Task147: actual input events and autonomous DAG goals

Primary-only DAG writes count an actual enabled event, consistently with mixed
and feedback paths. The pin may be outside the register dependency cone; it is
still explicitly emitted and independently replayed. Both paths retain explicit DataRegister goal intent when no emitted controller
write carries it, including autonomous DAGs and unchanged protected sources. No-event/read-only groups, disabled/unknown
enables, active reset and protected side effects remain constrained.

Seven focused methods and native independent concrete replay cover original/flat/
generated forms, aliases, masks, cascades and atomic failures. All33 qualification
steps pass:13 native/13 native sanitizer suites,56x913 tests per generator
variant, seven focused/71 main sanitizer methods, six quality gates and source
packaging. Frozen hashes cover4000 source/test/build inputs and29 products.
The54-process scale record compares matched writable-source controls and new
constant-source cases; an additional54-process mixed record gives108 total.
No physical timing or broad vendor-equivalence claim is
made; broader items remain tracked in [remaining work](remaining-work.md).
See [the evidence](pattern-generation.md#task147-actual-primary-events-and-autonomous-dag-goals).

## Task148: opt-in modeled PIO observations

--model_readback supplies actual observations for applied output/alias reads in
storage-free, TAP-free combinational PIO networks. Expected data determines
comparison care, without assigning observed data. Per-bit history, inversions,
unknowns, exact accumulated failure counts and known-count clear are supported.
Clock/raw/merge histories requiring new observations, stateful and interface/
chain histories and live transport remain open.

A direct independent review found two Important defects and one Minor cost:
final-state sampling loses earlier access-group observations; status clear left
a stale nonzero cache; labeling always copied history. One verified fix pass
captures batched actual selected-frame observations, immediately caches known
zero after successful clear, and copies only when removing an existing label.
Two added CLI methods reproduce three failing subcases, then17 methods pass
for both generator variants. A native oracle independently replays emitted pin
events and checks observed values. No rereview or independent post-fix approval
is claimed. The108-process scale record preserves parent/current controls and
source/flat/generated agreement, with wider sparse reads and no speedup claim.

See [the runnable example, limits and evidence](pattern-generation.md#task148-modeled-combinational-pio-readback)
and [remaining work](remaining-work.md). Full post-fix qualification is recorded
with that example after its gates finish. Coverage item1 remains excluded.

All35 post-fix qualification steps pass:14 native and14 leak-enabled ASan/UBSan
native suites,57 suites/930 tests for each dynamic/static generator,17 focused/
71 main sanitizer methods,30 changed-code format/tidy/strict C++17 gates and
source packaging. Frozen hashes verify4005 source/test/build inputs and31
products. Evidence is under build/modeled-pio-readback-20261122/final/.
The final108 scale processes use the same qualified generator binary.
Coverage item1 remains excluded with unchanged thresholds.

## Task149: modeled DataRegister observations

Opt-in --model_readback extends selected-frame observations to DataRegisters
accessed through existing PIO routes. Actual old storage is observed before a
later write, including supported DAG/feedback schedules. Manual read/miscompare
history suppresses its own older undelivered plane without changing hardware,
queues or failure counts. Later observations replace both planes. Existing
reset requirements, unknown status, source-order aliases and callback/scan/TAP/
clock/raw/merge boundaries remain. Legacy mode notes migrate once.

Sixteen new focused methods per variant, native emitted-event/state checks,
existing readback/runtime/data-state/primary-event callers and15 C++ quality
gates pass before broad qualification. The native oracle applies only emitted
pin writes; expected masks/goal markers never assign its state.
See [example and evidence](pattern-generation.md#task149-modeled-dataregister-observations-through-pio)
and [remaining work](remaining-work.md). Coverage1 remains excluded.

Full behavioral qualification at2641965 passes36 gates:14 native/14 native
sanitizer suites,58x946 tests per generator,15 quality gates and packaging.
One independent review found0Critical/0Important/2Minor. A help/assertion-only
verified fix pass passes13 checks/312 focused methods and packaging; all28 native
binaries/4004 other inputs retain baseline hashes. Final4006 inputs/31 products
are verified, with108 scale processes at2641965. Original full receipts and
generator snapshots are separate from final/post-review/ checks. Review used
relayed source and six independently selected root-executed probes; no rereview
or independent full rerun is claimed. See the guide for precise scope/evidence.

## Task150: standalone serial observations

Opt-in modeled readback now samples actual serial return wires before shift
edges, including chain inversion, sparse reads, scan-accessed DataRegister
captures and unequal disjoint client lanes. The existing model's reset/shift/
update behavior remains authoritative. No second simulator or device
transport is introduced. Bound-TAP, raw/clock/merge, callback execution and
interface/chain observations remain open.
See [the runnable example and limits](pattern-generation.md#task150-modeled-standalone-scan-observations).

Task150 post-fix evidence:37 gates,14 native/14 sanitizer suites,59x962
methods per generator,18 quality checks, source packaging,144 scale processes
and4007 input/31 executable hashes. A fresh direct review found two Important
fast-path delivery gaps; one verified fix pass resolves both. No Minor findings
or declined cases remain. The pre-review run is explicitly partial.

## Task151: bound-TAP register observations

The model mode includes actual pre-edge IR/DR returns for existing bound TAP
routes, captured DataRegisters and common-phase multiple TAPs. Runtime and
output generation share mismatch-accepting observation scheduling, including
exact physical-PDL mode. Default diagnostics and existing resource/topology/
synthetic-target guards remain. Raw/interface/chain and timed histories remain
open. See [the runnable example](pattern-generation.md#task151-modeled-bound-tap-observations).
Final post-fix evidence:38 gates,14 native/14 sanitizer suites,60x978 methods per generator,18 quality checks, packaging,144 scale processes and4008 input/31 executable hashes. One Important subscriber-state/cache issue is fixed by shared post-commit, post-route-cleanup delivery; two Minor help/prose findings are deferred. Earlier interrupted runs are partial, not full qualification.

## Task152: raw interface and opaque parallel-chain observations

Existing native scalar root/child and bound single-TAP IR/DR raw requests now
produce actual returned-word histories. Register and raw histories remain
independent. Named root opaque parallel chains support variable widths, reset
defaults and declared stable inputs; unknown storage returns x, with cared
unknown comparisons preserving an unknown count. Provider deltas validate
atomically, and public raw callbacks run after transaction commit and route
cleanup. Physical PDL's existing iScan guard remains.

See the [examples and limits](pattern-generation.md#task152-raw-interface-and-chain-readback).
Broader modeled parallel/multiple-TAP raw, opaque child, raw merge, timed,
callback and transport work remains pending.

Final Task152 evidence:40 gates,14 native/14 sanitizer suites,62x1008 tests
per generator,30 quality checks, packaging,4011 input/31 executable hashes
and57 matched scale processes. Both Important review findings pass test-first
fixes and the full post-fix suite. Two current Minor findings and the prior
Task151 numeric-prose Minor remain deferred; no rereview. See the guide for
their exact limits and the existing physical raw-PDL rejection ruling.


## Task155: fixed modeled parallel raw observations

One explicit root client now replays disjoint named static ScanRegister
fields with common controls, including unequal lengths, vector/inverted
paths, constant/unknown captures and current-fill completion. Actual source
histories belong only to explicit fields. Numeric held-storage assignments,
rollback and postcommit publication follow the native scalar contracts.
The old single-result API rejects parallel; the vector API returns all fields.

Sixteen CLI methods and native per-clock reference cases pass, along with
the focused dynamic/static compatibility run. Ninety-three fresh scale/control
processes pass exact replay and the representative45s/512MiB bounds.
All47 post-fix qualification gates pass, including14 native/14 sanitizer suites,
65x1074 tests per generator,15 quality checks, packaging and4018 input/31
product hashes. Four Important review findings are fixed in one pass; two
Minors remain deferred. All179 performance/control processes pass. Exact-byte
optimized-product reuse and fresh changed sanitizer checks are recorded;
integration is pending.

See [the runnable guide](pattern-generation.md#task155-modeled-parallel-raw-scans).
Hierarchical/dynamic/shared/hidden/gated routes, multiple TAPs, opaque children,
raw merge, timed histories, transport and the other pending inventory remain
outside this increment. Coverage item1 remains excluded.


## Task156: default source model and canonical corpus audit

Empty control-output bodies retain valid semicolon declarations and missing
RefEnum tables have source/node/name diagnostics. Native and eight CLI
methods retain scoped/forward/specialized enum and flat defaults.

The checked corpus contains2562 ICL files:1228 parser-rejected inputs keep
byte provenance;1334 accepted files yield1763 effective module defaults.
Actual canonical successes are1016;56 declared serialization boundaries and
691 original elaboration failures remain explicit. The model inventory
performs actual public-API flat roundtrips, with exact file/module identities
and fresh report/protocol checks. It does not supply missing models, bindings
or parameters. Both namespace and ordered replacement behavior are audited.

Six fresh normal/configured leak-sanitizer batch measurements meet10s/512MiB;
the documented500ms unused-page release preserves full ASAN quarantines and
checks. Default sanitizer retention does not meet that memory configuration.
All52 final qualification gates pass:14 native/14 sanitizer suites,66x1082
per generator,15 quality checks, packaging and4024 input/36 product hashes.
Five Important review issues pass one test-first fix cycle; one Minor remains
deferred, with no rereview. All92 measured corpus/callback/control/alias
processes pass. Broader inventory11/12 and excluded coverage1 remain unchanged. See [the commands and exact limits](pattern-generation.md#task156-default-model-and-source-round-trip-audit).


## Task160: fixed opaque child interval

One declared-width scalar opaque leaf can occupy a complete native root route
between modeled ScanRegisters or within pure scalar wrappers. Original node
IDs and exports stay unchanged. A private immutable interval feeds the existing
native scheduler; capture/reset/post-CSU hidden state is unknown, while input
fill follows explicit/default/stable length knowledge. Native copies and
transaction rollback retain queue state; subscriber failures retain committed
state. Every ordinary preparation uses only prior knowledge, before the current
raw request is consumed once. Root-only opaque and unrelated unused-child paths
retain their existing behavior.

CLI qualification covers source/flat/generated/reset-source replay, ordinary
parent observations, raw history separation, masks/status, prefix/namespace/
split inputs, inversions/aliases, reservations, cancellation and atomic errors.
Performance and full-regression receipts are recorded with final qualification.
Multiple/dynamic-width/parallel opaque children, gated selection, clocks/time,
TAPs, raw merge, iApplyEndState and physical PDL remain unsupported. Broader
items6/8 and excluded coverage1 are not closed by this increment.


Task160 passes58 post-fix qualification gates:15 normal and15 leak-enabled
sanitizer native suites,70 suites/1136 methods for each normal/static generator,
19 opaque-child CLI methods in each variant and selected sanitizer,21 changed
C++ format/tidy/C++17 checks, corpus/producer/protocol/Make checks and source
packaging. Exact4031 maintained-input hashes and38 qualified products are bound
in build/opaque-child-raw-20261123/final/receipt.json. The corpus retains1024
canonical/44 declared-unsupported/695 original elaboration failures among1763
effective modules. Broader source support and excluded coverage1 are unchanged.

One independent review found0Critical/5Important/1Minor. One test-first pass
fixes all five Important issues: physical-root stability with a non-stable modeled
sibling, sparse stable opaque data during implicit raw traversal, private targets
and reserved IDs in native reset queries, bit-specific alias side-channel
classification, and actual benchmark CSU/shifted-bit/control-width receipts.
The Minor remains deferred: an empty physical program can publish a header-only
output, executing no CSU. Rulings and the full fix record are retained in
build/opaque-child-raw-20261123/rulings.json; no second review was run.

All102 fresh opaque/control processes,60 preserved callback controls, six
wide-alias processes and six normal/configured-sanitizer corpus batches meet
their bounds. Three exact guide invocations also pass, for177 measured/example
processes in total. Maximum opaque normal runtime/RSS: 0.1820s/125388KiB; sanitizer:
0.4152s/432632KiB. The16K callback alias peak is165072KiB within256MiB. Generated opaque
runs execute four fresh root getters. Control receipts distinguish grid width,
actual opaque width, modeled storage, nine versus four CSUs and their total
shifted bits; no speedup or identical-workload claim is made. Commands, tool/
input/output hashes and runtime environment are retained under
build/opaque-child-raw-20261123/{final,scale-final,final-preserved,batch-scale,
example}/. Pre-review measurements are retained separately and are not used as
final-product measurements. The six corpus measurements retain unchanged
inventory product and input hashes, verified against final qualification.

Task160 is merged into master;20 integration checks pass on4031 identical input
hashes/20 copied qualified native products, preserving117 stashes. Receipt:
build/opaque-child-raw-20261123/merged/receipt.json. Broader items2–8/10–12 stay
open as tracked; the next increment continues the original requested inventory.
