# Pattern generation

`icl-pattern-generation` is an experimental executable. It currently expands
PDL procedures and Tcl computations and schedules single-input,
single-output scan networks, including single-TAP instruction/data scans. It is
**not yet the complete retargeter described in the requirements**.

## Build and run

The generator reuses the shared ICL parser, elaborated netlist, PDL parser and
bounded Tcl runtime. It additionally requires Z3 development files. Install Z3
through your system package manager, supply `Z3_CFLAGS` and `Z3_LIBS`, or explicitly
build the pinned dependency in the ignored build directory:

```sh
make z3-deps
make dynamic
make static                 # also requires the existing static Tcl dependency
make install-dynamic
icl-pattern-generation --help
```

Z3 4.13.3 is the pinned bootstrap version. Downloads, generated sources and
libraries stay outside Git. `make icl-dynamic` and `make pdl-dynamic` still build
only the editor programs and do not need Z3. Static builds require a static Z3
archive; see `Z3_PREFIX` and `Z3_STATIC_LIB` in the Makefile.

```sh
icl-pattern-generation \
  --icl design.icl overrides.icl \
  --pdl setup.pdl procedures.pdl \
  --top_module Top \
  --output result/Top.pdl --debug --log result/run.log
```

ICL files are loaded in order; later module definitions replace earlier ones
with warnings. PDL runs in file/source order, so a later iProc replacement only
affects later calls. If omitted, the top is the unique uninstantiated module
and the output is `out/<top>.pdl`. `--debug` also writes
`<output-directory>/debug/<top>.icl`. Generated PDL contains explicit prerequisite
writes and applies, without procedure calls or variables.

For an IEEE 1149.1 AccessLink, load its BSDL entity explicitly (repeat the option
for additional models):

```sh
icl-pattern-generation --icl design.icl --bsdl DEVICE device.bsdl \
  --pdl setup.pdl --output result/Top.pdl --debug
```

The PDL must explicitly establish TAP state with `iReset` before transactions.
`iReset -sync` uses five TMS-high cycles even when TRST exists; plain `iReset`
uses TRST when available. Flattened debug ICL embeds the BSDL metadata and can
be replayed without the external BSDL file.

## Complete serial example

Save this as `network.icl`:

```icl
Module Network {
  ScanInPort si;
  ScanRegister gate { ScanInSource si; }
  ScanRegister target[3:0] { ScanInSource gate; }
  ScanMux route SelectedBy gate { 1'b0: gate; 1'b1: target[0]; }
  ScanOutPort so { Source route; }
}
```

Save this as `configure.pdl`:

```tcl
iPDLLevel 0
iProcsForModule Network
iState gate 0
iWrite gate 1
iWrite target 10
iApply -together
```

Here `iState` is an explicit assumption supplied by the PDL author; the tool
cannot establish it by reading a ResetValue. Use it only when that starting
state is valid for the intended hardware.

```sh
icl-pattern-generation --icl network.icl --pdl configure.pdl \
  --output result/Network.pdl --debug
```

The first scan opens `gate`. A second scan writes `gate` and `target` on the
same update edge. The output therefore contains a prerequisite `iApply` followed
by `iApply -together`. The first scan alone cannot meet the request because
`target` is bypassed while that scan is shifted.

To check serialization and generated-PDL stability:

```sh
icl-pattern-generation --icl result/debug/Network.icl --pdl configure.pdl \
  --output result/from-flat.pdl
icl-pattern-generation --icl network.icl --pdl result/Network.pdl \
  --output result/second-pass.pdl
cmp result/Network.pdl result/from-flat.pdl
cmp result/Network.pdl result/second-pass.pdl
```

These comparisons check deterministic output for this example. They are not a
substitute for hardware-state replay or a vendor comparison.

## Single-TAP idle example

From the repository root, use the bundled self-constructed AccessLink/BSDL model:

