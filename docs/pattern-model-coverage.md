# Pattern generator model and connection audit

This is an implementation ledger, not a claim that the executable is delivered.
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

| Model or obligation | Current evidence | Remaining acceptance work |
| --- | --- | --- |
| Ordered ICL definitions, namespaces, top selection | `tests/src/pattern_design_test.cc` | CLI diagnostics and failed-output publication |
| Immediate PDL calls and procedure overrides | `tests/src/pattern_runtime_test.cc`, including specialized parameter injection | All remaining execution/control semantics and flat-design binding |
| Integer and numeric/string constants | `tests/src/pattern_expression_test.cc` | Broader real-corpus evaluation; resource-bound tests |
| Parameterized instance hierarchy and widths | `TestSpecialization`, `TestErrors` | Corpus specialization and flat serialization |
| Bit order, partial input bindings, inversion, aliases | `TestBitConnections`, `TestConnectionErrors`, `TestUnsizedSources` | Full semantic source-kind checks |
| Instance attributes and addresses | `TestAddressedInstanceMetadata`, `TestInstanceAttributeReplay`, `TestInstanceMetadataValidation`: parent-scope values, separate module/instance attributes, metadata validation and replay | Runtime address selection and scheduling |
| Reset polarity, unique-parent/default/ambiguous reset | `TestResetInference` | Child-output precedence, TRST/TMS, nested local-reset simulation |
| Enum reset values and ScanMux connectivity | `TestMuxAndEnum` | All mux kinds, overlapping selector checks, state transitions |
| OneHotScanGroup | `TestOneHotScanGroups`, `TestOneHotScanErrors`, `TestOneHotScanMuxFanIn`, `TestOneHotScanLimits`: contributors, inversion/ranges, structural enable fan-in, per-bit cycles and flat replay | Runtime enable solving and at-most-one-active-source constraints |
| OneHotDataGroup contributions | Port/Register/Instance contributor tests: widths/directions, unknown leading bits, structural enables, cycles, addressed data defaults and metadata-backed canonical replay | Runtime enable solving, uniqueness/conflict checks and vendor-portable flattened hardware semantics |
| Address/control requirements | `tests/src/pattern_address_test.cc`: implicit/explicit domains, aliases, wide additive offsets, scan-register boundaries, source provenance, resource guards and replay | Global feasibility/uniqueness, gated or partial/reordered bindings, read deactivation and scheduling |
| LogicSignal | `tests/src/pattern_logic_test.cc`: shared AST, exhaustive small truth tables, enum/parameter widths, X masks, invalid operands, recursion and allocation limits | Full corpus and retargeting integration |
| Selectable DataRegister and output enables | `TestDataRegisterControls`: WriteEnSource, double inversion, Enable and width errors | State transitions and access justification |
| Register semantic constraints | `TestRegisterRules`: required scan source/rightmost tap, reset/default conflicts, enum widths | Remaining signal-kind rules and state semantics |
| Clock/control gates | Existing parser and initial connections | Typed clock models and control associations |
| ScanInterface | `TestScanInterfaces` and `TestInterfaceKinds`: four interface types, chains, default loads, implicit controls and invalid membership | Blackbox reset path-length proof, unique-selectability proof and scheduler integration |
| DataRegister access models and AccessLink | Addressed register data connections and address/control requirements; selectable register structural inputs | Callback execution, AccessLink/TAP models, state transitions and scheduling |
| Tessent edge-triggered reset | Manual reviewed | Edge history and builtin model implementation |
| Flattened ICL | `tests/src/pattern_flatten_test.cc`: canonical replay of ports, registers, muxes, logic, enums, aliases, attributes, reset domains, typed one-hot contributors, addressed data defaults and original PDL bindings | Vendor-portable address/control flattening, PortGroup/AccessLink, clock properties and end-to-end scheduling replay |
| Shortest configuration sequence and iMerge | Not implemented | State search, cost oracle and semantic barriers |
| Local-reset source-preserving output | Not implemented | Transitive releases, safe batching, exact iProc preservation |
| CLI/build/install/output safety | Not implemented | All requested options, staged output, dynamic/static installation |

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

`src/pattern_address.{h,cc}` consumes the existing immutable `Netlist` and returns
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
idempotency alone is not treated as proof of correct wiring. Remaining gaps
include implicit BSDL register associations, unlisted instruction bit patterns,
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
