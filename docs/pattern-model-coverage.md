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
| Shared frontends and ordered inputs | Shared ICL AST/PDL tree; Design/Runtime tests and CLI ordered overrides | Complete runtime command coverage and broader real-pair generation |
| Elaboration and values | Namespaces, specialization, arithmetic, ranges, aliases, enum and compiled logic; source-located diagnostics | Full source-kind checks and corpus specialization |
| Address/one-hot models | Contributor/domain/default/alias resolution; primary-, scan- and mixed-driven DataRegister writes/reads; scalar nested OneHotScanGroup selection and canonical replay | Vector scan returns and broader model qualification |
| Native hardware state | Unknown initial state, reset propagation, separate shift/update storage, addressed DataRegister events, clock edges and event-free top-level pin assumptions | Feedback resets, general gated events and complete vendor builtin models |
| AccessLink/TAP | BSDL reader, IR/DR datapaths, FSM, child wiring and runtime metadata restored from flat ICL; explicit IR PDL reads/writes and ordered apply replay, with nine fresh SIB/reset directional certificates | Multi-TAP scheduling and remaining BSDL/private capture forms |
| ScanInterface controls | Runtime explicit/implicit membership; single-TAP phase-qualified visible/hidden capture, shift and update events with concrete replay | Shared-chain, serial-derived controls and general independently gated events |
| Serial/mixed scheduler | Shared parallel/scan frames, primary observations, register/mux/logic paths, SIB and acyclic reset prerequisites, read obligations, register/primary/computed-port AccessTogether groups, forwarded/logic/mux/one-hot port writes and partial/X writes; ScanRegister/primary-input iApplyEndState defaults and final restoration | Multiple roots, overlapping intermediate taps, broader access models |
| Idle cycles | Literal TCK counts on a reset single TAP; native state replay, fixed-point acceleration, retained output count | System clocks, absolute-time waits, external time-dependent instrument behavior |
| Optimization | Dependency-guided and shared-address write batches; nested iMerge normalization with disjoint multi-apply write batching and serial fallback; independent tree/finite-state oracles, concrete replay and common-update constraints | Broader merge optimization, timing/resources, whole-program optimization and broader scale/vendor qualification; no global optimality claim |
| Resource ownership | Bit-level ScanRegister/DataRegister/data-port alias/slice and instance-member takes; applied-write/iState protection, inherited ownership and scoped release; Tessent default/persistent takes within the input stream; protected dependency batches, per-clock replay, explicit-reset ownership clearing and atomic failures | Other control/clock/scan ports, pattern-set lifecycle, broader scale and vendor qualification |
| Flattened output | Shared netlist serializer; original/flat/generated-PDL tests, including single-TAP instructions | Full corpus and vendor-portable equivalence for all model kinds |
| CLI/build/install/safety | Five dynamic/static executables; staged outputs, input alias protection, BSDL loading, installed-program replay | Remaining command semantics and all required platform validation |
| Local-reset-only artifact | Transitive reset-control releases, grouped independent controls, ordered multi-file rewriting, unchanged procedures and actual rewritten-runtime validation | Internal procedure reassertions, conflicting repeated source sites, feedback and broader vendor qualification |
| Tessent oracle | Seven legacy and thirty-one directional standards for pin/vector, TAP, reservation, shadow storage, transitive-reset, end-state, scan-driven and mixed DataRegister cases; independently generated source schedules need not match | New one-hot scan and multi-apply merge changes have local replay coverage only; broader model/corpus equivalence remains unverified |
| Acceptance checks | Regression, sanitizers/style and independent review; latest local-reset evidence in the generator guide | Last strict coverage measurement remains below 100% lines/90% raw branches: 95.2%/60.5%; not remeasured for reset rewriting; full vendor equivalence is unverified |

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
The native test also closes the chain into a cycle and requires rejection.
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