```sh
mkdir -p out/tck-example
cat > out/tck-example/input.pdl <<'PDL'
iProcsForModule AccessTop
iReset -sync
iWrite core.r 0x59
iApply
iRunLoop 100 -tck
iRead core.r 0x59
iApply
PDL

icl-pattern-generation \
  --icl tests/cases/ours/pattern_generation/access_link/simple.icl \
  --bsdl AccessProbe tests/cases/ours/pattern_generation/access_link/simple.bsdl \
  --pdl out/tck-example/input.pdl --output out/tck-example/result.pdl --debug

icl-pattern-generation --icl out/tck-example/debug/AccessTop.icl \
  --pdl out/tck-example/input.pdl --output out/tck-example/flat-replay.pdl
cmp out/tck-example/result.pdl out/tck-example/flat-replay.pdl

icl-pattern-generation --icl out/tck-example/debug/AccessTop.icl \
  --pdl out/tck-example/result.pdl --output out/tck-example/pdl-replay.pdl
cmp out/tck-example/result.pdl out/tck-example/pdl-replay.pdl
```

The output retains all 100 TCK cycles. Matching files check deterministic replay;
they do not establish a live hardware readback or Tessent-equivalence result.

## Alias access-group example

From the repository root, this fixture places `a` and `b` behind one mux route
and `c` on the other. `pair` aliases `a, ~b` and requires `AccessTogether`.
The group must share an update; the independent `c` write may use another scan.

```sh
mkdir -p out/access-together-example
cat > out/access-together-example/input.pdl <<'PDL'
iProcsForModule Network
iState gate 0
iWrite pair 1
iWrite c 0
iApply
PDL

icl-pattern-generation \
  --icl tests/cases/ours/pattern_generation/access_together.icl \
  --pdl out/access-together-example/input.pdl \
  --output out/access-together-example/result.pdl --debug

icl-pattern-generation --icl out/access-together-example/debug/Network.icl \
  --pdl out/access-together-example/input.pdl \
  --output out/access-together-example/from-flat.pdl
icl-pattern-generation \
  --icl tests/cases/ours/pattern_generation/access_together.icl \
  --pdl out/access-together-example/result.pdl \
  --output out/access-together-example/second-pass.pdl
cmp out/access-together-example/result.pdl out/access-together-example/from-flat.pdl
cmp out/access-together-example/result.pdl out/access-together-example/second-pass.pdl
```

Both comparisons pass on the verified generator. The generated PDL resolves
inversion, retains the common update as `iApply -together`, and contains no
procedure calls. This example checks scheduling and deterministic replay; it
is not a Tessent equivalence result.

## Command support

| Command | Current generator behavior |
|---|---|
| `iProc`, `iCall`, prefixes and Tcl computation | Expand through the bounded runtime, preserving source order and call diagnostics. |
| `iRead`, `iWrite`, `iApply` | Schedule supported scan-register/alias accesses; numeric or target-local enum operands; `-together` enforces a shared update cycle. |
| `iState` | Numeric assumptions for scan registers and top-level DataInPort/SelectPort pins, including aliases and slices; default or `-LastWrittenValue`; no device transaction. |
| `iReset` | Explicit top-level reset, including `-sync` for a single TAP. |
| `iRunLoop` | Literal TCK waits on a reset single TAP; zero count needs no TAP reset. |
| `iNote` | Retain literal notes in generated PDL. |
| `iMerge`, resource and system-clock commands | Execution/scheduling remains unfinished and reports an error. |
| Hardware readback commands | Static generation cannot supply live device observations. |

This table describes generator execution. The formatter and LSP have separate,
broader syntax support and never execute PDL.

## Current transaction support

