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
| Instance attributes and addresses | `TestAddressedInstanceMetadata`, `TestInstanceAttributeReplay`, `TestInstanceMetadataValidation`: parent-scope values, separate module/instance attributes, metadata validation and replay | Addressed-group connections, address comparison and scheduling |
| Reset polarity, unique-parent/default/ambiguous reset | `TestResetInference` | Child-output precedence, TRST/TMS, nested local-reset simulation |
| Enum reset values and ScanMux connectivity | `TestMuxAndEnum` | All mux kinds, overlapping selector checks, state transitions |
| OneHotScanGroup | `TestOneHotScanGroups`, `TestOneHotScanErrors`, `TestOneHotScanMuxFanIn`, `TestOneHotScanLimits`: contributors, inversion/ranges, structural enable fan-in, per-bit cycles and flat replay | Runtime enable solving and at-most-one-active-source constraints |
| LogicSignal | `tests/src/pattern_logic_test.cc`: shared AST, exhaustive small truth tables, enum/parameter widths, X masks, invalid operands, recursion and allocation limits | Full corpus and retargeting integration |
| Selectable DataRegister and output enables | `TestDataRegisterControls`: WriteEnSource, double inversion, Enable and width errors | State transitions and access justification |
| Register semantic constraints | `TestRegisterRules`: required scan source/rightmost tap, reset/default conflicts, enum widths | Remaining signal-kind rules and state semantics |
| Clock/control gates | Existing parser and initial connections | Typed clock models and control associations |
| ScanInterface | `TestScanInterfaces` and `TestInterfaceKinds`: four interface types, chains, default loads, implicit controls and invalid membership | Blackbox reset path-length proof, unique-selectability proof and scheduler integration |
| DataRegister access models and AccessLink | Existing parser only | Address/callback/scan/TAP models and scheduling |
| Tessent edge-triggered reset | Manual reviewed | Edge history and builtin model implementation |
| Flattened ICL | `tests/src/pattern_flatten_test.cc`: canonical replay of ports, registers, muxes, logic, enums, aliases, attributes, reset domains and original PDL bindings | OneHotDataGroup/PortGroup/AccessLink, clock properties and end-to-end scheduling replay |
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
unresolved OneHotDataGroup padding decision submitted to the user.

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