- ScanRegister writes and reads, including slices and aliases; PDL numeric
  values and names from the target's `RefEnum`, last-write masks, and all-X
  observation requests. Enum resolution uses the named register or alias and
  its elaborated instance parameters. Aliases do not inherit a source enum.
  Enum constants use the same PDL width and X-mask rules as numeric values;
  a slice still rejects truncation of significant bits. `iState` and `iRunLoop`
  retain their numeric-only syntax. Generated PDL contains resolved numbers.
- `iApply -together` enforces a common update cycle for all queued writes
  (IEEE 1687-2014 §7.9.14(c), printed p. 173). Prerequisite scans can precede
  it. Read-only groups need not share a capture cycle. The common update is
  checked in concrete replay and retained in generated PDL. Mutually exclusive
  paths cannot satisfy it; a bounded search failure does not prove general
  hardware unreachability.
- `AccessTogether` on a referenced ICL Alias keeps its full ScanRegister group
  in a single operation, including nested aliases and partial references.
  Writes require simultaneous updates after reset release; reads compare their
  requested bits together after any prerequisite scans. Unrelated accesses may
  use separate scans. Unsupported port/DataRegister members are rejected.
  Flattened ICL preserves the marker; emitted common updates use
  `iApply -together`. Models containing alias `iApplyEndState` are explicitly rejected because
  restoration scheduling is not implemented. The directive cannot be ignored
  even when PDL accesses the underlying register directly.
- Explicit `iState` with default/`-LastWrittenValue` semantics and `iReset`.
  State assumptions accept scan registers and top-level DataInPort or SelectPort
  pins with no internal driver; aliases retain source bit ordering and inversion.
  They change only the assumed snapshot, without clock edges or reset
  propagation. Internal/output ports and controller-owned instruction storage
  are rejected before a mixed alias can partially change valid targets.
  This does not widen the ordinary serial backend's event model: explicit
  scan-interface control ports still require the supported single-TAP backend.
  Hardware starts unknown; ResetValue metadata is never treated as a power-on
  reset. A child-module iReset is rejected instead of promoted to a global reset.
- ScanMux and compiled LogicSignal selection, including register-controlled
  SIB and local-reset prerequisites. Reset release cannot enable an update on
  the same edge retroactively.
- Single-TAP IR/DR selection, retained instruction state across apply groups,
  and explicit/implicit scan-interface controls. Each complete scan must operate
  its observed storage without also clocking bypassed registers. Every accepted
  transaction executes through the native TAP controller and child event model.
  IR selection is internal to retargeting; generated PDL names actual instruments,
  never synthetic instruction registers.
- Source-attributed failures and invocation stacks. Structural disconnection,
  unsupported behavior and bounded solver exhaustion are distinct errors.
- Literal TCK `iRunLoop` counts (IEEE 1687-2014 §7.9.17, printed pp. 175–176)
  on a single TAP after explicit reset. `iRunLoop COUNT` defaults to TCK;
  `COUNT -tck` and `-tck COUNT` are accepted and emitted as decimal
  `COUNT -tck`. Counts must fit an unsigned 64-bit integer and contain no X.
  Queued reads/writes must first be applied. Zero cycles need no TAP or reset.
  Native idle replay includes any explicitly enabled storage events; it does
  not infer BIST completion or live hardware readback.

The backend accepts one SI/SO boundary and at most one TAP. It rejects
multiple TAPs, multiple outputs/shared chains, intermediate serial taps,
one-hot/addressed transactions, feedback reset scheduling, iMerge,
system-clock/absolute-time waits, resource commands and other unsupported
operations. Transactions needing shadow-register effects or independently gated
interface events are outside the current complete-scan model; inability to
schedule them is not proof of hardware unreachability.

`--local_reset PATH` currently accepts one PDL source and preserves its bytes,
including all procedures, when the elaborated reset paths need no local-reset
TDR release. The check follows single-source ResetPort/ToResetPort forwarding
to global reset, known constants, or an external top-level ResetPort. Register
or logic-driven reset paths, unknown/unresolved sources and cycles are rejected.
This conservative check visits all modeled register reset paths, even for
registers not accessed by the supplied procedure. Successful flattened generation
is required before any reset artifact is published. Multiple PDL inputs and
insertion of transitive local-reset releases remain unsupported by this option.
These restrictions are outstanding requirements, not a reduced final scope.

## Validation and limits

Every candidate scan is replayed through the separate concrete hardware-state
engine before it is accepted. Tests compare source and flattened ICL output,
then re-ingest generated PDL and require byte-identical output. Reads retain
expected values separately from update values; these tests do not claim that
unknown external hardware will match a requested expected value.

A small independently enumerated nested-SIB model checks minimum scan count and
shifted-bit cost. TAP transactions additionally count the IR/DR entry and exit
cycles when comparing schedules within a horizon. The solver searches increasing
scan horizons up to 32 scans,
with resource/time limits; exceeding a bound is not a proof of unreachability.
Optimality evidence is limited to this supported transaction model and each
apply group. It does not establish global optimality over an entire PDL program.

Output paths cannot alias inputs or each other, including symlinks/hardlinks.
All artifacts are staged after successful analysis, then renamed. Multiple file
renames are not atomic as a group; a publication failure reports that earlier
renames may have completed. Existing formatter file-replacement checks are
shared with the generator.

Developer obligations remain in the [design](superpowers/specs/2026-10-25-icl-pattern-generation-design.md),
[plan](superpowers/plans/2026-10-25-icl-pattern-generation.md), and
[model coverage ledger](pattern-model-coverage.md). No complete generator/Tessent
pattern-equivalence result has been established.

TCK idle replay stops simulating once the complete network state repeats on
consecutive Run-Test/Idle cycles with identical inputs; generated PDL still
contains the full requested count. Changing states are replayed up to a bound
of 4,096 cycles, reduced for large networks, and a 15-second deadline checked
between cycles.
Exceeding either bound reports an error without committing partial state.
This acceleration is limited to the current behavioral models; external clocks,
time-dependent instruments and live readback require additional modeling.

### Serial-backend checkpoint (`26cee4a`, 2026-10-26)

- `make check BISON=/usr/bin/bison -j4` passed after updating the installation
  test for the fifth executable and its Z3 header dependency. This includes
  dynamic/static tests, 334/334 grammar rules, 119/119 scanner rules, and Vim plus
  Neovim corpus runs over 38,627 ICL and 802 PDL files each.
- `make formatcheck namingcheck BISON=/usr/bin/bison` passed. Focused Clang
  ASan/UBSan scheduler, generator, symbolic, runtime and state tests passed with
  leak detection disabled. Z3 itself was not sanitizer-instrumented.
- The source archive built and installed all five executables with Bison/Flex
  disabled and static Tcl unavailable. Installed dynamic and static generator
  executables returned version/help successfully.
- The subsequent strict authored-code coverage check failed its unchanged
  100% line / 90% raw branch thresholds: **10,258/10,799 lines (95.0%)** and
  **16,671/27,698 branches (60.2%)**. Functional regression passing does not
  satisfy that coverage gate.
- On this host with g++ `-O2`, `/usr/bin/time build/pattern_scheduler_test`
  measured 0.12 s and 52,204 KiB maximum RSS. The native runtime suite measured
  0.07 s and 22,796 KiB, with its 1,000-interface construction at 0.00434 s.
  These are single focused-suite measurements during other verification work,
  not whole-corpus retargeting throughput or a speedup claim.

Logs are generated under `build/pattern-generator-*.log` and remain outside Git.

### Single-TAP integration verification (2026-10-26)

- Final complete `make check BISON=/usr/bin/bison -j4` passed, including
  dynamic/static generator tests, all 334 grammar/119 scanner rules, and both
  editors over 38,627 ICL and 802 PDL files. Eight generator CLI tests include
  TAP instruction switching, BSDL validation and original/flat/generated replay.
- Google format/naming checks and focused Clang ASan/UBSan scheduler, generator
  and state suites passed (leak detection disabled; Z3 was not instrumented).
  Independent review found no blocking issue.
- Strict coverage completed its instrumented suites but failed the unchanged
  gate: **10,443/10,977 lines (95.1%)**, **17,019/28,230 raw branches (60.3%)**.
  The new inline TAP accessors are included. This is not a coverage-gate pass.
- `make install-dynamic` passed. The installed generator accepted a source ICL,
  external BSDL and PDL, then reproduced identical PDL from its flattened ICL
  and generated PDL.
- On this host with g++ `-O2`, the expanded focused scheduler suite took 3.46 s
  and 63,580 KiB maximum RSS while other regression work was active. This suite
  includes bounded unsatisfiable cases; it is not a large-project throughput
  measurement or a comparison against the earlier, smaller suite.

Logs: `build/tap-scheduling-delivery-regression.log`,
`build/tap-scheduling-coverage.log`, `build/tap-scheduling-sanitize.log`,
`build/tap-scheduling-style.log`, and `build/tap-scheduling-install.log`.
Full Tessent pattern equivalence remains unverified.

### Shared-update scheduling verification (2026-10-26)

`iApply -together` now reaches the existing serial/single-TAP solver. The solver
requires a common update frame with all requested values and deasserted resets;
concrete replay independently verifies the actual writes before the emitter
retains `-together`. Matching a reset value without an update does not satisfy
the request. Additional constraints scale with horizon × written-register count;
the existing search and resource limits are unchanged.

- Complete `make check BISON=/usr/bin/bison -j4` passed: dynamic/static tests,
  334/334 grammar and 119/119 scanner rules, and both editor corpus runs.
- Nine generator CLI tests and native scheduler tests cover prerequisites,
  reset release, exclusive paths, read-only groups, TAP instruction changes,
  failure atomicity, and original/flat/generated replay.
- Independent review, Google format/naming checks and focused Clang ASan/UBSan
  passed (leak checks disabled; Z3 uninstrumented).
- Strict coverage still fails the unchanged gate: **10,475/11,010 lines
  (95.1%)**, **17,093/28,350 raw branches (60.3%)**.
- `make install-dynamic` and all nine CLI tests against the installed generator
  passed. The focused scheduler suite measured 2.98 s
  and 79,844 KiB peak RSS during other verification work; this is not a
  large-network throughput measurement or a speedup comparison.

Evidence: `build/apply-together-regression.log`, `-coverage.log`, `-style.log`,
`-sanitize.log`, `-measurement.log`, and `-install.log` (all share the
`build/apply-together` prefix). Merge/resource/timing semantics remain unfinished;
this change does not establish full retargeting or Tessent equivalence.

### TCK idle replay verification (2026-10-26)

- Final `make check BISON=/usr/bin/bison -j4` passed after all source/test
  changes: dynamic/static tests, 334/334 grammar and 119/119 scanner rules,
  and Vim/Neovim corpus checks over 38,628 ICL and 802 PDL files each.
- Eleven CLI tests and native scheduler/state tests cover huge stable counts,
  changing/periodic state, rollback at replay limits, invalid counts, pending
  operations, unsupported clock modes and source/flat/generated replay.
- Independent review found a transient local-reset pulse that cleared hardware
  but left stale fill. The new `access_link/idle_reset.icl` fixture reproduced
  an incorrect `0xff` write; reset-event tracking now produces `0x00` and passes
  both replay paths. Re-review found no further issues.
- Google format/naming checks and focused Clang ASan/UBSan passed; leak checks
  were disabled and the external Z3 library was not instrumented.
- Strict coverage completed its functional suites but failed the unchanged
  100% line / 90% raw-branch gate: **10,546/11,083 lines (95.2%)** and
  **17,198/28,510 raw branches (60.3%)**.
- Dynamic installation and all eleven installed-generator CLI tests passed.
  On this host, g++ `-O2` ran the focused scheduler suite in 3.10 s with
  77,944 KiB peak RSS during regression; this is not a large-network benchmark
  or a speedup claim.

Evidence: `build/tck-idle-final-regression.log`, `-coverage.log`, `-focused.log`,
`-sanitize.log`, `-timing.log` (same `build/tck-idle-final` prefix), plus
`build/tck-idle-install.log`, `build/tck-idle-installed-tests.log`, and the
reset reproduction in `build/tck-idle-reset-red.log`. No new Tessent-equivalence
run was performed; the remaining generator requirements above still apply.

### Enumerated operand verification (2026-10-26)

`iRead`/`iWrite` now use the elaborated target's enum table through the shared
operand converter. The new parameterized `enumerated_values.icl` fixture and
native tests cover distinct instance values, alias-local enums and inversion,
read/write X masks, slice sizing, unknown/unattached names and significant-bit
truncation. Source/flat/generated-PDL replay matches.

- Complete `make check BISON=/usr/bin/bison -j4` passed, including static/dynamic
  tests, all 334 grammar and 119 scanner rules, and both editor corpus runs over
  38,629 ICL and 802 PDL files each.
- Independent review, Google format/naming checks, focused Clang ASan/UBSan and
  all twelve installed-generator CLI tests passed. Sanitizer leak checks were
  disabled; Z3 remained uninstrumented.
- Strict coverage still fails the unchanged gate: **10,562/11,100 lines
  (95.2%)**, **17,241/28,562 raw branches (60.4%)**. No Tessent-equivalence
  run was performed for this continuation.

Evidence is in ignored `build/enum-operands-{red,focused,sanitize,regression,
coverage,install,installed-tests}.log`. Default generator outputs under `out/`
are also ignored; source fixtures remain tracked under `tests/`.

### Top-level pin assumptions verification (2026-10-26)

- Full `make check BISON=/usr/bin/bison -j4` passed after all source/test changes:
  dynamic/static tests, all 334 grammar and 119 scanner rules, and Vim/Neovim
  corpus runs over 38,629 ICL and 802 PDL files each.
- Native and CLI tests cover pin/register aliases, inversion, slices, X values,
  invalid mixed-alias atomicity, child/output-port rejection in source and flat
  netlists, controller IR protection, event-free snapshots and both replay paths.
- Independent review, Google format/naming checks, focused Clang ASan/UBSan,
  dynamic installation and thirteen installed-generator CLI tests passed.
  Leak checks were disabled and Z3 was not sanitizer-instrumented.
- Strict coverage completed its functional suites but still failed the unchanged
  100% line / 90% raw-branch gate: **10,586/11,118 lines (95.2%)** and
  **17,284/28,602 raw branches (60.4%)**.

The assumption loop previously copied an object's whole prior vector for every
selected bit. It now initializes that vector once per affected object. On this
x86_64 host with GCC 16.2.1 and `-O2`, three 65,536-bit pin-assumption samples
took 0.1550–0.1566 s before and 0.0021–0.0034 s after the change. Reproduce the
current measurement with `make pattern-scheduler-check`; its `pin assumption`
line measures only that call. These elapsed-time measurements do not establish
end-to-end throughput or peak-memory differences.

Logs: ignored `build/port-assumptions-{red,final-focused,sanitize,regression,
coverage,install,installed-tests}.log`, `baseline*.log` and `optimized*.log`
(all use the `build/port-assumptions-` prefix). Tessent equivalence was not rerun;
remaining generator requirements are unchanged.

### AccessTogether verification (2026-10-27)

The generator now preserves and enforces referenced alias `AccessTogether`
properties. Full groups include nested marked aliases even through slices;
unsupported members fail explicitly. Independent review identified two important
cases now covered by regression: a nested group hidden by an outer slice, and
overlapping read groups that must defer comparisons past prerequisite scans.
The shared target resolver, bounded scheduler and concrete replay implement
these rules without a second ICL parser or AST.

- Focused native scheduler/state/symbolic tests and fifteen generator CLI tests
  passed. Cases cover exclusive paths, failure atomicity, inversion, partial and
  nested aliases, reset-held unrequested members, unrelated accesses, overlapping
  reads and byte-identical original/flat/generated-PDL replay.
- Independent review and Google format/naming checks passed. Focused Clang
  ASan/UBSan passed with leak checks disabled and external Z3 uninstrumented.

- Full `make check BISON=/usr/bin/bison -j4` passed, including dynamic/static
  checks, 334/334 grammar rules, 119/119 scanner rules, and Vim/Neovim corpus
  runs over 38,631 ICL and 802 PDL files each.
- Strict coverage passed its functional suites but failed the unchanged
  100% line / 90% raw-branch gate: **10,657/11,191 lines (95.2%)** and
  **17,411/28,800 raw branches (60.5%)**. Grammar-rule reachability is distinct
  from authored-code coverage; it does not close this gap.
- `make install-dynamic` and all fifteen CLI tests against the installed
  generator passed. No generated sources, binaries or logs are added to Git.

Logs: `build/access-together-{regression,coverage,final-focused,sanitize,
install,installed-tests}.log`; failing pre-fix reproductions use the same prefix.
No Tessent equivalence or large-network throughput claim is made;
parallel-port/DataRegister access and `iApplyEndState` remain unfinished.

### End-state directive guard verification (2026-10-27)

A regression reproduced successful pattern publication while ignoring an alias
`iApplyEndState` obligation, both with and without `--debug`. The shared
scheduler now rejects these models with an ICL source location and alias name.
This protects direct-storage accesses as well as accesses through the alias.
Parsing, elaboration and flattened serialization continue to retain the legal
ICL property; this is a generator capability restriction.

Focused native, CLI, flattening and Google style checks passed after correcting
a test's source-location expectation: re-imported flattened ICL reports its own
input file. Independent review found no remaining issues. Native tests cover
source/flat behavior; CLI tests cover direct and aliased writes, reads, debug
mode, preserved existing output and absence of newly published debug output.

- Full `make check BISON=/usr/bin/bison -j4` passed, including dynamic/static
  checks, 334/334 grammar rules, 119/119 scanner rules and Vim/Neovim corpus
  runs over 38,632 ICL and 802 PDL files each.
- Focused Clang ASan/UBSan passed; leak checks were disabled and external Z3 was
  not instrumented. Dynamic installation and all sixteen installed-generator
  CLI tests passed.
- Strict coverage passed its functional suites but failed the unchanged
  100% line / 90% raw-branch gate: **10,661/11,195 lines (95.2%)** and
  **17,420/28,814 raw branches (60.5%)**.

Evidence: ignored `build/end-state-{red,native-red,final-focused,regression,
coverage,sanitize,install,installed-tests}.log`. Restoration scheduling and its
semantic validation remain mandatory unfinished work; no Tessent equivalence
is claimed. Generated artifacts remain outside Git.

### Tessent comparison before reset passthrough (2026-10-27)

The [earlier vendor rerun](tessent-oracle.md#reference-rerun-before-reset-passthrough-2026-10-27) matched all
33 expected Tessent outcomes and produced 15 verified PDL exports. The real
generator then failed all 33 comparison preparations because `--local_reset`
was unimplemented. No generator-equivalence comparison completed. The harness now
uses `--bsdl ENTITY FILE`; full restoration/reset-rewrite requirements remain
open. Reference results must not be reported as generator equivalence.


### Local-reset passthrough verification (2026-10-27)

The CLI now supports the restricted, single-input `--local_reset` behavior
specified above. Its memoized traversal reuses elaborated `node.reset` links;
it does not parse ICL again. Original PDL bytes, procedures and comments are
retained. Scheduling and validation must succeed before staged publication.
The option rejects controls that require real reset release insertion.

The old implementation rejected the newly supported cases; those regressions
now pass. Tests also cover root/global forwarding, X and cyclic reset sources,
instrument controls, source/output aliasing, staging failures, multiple input
rejection and source/flattened ICL replay. An initial cycle assertion incorrectly
expected the passthrough diagnostic when elaboration correctly rejected the
cycle earlier; only that expectation was corrected. A later full regression
correctly detected a fixture edited during the editor corpus run. After freezing
all fixtures, the complete suite was rerun successfully.

- `make pattern-generation-check tessent-oracle-check formatcheck namingcheck
  BISON=/usr/bin/bison -j4` passed: 18 CLI tests, 18 harness tests, three trace
  tests, Google formatting and naming checks.
- `make check BISON=/usr/bin/bison -j4`, with the existing reference environment,
  passed. Static/dynamic suites, all 334 grammar / 119 scanner rules, and both
  Vim and Neovim corpus tests passed (38,633 ICL / 805 PDL files per editor).
- Clang ASan/UBSan pattern-generation checks passed after the cycle assertion
  correction. Leak detection was disabled and external Z3 was uninstrumented.
- `make install-dynamic BISON=/usr/bin/bison` succeeded; all 18 CLI tests
  passed against `/home/banana/.local/bin/icl-pattern-generation`. Installation
  printed the combined ICL/PDL coc.nvim configuration.
- Strict coverage ran after the successful full check. Functional checks passed,
  but the unchanged 100% line / 90% raw-branch gate failed:
  **10,701/11,235 lines (95.2%)**, **17,487/28,916 branches (60.5%)**.

Evidence: `build/reset-passthrough-reviewed-focused.log`,
`build/reset-passthrough-frozen-{regression,coverage}.log`, and
`build/reset-passthrough-final-sanitize.log`, plus
`build/reset-passthrough-reviewed-{install,installed-tests}.log`. The [new vendor comparison](tessent-oracle.md#reset-passthrough-comparison-2026-10-27)
ran two TAP operation comparisons, both different, with one additional serial
case blocked by the vendor reference. It does not establish hardware equivalence.
General transitive reset release, batching and multiple-file rewriting remain
mandatory unfinished work.


### Tessent harness directory naming (2026-10-27)

New reference and comparison bundles use `test_<case-id>` directories under
`test_icl_toolsets/test_<topic>` on the remote host. The bundle records its prefix;
markerless archives keep their original paths. Both layouts pass runner and
static-comparison tests, with invalid prefixes, escaping IDs and case symlinks
rejected. The requested connections probe directory was moved intact to
`test_icl_toolsets/test_connections`.

`make tessent-oracle-check BISON=/usr/bin/bison` passed 22 harness tests and
three shared-parser trace tests. Independent review found no actionable issues.
The complete `make check BISON=/usr/bin/bison -j4` passed with the reference
environment, including all 334 grammar / 119 scanner rules and both editors over
38,633 ICL / 805 PDL files. Logs: `build/tessent-layout-final-focused.log` and
`build/tessent-layout-regression.log`. C++ production sources are unchanged;
sanitizers and strict coverage were not rerun for this harness-only change.
The last measured 95.2% line / 60.5% raw-branch coverage gate remains unmet.

The [fresh full Tessent rerun](tessent-oracle.md#prefixed-layout-rerun-2026-10-27)
matched all 36 expected vendor outcomes (17 exports / 19 errors). Comparison
retains 33 generator blockers, one vendor blocker and two differing TAP traces;
none is claimed as an exact or hardware-equivalence pass. All 227 staged input
hashes and 19 export hashes were verified, and every successful export parsed
through the shared trace helper. The renamed connections probe is preserved;
no random-suffixed icl_toolsets experiment directory remains at the remote root.
