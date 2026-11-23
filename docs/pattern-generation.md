# Pattern generation

`icl-pattern-generation` is an experimental executable. It currently expands
PDL procedures and Tcl computations and schedules single-path scans, shared
output observations, simultaneous named client chains, independently selected
standalone clients, inferred disjoint interface-free scan lanes (including vector
one-hot returns), indexed vector inputs and common-phase TAP instruction/data scans.
It also schedules supported primary-pin and mixed primary/scan DataRegister operations. It is
**not yet the complete retargeter described in the requirements**.

## Current verification

Task143 completes the inventory's state-dependent/identical-argument caller
reset repair:40 caller methods,35 reservation methods and compound12 pass.
All52 generator suites/879 cases per dynamic/static variant, fresh sanitizer
runtime/eight reset-resource suites/main71 and source packaging pass. The13
native suites reuse their actual full pass after3992 shared inputs and13 products
are verified unchanged. One Critical and four Important review findings are
fixed in one verified pass. Two fresh post-fix Tessent2026.2 runs produce equal
full exports; eight nonempty physical source/flat/generated replays match.

Task141 is integrated at0188a6a:13 native pattern suites,51 generator
suites/852 cases per dynamic/static variant, leak-enabled sanitizers,30
changed-code quality checks and source packaging pass. It repairs flat clock
override names, honest history/status and protected child serial ports.

Task142 adds typed disjoint vector-port functions. Its17 focused methods pass
per binary variant, nine normalized models run successfully through Tessent2026.2,
and72 nonempty native replays agree in18 directional comparisons. Full native,
generator, sanitizer, style and packaging checks pass. Independent whole-branch
review found two Important blockers, both fixed in one verified pass; see the Task142 section.

Task140's final reviewed BSDL/default qualification has19 focused methods,
13 native suites and51 suites/837 cases per variant, plus sanitizers,
36 style/C++17 gates and source packaging. Its standard/vendor compatibility
differences remain documented. Broader corpus/topology/event work stays open.


Task139 completes physical replay diagnostics. Its one independent review led
to RED→GREEN fixes for event-work exhaustion and suppressed route-trace causes,
plus a native-eligible ordinary fallback regression. All30 focused cases,
full native scheduler,51 generator suites/822 cases per dynamic/static variant,
SAN native/focused30/main71, six style/C++17 gates and source-distribution checks
pass. Evidence is retained under build/physical-replay-diagnostics-20261120/.
The optional active built-in register name remains a deferred message-quality
Minor. The broader tracker items and excluded coverage gate retain their status.

The following Task137 platform/package/whole-project baseline is historical:

Task137 is integrated and pushed at400d2ba (2026-11-20). It adds explicit
--physical_pdl replay of binary scan loads on one bound root TAP with scan-only
storage. Ordinary iWrite retains final-value semantics. Independent review found
two retained-fill defects, fixed at83a8eb8 with RED→GREEN regressions: repeated
reset must preserve unreset storage, and bypassed updates to partial-X values
must invalidate stale fill. Physical reset/CSU fill follows actual stored values.
Task139 resolves that diagnostic: physical replay identifies the zero-based serial
bit and expected/observed values, named closed routes, unknown retained loads,
and original reservation failures. Ordinary native probes retain solver fallback.

All28 reset tests pass for dynamic/static and leak-enabled sanitizer binaries.
Full native runtime/scheduler and all reset/source/main sanitizer suites pass.
The complete normal check exits0 in1750.319s:51 generator suites/820 cases per
dynamic/static variant,1916 regressions,2180 conformance cases,334 parser and119
scanner rules over2549 ICL files, and six editor tests.

Fresh Rocky Linux native/strict-C++17 checks and51 suites/820 cases per variant
pass. Source-archive build,12 local package gates and14 remote installation/
identity gates pass. The final archive contains4245 tracked shipped files plus
six generated entries; all4013 non-documentation files compared byte-for-byte and ten products match qualified
installation evidence. Only the two generator products changed from Task136;
all3994 runtime inputs match the verified merged tree.

Four archived vendor export/model combinations now replay successfully with
--physical_pdl, preserving13 explicit loads and seven apply boundaries each.
Generated outputs are identical and idempotent on original/flat models.
This is native replay of existing vendor exports, not a fresh vendor/hardware
run or a general equivalence certificate. See [physical replay](#physical-reset-pattern-replay).

Fresh unfiltered coverage passes58 suites/982 cases but fails the unchanged
100% line/90% raw-branch gate:17589/18193 lines(96.68%) and29784/47295
branches(62.97%). All54 reports,19 instrumented binaries,12347 missing throw
edges and5164 other missing edges are retained. Coverage closure is excluded
by the user's subsequent instruction; full-release acceptance is not claimed.

The Task137 full run skipped five optional ANTLR reference cases because
ICL_REF_DIR was unset. The Task138 follow-up executes all five plus the adapter
availability test with the pinned reference/runtime: six methods, zero skips,
exit0. The generator code and qualified products are unchanged.
[Items2–15](remaining-work.md) are the authoritative current completion tracker.

## Vendor round-trip acceptance

The [directional qualification harness](tessent-goldens.md) accepts either local
flat output re-exported unchanged by Tessent, or Tessent source output replayed
unchanged by the local generator. Compare ordered apply groups, allowing
independent writes inside a group to reorder. Independent source schedules need
not match. Stored directional pin/vector, TAP, reservation, reset, DataRegister,
computed-port, control-ownership and merged-observation standards pass offline.
The concrete scheduler test also covers writing a selected TDR while closing its
SIB on the same update, then reopening before the next target write.

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

## Combinational primary-input writes

For a network without scan elements, `iWrite` on data ports
or ordinary aliases can solve for primary DataInPort values. One transaction
satisfies the queued requirements and concrete replay verifies the result before
publishing any state or output. Inputs unrelated to the goals remain undriven.
The solver minimizes driven port vectors, then uses stable name/bit ordering
relative to controlling fills to choose a deterministic solution.

```icl
Module Pins {
  DataInPort data[7:0];
  DataInPort enable;
  DataOutPort value[7:0] { Source data; Enable enable; }
}
```

```tcl
iProcsForModule Pins
iWrite value 0x96
iApply
```

The generated transaction writes `data` as `0b10010110`, `enable` as `0b1`,
then applies them. Native tests cover initial zero controlling state, X retention,
independent output controlling history after upstream writes, last-write wins,
contradiction rollback and preservation of unused inputs. Ordinary ports retain
0/1 controlling values separately from potentially unknown hardware values.
The fixed OneHot Port-width padding exception is documented in the
[retargeting references](pattern-retargeting-references.md#combinational-port-writes).

OneHotDataGroup accepts Port contributors whose DataOutPort paths have uniform
explicit enables, including forwarding ports. Exactly one known-enabled
contributor selects its data; zero, multiple or uncertain enables produce X.
Addressed register/instance contributors and routed reads are supported on the
primary-input path described below. Scalar OneHotScanGroup execution is supported
by the serial backend described below. Direct primary-input write groups and
primary-output read groups use the same AccessTogether checks as registers.
Mixed DataRegister/primary-input/scan transactions use the shared
frame scheduler described in the mixed-I/O section below.
No new grammar or AST is introduced.

This path is bounded to 256 primary input vectors and 4096 total input bits,
with a two-second solver timeout. A limit is not proof of unreachability.
The 1024-bit native regression (`make pattern-scheduler-check
BISON=/usr/bin/bison`) measured 0.17–0.20 seconds for a direct output write on
Linux x86_64 with GCC 16.2.1, `-O2`, and Z3 4.13.3. Logs are
`build/primary-write-final-regression.log`; the timed region includes solving
and concrete replay, not compilation. The prior backend rejected this input,
so there is no supported old throughput baseline or claimed speedup. This is
a scale check, not a throughput guarantee for arbitrary logic cones.

## DataRegister access

The primary-input backend supports selectable and addressed DataRegisters,
including cascades, partial and enum writes, addressed instance offsets, shared
buses, and non-destructive reads. It reuses the shared ICL/PDL frontends, netlist,
address resolver, symbolic values, concrete event state and PDL serializer.
No second parser or register model is introduced.

```sh
build/icl-pattern-generation \
  --icl tests/cases/ours/pattern_generation/data_registers/addressed.icl \
  --pdl tests/cases/ours/pattern_generation/data_registers/addressed.pdl \
  --output out/addressed.pdl --debug
```

`DataRegisterPlan` caches activation and dependency order. Structural read routes
are built once per scheduler and reused across applies. `NetworkState` applies
parallel pin changes atomically, then settles cascaded data updates and resets.
The serial/single-TAP scheduler includes the same data storage in its temporal
frames and settles it after qualified scan updates. It replays each emitted scan
against the concrete state, checking data values, known-bit masks and controlling
fills. Cascades use the shared dependency order. Unrequested data retains its
controlling fill unless it supplies another required control or explicit scan
writes determine its source.
IEEE 1687-2014 §6.4.9, printed pp.118–121, defines cascaded evaluation after
UpdateEn, controlling-value fill, reset priority and address/read/write activation.
Tessent's v2023.4 DataRegister/Instance topics supplement inherited connections.

The scheduler models up to 32 parallel events, choosing shorter horizons first.
It then prefers controlling-value retention and fewer pin writes. It does not
claim global minimum pattern length. Simultaneous writes on incompatible address
codes fail under `iApply -together`; ordinary applies can split them. Already-known
DataRegister values need not be rewritten, which also preserves held end-state
overrides when low-level PDL is replayed. Physical pin writes remain explicit.

Read routes follow direct/inverted forwarding, DataMux choices and addressed
OneHotDataGroup register/instance contributors. Read expectations are comparisons,
not constraints that manufacture storage. Pending reads preserve observed storage;
held write enables are allowed only when both value and known-bit mask remain
unchanged. Output-port reads conservatively protect all DataRegisters in their
physical output's fan-in, including logic expressions; this can reject a route
that needs to change an unobserved alternative. Arbitrary logic does not become a
transparent route for direct register comparisons. Unknown or competing OneHot
contributors yield unknown data. Explicit output comparisons may compare unknown
hardware values, just as scan comparisons can.

Limits: 4096 writable DataRegisters, 4 Mi dependency visits, 256 primary vectors /
4096 input bits, 262144 read-route visits and at most 256 selection guards per path.
The temporal model limits horizon × (netlist nodes + input bits) to 262144, with
2 seconds per solver invocation and 15 seconds across horizons. Exhaustion reports
a limit, not proven unreachability. Self-referential and mutually dependent data
updates use bounded simultaneous snapshots within each feedback component,
starting from real pre-event storage. A self hold retains its actual value;
convergent unknown intermediates may settle, while known oscillations reject.
The limits are32 delta rounds and4 Mi feedback-bit evaluations per native event
or symbolic query. Settled symbolic events cannot discard previously known
storage bits. These are digital cascade semantics, without a physical timing or
metastability claim. DataRegister callback declarations and their parameterized
arguments survive flat ICL serialization. Callback procedure invocation remains
unsupported; declaration replay does not perform a register access.
Mixed primary-drive/scan DataRegister access
is supported. Ordinary write-only groups of at least four addressed data goals on
one physical scan-driven address vector use deterministic address batches.
Already-satisfied goals join the first necessary batch, or one controller update
if all are satisfied. Same-address members can share a scan; partial/X goals use
the fill from before any batch. Reads, AccessTogether, -together, end-state
obligations and unrelated address buses retain joint temporal solving.

Each batch reuses the existing solver and concrete replay on speculative state.
All final goals must hold before state, controlling fills and TAP execution are
published; a bounded infeasible ordering falls back to joint solving. Explicit
solver timeout/resource exceptions remain errors rather than unlimited retries.
Batch planning checks a 15-second attempt budget and a 262144 accumulated
scan/structure work limit between batches; each solver retains its existing
bounds. Structural limits do not guarantee scheduling of every smaller network. Control events must have definite activation unless reset dominates.

The focused suite is `tests/src/pattern/test_data_register_scheduling.py`; it is
included in dynamic/static and coverage recipes. Native tests check address
collisions, unknown enables/events, reset priority, atomic failures and a
reverse-declared 1024-register cascade. The source-derived probes and provenance
are in `tests/cases/ours/pattern_generation/data_registers/`.

### Scan-driven writes and captured reads

`tests/cases/ours/pattern_generation/scan_data_registers/` contains standalone
serial and BSDL-backed TAP examples. To run the TAP example:

```sh
build/icl-pattern-generation \
  --icl tests/cases/ours/pattern_generation/scan_data_registers/vendor.icl \
  --pdl tests/cases/ours/pattern_generation/scan_data_registers/vendor.pdl \
  --bsdl AccessProbe tests/cases/ours/pattern_generation/access_link/simple.bsdl \
  --output out/scan-data.pdl --debug
```

Writes justify data, address and enables through controlling scan registers.
Even a same-value explicit write must update those controllers at least once
(IEEE 7.9.14(a)). Capture routes follow forwarding, inversion, DataMux and
addressed OneHotDataGroup selection in the **pre-update** state. Comparisons do
not constrain stored data to match the expected value. Prerequisite scans preserve
pending observed storage; the observation scan may subsequently update that data.
Repeated reads use the last expectation for each bit, including an unqualified
read replacing earlier care bits. Distinct logical targets colliding on a capture
bit use separate scans. `-together` synchronizes writes only; Alias AccessTogether
still keeps its read members in one observation.

Physical writes and comparisons are emitted against ScanRegisters. Temporary
DataRegister end-state override writes are retained where needed for replay,
using the same marker logic as primary-input patterns. Ordinary DataRegister
accesses do not require a separate serializer or a second parser.

The focused suite `tests/src/pattern/test_scan_data_register_scheduling.py` is
included in dynamic/static and coverage recipes. Its cases exercise partial and
addressed writes, cascades, capture-before-update, inversion, mux/OneHot routes,
reservations, grouping, end-state restoration, rejected unobservable reads,
atomic failures and source/flat/generated-PDL replay.

## Command support

| Command | Current generator behavior |
|---|---|
| `iProc`, `iCall`, prefixes and Tcl computation | Expand through the bounded runtime, preserving source order and call diagnostics. |
| `iRead`, `iWrite`, `iApply` | Schedule supported scan-register/alias accesses, primary-input writes and selectable/addressed DataRegister access; numeric or target-local enum operands; `-together` enforces a shared update cycle. |
| `iState` | Numeric assumptions for ScanRegister/DataRegister storage and supported top-level data/control inputs, including aliases and slices; default/`-LastWrittenValue` updates overlapping queued write/compare values. `-LastReadValue`/`-LastMiscompareValue` supply observation history and are forwarded without hardware changes. No device transaction. |
| `iReset` | Explicit top-level reset, including `-sync` for qualified common-phase TAPs. |
| `iRunLoop` | Literal TCK waits on standalone clients or reset qualified common-phase TAPs; zero count is a no-op. |
| `iNote` | Retain literal notes in generated PDL. |
| `iMerge` | Flatten nested blocks; order direct notes first; batch compatible register and connected/computed data/control-port accesses, including slices/aliases/X masks, using disjoint dependency ownership and serial fallback. |
| `iTake`, `iRelease` | ScanRegisters, DataRegisters, DataInPort/DataOutPort, AddressPort/ReadEnPort/WriteEnPort, their indexed/alias resources and instance members; bit-level protection after applied writes or `iState`, inherited ownership and scoped release. All declared port kinds support membership; Tessent default-instance and persistent takes are supported within the input stream. Assignment/frequency and broader pattern-set lifecycle remain unsupported. |
| System-clock commands | iClock/iClockOverride and relative waits work on qualified clock-only routes; restricted state-free absolute waits preserve literal time. General stateful time/clock events remain pending. |
| Observation getters | Explicit history and known-zero status remain available by default. --model_readback supplies actual PIO and serial-register observations, including bound TAPs and exact physical-PDL replay, with bitwise miscompares and failure counts; see [the example and limits](#task151-modeled-bound-tap-observations). Raw/interface/chain, timed and live-device observations remain open. |

This table describes generator execution. The formatter and LSP have separate,
broader syntax support and never execute PDL.

## Current transaction support

- Contiguous vector-port functions may share a base name while retaining
  each function's type, attributes, enum and clock properties. Whole-bus and
  selected references, parameterized child bindings and associated-flat replay
  resolve the actual declared bits. Overlap, gaps, inconsistent direction and
  scalar/non-port duplicates reject. Access and enum eligibility remain tied
  to the selected functions; see [typed port functions](#task142-typed-vector-port-functions).

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
- `AccessTogether` on a referenced ICL Alias keeps its full supported group
  in a single operation, including nested aliases and partial references.
  Writes require simultaneous updates after reset release; reads compare their
  requested bits together after any prerequisite scans. Unrelated accesses may
  use separate operations. ScanRegister and writable DataRegister members, direct
  primary-input writes, primary-output reads, and forwarded/computed data-port
  write members are supported within the joint scheduler bounds described below.
  Flattened ICL preserves the marker; emitted common updates use
  `iApply -together`.
- Alias `iApplyEndState` follows IEEE 1687-2014 6.5.2(h): known, consistent
  per-bit defaults, matching register reset values, and ungated direct source
  paths. Supported ultimate sources are ScanRegisters, selectable/addressed DataRegisters, primary DataInPorts and
  compatible constants. Explicit writes override defaults for their next apply;
  a later apply or end of the input sequence restores them. Partial/X writes,
  inversion and direct-storage access retain the directive. Register defaults
  participate in final-state constraints, allowing a SIB to close while its
  selected target updates. Automatic restoration cannot change a protected
  reservation. Mixed frames retain temporary primary-port overrides in emitted
  PDL so intervening applies cannot restore defaults prematurely.
  End-state models use the bounded symbolic scheduler, not the large-network
  greedy path; the existing resource limits still apply.
- Explicit `iState` with default/`-LastWrittenValue` semantics and `iReset`.
  State assumptions accept scan registers and top-level DataInPort or SelectPort
  pins with no internal driver; aliases retain source bit ordering and inversion.
  They change only the assumed snapshot, without clock edges or reset
  propagation. Internal/output ports and controller-owned instruction storage
  are rejected before a mixed alias can partially change valid targets.
  This does not widen the ordinary serial backend's event model: explicit
  scan-interface controls still require qualified controller/interface event bindings.
  Hardware starts unknown; ResetValue metadata is never treated as a power-on
  reset. A child-module iReset is rejected instead of promoted to a global reset.
  Reset discards pending reads/writes and clears resource ownership; it does
  not apply the discarded queue. Targets, declaration kinds and operands are
  still validated. Reset-only rewriting retains canceled source commands but
  does not insert releases for them.
- ScanMux and compiled LogicSignal selection, including register-controlled
  SIB and local-reset prerequisites. Reset release cannot enable an update on
  the same edge retroactively.
- Single-TAP IR/DR selection, retained instruction state across apply groups,
  and explicit/implicit scan-interface controls. The native fast path proves
  complete-scan equivalence; bounded event planning also models clocked bypassed
  storage when that proof does not hold. Every accepted transaction executes
  through the native TAP controller and child event model.
  Generated PDL includes IR selection writes and their apply boundaries, and
  compares the BSDL capture pattern for automatically inserted instruction
  scans. Explicit instruction reads preserve the user's compare mask/value;
  explicit write-only instruction scans do not gain a comparison. The modeled
  `tap.INSTRUCTION` target supports reads, writes and ranges; synthetic TAP data
  registers remain rejected. An instruction read/partial update after an
  ambiguous reset encoding requires explicit values for the unknown bits.
  Storage scanned while held in reset is emitted as an unqualified read rather
  than an update. These operations also survive flat-ICL/generated-PDL replay.
- Source-attributed failures and invocation stacks. Structural disconnection,
  unsupported behavior and bounded solver exhaustion are distinct errors.
- Literal TCK `iRunLoop` counts (IEEE 1687-2014 §7.9.17, printed pp. 175–176)
  on standalone clients or qualified common-phase TAPs after explicit reset. Standalone clients
  need no TAP or initial reset. `iRunLoop COUNT` defaults to TCK;
  `COUNT -tck` and `-tck COUNT` are accepted and emitted as decimal
  `COUNT -tck`. Counts must fit an unsigned 64-bit integer and contain no X.
  Queued reads/writes must first be applied. Zero cycles need no TAP or reset.
  Native idle replay includes any explicitly enabled storage events; it does
  not infer BIST completion or live hardware readback.

The backend accepts common-phase TAPs on equivalent root TCKs, or standalone
client interfaces selected one at a time, each with one path or disjoint named
chains. Interface-free networks can infer independent lanes when all static
routes from each output reach one distinct input bit and no graph vertex is
shared between lanes. Vector one-hot returns use these same lanes. TAPs and
effective interfaces require their existing supported event/chain models.
Multiple scalar or vector output bits may also observe one path through transparent forwarding,
including inversion. Equivalent ungated observations use the first sorted
output; gated alternatives retain their enables and require a known-enabled
path. All declared controls are still checked. A path may use an indexed vector input
and an indexed one-hot return lane. Explicit or inferred standalone client
interfaces support bound select/capture/shift/update/clock controls. The fast
path proves complete-scan equivalence; bounded event planning also tracks hidden
capture/shift/update storage when that proof does not hold. Unbound vector control lanes retain
their state, and parallel input frames do not pulse scan controls.
It rejects
independent TAP phases, shared serial storage, overlapping paths within a selected chain group, intermediate serial taps,
general serial- or phase-dependent reset events,
stateful absolute-time waits, unsupported reservation types and other unsupported
operations. Transactions needing general independently gated
interface events are outside the current complete-scan model; inability to
schedule them is not proof of hardware unreachability.

### Local-reset output

`--local_reset PATH` preserves source text, including procedure definitions, and
inserts reset-control writes before main-level writes, calls, Tcl compounds or
whole merge blocks. Releases are never inserted between a merge's begin/end markers. Multiple PDL
files are combined in command-line order, with separators that preserve commands
after continued EOF comments. When no release is needed, a single input is
preserved byte-for-byte.

Task143 first executes and strictly replays the actual joined candidate. An
already valid source is preserved without adding redundant releases. Every
candidate must retain the original operations and final PDL level. Unsafe file
return boundaries or skipped invocation cleanup reject before publication.

The planner follows transitive reset dependencies, including nested
`AccessTogether` aliases and logic-driven resets supported by the symbolic
network. It uses actual reset polarity and the scheduler's current state to
choose release values. Ancestor controls are released on earlier apply
boundaries; independent controls share an apply. The reset-only artifact contains
no additional SIB setup. The normal flattened output still comes from the
original PDL operation stream and includes scan-access scheduling.

Release-value selection includes resource ownership before solving. Written
reservations must retain their held value; taken-but-unwritten controls remain
at their current hardware state so inserted writes cannot acquire protection.
For example, if `reset = ~(a | b)` and `b` is reserved at zero, a writable `a`
can release reset instead. `iRelease b` makes that control available again.
If all release choices are unavailable, report the reserved/unwritten controls
and original source command, preserving existing output artifacts. The same
4096-control-bit and two-second solver bounds still apply.


Source tracing belongs to the existing PDL runtime; reset planning shares the
scan scheduler's netlist, concrete state and symbolic model. Insertions are
hoisted before whole pending queues so an added `iApply` cannot flush a user
write early. Main loops, conditionals and dynamic commands retain their entry
prefix and whole executed operation span. Overlapping compound/merge spans
use the earliest caller site, including merges continued across files. The
rewriter first tries whole-command prerequisites, then original literal sites
from fresh state when internal resets require releases inside a loop. There are
at most five candidates, using the same scheduler and solver bounds. If both
early-insertion candidates fail, a Tcl apply/uplevel wrapper evaluates the
original command words once, releases prerequisites, then executes that command
in its caller scope. This repairs argument substitutions that reset state before
a write or call. Parsed PDL vector ranges remain literal while their variable
indices and real Tcl substitutions retain their original evaluation order.
If one repeated literal caller site needs different release preambles, the
fourth candidate dispatches on the once-evaluated argument list using an exact
Tcl switch inside that same wrapper. The original call still executes in its
caller scope. Unknown signatures fail rather than selecting an arbitrary
release. When identical arguments need different releases, the fifth candidate
checks the recorded invocation sequence. Distinct argument/preamble records are
stored once; each modified site uses a compact sequence and counter in a guarded
private namespace. Empty calls and caught errors consume their actual recorded
occurrences; argument-evaluation failures do not. Original words evaluate once
before the preamble and call. Unexpected arguments/counts and namespace
interference reject before publication. Normal completion checks final counts,
removes private state and restores the actual final PDL level, including across
ordered files. A persistent mismatch flag survives caller catch. Cleanup checks
all array keys and immutable values; a separate verifier attests that cleanup
completed even if user code returns early. Procedure definitions remain byte-for-byte. This is checked
specialization of the recorded execution, without a universal equivalence claim
for arbitrary external state or introspective Tcl.
After patching the source, a fresh runtime executes the actual
rewritten text and compares its ordered commands and resolved arguments with
the planned stream. All analysis and replay must succeed before publication.

Remaining limits: a procedure that reasserts reset internally and then needs
another release cannot always be repaired from its caller without changing the
procedure; preserving the definition and reporting that case is the original
required contract. Cases that none of these caller-site strategies can repair remain
rejected. Unresolved
reset sources, forwarding cycles, serial-dependent reset events and unsupported controller
models remain errors. Reset solving is bounded to 4096 control bits and two
seconds per solver query; a limit is not proof of unreachability. These are
outstanding requirements, not a reduced final scope.

### Merge scheduling

IEEE 1687-2014 §7.9.18 permits serial execution or merging, requires an empty
queue at merge entry and direct-call return, and forbids `iReset` inside any
merged call. Direct `iNote` commands execute before the calls. Recursive merges
through `iCall` are supported; a directly nested begin/end pair in the same
procedure is invalid. Tessent IJTAG User's Manual v2023.4, “How to Run iCalls in
Parallel”, also describes serial fallback and recursive merging.

`pattern_merge.cc` consumes the existing Runtime operation stream. Call-stack
invocation identities distinguish repeated calls without introducing another
PDL parser. It combines consecutive calls when each contains one or more
plain applies with ScanRegister/DataRegister reads and writes or data/control-port
accesses (including slices, aliases and X masks), and their owned dependency sets
over the entire calls do not overlap. Ownership includes logical controlling
history and all reachable combinational inputs, stopping at registers and
unconnected primary inputs. Mux branches/selectors, one-hot contributors and
implicit address/read-enable selection are included. Shared inactive branches
may conservatively prevent batching. Aliases occupy every resolved object; even
disjoint slices of one object retain serial order to preserve controlling fill. Reads may use
X masks or omit expected data. It merges one apply group
from each active call per round, preserving every call's apply order, and removes
exhausted calls from subsequent rounds. It does not merge across outside apply
boundaries. Empty applies, AccessTogether aliases, notes within
calls, state changes, `-together`, and clock waits retain their serial barriers. Nested blocks
are processed recursively. Calls containing resource commands remain serialized.

The combined stream uses the existing scheduler and concrete replay. A failed
attempt is discarded and retried serially with a fresh scheduler; artifacts are
published only after success. This is a conservative heuristic, not a proof of
the globally shortest pattern. A shared apply may still need multiple scans.
The merge pass uses ordered-set conflict checks, not interleaving enumeration.
Dependency references are charged before enqueueing against a shared 4,194,304
visit budget, including implicit one-hot selection traversal. Exhaustion retains
serial scheduling. Visited sets avoid revisiting nodes within a call; cached
one-hot selections avoid repeating completed selection walks.

Regression input is under `tests/cases/ours/pattern_generation/merge/`; the
runner is `tests/src/pattern/test_merge_scheduling.py`. It checks source/flat
ICL and generated-PDL fixed points, notes, nested/repeated calls, conflicting
writes, barriers, reset rules, reset-only hoisting and fresh-state fallback.
The scale case combines 256 calls into one apply. These are locally checked
cases, not newly qualified Tessent goldens.

Verification on 2026-10-27: full `make check BISON=/usr/bin/bison -j4` passed
(`build/merge-scheduling-regression.log`), including dynamic/static merge tests
and unchanged offline vendor goldens. ASan/UBSan with leak detection enabled
passed nine merge tests, 26 existing generator CLI tests and native runtime
tests. The 256-call case (26,471 source bytes) took 0.120 seconds; the Python
harness measured 0.17 seconds and 60,908 KiB peak RSS on WSL2 with g++ 16.2.1
`-O2` and Z3 4.13.3. See the [plan and evidence](superpowers/plans/2026-10-27-merge-scheduling.md)
for commands and limits. No fresh Tessent equivalence or strict authored-C++
coverage run was performed for this increment.

### Resource reservations

`pattern_resources.cc` tracks ownership using the existing runtime call stacks.
An `iTake` alone does not pin hardware: the retargeter remains free to configure
it until an explicit user write is applied (IEEE 1687-2014 §7.9.19), or an
explicit `iState` assigns it (Tessent Shell Reference Manual v2023.4, `iTake`). A take
within the same pending apply group also protects that group's applied value.
Preparatory writes generated by the scheduler do not acquire protection.

Supported resources are ScanRegisters, DataRegisters, DataInPort/DataOutPort,
AddressPort/ReadEnPort/WriteEnPort, their indexed ranges and aliases, and supported
members in an instance subtree. Ownership is tracked per resource bit: reversed, ascending, concatenated and inverted aliases resolve through
the shared PDL target resolver. Only explicitly assigned taken bits become
protected; other bits remain available to the retargeter. Partial release clears
only the named bits. Other serial-control/clock/scan ports remain unsupported.
Taking an instance does not add support for otherwise unsupported transactions. Calls inherit ownership; returning drops only acquisitions made
in that call. Explicit release propagates to inherited ownership, following
§7.9.20(e) and Tessent IJTAG User's Manual v2023.4, “How to Run iCalls in
Parallel”. Overlapping parent/child takes do not lose the parent's protection
on return.

Tessent's `iTake [resource] [-persistent]` extension is supported for this
resource subset. Missing or empty resources select the effective instance
from the current `iCall` and `iPrefix`. Persistent takes survive procedure
returns and later input files until explicitly released. Ordinary nested takes
do not erase overlapping persistent ownership when they return. This executable
does not model Tessent pattern-set, `test_setup` or `test_end` lifecycles.

`iTake` must precede `iState` to protect the assumed value. A later explicit
`iState` may replace it, including through an alias or slice, without a hardware
transaction. Unknown protected values and assumptions with pending actions are
rejected. An assumption made before the take is not retroactively protected.
These defaults and extensions follow the v2023.4 reference topic
`tshell_ref/topics/Command_Itake_idd8a68c5f.html`.

The scheduler constrains protected values at each scan or primary-input state
and checks them during concrete replay. DataOutPort protection checks the
computed value through its source and enable, using the shared symbolic model. Explicit user writes may replace a protected
value with their requested value; automatic reset-only preambles cannot.
Failed schedules or idle replays do not publish partial state or files. Taking
a resource after an earlier apply does not retroactively protect that write.
Reset-only rewriting also rejects insertion of an explicit prerequisite write
to a taken-but-unwritten bit: executing that file would otherwise acquire
a lock that the original source never acquired. The reset planner can emit
partial writes to the other bits of the same control register. Unknown unwritten
bits retain their unknown state in planning, so an unrelated unknown bit does
not prevent a legal release.
Read reservations do not authorize concurrent interleaving: calls using
reservations remain serialized.

The fully generated pattern resolves reservations into concrete writes/applies,
without retaining procedure scopes or resource directives. Generated-PDL and
flattened-ICL replay are tested. Source-preserving `--local_reset` output retains
the original resource commands and is separately re-executed before publication.

Large write-only applies can use dependency-guided scheduling with reservations;
reads and incompatible prerequisite configurations use the bounded solver.
`iReset` and `iReset -sync` clear resource ownership and protected values after
the physical reset succeeds. A subsequent write alone does not reacquire an old
take; a new `iTake` is required. Pending actions still make reset invalid.
TCK waits must preserve protected values. Full resource/model support and
broader scheduling scale qualification remain outstanding requirements.

Fixtures: `tests/cases/ours/pattern_generation/resources/`; regression:
`tests/src/pattern/test_resource_reservations.py`. Tests include protected SIB
failures, same-group takes, explicit updates, inherited/overlapping ownership,
instance and alias/partial writes, merges, reset-rewrite protection and TAP
waits. Native tests also check rollback after rejected writes and changing idle
states. The scale case protects 128 registers and updates an independent target.

Verification on 2026-10-27: the final full regression passed after the review
fix (`build/resource-reservations-final-regression.log`), including thirteen
dynamic/static resource cases and seven unchanged offline vendor goldens.
ASan/UBSan with leak detection enabled passed those thirteen cases, 26 existing
generator cases and native scheduler checks. The 128-register case took 0.073 s
for the CLI, or 0.13 s / 57,616 KiB peak RSS including its Python harness, on
WSL2 with g++ 16.2.1 `-O2` and Z3 4.13.3. Per-clock reservation checks scale with
protected bit count; this measurement does not establish a general performance
bound. See [plan and evidence](superpowers/plans/2026-10-27-resource-reservations.md).
Fresh Tessent equivalence and strict authored-C++ coverage were not rerun.

Persistent/default takes and taken-state assumptions were added in the
[next increment](superpowers/plans/2026-10-27-persistent-reservations.md).
Its full regression passed (`build/persistent-regression.log`), including
17 resource tests for both dynamic/static executables, seven offline vendor
goldens, full grammar/scanner rule coverage, and Vim/Neovim corpus checks.
Leak-enabled ASan/UBSan passed those 17 cases, 26 existing generator cases and
native scheduler/runtime checks. Independent review found no defects.
Fresh Tessent execution and strict authored-C++ coverage remain unrun for this
increment; these checks do not establish complete vendor equivalence.

### Dependency-guided scheduling

Write-only applies on networks with at least 32 ScanRegisters first try a
dependency-guided planner. This is an implementation threshold, not an IEEE
limit. A shared serial-path constraint pass solves one desired selector/reset
configuration, minimizing the number of changed control registers. Only the
controllers needed to establish access are proposed; actual reset polarity and
logic remain in the existing symbolic netlist model.

The planner batches currently reachable, reset-released controllers toward that
configuration. It does not assume a reset release or SIB update can take effect
earlier in the same scan. Each batch uses the existing solver for at most one DR
scan plus a TAP instruction scan when necessary, followed by concrete replay.
Known prerequisite bits are retained across batches. The final batch preserves
the original resolved full-register values for partial/X writes and the original
alias and `iApply -together` obligations. Explicit requests may close SIBs in
that final update; otherwise prerequisite selectors may remain open.

All speculative state and controlling fill belong to a trial scheduler. They
are committed only after the final request succeeds. No-progress, incompatible
snapshot configurations or exhausted bounded batch attempts return to the
original solver without publishing trial state. A directly accessible request
tries a single scan first, avoiding unnecessary prerequisite solving when
replaying generated PDL. Reads continue through the original solver.

Reservations are checked in both the directly accessible fast path and every
prerequisite batch. A protected controller can stay at its held value or advance
to the original apply's explicitly requested full-register value, including its
non-control bits. Batch boundaries retain the new protected value, preventing a
return to the old value during later preparation. Automatic local-reset writes
do not acquire permission to change protected values. Partial/X requests retain
the controlling fill from before preparation. A protected register unrelated to
the access path therefore does not force a deep network into the 32-scan solver.

Protected-batch regression covers a 40-level SIB chain (41 scans), explicit
partial/X/full updates to a protected controller, forbidden intermediate values,
automatic-write restrictions, failed-trial rollback and TAP replay. The full
regression passed (`build/protected-dependency-regression.log`), as did independent
review and leak-enabled ASan/UBSan checks of 13 dependency cases, 17 reservation
cases and the native scheduler. The 5398-byte protected deep-chain case took
0.313 s for generation; measurements overlapped other checks and are not a
speedup claim. See the [plan and evidence](superpowers/plans/2026-10-27-protected-dependency-scheduling.md).
Fresh Tessent equivalence and strict authored-C++ coverage were not rerun.

This is a heuristic, not a globally optimal scheduler. It groups targets whose
reset/select requirements can coexist; mutually exclusive paths use the bounded
fallback. Snapshot queries retain a two-second solver timeout and a 4096-bit
control-choice limit. The planner checks a 15-second attempt budget between
batches and requires fewer unresolved controls after each preparation batch.
These limits are separate from the existing structural bounds. Unsupported
models and solver exhaustion are not evidence of hardware unreachability.

Self-constructed fixtures and an independent tree model check 40 nested SIBs
(41 scans), 64 independent SIB/TDR pairs (two scans), and 256 pairs/512 registers
(two scans). Additional cases exercise TAP select gating, reset dependencies,
aliases, explicit closure/reopening, partial and X writes, fallback, unknown
initial state, rollback and source/flat/generated-PDL replay. Small finite-state
searches remain independent optimality oracles for their specific topologies.

## Validation and limits

### Dependency-guided scheduler verification (2026-10-27)

The final full regression exited 0 (`build/dependency-scheduling-regression.log`).
Dynamic and static builds each passed 26 existing generator CLI cases and ten
new dependency-scheduling cases, plus native scheduler tests. All seven offline
Tessent goldens still match. Grammar/scanner rule coverage remains 334/334 and
119/119; Vim and Neovim each checked 38,639 ICL and 811 PDL files. The corpus
audit found no duplicate-file groups. Style checks passed.

ASan/UBSan passed the new/existing CLI and native scheduler tests with leak
detection enabled, outside the sandbox. Independent review caught a partial-write
fill regression; the reproduced failure and fix are recorded in the
[implementation plan](superpowers/plans/2026-10-27-dependency-guided-scheduling.md).
The strict authored-C++ coverage gate and fresh Tessent equivalence were not
rerun for this scheduler change; existing gaps remain open.

Repeated full CLI measurements with g++ 16.2.1 `-O2` on Linux 6.18.33.2 WSL2:

| Input | ICL bytes | Result | Wall time | Peak RSS |
|---|---:|---|---:|---:|
| 40 nested SIBs | 5,329 | 41 scans | 0.31–0.34 s | 74,364–74,704 KiB |
| 64 independent SIB/TDR pairs | 13,245 | 2 scans | 0.23–0.24 s | 75,460–75,784 KiB |

The 256-pair/512-register test uses 53,661 input bytes and completed in 2.97 s
in the focused suite. These measurements establish the tested scale, not a
general optimality or maximum-throughput claim. Commands and repetitions are in
`build/dependency-scheduling-measurements.log`.

### Local-reset rewrite verification (2026-10-27)

The local-reset checkpoint regression exited 0 after the rewrite, source reorganization
and reset dependency ordering change. Both dynamic and static CLI suites passed
26 tests; the seven existing offline Tessent standards still match. Grammar
coverage reached 334/334 productions and 119/119 scanner rules, and both editor
runners checked 38,637 ICL and 809 PDL files. See
`build/local-reset-final-regression.log` and the
[implementation record](superpowers/plans/2026-10-27-local-reset-rewrite.md).

ASan/UBSan passed the 26 CLI cases and native runtime/scheduler tests with leak
detection enabled. LeakSanitizer cannot run under this sandbox's ptrace
environment; the same checks passed outside it. Z3 itself was not instrumented.
The authored C++ strict coverage gate was not rerun for these changes, and the
previous coverage gap remains recorded in the model ledger.

Tessent accepted the explicit TAP fixture's reset-only rewrite, but its exported
trace differs, including the final SIB state. This establishes vendor acceptance
of that input, not full equivalence. No new golden was approved; details are in
the [oracle report](tessent-oracle.md#transitive-local-reset-rewrite-2026-10-27).

### Replay guarantees

Every candidate scan is replayed through the separate concrete hardware-state
engine before it is accepted. Tests compare source and flattened ICL output,
then re-ingest generated PDL and require byte-identical output. Reads retain
expected values separately from update values; these tests do not claim that
unknown external hardware will match a requested expected value.

A small independently enumerated nested-SIB model checks minimum scan count and
shifted-bit cost. TAP transactions additionally count the IR/DR entry and exit
cycles when comparing schedules within a horizon. The bounded fallback searches
increasing scan horizons up to 32 scans,
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
consecutive idle cycles with identical inputs; generated PDL still
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
parallel-port/DataRegister access and `iApplyEndState` were unfinished at that checkpoint.

### Historical end-state directive guard verification (2026-10-27)

A regression reproduced successful pattern publication while ignoring an alias
`iApplyEndState` obligation, both with and without `--debug`. The shared
scheduler at that checkpoint rejected these models with an ICL source location
and alias name. This guard is superseded by the implementation described above.
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


The requested repeat against `a39fc7b` reproduced all 36 vendor outcomes and
all 19 exported static traces; both TAP comparison files are byte-identical to
the prior run. The 33 generator failures break down into 24 addressed/one-hot
targets, three interface-event scheduling cases, one multi-source reset artifact,
one invalid contributor width and four procedures without explicit TAP reset.
These are first diagnostics, and include intentionally negative fixtures.
See [the current report](tessent-oracle.md#prefixed-layout-rerun-2026-10-27) for
case counts and evidence. The repeat passed 22 harness and three trace tests;
only documentation changed, so the previous full regression remains the latest
complete check. Full generator equivalence and general reset rewriting remain
unfinished.


## Primary-input verification (2026-10-27)

The complete `make check BISON=/usr/bin/bison -j4` passed with
`ICL_REF_DIR=/tmp/icl-reference-honza-20261021` and the existing reference Python
environment on PATH. This includes dynamic/static generator checks, 19 CLI
cases, the symbolic and scheduler tests, 334/334 grammar rules, 119/119 scanner
rules, and Vim/Neovim corpus runs of 38,633 ICL and 805 PDL files per editor.
The final log is `build/primary-write-final-regression.log`.

Focused checks, Google format/naming checks, and Clang AddressSanitizer plus
UndefinedBehaviorSanitizer runs passed. Sanitizer targets were
`pattern-generation-check pattern-symbolic-check pattern-scheduler-check`,
with `ASAN_OPTIONS=detect_leaks=0`, `-O1 -g -fsanitize=address,undefined
-fno-omit-frame-pointer`, and the existing external Z3 archive. That archive
is not instrumented, and leak detection was not run in this historical check.
A [subsequent leak-enabled run](tessent-goldens.md#leak-detection) passed outside
the sandbox. Evidence for the earlier check is in
`build/primary-write-final-{focused,sanitizer}.log`.

Independent review found incorrect output controlling-state initialization.
A failing regression reproduced an all-X write incorrectly emitting no pin
stimulation. The fix preserves controlling history separately, and the new
case checks both initial state and later upstream changes. An additional
partial AccessTogether port-alias regression initially enforced explicit rejection
of unsupported whole-group access. The later primary-group extension replaces
that rejection with a native whole-group update assertion. Existing elaboration already rejects
an addressed-instance bypass; no redundant validation was added.

`make coverage BISON=/usr/bin/bison -j4` completed its tests but failed the
unchanged strict coverage gate: **95.3% lines / 60.5% raw branches**, against
100% / 90%. It is not reported as passing. Evidence is in
`build/primary-write-coverage.log`; generated Flex/Bison internals remain excluded.

`make install-dynamic BISON=/usr/bin/bison` succeeded; all 19 CLI tests passed
against `/home/banana/.local/bin/icl-pattern-generation`. Installation and
installed checks are logged in `build/primary-write-{install,installed-check}.log`.
The [Tessent rerun](tessent-oracle.md#primary-input-rerun-2026-10-27) verifies the
nine Port-width accept/reject outcomes and records the remaining comparison
gaps. This continuation does not complete addressed scheduling, general local
reset rewriting, mixed scan/parallel execution or full pattern equivalence.


### Reservation-aware reset release (2026-10-28)

The local-reset solver now receives held resource values and taken-but-unwritten
controls before selecting release values. This fixes a feasible alternative
being rejected after the solver chose a protected TDR. A two-control fixture
checks applied writes, state assumptions, unwritten takes, persistent takes,
release, atomic failure, rewrite replay and idempotency. A 40-control fixture
with 39 protected alternatives exercises the same path at larger scale.

Dynamic/static generator and resource suites, native scheduler tests, and Google
format/naming checks passed. The style check initially exposed missing braces
and getter naming in existing pattern code; those were repaired at all callers,
with no check suppressions. Independent review found no blocking issue.
ASan/UBSan with `detect_leaks=1` passed 21 resource tests, 26 generator CLI tests
and the scheduler suite. Evidence is in ignored
`build/reset-reservations-{focused-final,sanitizer-tests,performance}.log`.
The 40-control CLI run took 0.105 seconds (2663-byte ICL, 0.14 seconds including
the test process, 76356 KiB peak RSS); this is a scale check, not an optimality
claim. No fresh Tessent run or unrestricted source-rewrite support is claimed.

Full `make check BISON=/usr/bin/bison -j4` also passed with the established
reference Python environment and `ICL_REF_DIR`. Log:
`build/reset-reservations-full-regression.log`. It includes 334/334 grammar
alternatives, 119/119 scanner rules, seven legacy plus six directional offline
standards, and both editors over 38,646 ICL / 815 PDL files. The editor suite
completed in 167.685 seconds. Strict authored-code coverage was not remeasured.

### Explicit TAP replay and reset qualification (2026-10-28)

The scheduler now emits the instruction scans it already executed, and accepts
explicit instruction-register transactions on replay. Known reset instruction
bits initialize their controlling fill. If reset admits multiple encodings,
unknown bits stay unknown; an explicit read/partial write must supply them rather
than silently selecting an unrelated instruction. Synthetic DR access and
controller-owned iState remain rejected. Scanned reset-held storage retains an
unqualified read in the flat PDL. Reset-only source rewriting is unchanged.

Nine TAP replay regressions cover masks, readonly/partial operations, ambiguous
reset encodings, IR/DR grouping, output protection, flat replay, repeated SIB
closure/reopening and transitive-reset observations. Independent review's
ambiguous-reset finding was reproduced and fixed before final checks.

Fresh Tessent 2026.2 runs produced nine new directional certificates: both
directions for three reservation probes and repeated SIB closure/reopening;
forward for transitive-reset release. All stages retain apply boundaries and
TAP operations. See [certification evidence](tessent-goldens.md#tap-and-reset-qualification-2026-10-28).

Full `make check`, final dynamic/static tests and Google style checks passed.
ASan/UBSan with leak detection enabled passed the nine new tests, 26 CLI tests,
21 resource tests and native scheduler suite. Logs are
`build/tap-pdl-{full-regression,static-final,style-final,sanitizer-tests}.log`.
Both editors checked 38,646 ICL / 816 PDL files; all 334 grammar and 119 scanner
rules were reached. Strict authored-code coverage and RTL simulation were not
run. Other documented retargeter/source-rewrite limits still apply.
### End-state defaults and final restoration (2026-10-28)

The earlier blanket rejection is replaced with per-bit default resolution and
shared scheduler constraints. Explicit overrides last for one apply; empty
applies, subsequent reads and program completion can restore defaults. Inverted
and partial aliases preserve unaffected bits. Final restore failures identify
the PDL source and call stack. Conflicting reservations are rejected before
searching scan horizons. Failed solves retain the prior scheduler snapshot and
published output files.

Eleven CLI tests cover these behaviors, invalid/unknown/reset-incompatible/gated
directives, primary paths, nested SIB access and original/flat/generated-PDL
replay. The native test checks failed restoration rollback and repeated no-op
restoration. Independent review found a stale forwarded-port fill for X writes;
both polarities reproduced the error and now use the resolved source default.

A 1024-bit primary-input default/override/restore case generated its source
sequence in 0.150 seconds with g++ 16.2.1 `-O2` on the existing Linux/WSL2 host,
while regression/build jobs ran concurrently. This is an observed scale check,
not a controlled speedup. Reproduce with
`python3 tests/src/pattern/test_apply_end_state.py build/icl-pattern-generation ApplyEndState.test_wide_primary_default_round_trip`.
End-state models retain the bounded symbolic solver's limits; the large-network
greedy heuristic is not used for these models.

One new immutable TAP/SIB forward certificate passes the requested Tessent
round-trip rule. The primary-input and reverse-export differences remain
unqualified; see [vendor evidence](tessent-goldens.md#end-state-qualification-2026-10-28).
At this checkpoint the larger retargeter remained incomplete for DataRegister
execution (subsequently extended below), OneHotScanGroup execution, mixed pin/scan transactions,
multiple TAP/output scheduling, independently gated events and system clocks.

Final `make check` passed (334/334 grammar alternatives, 119/119 scanner rules,
16 directional plus seven legacy standards). Vim and Neovim each exercised
38,648 ICL and 818 PDL files; the editor suite took 197.371 seconds.
Dynamic/static generator and native scheduler suites and Google style checks
passed. ASan/UBSan with `detect_leaks=1` passed all eleven end-state cases,
25 CLI cases, 21 reservation cases and the native scheduler suite. Logs:
`build/apply-end-state-{regression-final,focused-final,style-final,sanitizer-tests}.log`.
Strict authored-code coverage, RTL simulation and a new installation were not run.

### DataRegister execution verification (2026-10-28)

The primary backend now handles selectable/addressed writes, cascaded data,
forwarded controls, nested OneHot observations and non-destructive routed reads.
Eighteen CLI tests exercise source/flat/generated-PDL replay, partial and unknown
comparisons, incompatible shared-bus `-together` requests, and atomic failures.
The native model additionally covers uncertain scan events and address collisions.

Independent review reproduced three important defects before publication:
unknown write-enables could corrupt unrelated storage, output-port reads could
rewrite the storage they observe, and held end-state markers could add apply
groups during replay. Each received a failing regression before its fix. The read
protection also covers logic-derived outputs; explicit inherited enable writes
reuse `AddressResolver`, rather than adding a second connectivity resolver.

Full `make check` passed after the fixes: dynamic/static suites, grammar
334/334 alternatives and scanner 119/119 rules. Vim and Neovim each exercised
38,650 ICL and 821 PDL inputs. ASan/UBSan with LeakSanitizer enabled passed native
state/scheduler checks and all 18 DataRegister CLI tests. LeakSanitizer's sandbox
ptrace failure was rerun outside the sandbox, never disabled. Google formatting
and naming checks passed; strict authored-code coverage was not remeasured.

The 1024-register reverse-declared cascade took 0.00098 seconds for model
construction and two events (source parsing excluded), GCC 16.2.1 `-O2`, Linux
x86_64. The complete native state suite took 0.10 seconds / 22,912 KiB peak RSS,
measured by `/usr/bin/time -f 'wall_s=%e max_rss_kib=%M' build/pattern_state_test`.
This measures concrete replay, not maximum-size scheduling throughput, and is
not a speedup claim.

Three new Tessent certificates qualify selectable storage in both directions
and shared-bus addressed storage in reverse. The nested address-default case and
the addressed forward comparison remain unqualified; see
[the vendor evidence](tessent-goldens.md#dataregister-qualification-2026-10-28).
The complete retargeter remains unfinished for mixed primary/scan DataRegister access,
callbacks, feedback, OneHotScanGroup, multiple controllers/outputs, independently
gated or shadow events, system clocks and the other limits above.

Evidence logs: `build/data-register-regression-published.log`,
`build/data-register-style-passed.log`, `build/data-register-sanitizer-*-final.log`,
`build/data-register-scale-final.log`, and `build/data-register-goldens-final.log`.
No installation or RTL simulation was performed in this phase.

### Scan-driven DataRegister verification (2026-10-28)

The complete `make check` run exited 0 after review fixes. Dynamic and static
builds each passed 19 serial DataRegister cases and 20 primary DataRegister cases.
The corpus covered 334/334 grammar alternatives and 119/119 scanner rules; Vim
and Neovim each checked 38,654 ICL and 824 PDL files. All 21 directional and seven
legacy Tessent standards passed offline. Google formatting and the changed C++
files' naming checks passed.

The independent review found three important defects, each reproduced before
fixing: `-together` incorrectly constrained reads, same-value data goals could
skip a controlling ScanRegister UpdateEn, and repeated data reads did not replace
earlier expectations. The latter fix also covers the primary-input backend.
The native regression's unsupported-alias assertion was updated to match the
broader supported-register diagnostic; the invalid port-member case still fails
atomically.

ASan/UBSan with LeakSanitizer enabled passed the native state/scheduler tests,
18 serial correctness cases and all 20 primary-data cases. The eight-register
stress case reaches the existing two-second solver timeout under sanitizers,
including an isolated rerun; the complete 19-case sanitizer suite therefore did
not pass. This is a reported scheduling resource limit, with no sanitizer memory
error reported. Sanitizers were not disabled and solver limits were not raised.
LeakSanitizer required execution outside the ptrace sandbox.

The 925-byte `scan_data_registers/addressed_eight.icl` uses eight four-bit addressed
registers on one eight-bit scan bus. GCC 16.2.1 `-O2`, Linux 6.18.33.2 WSL2,
`/usr/bin/time -v build/icl-pattern-generation --icl
 tests/cases/ours/pattern_generation/scan_data_registers/addressed_eight.icl --pdl
 tests/cases/ours/pattern_generation/scan_data_registers/addressed_eight.pdl --output
 build/scan-data-eight.pdl --debug` measured 1.95 s and 62,032 KiB peak RSS,
producing eight scans. The instrumented run stopped at the limit after 2.54 s /
488,932 KiB. This establishes a small tested scale, not maximum throughput or
optimality. This historical timeout is resolved by the address-batching change below;
its measurements and supported topology remain narrower than general retargeting.

Evidence: `build/scan-data-regression-published.log`,
`build/scan-data-review-{red,green}.log`,
`build/scan-data-sanitizer-{state,scheduler,primary,correctness}-final.log`,
`build/scan-data-eight{,-sanitized}-measurement.log`, and
`build/scan-data-goldens-final.log`. Strict authored-code coverage, installation,
and RTL simulation were not rerun. Mixed primary/scan transactions, multi-TAP
and shared-output scheduling, OneHotScanGroup, callbacks/feedback, shadow events,
system-clock waits, broader reservations and general local-reset rewriting
remain separate unfinished capabilities.

### Shared-address scheduling verification (2026-10-29)

The eight-register sanitizer timeout above is resolved without changing solver
limits or disabling sanitizers. Six additional CLI cases cover 64 registers,
partial/reordered goals, already-satisfied goals, simultaneous independent lanes,
different physical address buses, reservations and infeasible -together requests.
A cross-address cascade checks successful fallback to joint solving and all
replay forms. A native reset-dependency case checks failed speculation rollback
and subsequent reuse of unchanged controlling fills.

Independent review confirmed the rollback and successful-fallback cases, and
identified no additional important correctness issues. The already-satisfied-goal
regression was reproduced before fixing it: eight unnecessary scans became one
required controller update, both with no changed data and with one changed target.

Final complete `make check` exited 0, including dynamic/static 25-case scan data
suites, all 334 grammar alternatives/119 scanner rules, 21 directional and seven
legacy Tessent standards, and Vim/Neovim corpora (38,656 ICL / 824 PDL each).
All 25 scan data cases and native scheduler tests passed ASan/UBSan/LeakSanitizer
outside the ptrace sandbox. Formatting and changed-source naming checks passed.
No sanitizer cases were excluded in this phase.

Full CLI measurements with GCC 16.2.1 `-O2`, Linux 6.18.33.2 WSL2:

| Bank | ICL bytes | Applies | Wall time | Peak RSS |
|---|---:|---:|---:|---:|
| Eight registers, before address batching | 925 | 8 | 1.95 s | 62,032 KiB |
| Same eight-register input, after (two runs) | 925 | 8 | 0.10–0.12 s | 73,644–75,960 KiB |
| 64 registers, reversed partial writes | 4,509 | 64 | 1.52 s | 77,212 KiB |

Measured with `/usr/bin/time -v build/icl-pattern-generation --icl INPUT.icl
--pdl INPUT.pdl --output RESULT.pdl --debug`; the eight-register source is
`tests/cases/ours/pattern_generation/scan_data_registers/addressed_eight.*`.
The 64-register input is constructed by
`test_large_addressed_bank_and_partial_reordered_goals` from that fixture.
Other checks overlapped some measurements; these show tested scale and a runtime
reduction on this shared-bus workload, not global optimality or a universal speedup.
The earlier 64-register implementation reached its solver limit before producing
a pattern.

Evidence: `build/addressed-scheduling-regression-final.log`,
`build/addressed-scheduling-sanitizer-{cli,native}-final.log`,
`build/addressed-eight-after{,-repeat}.log`,
`build/addressed-sixty-four-measurement.log`, and the
[implementation plan](superpowers/plans/2026-10-29-addressed-scheduling.md).
Fresh Tessent runs, installation, RTL simulation and strict authored-code
coverage were not rerun; certified patterns were checked offline. Mixed
primary/scan access, multiple controllers/shared outputs and the other remaining
capabilities listed above are unchanged.

### One-hot serial paths and multi-apply merges (2026-10-29)

Scalar `OneHotScanGroup` paths now execute on standalone SI/SO networks and
single-TAP networks. IEEE 1687-2014 §6.4.14, printed pp. 127–128, requires exactly
one active contributor on the active scan path and permits zero-hot inactive
subgroups. The existing contributor resolver now follows ScanOutPort enables and
nested groups. Symbolic route selection requires one known-active contributor and
all others known-inactive. Concrete replay uses shift storage for serial data,
including inversion. TAP path tracing respects output enables instead of clocking
disabled branches through fallback selection. Explicit interface controls still
take precedence; structural ownership discovery still includes every branch.

Self-constructed inputs are under
`tests/cases/ours/pattern_generation/one_hot_scan/`. Seven CLI cases cover nested
writes/reads, inverted returns, unknown/conflicting enables, incompatible together
groups, reservations, a TAP wrapper and 32 distributed returns. Native symbolic
tests exhaust all 27 combinations of three ternary enables against an independent
truth table and deliberately distinguish shift from held register values. Positive
cases exercise original/flat ICL and generated-PDL replay; standalone cases also
compare the serialized ICL byte-for-byte. Failed generation preserves old outputs.

Merge normalization now combines disjoint write-only calls containing multiple
applies, preserving each call's apply order (IEEE §7.9.18(i), printed p. 178).
Whole-call target conflicts, empty applies and opaque effects retain barriers.
Emission retires exhausted streams, so unequal stream lengths take work linear
in the total operation count after target lookup. The existing scheduler and
fresh-state serial fallback remain responsible for validating the combined
sequence. The new unequal-length example reduces five applies to three without
changing either procedure's order. Eleven merge CLI tests pass, including replay,
conflict and barrier tests and the existing 256-call example. This remains a
heuristic and does not establish global optimality or arbitrary merge support.

Verification:

- Full `make check BISON=/usr/bin/bison -j4` passed with
  `ICL_REF_DIR=/tmp/icl-reference-honza-20261021` and the existing reference venv
  on PATH. Dynamic/static new tests, 334/334 grammar rules, 119/119 scanner rules,
  both editors and all 21 directional/seven legacy offline vendor standards passed.
  Final log, including the reviewer's late-failure rollback case:
  `build/pattern-completion-regression-reviewed.log`.
- Clang ASan/UBSan and LeakSanitizer passed the seven one-hot CLI tests, eleven
  merge CLI tests and native symbolic/state tests. The sandbox's ptrace boundary
  prevented leak checks on the first attempt; successful runs were outside that
  boundary with `ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1`.
- Independent review found no Important/Critical issues in either change and
  independently verified late combined-merge failure falls back to fresh serial
  execution. Google formatting and changed-production-source naming checks passed.
- A 4,945-byte, 32-contributor input took 0.76 s for the complete Python test,
  including original, flat and generated-PDL runs; peak RSS was 76,264 KiB.
  Command: `/usr/bin/time -v python3 tests/src/pattern/test_one_hot_scan_scheduling.py
  build/icl-pattern-generation OneHotScan.test_many_distributed_returns` (the
  recorded equivalent harness additionally prints input bytes). Environment:
  GCC 16.2.1 `-O2`, Z3 4.13.3, Linux/WSL2. Evidence: `build/one-hot-scale.log`.
  Concurrent regression makes this a scale observation, not a speedup claim.

Wide one-hot groups still incur quadratic selection-constraint work; only the
32-contributor scale above was measured. Vector/multiple-output paths, mixed
primary/scan execution, multiple TAPs, shadow/independently gated events, feedback,
system-clock/time waits, additional reservation types and general source reset
rewriting remain acceptance work. No fresh Tessent certification, RTL simulation,
installation or strict authored-code coverage measurement was performed here.


### Bit-level reservations (2026-10-29)

IEEE 1687-2014 §7.9.19–20 specifies take/release ownership and assignment-driven
protection; Annex B's `reg_port_or_instance : hier_signal` permits indexed
resources. The v2023.4 Tessent `iTake`/`iRelease` reference topics describe
inheritance, persistent takes and `iState`, but do not settle partial-write
protection explicitly. Five fresh Tessent 2026.2 probes establish these empirical
results: taking the high bit leaves the low selector free; taking/writing the
low bit blocks opening it; releasing only the low bit permits access; inverted
alias takes work; taking a whole register and writing only its high bit leaves
the low bit unprotected.

Inputs are classified as self-constructed under
`tests/cases/ours/pattern_generation/resources/slices.icl` and `slice_*.pdl`.
`tests/src/pattern/probe_resource_slices.tcl` runs the vendor probes. Logs and
successful vendor PDL outputs are in
`test_icl_toolsets/test_resource_slices/run_2026_10_29_initial/` beneath the remote
`/data/workspaces/lianghuaxiao/test/tessent_ijtag/` directory. The low-bit probe
exits with the expected taken-bit conflict; the other four exit successfully.
These observations are not new certified round-trip goldens.

The vendor probe also exposes a separate outstanding capability: its bypassed
TAP target shares unconditional interface controls with the visible register.
At this checkpoint the scheduler rejected that hidden clocking. The hidden-event
implementation below now supports this original network and certifies its four
positive cases in both directions; the blocked low-bit reservation still fails.

Implementation reuses the target resolver, shared scheduler and concrete replay.
Per-register masks and ownership counters avoid one map allocation per physical
bit. Symbolic and concrete constraints compare only protected bits; explicit
updates retain the existing old-to-requested transition rule. The 31 CLI tests
cover partial writes/releases, overlapping owners, alias direction/inversion,
unknown bits, reset preambles, greedy planning and original/flat/generated-PDL
replay. Native scheduler tests check masked failure rollback and permitted
selector changes. Independent review found an overconstraint on unknown
unwritten reset bits; the regression reproduced it before the knownness fix.
The reviewer found no remaining issues after that correction.

The 256-bit reservation/34-register case (2,237 bytes of ICL and PDL) took
0.18 s including Python startup, with 81,660 KiB peak RSS. Command:
`/usr/bin/time -v python3 tests/src/pattern/test_resource_reservations.py
build/icl-pattern-generation Reservations.test_large_masked_register_and_greedy_scheduling`.
Environment: Linux 6.18.33.2 WSL2, GCC 16.2.1 `-O2`, Z3 4.13.3, concurrent
regression. This is a scale observation, not a speedup or maximum-scale claim.
Evidence: `build/resource-slices-scale.log`.

Clang ASan/UBSan and LeakSanitizer passed all 31 reservation cases and native
scheduler tests on the final build. Leak checking remains enabled; the runs
require execution outside the ptrace sandbox. Google formatting, changed-source
clang-tidy checks and diff whitespace checks pass. Logs:
`build/resource-slices-asan-final.log`,
`build/resource-slices-native-asan-final.log` and
`build/resource-slices-naming-final.log`.
No new vendor-certified goldens, installation or strict authored-code coverage
measurement is claimed for this increment.


Final full regression: `make check BISON=/usr/bin/bison -j4` with
`ICL_REF_DIR=/tmp/icl-reference-honza-20261021` and the reference venv on PATH
passed (exit 0), recorded in `build/resource-slices-regression-reviewed.log`.
This includes dynamic/static reservation tests, native tests, all 334 grammar
and 119 scanner rules, the existing 21 directional/seven legacy offline vendor
goldens, and Vim/Neovim tests on 38,658 ICL and 830 PDL files per editor.
An initial full run exposed an overly strict new native assertion that required
an unprotected selector to stay open. The test now verifies protected bits and
target data while allowing the valid final-update close; the final full rerun
passes. No production protection constraint was relaxed to satisfy that test.

### Hidden serial storage events (2026-10-29)

IEEE 1687-2014 §6.4.8 (printed pp. 113–115) describes capture/shift/update
storage and shadow ScanRegisters. A bypassed register may still capture, shift
or update if its interface controls qualify those events. The scheduler now
models these side effects for a single TAP with phase-qualified controls.
Selectors and logic read held values; serial sources read separate shift values.
Every event uses one input snapshot, and all register states are checked against
concrete `TapExecution` replay before publishing the transaction. Unknown capture
sources stay unknown; expected read data never supplies hardware state.

The ordinary path scheduler remains the fast path when a bounded symbolic check
proves that events exactly match the visible path. Otherwise capture/shift/update
transitions model visible and hidden storage together. Small networks retain joint
planning; write-only paths longer than 32 bits (or at least 32 registers) reuse existing
dependency-guided batches, with concrete replay between batches to avoid expanding
serial histories across the entire prerequisite sequence. Neither path enumerates
all possible patterns or relaxes reservation checks. Planning remains bounded by
the existing solver limits and a 1,048,576-unit event-work budget computed from
horizon, maximum scan length, register bits and node count.

The self-constructed slice probes now cover the original ungated vendor network,
partial and alias reservations, unknown/known capture data, suppressed capture,
unknown controls, hidden protected storage, reset after failed planning and
output/state rollback. Native tests distinguish held and shifted data, including
all 27 ternary combinations of three one-hot enables. CLI tests exercise original,
flat-ICL and generated-PDL replay. A 32/128-bit scale test includes all three
forms; a 1,024-bit case checks bounded refusal and preservation of an older output.

Four fresh Tessent 2026.2 cases each passed both directional fixed points, adding
eight certified standards under `tests/golden/tessent_roundtrip/shadow_slice_*`.
The original source and its independently retargeted schedule need not match.
Certificates compare ordered apply groups, ignoring only write order inside a
group. See [vendor evidence](tessent-goldens.md#hidden-storage-certification-2026-10-29).

This does not establish general independently gated event support: serial-derived
controls/resets, gates changing on clock edges, events outside their designated
phase, multiple TAPs and shared outputs remain unfinished. Concrete mismatches
fail closed. Completely unobservable registers are not automatically attached to
a controller merely because they share a module. Verification results follow below.


Full dynamic/static `make check BISON=/usr/bin/bison -j4` passed with the reference
checkout and venv (`build/hidden-regression-final.log`): 334/334 grammar alternatives,
119/119 scanner rules, seven legacy and 29 directional offline vendor standards,
and Vim/Neovim corpora containing 38,658 ICL and 830 PDL files per editor.
Clang ASan/UBSan/LeakSanitizer passed the four hidden-event CLI tests and native
symbolic/scheduler tests with `detect_leaks=1`, outside the ptrace sandbox. No
sanitizer or solver timeout was disabled. Changed-source naming, Google formatting
and whitespace checks pass. Independent review found no correctness defects.

The 32/128-bit replay plus 1,024-bit bounded-rejection test took 2.89 s and
131,484 KiB peak RSS on Linux 6.18.33.2 WSL2, GCC 16.2.1 `-O2`, Z3 4.13.3.
Command: `/usr/bin/time -v python3 tests/src/pattern/test_hidden_scan_events.py
build/icl-pattern-generation HiddenEvents.test_wide_shadow_storage_and_resource_limit`.
Before dependency batching, the 32/128-bit cases timed out instead of generating
patterns (5.82 s combined, 709,260 KiB). This is a tested workload and bounded
failure, not a maximum throughput or global-optimality claim. Evidence:
`build/hidden-scale-final.log`, `build/hidden-sanitizer-{cli,native,symbolic}.log`.
Installation, RTL simulation and strict authored-code coverage were not rerun.


### Explicit reset and resource ownership (2026-10-29)

The prior unconditional rejection of reset after a reserved write is removed.
IEEE 1687-2014 §7.9.9 (printed p. 164) resets controlling/expected state and
§7.9.19 (pp. 178–179) constrains retargeter changes to taken resources; their
interaction with ownership is not explicit. Tessent Shell Reference Manual
v2023.4, `iReset` (`Command_Ireset_id2ca32290.html`) describes a global reset;
`iTake` (`Command_Itake_idd8a68c5f.html`) describes persistent ownership carrying
between pattern sets when initial reset is suppressed.

Fresh Tessent 2026.2 observations resolve ordinary ownership: eight low/high-bit,
zero/one, asynchronous/synchronous cases allow explicit reset after applied
reserved writes. Four additional asynchronous cases write again after reset;
the low-bit-zero case can subsequently open the selector for target access,
showing that reset clears the prior take, rather than only its protected value.
The reusable probe is `tests/src/pattern/probe_reset_reservations.tcl`; evidence
is under remote `test_icl_toolsets/test_reset_reservations/run_2026_10_29_initial/`
and locally under ignored `build/reset-reservations-evidence/`. These are
behavior probes, not newly certified directional golden snapshots.

The implementation applies the same reset lifecycle to normal and persistent
takes, slices, aliases and inherited ownership. Owner frames remain aligned with
the active call stack so returning from a procedure still releases only its
current local takes. Clearing occurs only after physical reset succeeds. Existing
transaction replay and output staging remain unchanged. Direct vendor evidence
above concerns ordinary takes; persistent clearing and synchronous post-reset
reacquisition are covered locally, not individually vendor-certified.

The resource suite now contains 34 tests, including reset followed by writes,
retaking and blocked access, nested calls, async/sync TAP reset, unapplied-action
rejection and original/flat/generated/reset-only replay. No broader pattern-set
lifecycle, port reservation or live-hardware execution support is implied.


Verification: full dynamic/static `make check BISON=/usr/bin/bison -j4` passed
with the reference checkout and venv (`build/reset-ownership-regression.log`),
including 34 resource tests per linkage and existing language/editor/vendor
checks. All 34 resource tests also passed Clang ASan/UBSan/LeakSanitizer with
`detect_leaks=1` outside the ptrace sandbox (43.288 s). Naming/format checks and
independent review passed; the reviewer also checked local and persistent
retakes after a nested reset. Installation and strict coverage were not rerun.


### Data resource reservations (2026-10-29)

The ownership tracker now covers DataRegister, DataInPort and DataOutPort using
its existing physical-bit masks, aliases, instance membership and call lifetimes.
Primary and serial scheduling share protected-value validation and old-to-requested
transition constraints. Computed output ports are checked through the shared
symbolic/concrete netlist. Reads do not establish protected hardware values.

Address batches carry the masked, replayed protected value into the next batch.
Independent review caught the missing handoff: replacing all 32 taken registers
previously fell back to joint solving and exceeded its resource limit. The
regression now covers full and partial replacements, allows subsequent changes
to unprotected bits, and rejects attempts to restore an earlier protected value.
Native tests also check that failed indirect and automatic writes preserve both
hardware and driver state, followed by a successful explicit replacement.

The nine tests in `tests/src/pattern/test_data_resource_reservations.py` include
primary and scan storage, inverted output aliases, slices, instance/nested
ownership, reset clearing, end-state conflicts and original/flat/generated replay.
The addressed bank has 32 four-bit registers and a 2459-byte ICL model. Running
`/usr/bin/time -v python3 tests/src/pattern/test_data_resource_reservations.py
build/icl-pattern-generation
DataResources.test_addressed_bank_preserves_taken_data_across_batches` took
19.76 seconds and 120980 KiB peak RSS on WSL2, g++ 16.2.1 `-O2`, Z3 4.13.3.
This includes multiple replays and deliberately rejected schedules; it is not
single-pattern throughput. Raw evidence is in
`build/data-resources-scale-reviewed.log`.

These cases are self-constructed and locally verified; no new Tessent
certification is claimed. Other control/clock/scan-port ownership and
cross-pattern-set lifecycles remain required future work.


### Mixed parallel and serial frames (2026-10-29)

The existing bounded scheduler can alternate parallel I/O and serial frames for
DataRegister access, explicit top-level primary-input writes and primary-output
observations. Both frame kinds share hardware/controlling state, resource masks,
read completion and required scan-controller updates. This follows IEEE §5.16.1
(printed p.54), §6.4.9 (p.119) and §7.9.14(a) (p.173). Existing primary-only and
serial-only paths remain available. Forwarded/computed data-port writes are
covered by the later increment below. Multiple scan outputs or TAPs still require
additional implementation.

Parallel comparisons use read-only frames; pin drives are separate events. The
expected value is never treated as known hardware. Pending observations retain
both DataRegister and ScanRegister sources until the observation, after which a
scan update may change them. Explicit `-together` still requires a common write
cycle; it does not force reads to occur on that cycle. TAP scans finish in idle
before the next parallel event. Concrete replay checks all modeled storage and
planned primary drivers; TAP protocol pins remain owned by TAP replay.

Primary planning includes only relevant controls, requested pins and read-route
requirements, with 256-vector/4096-bit limits. Shared temporal work remains
bounded by the 32-frame horizon, existing solver timeouts and expression budgets.
An unrelated output must not disable the existing dependency/address batching
paths: a 64-register scale regression covers this distinction. Neither shortest
patterns nor complete mixed-port model coverage is claimed.

Tests in `tests/src/pattern/test_mixed_io_scheduling.py` cover both dependency
directions, TAP and hidden-storage events, taken drivers, explicit replacements,
common-cycle rejection, AccessTogether register groups, end-state restoration,
addressed read controls, prior scan-storage observations, 128-bit data and width
limits. Every successful case reimports flat ICL, compares the second flat ICL,
and replays generated PDL. Native scheduler tests inspect actual observation-time
state, mandatory controller updates and failed-plan rollback for primary drivers,
storage and TAP instructions. The native tests then perform a successful access
to ensure failed planning did not leave the TAP executor unusable.

Independent review found and reproduced two defects before publication: missing
pending-read protection for scan-backed primary outputs, and disabling scalable
address batching merely because an unused output exists. Both have dedicated
regressions. The initial 128-bit source/flat/generated check took 2.37 seconds and
71820 KiB peak RSS on WSL2, g++ 16.2.1 `-O2`, Z3 4.13.3; raw measurement is
`build/mixed-io-scale.log`. This is the complete replay test, not single-pattern
throughput.

Fresh Tessent 2026.2 forward certificates qualify the two mixed DataRegister
fixtures. Reverse export succeeds but certification fails because local replay
separates combined parallel read/write groups; no reverse standards were recorded
and no comparison was weakened. Exported Verilog/testbench vectors establish
emitted timing only; no RTL simulation is claimed. See the Tessent golden guide
for source/evidence locations and the limits of the fixed-point criterion.

Final verification passed full dynamic/static `make check`
(`build/mixed-io-regression-final.log`, exit 0), including all 31 directional and
seven legacy vendor standards. The native scheduler and 15 mixed, nine
data-resource, 34 reservation and four hidden-event cases passed Clang
ASan/UBSan/LeakSanitizer with leak detection enabled
(`build/mixed-io-sanitizer-final.log`). Google formatting, changed-source naming
and whitespace checks also pass.

### Primary-port AccessTogether (2026-10-29)

IEEE 1687-2014 §6.5.2(i), printed p.137, permits a common parallel-port operation
as well as a common scan. Direct primary input members and DataRegister writes
can now share an update; DataOutPort members can share a parallel observation.
Primary-only grouped operations use the existing joint scheduler so their group
constraints and concrete replay are identical to those in mixed networks. The
ordinary primary-only fast path remains available for ungrouped operations.
Nested/inverted/sliced aliases keep obligations for the entire marked group.
Forwarded/computed write members are covered by the following increment.

Unrequested group controls remain selectable: a partial write to the data portion
of an `enable,value` alias can drive its implicit enable high in the same event.
Reservations and end-state requirements remain hard constraints. Pin-value
preferences retain controlling values when other requirements permit. Independent
review reproduced an initial error that made these preferences hard goals; the
partial-enable case failed before the fix and now succeeds in primary-only and
serial designs. An old native test that required all primary groups to be rejected
now checks both actual pin drives and the resulting output value.

The 13 self-constructed cases in
`tests/src/pattern/test_primary_access_together.py` cover direct and partial
writes/reads, implicit controls, nested aliases, inverted slices, common
DataRegister updates, reservations, end states, incompatible groups, TAP scans
before/after a parallel group, and source/flat/generated fixed points. Native
checks inspect actual storage and driver values, simultaneous observations and
rollback after protected-state conflicts. No new vendor certification is claimed.

The existing 256-vector/4096-bit and temporal-work bounds remain. On WSL2,
g++ 16.2.1 `-O2`, Z3 4.13.3, the 64-primary-pin source/flat/generated replay test
completed in 0.18 seconds with 56764 KiB peak RSS. Command:
`/usr/bin/time -v python3 tests/src/pattern/test_primary_access_together.py
build/icl-pattern-generation PrimaryAccessTogether.test_group_with_64_primary_members`.
Evidence: `build/primary-access-scale-reviewed.log`. This is a bounded case
measurement, not a shortest-pattern or general performance claim.

Final full dynamic/static regression passed (exit 0,
`build/primary-access-regression-reviewed.log`), including 31 directional and seven
legacy vendor standards. Native scheduler tests and all 13 primary-group, 15 mixed
and nine data-reservation cases passed Clang ASan/UBSan/LeakSanitizer with leaks
enabled (`build/primary-access-sanitizer-reviewed.log`). Formatting, naming and
whitespace checks pass.


### Computed data-port writes (2026-10-29)

Forwarded, inverted and computed DataInPort/DataOutPort write goals now use the
shared symbolic signal evaluator in mixed parallel/scan networks. Mux and
one-hot sources retain their route and read-enable conditions. Hierarchical
input bindings and resolved enable-port goals use the same driver discovery.
Logical controlling history remains separate for each requested port, including
partial and X writes; concrete evaluation must satisfy that logical goal before
hardware state or controlling fill is committed. Constants can satisfy a goal
without an unnecessary physical update.

Explicit scan-driven DataInPort writes retain controlling-register update
obligations even when their requested value is unchanged (IEEE 1687-2014
§7.9.14(a), p.173). Inactive mux branches do not force disconnected registers to
update. Controller tracing caches each bit's guarded dependencies; it combines
conditions for shared controllers rather than enumerating reconvergent paths.
Logic-cone tracing remains conservative about required scan controllers; this
is not universal vendor qualification of every combinational model.

AccessTogether activity follows selected physical sources and output enables.
A literal selector and a masked logic operand cannot count as an update event.
Logic activity uses alternative values and knownness for physically updated drivers through
the existing symbolic evaluator, preserving shared-reference correlations.
Each distinct logic output has an independent witness so grouped outputs do not
need a single hypothetical input that changes them all simultaneously. Existing
pin, frame and solver limits remain; dependency/activity traversal and witness
construction also have explicit work bounds. No global shortest-pattern claim
is made.

The 26 self-constructed cases in
`tests/src/pattern/test_mixed_port_writes.py` cover these paths, hierarchy,
contradictory/reserved goals, failed-output preservation, grouped updates,
TAP replay, one-hot padding and enabling, 128-bit outputs and a 64-level
reconvergent logic graph. Every successful case checks source/flattened-ICL and
generated-PDL fixed points. Native scheduler tests inspect physical storage,
mandatory unchanged-value scan updates, independent controlling fills and
rollback. Independent review found masked-source activity, inactive-branch
requirements, shared logic witnesses, missing unknown-to-known activity and repeated
dependency traversal defects;
each was reproduced by a failing test before repair.

The 64-level reconvergent source/flat/generated replay check completed in
0.15 seconds with 59040 KiB peak RSS on WSL2, g++ 16.2.1 `-O2`, Z3 4.13.3.
Command: `/usr/bin/time -v python3 tests/src/pattern/test_mixed_port_writes.py
build/icl-pattern-generation MixedPortWrites.test_reconvergent_controller_dependencies_are_not_enumerated`.
The earlier 128-bit replay check took 0.24 seconds / 60116 KiB; raw evidence is
in `build/mixed-port-reconvergent-scale.log` and `build/mixed-port-scale.log`.
These are complete replay-test timings, not standalone generation throughput.
No new Tessent certificate is claimed for this increment.

Final full dynamic/static `make check` passed with exit 0
(`build/mixed-port-regression-verified.log`), including all 31 directional and
seven legacy offline vendor standards and the full editor corpus. Native
scheduler tests plus 26 mixed-port, 13 primary-group, 15 mixed-I/O and nine
data-resource cases passed Clang ASan/UBSan/LeakSanitizer with leak detection
enabled (`build/mixed-port-sanitizer-final.log`). Google formatting, changed-source
naming and whitespace checks pass; independent review has no remaining findings.


### Merged observations (2026-10-29)

Independent full ScanRegister observations now participate in the existing
apply-depth merge pass, alongside known full-register writes. Reads include
masked comparisons and omitted expected values. Expected values remain
observations; they never become assumed hardware or controlling assignments.
A common target set across each entire call prevents read/write overlap between
concurrent calls. Notes, resource operations, partial targets, X writes,
explicit together options, waits and empty applies retain their conservative
barriers. Each call's apply order is preserved, as IEEE 1687-2014 §7.9.18(e,h,i),
pp.177–178 requires. Failed joint scheduling retries serially from fresh state.

`tests/src/pattern/test_merge_observations.py` adds ten self-constructed tests
covering read/write and read-only batching, masks and no-expected reads,
conflicts, barriers, unequal call lengths, reset-conflict serial fallback,
source/flat/generated replay and 64 independent two-apply calls. Existing
write-merge tests retain their ordering, ownership and failure checks. The
64-call input produces two applies rather than the prior 128, with 128 read
observations preserved. On WSL2, g++16.2.1 `-O2`, Z3 4.13.3, its complete
three-replay test took 0.27 seconds / 60288 KiB peak RSS for 8905 source bytes:
`/usr/bin/time -v python3 tests/src/pattern/test_merge_observations.py
build/icl-pattern-generation MergeObservations.test_many_read_write_calls`
(`build/merge-observations-scale.log`). This measures the specified case,
not global optimality or arbitrary merge support. No new vendor certificate
is claimed.

Final full dynamic/static `make check` passed (exit 0,
`build/merge-observations-regression-final.log`), including 31 directional and
seven legacy offline vendor standards and the editor corpus. Native scheduler
plus all ten new and eleven existing merge cases passed Clang ASan/UBSan/
LeakSanitizer with `detect_leaks=1`
(`build/merge-observations-sanitizer-final.log`). Style, naming, whitespace and
independent review pass. An initial run used a stale static binary for the new
omitted-expected read case and failed; the final rebuild/run passes that case.


### Parallel control-port ownership (2026-10-29)

AddressPort, ReadEnPort and WriteEnPort now use the same bit-level ownership
mechanism as data ports. Direct takes and whole-instance takes include these
ports. After an applied write, reservations constrain automatic address selection
and read/write activation; explicit replacement, partial release, inherited
ownership, return and reset clearing retain the existing rules from IEEE
1687-2014 §7.9.19–20. Primary-only and mixed schedulers enforce the same masks,
and failures preserve existing artifacts and hardware state.

Ownership follows the original logical write bits. An explicit write to a child
enable can replace that child's held value even though scheduling resolves it
to a parent signal. It cannot replace a separately reserved parent, including
unwritten bits of a partially written parent register. Likewise, a physical
parent write cannot replace a child's reservation. This distinction is checked
before symbolic planning and again by concrete state replay. Expected reads do
not establish protected hardware state, and automatic prerequisite writes cannot
acquire permission to change a held value.

The nine self-constructed tests in
`tests/src/pattern/test_parallel_control_reservations.py` cover both backends,
address/read/write conflicts, aliases and slices, mapped child replacement,
scalar and partial parent protection, whole-instance membership, call/reset
lifetimes, source/flat/generated replay and 64 scan-driven enable ports. Native
checks inspect actual parent/child values, failed-plan rollback and automatic
versus explicit replacement. Review reproduced the parent-protection defect
before the fix; ownership replacement now uses original per-bit assignments
instead of resolved physical goals.

Read/write-enable bindings still obey IEEE §6.4.6.25(c)/26(d), pp.109–110:
parent same-kind ports or ScanRegister sources with no intervening logic.
Alias inversion in PDL does not insert hardware inversion. Initial tests that
assumed inverted wiring or generic primary data bindings were corrected against
these rules; the address resolver was not relaxed. Serial-control/clock ports,
frequency reservations and cross-pattern-set lifecycle remain unfinished.
No new Tessent certificate is claimed for these ownership cases.

The 64-enable, 64-bit scan-source test completed all three replays in 0.16
seconds with 58848 KiB peak RSS on WSL2, g++16.2.1 `-O2`, Z3 4.13.3.
Command: `/usr/bin/time -v python3 tests/src/pattern/test_parallel_control_reservations.py
build/icl-pattern-generation ParallelControlReservations.test_many_forwarded_enable_reservations`.
Raw evidence is `build/parallel-control-scale.log`; no general speedup is claimed.

Final full dynamic/static `make check` passed (exit 0,
`build/parallel-control-regression.log`), including 31 directional and seven
legacy offline standards and the complete Vim/Neovim corpus. Native scheduler
and all nine new, nine data-resource and 34 original reservation cases passed
Clang ASan/UBSan/LeakSanitizer with `detect_leaks=1`
(`build/parallel-control-sanitizer.log`). Google formatting, changed-source
naming and whitespace checks pass. Independent review confirmed the scalar and
partial-parent fixes against the rebuilt executable and has no remaining finding.


### Port/control/merge vendor qualification (2026-10-30)

Three shared TAP fixtures now have six fresh Tessent 2026.2 directional
certificates: inverted scan-backed DataOutPort writes, reserved child WriteEnPort
replacement/release, and merged two-round observations/writes. All forward and
reverse ordered-apply comparisons pass without comparator changes. The source,
flat-ICL and generated-PDL replay test is included in both dynamic/static suites.
This raises directional offline standards from 31 to 37. See
[exact scope and retained evidence](tessent-goldens.md#port-control-and-merge-qualification-2026-10-30);
broader model support and vendor qualification remain unfinished.

### Indexed scan inputs (2026-10-30)

The shared scheduler accepts scalar paths terminating at a selected bit of a
top-level vector ScanInPort. Its existing bit-keyed graph retains the endpoint
index; concrete replay changes only that input position, and ScanStep records
indexed endpoint names. This also permits a selected lane of a vector
OneHotScanGroup to feed a scalar chain. It does not enable simultaneous output
lanes or multiple TAPs.

IEEE 1687-2014 §6.4.6.1 (printed p82/PDF96) defines ScanInPort with `port_name`;
§6.4.7 rules d/f (p111/PDF125) permits width-matched sliced connections. Declared
controls remain authoritative: internal ScanInterface declarations are optional
(§6.4.16(a), p132/PDF146), only missing controls may be inferred (§6.7(a)), and
SelectPort is active-high (§6.8/Table 5, p142/PDF156). Because scalar interface
inference does not cover vector ports, the standalone scheduler also checks
declared controls in reachable serial scopes. Unsupported events cause an error
rather than silently writing a deselected child. Reset-only scopes remain usable.

Self-constructed fixtures are in
`tests/cases/ours/pattern_generation/vector_scan_inputs/`. Native tests check
literal serial bits, endpoint indices, held register values, untouched unknown
input bits and failed-transaction rollback. Seven CLI tests cover ascending and
descending ranges, a nonzero scalar index, inverted mux inputs, vector one-hot
returns, unknown/contending enables, control rejection, reset-only children and
a 4096-bit bus. Successful cases check source/flat-ICL/generated-PDL fixed points.
These tests are local evidence, not new Tessent certificates.

The wide-input check runs a 4096-bit input bus and a 64-bit register through
all three replay forms. With g++ 16.2.1 `-O2` on Linux 6.18.33.2 WSL2 it took
0.16 seconds wall time and 57,700 KiB peak RSS, including the Python runner
(`build/vector-input-scale-final.log`). Reproduce with `/usr/bin/time -f
'wall_seconds=%e peak_rss_kib=%M' python3 tests/src/pattern/test_vector_scan_inputs.py
build/icl-pattern-generation VectorScanInputs.test_wide_input_bus`.
Whole-vector drives cost work proportional to input width times shifted bits;
this measures that case's scale and does not claim a speedup or a general bound
on solver runtime.

Final `make -j2 --jobserver-style=pipe check` passed (exit 0,
`build/vector-input-regression-final.log`), including dynamic/static generator
tests, 37 directional/seven legacy offline standards, grammar-rule coverage and
Vim/Neovim corpus checks. Five optional ANTLR integration tests were skipped
because `ICL_REF_DIR` was unset. Native scheduler tests plus the seven new and
seven existing one-hot cases passed ASan/UBSan/LeakSanitizer with leaks enabled
(`build/vector-input-sanitizer-final.log`). Formatting, changed-source naming and
whitespace checks pass. Independent review exposed the declared-control bypass;
its retained test reproduced the failure before the guard fix and passed afterward.

## Shared scan-output observations (2026-11-13)

Direct, inverted, vector and hierarchical ScanOutPort observations of the same
physical serial path now share one transaction. The scheduler resolves
transparent forwarding in graph dependency order and keeps the first sorted
output endpoint. It stops equivalence at storage, selection or an Enable;
separate gated paths are not treated as interchangeable. All original graph
vertices remain available to declared-control validation. Existing concrete
capture/shift/update replay, read comparisons and reservation rollback are reused.

Five CLI tests in `tests/src/pattern/test_shared_scan_outputs.py` use
self-constructed inputs and check source/flat/generated-PDL fixed points,
shared muxes, width/direction, and preservation of prior outputs on failure.
The native scheduler test independently checks actual register/output state,
serial comparisons and protected-write rollback. Positive CLI/native cases
failed on the former multiple-output rejection before the implementation.
Independent review found no correctness issue; its stale support-description
finding was corrected. Additional reviewer probes retained declared controls
reachable through a discarded observation and rejected distinct selector paths.

Fresh `make -j2 --jobserver-style=pipe check` passed (exit 0,
`build/shared-output-regression.log`), including dynamic/static generator tests,
1,916 ICL regression cases, 2,180 conformance cases, 37 directional and seven
legacy offline Tessent standards, and Vim/Neovim integration. All 334 grammar
and 119 scanner rules were reached; five optional ANTLR checks were skipped.
`make formatcheck`, changed-source clang-tidy and `git diff --check` passed.

The native scheduler and all five new CLI tests passed Clang ASan/UBSan/LSan
with `ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1` using the rebuilt
`build/local-reset-sanitize/` binaries. Logs:
`build/shared-output-sanitizer-{build,native,cli}.log`. Initial sandbox runs
failed because LeakSanitizer cannot run under ptrace; successful runs used the
approved outside-sandbox execution with leak checking retained.

The 257-output, 64-storage-bit input is 10,015 bytes; its three replay forms
took 0.154 s. The complete five-test measurement took 0.67 s and 59,756 KiB peak
RSS with GCC 16.2.1 `-O2` on Linux 6.18.33.2 WSL2. Command:
`/usr/bin/time -f 'wall_seconds=%e peak_rss_kib=%M' python3 tests/src/pattern/test_shared_scan_outputs.py build/icl-pattern-generation`.
Log: `build/shared-output-scale.log`. The additional graph pass uses O(V+E)
work/storage, with O(R log R) root deduplication for R output bits. These are
scope measurements, not a speedup or global optimality claim.

Distinct scan chains, overlapping intermediate taps and multiple TAPs still
need event-aware shared-state scheduling. No new vendor qualification,
installation, RTL simulation or strict authored-coverage measurement was run.

## Sliced and aliased merge observations (2026-11-13)

`iMerge` now batches sliced and aliased ScanRegister reads with independent
full known register writes. Target resolution preserves inversion, bit order,
enumerations and expected-data masks. Each call occupies every underlying
physical register for its entire stream, so aliases and separate slices of one
register cannot bypass conflict ordering. AccessTogether aliases remain barriers
because their implicit members can extend beyond the requested bits. Partial,
alias and X writes retained controlling-fill barriers at this stage; the later
[masked-write update](#masked-and-aliased-merge-writes-2026-11-13) extends batching.

Six positive subcases first failed the conservative serialization behavior;
all 13 observation tests now pass, including source/flat/generated replay,
unequal streams, resource barriers, rollback and alias conflicts. The independent
review found no actionable issue and additionally verified enum/concatenated
aliases, nested group membership and invalid-input output preservation.

Full dynamic/static `make -j2 --jobserver-style=pipe check` passed (exit 0,
`build/merge-slices-regression.log`), retaining 1,916 ICL cases, 2,180 conformance
cases, complete grammar/scanner rule hits, 37 directional/seven legacy offline
vendor standards and editor integration. Five optional ANTLR checks were skipped.
The 13-case observation suite passed ASan/UBSan/LSan with leak detection enabled
(`build/merge-slices-sanitizer.log`); original merge tests, Google formatting,
changed-source clang-tidy and whitespace checks also passed.

The 64-call, two-apply-per-call workload (9,801 ICL+PDL bytes, 128 storage bits)
now emits two applies instead of the baseline 128. Three replay forms took
0.185 s; the complete 13-case suite took 2.32 s and 58,244 KiB peak RSS under
`/usr/bin/time -f 'wall_seconds=%e peak_rss_kib=%M' python3 tests/src/pattern/test_merge_observations.py build/icl-pattern-generation`
(GCC 16.2.1 `-O2`, Linux 6.18.33.2 WSL2;
`build/merge-slices-scale.log`). This is an apply-count reduction, not a measured
runtime speedup or a global optimization claim. No new vendor certificate,
installation or strict authored-coverage measurement was made for this change.

## Reset cancels queued accesses (2026-11-13)

An explicit `iReset` or `iReset -sync` discards unapplied `iWrite` and `iRead`
operations, including queues spanning input files or a caller and its reset
procedure. Previously applied accesses remain in the generated pattern. Reset
also clears resource ownership. Discarded accesses still undergo target-kind,
range and value validation; malformed input must fail without replacing an
existing output. No scan or reset-release preamble is generated solely for a
canceled access.

IEEE 1687-2014 §7.9.9(a), printed p164/PDF p178, specifies resetting controlling
and expected state. Six Tessent 2026.2 probes additionally establish the queue
ordering: queued writes, reads and combined reads/writes disappear on async or
sync reset, while a preceding applied write remains. The maintained probe is
`tests/src/pattern/probe_pending_reset.tcl`. Vendor logs and exported PDL remain
under `/data/workspaces/lianghuaxiao/test/tessent_ijtag/test_icl_toolsets/test_pending_reset/run_2026_11_13`,
with a local ignored copy in `build/pending-reset-probe/`. These are behavior
observations, not directional qualification certificates.

The seven CLI tests in `tests/src/pattern/test_pending_reset.py` cover queue
boundaries, TAP reset, ownership, original/flat/generated replay, immutable
procedure rewriting, real local-reset prerequisites and failed-output
preservation. A native runtime test covers a queue crossing files. Positive
cases failed before the fix. Independent review found that unsupported target
kinds could escape validation when canceled; five additional rejection cases
failed before the shared target-kind check and now pass.

Queue clearing and reset-target filtering are linear in operation count,
apart from existing target/value resolution. As a representative check of the
added per-access validation, the existing 64-call, 128-storage-bit merge case
(9,801 ICL+PDL bytes) took 0.205 s for three replay forms. The complete 13-case
observation suite took 2.40 s and 59,124 KiB peak RSS with GCC 16.2.1 `-O2` on
Linux 6.18.33.2 WSL2. Command:
`/usr/bin/time -f 'wall_seconds=%e peak_rss_kib=%M' python3 tests/src/pattern/test_merge_observations.py build/icl-pattern-generation`.
Log: `build/pending-reset-scale.log`. These measurements establish tested scale,
not a runtime speedup.

The native runtime and scheduler tests, seven reset tests and 34 resource tests
passed Clang ASan/UBSan/LeakSanitizer with
`ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1` using the rebuilt
`build/local-reset-sanitize/` binaries. Logs:
`build/pending-reset-sanitizer-{native,cli}-final.log`. Google formatting and
changed-source clang-tidy checks pass. No installation, RTL simulation, strict
authored-code coverage or new directional vendor certification was run.

The final full dynamic/static `make -j2 --jobserver-style=pipe check` passed
(exit 0, `build/pending-reset-regression-final.log`), including 1,916 ICL
regression cases, 2,180 conformance cases, 37 directional and seven legacy
offline vendor standards, all 334 parser and 119 scanner rules across 2,504
files, and the Vim/Neovim corpus integration suite. Five optional ANTLR checks
were skipped because `ICL_REF_DIR` was unset. Whitespace checks also pass.

## Enabled observations of one scan path (2026-11-13)

Multiple ScanOutPorts may observe the same physical path through distinct
Enable conditions. The scheduler retains those gated alternatives and selects
a known-enabled complete path. It does not add a second shift or update to
the shared storage. Direct, inverted, hierarchical and vector observations are
covered. IEEE 1687-2014 §6.4.6.2, printed p82/PDF p96, requires Enable to be
active for the source to connect to its ScanOutPort.

The graph pass computes both physical identity and observation identity in
O(V+E). Physical identity crosses transparent forwarding; observation identity
stops at Enable. Registers and selectors stop both identities. Distinct paths,
including a different disabled path, retain their rejection. All graph vertices
remain available for declared-control checks. A free primary input may be
configured to enable an observation; a protected off input may not.

Seven positive CLI subcases and the new native test failed on the former
multiple-output guard. All nine CLI tests now pass original/flat/generated
replay. Native tests check selected endpoints, physical input bits, inverted
comparisons, state transitions and rollback for blocked or reserved accesses.
Independent review found no actionable issue and additionally checked nested
disabled/unknown gates, controls on unused branches and distinct disabled paths.

Native scheduler tests and all nine CLI tests passed Clang
ASan/UBSan/LeakSanitizer with `detect_leaks=1` and `halt_on_error=1`;
logs: `build/enabled-output-sanitizer-{native,cli}.log`. Formatting and
changed-source clang-tidy checks pass.

The 257-output, 64-storage-bit case (13,343 bytes) took 0.234 s for three replay
forms; the complete nine-case suite took 1.35 s and 57,692 KiB peak RSS on
Linux 6.18.33.2 WSL2 with GCC 16.2.1 `-O2`. Command:
`/usr/bin/time -f 'wall_seconds=%e peak_rss_kib=%M' python3 tests/src/pattern/test_shared_scan_outputs.py build/icl-pattern-generation`.
Log: `build/enabled-output-scale.log`. This records supported scale, not a
speedup. Distinct chains, simultaneous lanes and multiple TAPs remain open.

Full dynamic/static `make -j2 --jobserver-style=pipe check` passed (exit 0,
`build/enabled-output-regression.log`), including 1,916 ICL cases, 2,180
conformance cases, 37 directional/seven legacy offline vendor standards,
334 parser/119 scanner rules over 2,504 files and Vim/Neovim integration.
Five optional ANTLR checks were skipped because `ICL_REF_DIR` was unset.
No new vendor certificate, installation, RTL simulation or strict authored-code
coverage measurement was run for this change.


### Standalone client scan controls (2026-11-13)

Standalone client interfaces now reuse explicit/implicit interface membership
and the concrete executor's control resolution. Top-level select, phase and
clock pins drive the interface; child controls retain their actual bindings.
The symbolic solver proves capture/shift/update equivalence to the selected
whole path, and concrete replay samples clock edges and checks all storage.
Indexed control ports drive only their bound lanes. Unknown or disabled required
controls reject the access without publishing outputs. General hidden and
independently timed client events remain separate acceptance work.

Native tests and ten CLI cases cover explicit/inferred clients, nested controls,
SIB setup, inverted select, gated clock, indexed lanes, local reset rewriting,
reservations, parallel-frame phases and source/flat/generated replay. A synthetic
parallel-storage regression uses UpdateEnPort as DataRegister WriteEnSource;
Tessent rejects that source kind (ICL65-1), so it is not a valid-ICL or vendor
qualification claim. The model's broader source-kind validation remains open.

Tessent 2026.2 accepted the standalone four-bit fixture in both runs. Only reverse
ordered-apply equivalence passed: the forward result added an initial reset-value
read, so the unchanged comparator rejected it. The new reverse certificate
raises the offline total to 38 directions. Evidence was produced under
`test_icl_toolsets/test_client_scan_controls/run_2026_11_13_01`; local retained
copies are `build/client-controls-{forward,reverse}-evidence`.

Verification: full dynamic/static `make check` passed
(`build/client-controls-regression-verified.log`), including 38 directional/seven
legacy standards and all334parser/119scanner rules over2505ICL files; five optional
ANTLR checks were skipped. Native scheduler and ten CLI cases passed ASan/UBSan/
LeakSanitizer with leak detection enabled. Style/naming/whitespace and independent
source/certificate review passed. The64-register,3806-byte case took0.167s for
three replays; all10CLI cases took4.56s with60504KiB peak RSS. These measurements
do not establish broader event-model completeness.


### Simultaneous independent client chains (2026-11-13)

A standalone client may contain multiple named `Chain` SI/SO pairs. The scheduler
clocks their disjoint paths together, captures and updates once, and shifts for
the longest selected path. Shorter chains receive leading input padding and
trailing X comparison masks. Vector ports retain unused lanes, and unlisted
outputs cannot replace the declared chain outputs. Each path must reach its
declared input. Shared scan ports within named chains remain invalid under
IEEE6.4.16(q). Independently selected clients can now share internal storage;
combining multiple clients with named-chain groups remains separate work.

Native state assertions and nine CLI cases cover unequal lengths, inversion,
indexed lanes, declared-output enables, cross-chain and primary-control
prerequisites, grouped access, aliases, reservations, rollback and
source/flat/generated replay. Independent review additionally checked
cross-chain reset assertion/release and impossible simultaneous reset/write
goals, with no Important or Critical finding. New ownership and path-length
passes are O(V+E); the inherited graph builder still scans accumulated edges
per output. Constructor-only probes through 4,096 one-bit lanes took about 0.32 s.
The 64-lane, 7,230-byte case took 0.154 s for three replay forms; the full nine-case
suite took 1.35 s / 73,968 KiB. No overall linear-time or speedup claim is made.

Tessent 2026.2 exported the fixture in both directions. Reverse ordered-apply
equivalence certified; forward equivalence failed because Tessent omitted
`-together`. The unchanged comparator rejects that option difference. The new
reverse certificate raises the offline total to 39 directions. Evidence:
`test_icl_toolsets/test_simultaneous_chains/run_2026_11_13_01` and local
`build/simultaneous-chains-{forward,reverse}-evidence`.

Verification: full dynamic/static `make check` passed
(`build/simultaneous-chains-regression.log`), including native scheduler and
nine CLI cases, all grammar/scanner rules over 2,506 ICL files and the editor
corpus. Five optional ANTLR checks were skipped. The full run checked 38
directional standards; after adding the new certificate, all 39 passed the
separate offline check. Native and CLI ASan/UBSan/LeakSanitizer checks passed
with leak detection enabled. Google formatting/naming/whitespace and independent
source/certificate review passed. Broader shared-path/controller events remain
unfinished.


### Hidden standalone client events (2026-11-13)

Standalone clients now share the existing bounded capture/shift/update model
with TAP scheduling. Hidden registers retain separate shift and held state;
short scans can therefore change hidden storage without pretending that it was
explicitly written. Concrete replay resolves actual interface controls and
checks every register before committing state. Indexed inputs preserve unused
lanes and unequal simultaneous chains use the same leading-zero padding in
symbolic and concrete execution. Event work includes primary scan-input widths
and retains the existing expression budget and solver timeout.

The fast-path equivalence probe now considers schedulable primary inputs as well
as registers. An unknown or initially disabled enable can be prepared before a
hidden access; a reserved disabled input still blocks access. This fixes the
independent review's vacuous-proof finding, reproduced before the fix. Known
disabled CaptureEn is supported by retaining shift storage; unknown event
controls still reject. General serial-derived/per-edge events remain unfinished.

Native assertions check actual hidden values (`xx10` for a short single-chain
scan versus `00xx` with longer simultaneous-chain padding), unused input lanes,
visible writes and reservation rollback. Eight CLI cases cover resource slices,
primary enables, suppressed capture, unknown shift, indexed/simultaneous inputs,
original/flat/generated replay and the event-work limit. The 128-bit hidden case
took 1.055 s for three replay forms; all eight cases took 9.38 s / 119,816 KiB.
The 1,024-bit case rejects at the work limit.

Tessent 2026.2 accepted both exports for the self-constructed hidden-client
fixture. Only reverse ordered-apply equivalence certified; extra vendor read
comparisons made forward comparison fail. The new certificate raises the
offline total to 40 directions. Evidence is under
`test_icl_toolsets/test_hidden_client/run_2026_11_13_01` and local
`build/hidden-client-{forward,reverse}-evidence`.

Verification: all reported suites in the final full dynamic/static regression
passed (`build/hidden-client-regression-reviewed.log`), including 40 directional
and seven legacy standards, all parser/scanner rules over 2,507 ICL files and
the editor corpus. Five optional ANTLR checks remain skipped. Native scheduler,
eight hidden-client and four TAP-shadow cases passed leak-enabled ASan/UBSan/
LeakSanitizer. Formatting/naming/whitespace and independent source/certificate
review passed. The earlier full run used a stale static binary and is superseded
by the reviewed run.


### Masked and aliased merge writes (2026-11-13)

The existing apply-depth merge pass now accepts ScanRegister slices, aliases and
X-masked writes. Each call owns every physical register it touches for its entire
stream, including controlling fill; disjoint slices of the same register remain
serialized. Each call keeps its apply order and last-write behavior. AccessTogether,
resource, wait, empty-apply and opaque-effect barriers remain in force. Joint
scheduling still uses concrete replay and retries serially from fresh state when
necessary. This adds no alternate scheduling engine or dependency.

Twenty observation/write cases and eleven ordering cases cover inverted/nested/
ascending aliases, partial and all-X operands, inherited fill, repeated writes,
reset-conflict fallback and failed-output protection across original, flattened
and generated replay. All 31 cases passed ASan/UBSan/LeakSanitizer with leaks
enabled. Independent review found no actionable issue. The 64-call, two-apply
masked case (9,299 source bytes) took 0.204 s for three replay forms; the full
20-case suite took 3.62 s / 60,264 KiB. This is a measurement, not a speedup or
global-optimality claim. Broader target kinds and event/resource optimization
remain open.

Full dynamic/static `make check` passed with exit 0
(`build/merge-masked-regression.log`, recorded in `.exit`), including 40 directional
and seven legacy standards, all parser/scanner rules over 2,507 ICL files and
the editor corpus. Five optional ANTLR checks remain skipped. Google formatting,
changed-source naming and whitespace checks pass. No new Tessent certificate
is claimed for masked merge writes.


### Main Tcl command reset prerequisites (2026-11-13)

Source tracing now records complete main Tcl commands alongside literal PDL
sites. Hoisting releases before loops and conditionals keeps repeated pending
writes together, preserves the entry prefix and leaves procedure text intact.
Overlapping merge/compound spans are combined, including a loop that closes a
previously opened merge. Literal-site fallback retains existing repairs inside
loops that reset on each iteration. Both candidates run from fresh state and
must reproduce the exact expected operation order before publication.

Native span assertions and twelve CLI cases cover repeated queues, changing
prefixes, nested conditionals, dynamic/eval commands, comments, cross-file
merges, early return/break/continue, procedure preservation, literal fallback,
failed-output protection and original/flat/generated/reset-source fixed points.
Independent review's merge-boundary case failed before its overlap fix.
The source-action pass uses ordered maps and visits each erased action once;
it does not enumerate loop interleavings. General feedback resets and source
transformations beyond these verified insertion strategies remain open.

The 256-iteration input is 96 PDL bytes; four replay/rewrite runs completed in
about 0.24 s. Twelve focused cases took about 4.2 s in the ordinary build. Native
runtime and twelve new plus 25 existing CLI cases pass ASan/UBSan/LeakSanitizer
with leak detection enabled. Formatting/naming/whitespace checks pass, and the
bounded review of the overlap fix found no remaining findings. No new vendor
certificate or general source-transformation completeness is claimed.

The reviewed full dynamic/static `make check` passed with exit 0
(`build/compound-reset-regression-reviewed.log` and `.exit`), including 40
directional/seven legacy standards, all parser/scanner rules over 2,507 ICL
files and editor corpus checks. Five optional ANTLR checks remain skipped.
The final twelve-case measurement took 4.35 s / 59,556 KiB; the 256-iteration
case took 0.263 s for four runs. The initial full run was stopped for the review
fix and is superseded by this result.


### Merged DataRegister streams (2026-11-13)

Independent DataRegister access streams now use the existing merge pass,
including slices, inverted aliases and masked writes. Primary, serial, mixed
and addressed access still use the common scheduler. Shared buses and enable
requirements may require multiple physical frames within a merged apply;
primary observations occur before pin changes that perform writes. Failed
joint scheduling still retries serially from fresh state.

All calls share controlling and expected state. IEEE 1687 §7.9.18 permits
unreserved values to change through interleaving; final state follows the merged
sequence rather than the original serial order. Per-call apply order remains
fixed. Resource commands retain their serialization barriers. A reset-dependent
regression checks the actual post-merge data value and verifies that a reserved
variant retains its serial result. Physical overlap and AccessTogether/opaque/
resource/timing barriers retain their prior handling.

Thirteen new cases cover primary/serial/mixed/addressed access, ordered
observations and writes, masks/aliases, shared buses, reset-conflict fallback,
physical conflicts and failed-output safety. The original 31 merge, 20 primary
data and 25 serial-data cases pass. Leak-enabled ASan/UBSan/LeakSanitizer checks
pass for all new cases, including the review's final-state probe. Independent
review found no actionable issue. The 64-register, two-apply-call workload has
4,578 PDL bytes and takes about 2.3 s for three replay forms, producing two
applies. The prior serial path exceeded the test's 60-second deadline; no
precise speedup ratio is claimed. Twelve cases before the added final-state
probe took 4.36 s / 115,424 KiB.

Computed-output and connected-port ownership, timing/resource optimization and
broader vendor qualification remain open. No new vendor certificate is claimed.

Full dynamic/static `make check` passed with exit 0
(`build/merge-data-regression.log` and `.exit`), including all thirteen new cases,
40 directional/seven legacy standards, all parser/scanner rules over 2,507 ICL
files and editor corpus checks. Five optional ANTLR checks remain skipped.
Google formatting, changed-source naming and whitespace checks pass.


### Merged direct primary-input writes (2026-11-13)

Unconnected top-level DataInPort, AddressPort, ReadEnPort and WriteEnPort writes
now share the existing merge pass. Aliases and slices retain whole-port ownership
so controlling fill cannot interleave across calls. Connected child inputs and
DataOutPort targets retain serial barriers; their names do not establish physical
independence from upstream storage. Reads of input ports still reject through
normal operand validation before any output is published.

Eight new cases cover direct/aliased/masked writes, apply order, same-port slice
conflicts, primary controls, mixed scan/input streams, connected/output barriers,
resource handling and failed-output protection. All eight pass leak-enabled
ASan/UBSan/LeakSanitizer; existing 44 merge cases also pass. Independent review
found no actionable issue and additionally checked input-read rejection and
concatenated alias overlap. The 64-port/two-apply-call case has 4,834 PDL bytes
and takes 0.512 s for three replay forms; all eight cases took 1.60 s / 69,564 KiB.
Style/naming/whitespace checks pass. Computed/connected-port ownership, broader
timing/resource optimization and vendor qualification remain open.

Full dynamic/static `make check` passed with exit 0
(`build/merge-primary-regression.log` and `.exit`), including 40 directional/seven
legacy standards, all parser/scanner rules over 2,507 ICL files and editor corpus
checks. Five optional ANTLR checks remain skipped. Broader model, optimization
and coverage requirements remain open.

## Complete generator coverage invocation (2026-11-13)

Dynamic, static and coverage checks now share the same ordered 26 generator CLI
suites. The coverage link recipes retain header rebuild prerequisites while
passing only source/object files to the compiler. The LSP lifecycle suite uses
an isolated temporary workspace, avoiding unrelated files under /tmp.

The coverage source ledger now includes all authored implementation files,
including pattern_data, pattern_merge and pattern_resources, plus their emitted
inline headers. Twelve Makefile tests and seven coverage-reader tests pass;
independent review found no remaining wiring issue. The instrumented run passed
all native/editor/generator tests before the unchanged strict gate failed:
13,750/14,355 lines (95.8%) and 22,906/37,317 raw branches (61.4%).
Raw branches include compiler-emitted exception and cleanup edges.
Evidence: build/coverage-complete-suite-resumed.log (make exit 2) and
build/coverage-complete-report.log (report exit 1, including the three additional
inline-header lines). Earlier runs stopped on the repaired build/test issues or
the user interruption and are not complete coverage evidence.

The required 100% line/90% raw-branch acceptance remains unfinished. This wiring
fix and the successful instrumented tests do not establish that acceptance.

## Source archive and Rocky Linux 8 qualification (2026-11-13)

The source archive builds with BISON=false, FLEX=false and no static Tcl
archive. Temporary local dynamic and static installations pass all five version
checks, installed PDL formatter/LSP checks, original/flat/generated pattern
identity, intended reset-target writes, reset-pattern replay identity and
reset-source idempotence. Uninstall preserves an unrelated sentinel and removes
installed headers. Local logs are build/source-distribution-reset-reviewed.log
(exit 0,44.110s) and build/static-installation-reviewed.log (exit 0).
The ICL LSP lifecycle suite separately passes all62 checks for the static install.

Actual target qualification used Rocky Linux8.10 x86_64, GCC Toolset15.2.1,
GNU Make4.4.1, CMake3.26.5, Python3.8.16, system Tcl8.6.8 and pinned Z3 4.13.3.
Prerequisites, source builds, installations and test output stay under
/data/workspaces/lianghuaxiao/test/tessent_ijtag/test_icl_toolsets/test_platform/run_2026_11_13_01.
Dependency builds used two jobs. Archive SHA256 values:
- GNU Make4.4.1: dd16fb1d67bfab79a72f5e8390735c49e3e8e70b4945a15ab1f81ddb78658fb3.
- Z3 4.13.3: f59c9cf600ea57fb64ffeffbffd0f2d2b896854f339e846f48f069d23bc14ba0.
- Tested source archive: b279287fbd2d7da3f435237dc499fb77c0af8ffcaa9927657e72bf6c36a80ab8.

From that extracted archive, make -j2 dynamic BISON=false FLEX=false
TCL_STATIC_LIB=/does/not/exist Z3_PREFIX=<isolated-prefix>/deps/z3 succeeded.
The manifest installer/uninstaller and both native dynamic and transferred static
binaries passed the installed-tool checks above and both62-check ICL LSP runs.
Static ELF files have no interpreter; dynamic dependencies all resolve.
The Python launcher initially selected an EDA-bundled Tcl library. A second run
prepended the system library directories and used Python's actual binary path;
ldd confirmed /usr/lib64/libtcl8.6.so for all three Tcl-linked commands. Installed
checks and all26 generator CLI suites pass in that matching-header/runtime
environment: build/rocky-system-tcl-qualification.log and .exit (0).
The default EDA runtime also passed all26 suites
(build/rocky-generator-regression.log and .exit,0).

Qualification exposed one test-harness portability issue: Python3.8 lacks
Path.hardlink_to. The equivalent os.link test passed locally and was copied into
the extracted remote test tree before both complete CLI runs. No product source
was patched there. An earlier archive trial used an incorrect original/reset
byte-equality assertion; its failure is retained, superseded by the intended-write
and reset replay checks. build/rocky-source-qualification-final.log records all
installation successes followed by the original Python API failure; it is not
a passing whole-run log.

This qualifies the generator implementation through eecab17 and the described
build/install paths. It does not qualify the subsequently developed computed-port
merge extension, a remote static-Tcl source build, or every Linux distribution.
Strict coverage and remaining model requirements are still open.

## Merged connected and computed ports (2026-11-13)

Connected data inputs and computed outputs now participate in ordered iMerge
batching. Each call owns its logical targets and reachable combinational
dependencies, stopping at physical storage and primary inputs. This covers
output enables, mux/logic operands, one-hot contributors and their implicit
address/read-enable controls. Shared dependencies retain serial order, including
inactive branches that a more selective optimizer might prove independent.
Resource and timing barriers, logical controlling history and fresh-state
serial fallback remain in force.

Dependency enqueueing and implicit one-hot selection share a 4,194,304-reference
analysis budget. Exhaustion preserves the serial operations. Existing one-hot
resolver callers retain their per-source limit; cached selections require no
new traversal. The native test checks small-budget exhaustion, cache/default
behavior and 1,024 calls through a 4,096-bit port, with exact fallback operand/call
order. The latter analysis took 0.079 s in the focused run.

Twelve new CLI cases cover connected/inverted/aliased ports, masks, ordered
reads/writes, shared drivers, logic/mux/enables, ordinary/addressed one-hot
contributors, constants, resource barriers, failure publication and 64 independent
two-apply calls. The 64-call case emits two applies instead of 128; source/flat/
generated replay took 0.756 s (wall 0.82 s, peak 72,472 KiB) with g++ 16.2.1 -O2 on the
local Linux host. This measures the stated workload, not a global optimum.

Full dynamic/static make check passed (build/merge-computed-regression-final.log
and .exit,0), including all 27 generator CLI suites, native tests, existing vendor
standards, all 334 parser/119 scanner rules over 2,507 ICL files and Vim/Neovim
checks. Five optional ANTLR integration tests remain skipped. The 12 new CLI cases
and native scheduler tests pass ASan/UBSan/LeakSanitizer with detect_leaks=1
(build/merge-computed-sanitizer-final-{cli,native}.log). Formatting, naming,
whitespace and independent review pass. Review's addressed one-hot and aggregate
budget findings were reproduced before repair. The initial full run was stopped
with 143 for the final budget fix and is not completion evidence.

No new vendor certificate, current strict-coverage pass or Rocky8 qualification
of this extension is claimed. Broader timing/resource optimization and the
remaining event models are still open.

## Reset releases after argument evaluation (2026-11-13)

When main-level iWrite/iCall arguments execute iReset, the third source-rewrite
candidate now evaluates those arguments once before inserting reset releases.
It uses Tcl apply/uplevel and the existing parser's vector ranges, preserving
caller scope, argument ordering, literal values, vector indices and procedure
bytes. Repeated sites share one textual insertion. Fresh execution still must
match the planned ordered operations before outputs are published.

Twelve CLI cases cover whole/sliced/quoted/vector-variable targets, expanded
arguments, calls, multiple side effects, prefixes, loops, nested substitutions,
immutable-procedure failure and 64 iterations. Each successful case checks
original/flat input, rewritten-source idempotence and both pattern fixed points.
The 64-iteration, 168-byte PDL case took 7.773 s for five generator invocations
(g++ 16.2.1 -O2, local Linux host, concurrent verification jobs); this is a scale
measurement, not a speedup claim. Native tracing verifies vector ranges exclude
actual command substitutions.

Full dynamic/static make check passed: build/argument-reset-regression-final.log
and .exit (0), all 28 generator CLI suites, native tests, 40 directional and seven
legacy vendor standards, 334 parser/119 scanner rules across 2,507 ICL files,
and six Vim/Neovim tests. Five optional ANTLR tests remain skipped.
Leak-enabled ASan/UBSan/LSan passed the native runtime, new 12 cases, existing 25
generator cases and 12 compound-reset cases; see
build/argument-reset-sanitizer-{native,cli,existing}.log and .exit (0).
Formatting, naming, whitespace and independent review passed. Review exposed
PDL vector brackets being interpreted as Tcl substitutions; failing bit/slice/
variable-index cases preceded the parser-metadata fix. The initial full run
was stopped with 143 for that repair and is not completion evidence.

Tessent 2026.2 accepted and exported generated wrappers for whole-register and
sliced writes, including the reset followed by root/branch releases and the
intended value 3. Evidence is build/argument-reset-vendor/{whole,slice} and remote
test_icl_toolsets/test_argument_reset/run_2026_11_13_01. This checks Tcl acceptance
and export; it adds no directional equivalence certificate. Current strict
coverage and later-change Rocky8 qualification remain unverified.

## Independently selected client interfaces (2026-11-13)

Multiple standalone client interfaces with one SI/SO pair each and explicit
SelectPort controls now share one scheduler state. One interface is selected per
serial frame; all other interfaces must be disabled. The chosen path must reach
that interface's declared input. Shared selectors are allowed only when other
controls permit unique selection. Shared internal storage retains the value
written through the previous interface. Single-client named chains continue to
shift together; multi-client named-chain groups and ShiftEn-only selection remain
unsupported, as do multiple TAPs and intermediate register taps.

Selection changes settle asynchronous resets before evaluating routes, controls
and capture data. Symbolic snapshot passes match the concrete model's simultaneous
passes, retaining transient assertions that a single topological pass misses.
The scan reset DAG depth plus the count of reset-bearing data registers bounds
the passes; bit/object work is capped at 1,048,576 per constructor analysis and
across a scheduling horizon. The data-register bound is conservative. Larger
acyclic models may report a resource limit even within the structural bounds;
this is not proof of unreachability. Definite reset events clear controlling
history unless a subsequent successful write replaces it. Unknown resets retain
unknown hardware state and do not invent successful controlling writes.

Seventeen CLI cases cover independent/shared paths, reads, partial aliases,
inversion, unequal widths, reservations, shared selectors/outputs, cross-client
reset prerequisites, reset-before-capture cascades, transient data-register
history, parallel selector preservation, failure publication and resource limits.
Native assertions verify SI/SO streams, capture comparison bits, selected pins,
untouched input lanes and rollback. The 512-stage reset chain exercises the work
limit and output preservation. Original/flat/generated replay is required for
every successful CLI case.

The 16-client, 3,637-byte ICL workload took 2.175 s for three replay forms, wall 2.24 s,
peak 65,640 KiB (g++ 16.2.1 -O2, local Linux host, concurrent validation jobs).
Reproduce with /usr/bin/time -f 'wall=%e seconds max_rss=%M KiB' python3
tests/src/pattern/test_multiple_client_interfaces.py build/icl-pattern-generation
MultipleClients.test_multiple_client_scale. This measures the stated workload.

Tessent 2026.2 exported separate-client and shared-storage source cases; both
reverse round trips have matching ordered apply groups and local fixed points.
The offline total is 42 directions plus seven legacy standards. Inputs and hashes
are retained in tests/golden/tessent_roundtrip/multiple_clients_{separate,shared}-reverse;
remote evidence is test_multiple_clients/run_2026_11_13_01/
multiple-client-reverse-final-bundle, retrieved as
build/multiple-client-reverse-final-evidence. These certify those two cases,
not the broader reset/event model. Full dynamic/static regression passed
(build/multiple-client-regression-complete.log and .exit,0). Leak-enabled
sanitizers passed native scheduler tests and the 17 new/nine simultaneous/eight
hidden-client cases (build/multiple-client-sanitizer-{native,cli,existing}.log
and .exit,0). Naming, formatting, whitespace and independent review passed.
Earlier runs failed the duplicate-fixture and central-manifest checks; another
was stopped for the required data-fill helper extraction. They are retained
as failure evidence, not counted as passing runs.

## Selected client chain groups (2026-11-13)

A selected standalone client may now contain several named chains. All chains
in that group capture, shift and update together, with padding for unequal
lengths; other clients remain disabled. Groups may have different chain counts,
and named groups may coexist with ordinary single-pair clients. Different
clients can share hardware storage or physical outputs. Each active output
must reach its own declared input; merely reaching another input of the same
group is insufficient. Repeated Chain ports within one interface remain invalid.

The scheduler reuses the existing route DAG, events and concrete replay. For
multi-chain groups, one symbolic input tag per vertex enforces declared SI/SO
pairing through active edges. Constraints grow with vertices, edges and route
declarations, without enumerating paths. Ordinary single-pair clients retain
their original constraints: applying tags to them regressed the 16-client
workload into its solver limit during development, so that unnecessary cost
was removed before installation.

Eleven CLI cases cover unequal chains, mixed group sizes, ordinary clients,
inversions, aliases, reservations, shared storage/outputs, reset prerequisites,
together obligations, wrong pairings, disabled outputs and failure publication.
Native assertions check SI/SO streams, padded comparisons, inactive storage and
rollback. A reserved swapped route fails; an unreserved routing control may be
repaired. Every successful CLI case checks original/flat/generated replay.

The 8-client, 16-chain, 3,580-byte ICL case took 3.649 s for three replay forms,
wall 3.70 s, peak 65,512 KiB on the local Linux host (g++16.2.1 -O2, concurrent
validation jobs). Reproduce with /usr/bin/time -f 'wall=%e seconds max_rss=%M KiB'
python3 tests/src/pattern/test_multiple_client_chain_groups.py
build/icl-pattern-generation MultipleClientChains.test_group_scale.
This is a workload measurement, not a speedup claim.

Tessent2026.2 exported the four-register/two-group fixture. Its reverse
certificate brings the offline total to43 directions over27 cases plus seven
legacy standards. Evidence is build/multiple-client-groups-reverse-evidence and
remote test_multiple_client_groups/run_2026_11_13_01/
multiple-client-groups-reverse-bundle. The certificate covers that fixture;
general shared-storage/reset/event equivalence remains unverified.

Native and eleven CLI checks passed (build/multiple-client-groups-native-reviewed
and -installed-cli.log). The initial native assertion omitted a reservation and
incorrectly forbade a legal route repair; its failing installed.exit is retained,
not counted as passing evidence. Native/new11/old17-client/old9-chain checks
passed leak-enabled ASan/UBSan/LSan
(build/multiple-client-groups-sanitizer-{native,cli,existing}.log and .exit,0).
Naming, formatting, whitespace and independent source/certificate review passed.

Full dynamic/static make check passed in build/multiple-client-groups-regression.log
and .exit(0): all30 generator suites, native tests,43directional/seven legacy
standards,334parser/119scanner rules over2,511ICL files and six editor checks
(192.033s). Five optional ANTLR tests remain skipped. Strict authored-code
coverage and Rocky8 qualification of later changes remain outstanding.

## ShiftEn-only client selection (2026-11-13)

Standalone clients may omit SelectPort when they have ShiftEnPort, as allowed
by IEEE1687-2014 §6.4.16(e)(2). For multiple clients, the scheduler uses ShiftEn
membership where no explicit SelectPort exists and proves unique selection.
An enable shared with an explicit-select client retains every owner: selecting
that client cannot silently disable a pin it needs or accidentally enable a
ShiftEn-only peer. Uniqueness is independent of the current phase.

Symbolic and concrete execution assert these ShiftEn pins only during shifting;
capture, update and idle drive them low. Missing SelectPort uses the existing
active-path selection semantics (§6.4.6.11). Explicit-select clients keep their
existing behavior. Route pairing, shared storage, reset settlement and named
chain groups reuse the same scheduler and event implementation.

Eleven CLI cases cover independent clients, named groups, mixed control types,
ascending/inverted lanes, shared enables, shared storage/output, reset
prerequisites, reservations, together failures and16-client scale. Every
successful case checks original/flat/generated replay. Native checks verify
physical serial lanes and capture masks, inactive storage, idle pins and
reservation rollback. The original nine-case suite failed seven positive cases
at the SelectPort-only restriction; the two negative cases already passed.

Focused native/new11/existing17-client/11-group/eight-hidden checks pass;
see build/shift-enable-clients-{native,green,existing}.log and .exit(0).
Formatting, naming, whitespace, Makefile registration and independent source
review pass. Tessent2026.2 accepted/exported the two-client fixture; its reverse
certificate and all44 offline directional standards pass. Evidence is
build/shift-enable-clients-reverse-evidence and remote
test_shift_enable_clients/run_2026_11_13_01/shift-enable-clients-reverse-bundle.
This certifies that fixture, not every shared-enable/reset/event model.

Leak-enabled ASan/UBSan/LSan passed native scheduler checks, the11 new cases
and existing17-client/11-group cases
(build/shift-enable-clients-sanitizer-{native,cli,existing}.log/.exit,0).
The16-client/3,528-byte ICL workload took2.713s for three replay forms,
wall2.77s, peak69,012KiB on the local Linux host (g++16.2.1 -O2, concurrent
validation). Reproduce with /usr/bin/time -f 'wall=%e seconds max_rss=%M KiB'
python3 tests/src/pattern/test_shift_enable_clients.py build/icl-pattern-generation
ShiftEnableClients.test_client_scale. The earlier focused run measured2.103s;
these are workload measurements, not a claimed speedup.

Full dynamic/static make check passed in build/shift-enable-clients-regression.log
and .exit(0), including all31 generator suites,44directions/seven legacy standards,
334parser/119scanner rules over2,512ICL files and six editor checks(196.777s).
Five optional ANTLR checks remain skipped. Strict authored-code coverage and
Rocky8 qualification of later changes remain outstanding.

## Unsized literal inversion validation (2026-11-13)

The constant evaluator and signal resolver reject inverted unsized operands
before contextual sizing. This includes unsized parameter substitutions and
double inversions in WriteEnSource. Sized literals/parameters, signal inversion,
sized/signal double inversion and legal unsized X/zero padding retain their
behavior. Output publication remains atomic on rejection.

The chosen rule is IEEE1687-2014 §6.3.12(m), which explicitly prohibits inversion
of unsized elements. There is conflicting text: §6.3.11's known-width example
uses 1'b0, ~0, 5'h1a and describes its contextual inversion. Under the approved
standard-first policy, the explicit normative prohibition takes precedence.

This differs from Tessent2026.2: the vendor accepts unsized ResetValue and
DataOutPort Source inversions, their sized controls, and that exact ten-bit
concatenation example. Five corrected probes returned0 and exported PDL.
Evidence is build/source-literals-vendor-selected-evidence, with remote inputs
and logs under test_source_literals/run_2026_11_13_01/
source-literals-probe-selected. These are acceptance probes, not rejection
agreement or additional directional certificates. Initial probes used an
unsupported -batch argument or omitted mandatory scan controls; their failures
are retained separately and excluded from this evidence.

Eight CLI cases and native expression/netlist tests check the literal contexts,
parameters, source connections, mux branches, X padding, zero available padding,
double inversions and output preservation. The original contextual cases
reproduced acceptance of forbidden operands. Review found the double-inversion
parity bypass, reproduced in build/source-literals-double-red.log, before the
operator-presence fix. Reviewed focused checks pass in
build/source-literals-reviewed-{native,green}.log and
build/source-literals-reviewed-focused.exit(0). Native and eight CLI cases
passed ASan/UBSan/LeakSanitizer with detect_leaks=1
(build/source-literals-sanitizer-{native,cli}.log/.exit,0).
Formatting, naming and independent review pass. Validation adds constant work
per literal segment; existing bit limits and allocation/sizing behavior remain.
Broader source-kind and hierarchy validation remains unfinished.

Full dynamic/static make check passed in build/source-literals-regression.log
and .exit(0): all32 generator suites,44directions/seven legacy standards,
334parser/119scanner rules over2,512ICL files and six editor checks(208.212s).
Five optional ANTLR tests remain skipped. This change does not complete the
strict coverage gate, later-change Rocky8 qualification or source-kind audit.

## All-port reservations and DataRegister state (2026-11-13)

iTake/iRelease now accept all declared ICL ports, as well as registers and
instance members, under IEEE1687-2014 §7.9.19–20. Membership does not broaden
iRead/iWrite or iState target types. Unwritten taken ports remain available to
automatic scheduling; a valid SelectPort iState becomes protected until release.
Vector masks, nested ownership, persistent takes and instance traversal reuse
the existing reservation representation.

iState now accepts DataRegister storage under §7.9.22, including aliases, slices,
inversion and hierarchical registers. Assignment validates and changes held
storage directly, leaving shift storage, reset propagation and write callbacks
untouched. Read-only apply frames preserve that snapshot and controlling fill
when no hardware event occurs. This applies to data-only networks, mixed scan
networks, selected clients and the hidden-event backend. Explicit read
expectations remain comparisons even when stored hardware knowledge is unknown.

The native state test checks event-free assignment despite an asserted reset
and active write-enable, preservation of scan shift storage, unknown bits,
width rejection and later explicit reset/update behavior. Eleven CLI methods
cover original/flat/generated replay, reservations, aliases, wide values and
the no-event paths found during independent review. Eight new reservation
methods and35existing reservation methods cover the separate membership change.
The16,384-bit state-only workload (197ICL bytes) completed three replay forms in
0.129s (wall0.18s, peak49,836KiB) on the local Linux g++16.2.1 -O2 build,
recorded in build/data-register-state-scale.log; it measures state assignment,
not scan planning. Reproduce with python3
tests/src/pattern/test_data_register_state.py build/icl-pattern-generation
DataState.test_wide_state_scale.

Tessent2026.2 accepted both saved fixtures, with reverse certificates retained.
See [vendor evidence](tessent-goldens.md#all-port-reservations-and-dataregister-state-2026-11-13).
At this checkpoint DataRegister-controlled scan resets hit a scan-only reset
dependency boundary, subsequently addressed by Task35 below.
At that checkpoint, observation-history iState flags, iScan/iClock,
system-clock/time waits and broader pattern-set ownership remain unfinished.
Final native state/scheduler tests and11CLI methods pass ASan/UBSan/
LeakSanitizer with detect_leaks=1, recorded in build/access-state-sanitizer-
{state,scheduler,cli}.log/.exit(0). The earlier data-register-state sanitizer
CLI failure predates the final hidden-event fix and is superseded.
Reservation native/new8/old35 leak-enabled checks also passed in
build/port-reservations-sanitizer-{native,cli,existing}.log/.exit(0).
Changed-source naming, formatting, whitespace and independent review pass.
Combined full dynamic/static regression passed in
build/access-state-regression.log/.exit(0): all34generator suites,46directional/
seven legacy standards,334parser/119scanner rules over2,513ICL files and six
editor checks(191.518s). Five optional ANTLR checks remain skipped.
The two earlier stopped reservation full runs do not count as completed checks.
Strict authored-code coverage and later-change Rocky8 qualification remain open.


## Mixed storage reset dependencies (2026-11-13)

This Task35 checkpoint predates automatic DataRegister reset releases and
register feedback support, described in the later sections below. Its cycle
rejection and fixed-data-control restrictions are historical.

Reset ordering now includes ScanRegister and DataRegister storage in one acyclic
graph, separate from data write/address/enable ordering. Source-cone discovery
includes one-hot contributors and their selection conditions. Snapshot passes
preserve transient assertions before capture, after scan update and after data
update; convergence stops early only when simplified symbolic expressions are
identical. Actual cycles retain the feedback diagnostic.

Native replay records the last known controlling event per register: a known
write payload or an asserted reset. Unknown writes/resets do not replace that
history, and an unchanged reset still counts. Event-free assumptions do not
create events. Copies and rollback retain history; frame and idle boundaries
drain it, including the authoritative TAP execution. Hidden scan writes keep
the existing fill policy, while resets still affect controlling fill. Data
updates use the existing phase events, excluding the TAP instruction scan.

Local-reset prerequisite and greedy planning include held DataRegister state.
Automatic repair through a DataRegister write path remains unsupported and is
diagnosed when a fixed data control prevents release. Requested scan loads keep
implicit fill until their reset is known inactive, allowing prerequisite scans
to release the reset before the requested write. A known asserted reset makes
an omitted payload irrelevant only in the backend without hidden events.
The solver constrains reset-sensitive loads to representable updates or proven
no-effect payloads, with concrete replay retaining the final check. These
capture/final-frame predicates do not establish arbitrary per-edge support.
Likewise, a scan payload
that differs from implicit fill but cannot be represented by a valid emitted
iWrite either fails the bounded schedule search or reports that raw scan output
is required during concrete replay. This is a serialization limit,
not proof that no physical schedule exists. Feedback and general per-edge
event solving remain open.

Thirteen CLI methods exercise original/flat/generated replay, mixed cascades,
transient reset history, one-hot forwarding, actual-cycle rejection, greedy
planning with read-only data storage and preserved output on rejection.
Native tests check actual storage, scan events, controlling fill, event-free
state, copying and TAP history draining. Independent review reproduced and
closed transient settlement, one-hot dependency, TAP phase, greedy-state and
local-reset prerequisite payload defects; the corresponding failing logs remain under build/data-reset-*.

The 64- and128-register alternating reset chains (4,509 and8,706ICL bytes)
completed three replay forms in0.212s and0.398s in
build/data-reset-final-cli.log/.exit(0), on the local Linux g++16.2.1 -O2 build.
Reproduce with python3 tests/src/pattern/test_data_reset_dependencies.py
build/icl-pattern-generation DataResetDependencies.test_mixed_reset_chain_scale.
Before expression simplification and proved fixed-point termination, the128
case exceeded45s (build/data-reset-scale.log); concurrent checks make those
runs unsuitable for a precise speedup ratio. Existing traversal and expanded
reset-work budgets remain enforced.

A DataRegister-selected reset mux passes Tessent2026.2 reverse certification.
Direct DataRegister-to-ResetPort wiring is rejected by Tessent ICL65-1; local
semantic fixtures accepting that form do not establish vendor portability or
complete source-kind validation. See the [probe record](tessent-goldens.md#mixed-storage-reset-dependencies-2026-11-13).

Final sanitizer evidence is build/data-reset-durable-sanitizer-{native,main,
arguments,cli}.log/.exit(0), with ASAN_OPTIONS=detect_leaks=1 and
UBSAN_OPTIONS=halt_on_error=1. The unchanged native state source also passes in
build/data-reset-sanitizer-state.log/.exit(0). The final13 CLI methods and12
argument-reset methods pass in build/data-reset-representable-{final-cli,
arguments}.log/.exit(0); the final64/128 chain measurements are0.228s/0.385s.
The47direction offline check, formatting and naming records also have exit0.
Task35's full command retained exit2 in build/data-reset-durable-full.log/.exit
solely for its final editor target: Neovim inherited Vim's runtime directory.
The corrected harness clears VIM/VIMRUNTIME; all six editor tests passed in
181.358s in build/data-reset-editor-isolated.log/.exit(0). Those two records
together cover Task35's required checks; the original command was not exit0.
Interrupted earlier runs and superseded failures remain recorded, not counted
as passes. Strict coverage and later-change Rocky8 qualification remain open.

## State assumptions with queued accesses (2026-11-13)

Default iState and -LastWrittenValue now work between queued iWrite/iRead and
iApply. Under IEEE1687 §7.9.11 and §7.9.22(d), they overwrite the controlling
and expected values of overlapping target bits. For example:

```tcl
iWrite value 5
iRead value 5
iState value 9
iApply
```

The pending write and comparison use9. The assumption itself emits no device
operation and does not create an apply. Aliases and slices match by resolved
bit identity, including inversion; a connected output is a distinct expected
state target. Unrelated queued bits remain unchanged. Unknown assumption bits
clear previous queued values instead of resurrecting an older write. A later
explicit write still replaces the corresponding pending value.

Local-reset rewriting previews state-bearing access groups with copied hardware,
controlling fill, TAP execution and reservation state. It omits a release
preamble only when the group passes strict checks. Later groups can still
receive required releases. Previews do not restore the final end state or
mutate live execution. State-free caller transformations retain their existing
limits. Observation-history forwarding is covered below; live hardware readback remains
unavailable during static generation.

The pending-reference index visits overwritten references once and materializes
operands at apply. Work is amortized over queued references and state bits with
ordered-map lookup factors. The isolated15-method check took2.44s wall time and
91,404KiB peak RSS on local Linux g++16.2.1 -O2. Three replay forms for1,024 and
4,096 write/state pairs (33,552 and134,160PDL bytes) took0.124s/0.219s.
A64-register,16-group input (3,435ICL and323PDL bytes) took0.141s normally and
0.469s with reset rewriting. These measurements are scale evidence, not a
speedup claim. Records: build/queued-state-final-focused.log/.exit(0).

Tessent2026.2 rejects original pending-iState input, so input compatibility is
not claimed. A generated-output fixture with an observation at each apply
passes exact forward certification. The earlier empty-apply fixture was
accepted but lost that boundary in the vendor export and failed comparison;
its evidence remains preserved. See the [vendor record](tessent-goldens.md#queued-state-assumptions-2026-11-13).
Integrated15-method replay passes in build/queued-state-integrated-focused.log/
.exit(0). Leak-enabled ASan/UBSan checks pass for native scheduling,new15,main25,
resource35,argument12 and reset13 CLI methods in build/queued-state-sanitizer-*
logs with matching exit0 files. The incorrectly named initial main invocation
retains exit2; build/queued-state-sanitizer-main.log/.exit(0) is the corrected
run. Formatting, naming and independent source/certificate review pass.
Complete dynamic/static regression passes in build/queued-state-full.log/.exit(0):
36generator suites,48directions/seven legacy standards,2,516ICL files covering
334/119parser/scanner rules, and six editor tests in182.910s. Five optional ANTLR
checks remain skipped. Strict authored coverage and later-change Rocky8
qualification remain open.

## Observation-history state forwarding (2026-11-13)

The generator accepts iState -LastReadValue and -LastMiscompareValue on valid
state points, preserving the numeric value, target and explicit flag in ordered
output. Under IEEE1687 §7.9.22(g–i), these statements describe history. They do
not change hardware assumptions, controlling fill, pending read/write values,
or taken-resource protection. A historical gate value does not release a reset
or prevent a required automatic gate write.

Shared state-point validation accepts registers and supported top-level
DataInPort/SelectPort targets, with aliases and slices. History may describe
controller-owned instruction storage because it does not change TAP state;
default/-LastWrittenValue assumptions on that storage remain rejected.

A file containing only state statements creates no device transaction, including
when iApplyEndState is declared. Final restoration requires a preceding iApply,
iReset or iRunLoop. This also fixes the earlier default-state-only case that
could emit an unintended write/apply solely to restore an end-state directive.

At this Task37 checkpoint, all getters reported unavailable hardware readback.
Task43 adds [explicit-history getters](#explicit-observation-history-getters-2026-11-13);
expected comparison data is never substituted for an observation. Tessent2026.2 rejects
the history flag in the recorded probe, so vendor portability is not claimed.

Thirteen new CLI methods cover both flags, pending accesses, scan/data/control
state, aliases/prefixes, merge order, reservations, reset rewriting, TAP
instruction history and atomic rejection. The isolated run passed in2.338s;
4,096history statements (140,800PDL bytes) completed three replay forms in0.194s
on local Linux g++16.2.1 -O2. Forwarding is linear in resolved bits and output
size; no additional history database or repeated prior-operation scans are used.
Reproduce with tests/src/pattern/test_observation_state.py.

Isolated native state,new13,DataRegister-state11,end-state11 andqueued-state15
checks pass ASan/UBSan with leak detection enabled in
build/observation-state-sanitizer-*.log/.exit(0). Formatting, naming and independent
review pass. Complete dynamic/static regression passes in
build/observation-state-full.log/.exit(0):37generator suites,48directional/seven
legacy standards,2,516ICL files covering334/119parser/scanner rules, and six
editor tests in190.833s. Five optional ANTLR checks remain skipped. Strict
authored coverage and later-change Rocky8 qualification remain open.

## Automatic DataRegister reset releases (2026-11-13)

Local-reset rewriting can now choose DataRegister reset controls with declared
write paths. It reuses the existing symbolic reset solve, preservation masks
and dependency layers, then emits ordinary iWrite/iApply releases outside
unchanged procedure bodies. Read-only data controls remain fixed. Existing
scheduling and concrete replay must implement each chosen payload; a declared
write path does not guarantee every payload is feasible.

Scan and data ancestors are released in dependency order. Reserved controls,
taken-but-unwritten bits and reserved upstream write enables retain their
constraints. Disabled or incompatible data paths fail atomically. Existing
solver/traversal limits remain; failure within those limits is not proof of
physical unreachability.

An original source and its local-reset rewrite need not produce the same scan
schedule: explicit release commands add boundaries and establish controlling
fill. Both sources retain their own original/flat/generated replay checks.
Native assertions independently check reset release, the target value9,
different scan-source fill and subsequent X-write retention.

Fourteen CLI methods and native checks pass, including unsigned reset-choice
ordering. The reset objective keeps its XOR distance as a bit vector, preserving
unsigned ranking while avoiding a conversion that exceeded the unchanged solver
limit under sanitizers. The same64/256-bit cases (458/460ICL bytes, six runs each)
measured0.643s/4.880s before and0.487s/3.729s after on local Linux g++16.2.1 -O2.
Paired total wall/RSS were6.19s/64900KiB and4.84s/64724KiB. These single paired
measurements do not establish a universal speedup or a4096-bit runtime guarantee.
See build/data-reset-releases-{baseline,bv}-benchmark.log/.exit(0).

The14CLI cases and native/reservation/argument/reset-dependency/queued-state
suites pass ASan/UBSan with leak detection in the host environment. The first
full run failed a duplicate-module audit; the fixtures now share one definition
and the corrected audit passes. A stale native fixture filename was corrected.
An initial sanitizer run was blocked by sandbox tracing; the host rerun retained
leak detection. Failure logs and the detailed evidence are retained in the
[implementation plan](superpowers/plans/2026-11-13-data-reset-releases.md).

The generated reset-mux artifact passes reverse Tessent certification; the
forward comparison failed after Tessent added an implicit read. See the
[vendor record](tessent-goldens.md#dataregister-reset-release-artifact-2026-11-13).
Full dynamic/static regression passes in build/data-reset-releases-final-full.log/.exit(0):
38generator suites,49directional/seven legacy standards,2,517ICL files covering
334/119parser/scanner rules, and six editor tests in186.241s.

## Register reset feedback (2026-11-13)

The scheduler now accepts reset feedback among ScanRegisters and DataRegisters.
It groups mutually dependent reset controls, propagates resets through global
simultaneous snapshots, and preserves unknown state, transient assertions and
controlling-fill history. A component of N registers needs at most 2N changing
passes because each vector moves only toward its fixed ResetValue; one extra
pass records reset events after hardware stops changing. Existing work and solver
limits remain. Grouping reset releases does not prove that held controls are
writable: scheduling and concrete replay still validate each selected payload.

Capture uses the earlier state for comparisons. Update uses the state after the
capture-triggered reset closure, including writability and fill checks. General
resets driven by changing serial or phase pins during shifting remain outside
this increment; the remaining per-edge model work is still required.

Reset muxes now retain a known result under an unknown selector when all possible
branches agree. A shared cube-subtraction calculation identifies uncovered
selector encodings; any possible uncovered encoding keeps the result unknown.
Overlapping branches retain their per-bit join. Known selectors avoid coverage
expansion, and unknown-selector coverage has a 4 MiB character-work budget that
counts full-width scans and copies before allocation.

Ten focused CLI methods pass in build/feedback-mux-cli.log/.exit(0), as do the
native state and symbolic tests. An independent binary-completion oracle checks
complete, partial and overlapping mux codes, including symbolic known masks.
The 8/32-register feedback rings (801/2,475 ICL bytes) take 0.165s/1.144s across
three replays on local Linux g++16.2.1 -O2; the whole suite takes 2.67s wall time
and 66,940 KiB peak RSS. Named symbolic snapshots are retained: the same factored
expressions without them took 20.828s for 32 registers. These measurements do not
establish a runtime guarantee for arbitrary feedback networks.

The reset-mux feedback fixture passes reverse Tessent certification after the
mux precision fix. Leak-enabled ASan/UBSan checks pass for three native tests
and eight affected CLI suites in build/feedback-final-sanitizer-*.log/.exit(0).
Full dynamic/static regression passes in build/feedback-final-full.log/.exit(0):
39 generator suites per build, 50 directional and seven legacy standards,
2,518 ICL files covering all 334 parser and 119 scanner rules, and six editor
tests in 179.205s. Five optional ANTLR tests remain skipped. See the
[implementation plan](superpowers/plans/2026-11-13-feedback-resets.md).

## Direct hierarchical connections (2026-11-13)

Direct ICL hardware connections now require a hierarchical reference to name
an output port. A child register, logic signal, alias or input port must not be
used as an instance output. This applies through the shared resolver to bindings,
mux inputs/selectors and logic expressions. The parser already limits ordinary
connections to immediate child instances under IEEE1687 §6.3.10.5; its existing
depth diagnostic is retained.

Alias definitions retain deep register/port references, and PDL still accesses
deep targets. Original, flat-ICL and generated-PDL replay preserve those targets.
Sibling output bindings, control outputs and inverted slices remain supported.
Invalid inputs are rejected before replacing output artifacts. Further
signal-kind checks and restrictions on later ICL use of deep aliases remain
outstanding requirements.

Native netlist tests and27 main CLI methods pass. A fresh leak-enabled sanitizer
build passes native netlist, main27, multiple-client17, named-group11, ShiftEn11,
port-reservation8 and data-reset13 tests. Full dynamic/static regression passes
in build/connection-hierarchy-final-full.log/.exit(0), including39 generator
suites per build,50 directional/seven legacy standards,2,519 ICL files covering
334/119 parser/scanner rules, and six editor tests in177.025s. No new vendor
certificate is claimed. See the
[implementation plan](superpowers/plans/2026-11-13-connection-hierarchy.md).

## Direct ScanOutPort register sources (2026-11-13)

A ScanOutPort Source that directly names a ScanRegister must use its declared
rightmost bit, under IEEE1687 §6.4.6.2(b). Elaboration now checks this before
scheduling, including ascending declarations, ranges and concatenations.
Inversion and scalar registers remain supported. Bare vector references retain
normal width rules; only ScanInSource has the implicit-rightmost permission.
DataOutPort, Enable and PDL may still refer to intermediate register bits.
Alias and mux traversal are outside this direct check.

Native netlist and29CLI methods pass, including original/flat/generated replay
and preservation of pattern/reset/debug artifacts on invalid inputs. Independent
source review, formatting/naming and the existing5000-instance/wide benchmark
pass. Leak-enabled ASan/UBSan checks pass for native netlist, main29,
shared-output9 and named-group11 tests. Full regression passes in
build/scan-output-final-full.log/.exit(0):39 generator suites per build,
50 directional/seven legacy standards,2,519 ICL files covering334/119 rules,
and six editor tests in177.787s. See [the focused plan](superpowers/plans/2026-11-13-scan-output-validation.md).

## Deep alias consumers (2026-11-13)

Aliases whose definitions contain a reference through two or more instance
levels remain available to PDL but cannot be used by another ICL construct,
including an Alias definition. The restriction applies to the whole alias name,
even when a consumer selects only a local portion of a mixed alias. Local alias
chains and immediate-child output aliases retain their behavior.

This follows the specific normative IEEE1687 §6.5.2(g) rule. The broader prose
in §6.3.10.5 allows alias wrappers; the implementation gives the specific rule
precedence and records that wording conflict. Tessent2026.2 rejects the tested
deep-alias wrappers and accepts local and immediate-child output wrappers.
See [the observational vendor evidence](tessent-goldens.md#alias-consumer-probes-2026-11-13).
Immediate-child register/port restrictions and other source-kind rules remain
separate requirements.

The check runs after all connections are resolved and before flat-origin
restoration. Native tests cover each resolved consumer collection and both
declaration orders;31CLI methods pass, including original/flat/generated
replay and atomic artifact preservation. A5000-alias local chain and existing
wide benchmarks pass; format/naming and independent review pass. Leak-enabled
ASan/UBSan checks pass for native netlist, main31, apply-end-state11 and
merge-observation20 tests. Full regression passes in
build/deep-alias-final-full.log/.exit(0):39 generator suites per build,
50 directional/seven legacy standards,2,519 ICL files covering334/119 rules,
and six editor tests in177.360s. See
[the focused plan](superpowers/plans/2026-11-13-deep-alias-consumers.md).

## Explicit observation-history getters (2026-11-13)

iGetReadData and iGetMiscompares can now return history explicitly supplied by
iState -LastReadValue and -LastMiscompareValue. Every requested bit must have
supplied history; expected comparison values and current modeled storage never
stand in for an observation. For example:

```tcl
iPDLLevel 1
iState value 9 -LastReadValue
iNote [iGetReadData value -hex]
```

For a four-bit value, the note is 0x9. Getters expand to Tcl literals without
adding hardware operations. Binary output has a 0b prefix; known hexadecimal
digits are lowercase. An entirely x hexadecimal group becomes X, a partially
unknown group becomes x, and decimal with any x becomes X. Known decimal
conversion supports bignums and uses Tcl's C API, so a user-defined expr command
cannot alter the value.

Aliases, slices, prefixes and procedure calls share resolved register/port bit
identity. Read history follows alias inversion; miscompare masks do not invert,
because complementing both observed and expected values preserves their XOR.
A DataOutPort remains distinct from its driving register. History assignments
use the existing state-target contract; getters use the observable-target
contract. The specific IEEE1687 §7.9.22(h) rule describes miscompare history;
its introductory text instead says expected values. This implementation follows
the specific rule and records that wording conflict.

An iRead invalidates both history categories for its target bits at iApply,
including reads with no expected value or all-X comparisons. A queued read alone
does not invalidate history. iReset discards pending reads without erasing
supplied observations; later empty applies cannot revisit those discarded reads.
Subsequent explicit history restores availability. Writes, controlling-state
assumptions and unrelated reads preserve it. Live readback, iGetStatus and
ScanInterface/-chain history remain unavailable during static generation.
Tessent history-flag portability remains unverified.

Storage is sparse by node and bounded by existing elaborated bit limits.
Getters visit requested bits; apply visits its pending operation range once.
On local Linux g++ 16.2.1 -O2, three replay forms for 1,024 repeated getters,
1,024 separate targets and a 65,536-bit value took 0.143s, 0.214s and 0.408s.
The 20-method CLI suite took 4.259s (4.31s wall, 75,660KiB peak RSS).
These are supported-scale measurements, not a speedup claim.

The wide case exposed an existing serializer defect: an empty DataRegister
declaration was emitted with an illegal empty body. Serialization now retains
the semicolon form; attribute-bearing bodies are preserved. Native tests cover
scalar, vector and nested-instance roundtrips.

Focused runtime/flatten tests, all 20 observation-state CLI methods and the
independent review correction pass. Leak-enabled runtime/flatten and observation-state (20), queued-state (15),
argument-reset (12) and compound-reset (12) CLI checks pass. Fresh instrumented
runtime/flatten builds and tests also pass. Complete regression passes in
build/observation-getters-final-full.log/.exit (0): 39 suites per build, 50
directional/seven legacy standards, 2,519 ICL files, all 334/119 grammar/scanner
rules, and six editor tests in 180.590s. Five optional ANTLR tests were skipped.
Evidence is retained under build/observation-getters-final-sanitizer-* and
build/observation-getters-instrumented-*, all with matching exit 0 files. See
[the focused plan](superpowers/plans/2026-11-13-explicit-observation-getters.md).


### Standalone TCK idle replay (2026-11-13)

Literal TCK waits now replay standalone client clocks through the shared network
state. Idle drives owned capture/shift/update and clock pins low while preserving
SelectPort assumptions and unowned vector bits. Each clock edge uses actual
child clock gates and controls; hidden capture/update, reset and DataRegister
fill events retain their existing ordering. Idle normalization does not invent
a primary-port write. A wait creates no new observation-history value.

Missing selection is inferred only when the controls mask it, or when one scan
route is proven and every explicitly sourced/driven interface select is known
active. This deliberately rejects some valid selectively enabled networks;
ancestor-aware general selection remains required. Explicit selection and
unknown clock gates retain their modeled values. Inverted child clock phases
remain unsupported. Reservations are checked at transitions, and failure rolls
back hardware state, fill and output publication.

The existing complete-state fixed point accelerates stable waits, including
UINT64_MAX counts; changing states retain the cycle/work/time bounds. The
65536-bit stable case passed three replay forms in1.440s in the final focused
17-method CLI run (6.786s total). This measures the existing bounded algorithm,
not an unbounded periodic-state accelerator. The later eighteenth method replays
the vendor fixture. Native tests cover transient reservations, reset edges,
unknown gates, mux-hidden storage and DataRegister fill after clocked writes.

Tessent2026.2 exact forward certification covers the explicit client fixture
with waits of3 and5 TCK cycles between accesses. The first comparison failed
because Tessent emitted an extra initial reset-value read; its evidence is
preserved. A fresh fixture explicitly requests that read and passes the unchanged
comparator. This does not certify arbitrary hidden-event or clock-gate models.
System clocks, absolute time, multiple TAPs and external instrument behavior
remain separate requirements. Full integration checks passed on2026-11-14:
all39 dynamic/static suites,51directional/seven legacy standards,2519ICL files
and six editor tests in178.871s. Evidence: build/standalone-idle-reviewed-full.log
and .exit(0), plus standalone-idle-final-sanitizer-all.exit(0).


### Alias instance boundaries (2026-11-14)

An Alias cannot turn an internal child register, input port or internal alias
into a hardware connection across an instance boundary. Such aliases remain
usable by PDL and other Alias definitions; the restriction follows wrappers,
forward declarations and slices. A hardware consumer of a mixed alias is still
rejected when it selects only local bits, because the restriction applies to the
alias name. Local register/input aliases and aliases of child output ports keep
their existing hardware use. Output-port drivers are not traversed by this check.
The stricter two-level alias rule remains unchanged.

This enforces the output-port boundary already required by direct connections,
using IEEE1687 §6.3.10.5, §6.3.12 and §6.5.2. It does not claim to settle every
wording ambiguity in §6.5.2(f) or implement every signal-kind rule. Existing
Tessent2026.2 probes also reject child-register hardware consumers, but reject
wrapper Alias definitions too; the local implementation retains that documented
standards-first difference. No new vendor certificate is claimed.

Classification runs after connections resolve and before flat-origin restoration.
An iterative reverse dependency queue marks each alias at most once, taking
O(nodes + resolved references) time and space. Native scale cases cover256 and
4096 restricted wrappers (6942/101454 bytes,0.001/0.022s) and a65536-bit alias;
the existing legal5000-wrapper case still passes (0.018s). The complete native
run took0.46s wall and118388KiB RSS on the local Linux/GCC build. These measurements
do not claim a speed improvement over the previous implementation.

The native tests and33 main CLI methods pass, including unchanged output files
on rejection and source/flat/generated PDL-wrapper replay. Leak-enabled ASan/UBSan
netlist/flatten tests and main33/end-state11/merge-observation20 methods pass.
Style/naming and independent review pass. Full integration passed with exit 0:
39 dynamic/static suites, 51 directional/seven legacy standards, 2,519 ICL files
and six editor tests in178.257s. Fresh instrumented tests complete, but the strict
coverage gate fails at96.2% lines/62.0% raw branches. The final help-only correction
has dynamic/static smoke verification. Evidence uses build/alias-boundary-*.

### Rocky Linux 8 refresh (2026-11-14)

The source archive from473bc9e builds unchanged on Rocky Linux8.10 x86_64 with
GCC Toolset15.2.1, Make4.4.1, Python3.8.16, system Tcl8.6.8 and pinned Z3 4.13.3.
Bison/Flex are disabled and no static Tcl archive is supplied. The archive has
SHA256 cccf17efc3aebc19e3d185ee9d7c3e440a04a89870893b4887877b3255705805;
the transferred local static-binary archive has
SHA256 786f21a67ec89ad8dc2af595c28c8c2bf7ba3d500f6b00b2c46690743b4d7a3f.
Both hashes were verified after transfer. No extracted source/test file needed
patching. This extends the dated qualification above through Tasks27–45,
including the final standalone-wait help correction.
Independent review also confirms that all3775 tracked files in the source
archive match their Git blobs at473bc9e; only the expected generated parser,
scanner and .prebuilt files are additional.

The remote run is retained at /tmp/icl-platform-473bc9e-20261114. It reuses the
previous run's Make/Z3 dependencies read-only, with isolated dynamic/static
install prefixes and scratch directories. The actual Python executable is
/data/eda/mentor/calibre_2016/aok_cal_2026.1_17.11/pkgs/icv_python/asp/bin/python3;
its directory follows Make/GCC in PATH. LD_LIBRARY_PATH begins with
/usr/lib64:/lib64, then the GCC and Python library directories. This avoids the
EDA Python wrapper's Tcl override. ldd records system /usr/lib64/libtcl8.6.so
for all three Tcl-linked dynamic commands.

The core source commands, from the extracted archive, are:

```sh
make -j2 --jobserver-style=pipe dynamic BISON=false FLEX=false \
  TCL_STATIC_LIB=/does/not/exist Z3_PREFIX=<previous-run>/deps/z3
make -j2 --jobserver-style=pipe pattern-netlist-check pattern-runtime-check \
  pattern-flatten-check BISON=false FLEX=false \
  TCL_STATIC_LIB=/does/not/exist Z3_PREFIX=<previous-run>/deps/z3
make pattern-generation-check BISON=false FLEX=false \
  TCL_STATIC_LIB=/does/not/exist Z3_PREFIX=<previous-run>/deps/z3
```

All39 dynamic CLI suites pass (514 tests), as do all39 suites using the
transferred static generator. Both isolated five-command installations pass
version, PDL formatter/LSP, original/flat/generated pattern and reset-artifact
replay checks through tests/tests/test_source_distribution.py's existing
check_installed_tools helper. Both installed ICL LSP runs pass62 checks.
Manifest uninstall removes the installed commands/headers and preserves an
unrelated sentinel file in each prefix. All five static ELF files lack an
interpreter. The static installer and static CLI target use make -o for each
transferred build/<command>-static executable; no remote static source build
is claimed.

All16 exit files, including the aggregate qualification.exit, are0. Local
evidence is retained in build/qualification-473bc9e/remote-evidence, including
environment.log, archives-check.log, dynamic-libraries.log, static-elf.log and
the separate build/install/uninstall/native/CLI logs. Source distribution
creation passed in build/qualification-473bc9e-dist.log/.exit(0). Personal
installations and editor configuration were not changed. This is platform and
installer evidence, not another remote full make check, a new Tessent certificate,
or qualification of later source changes. Strict coverage and unfinished model
requirements remain open.
The exact submitted command body is retained in
build/qualification-473bc9e/commands.log. Independent evidence review passed.

### Common-edge TAP replay (2026-11-14)

Concrete replay now computes every TAP's actions before publishing shared
control outputs, then evaluates register controls after all outputs are ready.
This removes dependence on AccessLink name order when one child forwards capture,
shift or update controls to another. One joint register clock event and subsequent
instruction/reset settlement preserve the shared hardware state.

Six native variants cover the three forwarded controls in both name orders,
with source/flat equality at every cycle. The old product fails the capture
assertion when the producer sorts last. Existing unknown-controller and old
serial-state tests remain passing. Two TAPs replaying 1000 idle cycles in both
forms take 0.0339409s locally; the complete focused native run takes 0.12s and
23244KiB maximum RSS with GCC16.2.1. No speed improvement is claimed.

Full regression and leak-enabled sanitizer evidence is recorded above and in
[the focused plan](superpowers/plans/2026-11-14-tap-edge-snapshot.md). An earlier
detached full run ended without a completion status; its logs are preserved,
and only the completed foreground rerun is counted as passing. This replay fix
does not enable multiple-TAP scheduling or resolve shared-register ownership,
implicit multiple-TAP reset ownership or general clock/event support. Strict
coverage was not rerun, and Rocky8 qualification remains at473bc9e.

### Common-phase multiple TAPs (2026-11-14)

Multiple TAPs can now share one actual top-level TCK node while using distinct
serial lanes. Shared or separate TMS pins carry the same IR/DR phase. Unequal
instruction/data widths use aligned lanes, and every TAP participates in every
common scan, including an unrequested TAP's current instrument, BYPASS or IDCODE
path. Instruction captures and reset fills use each TAP's own BSDL metadata.
Synthetic BSDL data storage remains in internal replay and fill history; it is
not emitted as a user PDL target. A meaningful synthetic comparison or a physical
scan with no expressible remaining access is rejected.

Reset dependencies must resolve through explicit connectivity; virtual global
reset ownership for multiple TAPs remains undefined. Explicit root ResetPorts
follow their declared polarities, and applicable TRST pins are asserted together
before shared reset settlement. Mixed TRST availability uses common reset clocks
where needed. Synchronous TAP reset does not invent an asynchronous TRST pulse.
The batched pin snapshot also stages every lane's TMS/TDI before the common edge.
A native gated-reset case proves that reversing TAP names cannot lose a reset.

IR-only and DR-only AccessTogether groups can span TAPs. A simultaneous group
mixing IR and DR still requires independent-phase work. One joint solver and
concrete state retain cross-TAP reset, capture/control and resource dependencies;
no per-TAP state partition is introduced. Generated applies also preserve
ScanRegister end-state overrides only when that frame represents their write.
This fixes an existing single-TAP mixed-frame case where regeneration inserted
an extra restoration scan after an override expired early.

Nineteen new CLI methods cover source/flat/generated fixed points, distinct IR
capture patterns, root reset polarities, mixed TRST, cross-TAP controls/data,
held unrequested lanes, AccessTogether, mixed primary/end-state frames, hidden
instrument events, local-reset rewriting and atomic rejections. Native tests
check held state, aligned lanes, common updates and failed-plan rollback.
Independent source and corpus-policy reviews pass; the exact submitted vendor
probe files are retained, with their one duplicated instrument definition
recorded by occurrence and hash in the corpus policy.

On local Linux/GCC16.2.1, four replay forms for4/16/32 TAPs take approximately
0.575/3.717/6.309s with6907/26431/52703 model bytes. IR widths vary4–6 bits and DR
widths4–8 bits. The combined run takes10.65s and80220KiB maximum RSS. The32-TAP
case explicitly sets its instructions first; implicit joint32-TAP setup reaches
the unchanged2s solver budget. A hidden-SIB two-TAP case likewise uses explicit
IR/gate setup within the current budget. No global optimality or speedup is
claimed, and unsuccessful bounded plans are not counted as supported schedules.

Final full regression passes in build/multiple-tap-confirmed-full.log/.exit0:
40 dynamic/static suites,51 directional/seven legacy goldens,2523 ICL334/119
rules and six editor tests in178.774s. The first full attempt's corpus-policy
failure remains in build/multiple-tap-final-full.log/.exit2. Focused19-method
and existing-suite checks, native state tests, formatting and naming pass. Leak-enabled native,
main/TAP/hidden-event checks pass. The instrumented19-method run records18
functional passes and a4-TAP scale pass, but its16/32-TAP cases reach the solver
budget; that failed run is retained and is not a full sanitizer-suite pass.
Separate18-functional-method, end-state11 and mixed-frame15 sanitizer runs pass;
the exact scope is recorded in [the focused plan](superpowers/plans/2026-11-14-common-phase-multiple-taps.md).

Tessent2026.2 accepts the single explicit-TAP control probe. The two-TAP loading
probes fail at set_current_design with an empty filename diagnostic and SIGSEGV;
a single TAP naming the second BSDL entity fails because the loader selects the
first entity. These are loading failures, not proof that the IEEE model is
invalid, and they provide no multiple-TAP equivalence certificate. Evidence is
retained in build/multiple-tap-probe-evidence and
build/multiple-tap-loading-probe-evidence.

Independent TAP phases, distinct equivalent TCK roots, shared intermediate
serial storage and general implicit multiple-TAP reset ownership remain required.
Strict coverage and Rocky8 qualification still predate this implementation.


### Equivalent TCK root snapshots (2026-11-14)

Common-phase TAP scheduling now supports distinct top-level TCKPort roots.
IEEE1687-2014 §6.4.6.17(a) defines those ports as equivalent with the same polarity.
Every unconnected root participates, including an extra root connected to an
instrument clock but not named by any TAP BSDL. Each logical edge stages all old
root levels, samples child clocks, stages all new levels, publishes every affected
TAP control, and clocks shared storage once. The native single-root edge API
continues to advance only controllers driven by that root.

Actual child-clock qualification remains required. Disabled or unknown gates do
not acquire manufactured edges, and unsupported inverted child phases retain their
diagnostic. Batch inputs must be scalar, unconnected top-level TCKPorts and affect
at least one TAP; invalid input is rejected before state changes. Independent
TMS phases, shared serial storage and implicit multi-TAP reset ownership remain
separate requirements.

The22-method CLI suite covers original, flat and regenerated PDL, unequal lane
widths, extra roots, reset and idle, gates and atomic output preservation. Native
checks reverse root/controller names for forwarded capture/shift/update controls,
verify cross-root capture from old held state and a single common update, and
check singleton and invalid-batch behavior. Independent source review is clean.
With local GCC16.2.1, four forms for4/16 roots take0.536/3.540s, using6919/26497
model bytes. The entire22-method run takes23.41s and78112KiB maximum RSS. Native
1000-cycle two-form replay takes0.0382814s with one root and0.0381881s with two;
the complete native run takes0.20s and23168KiB. These are measured bounds for
these inputs, with unchanged solver budgets, rather than a speedup claim.

Focused evidence: build/equivalent-tck-reviewed-native.log/.exit0 and
build/equivalent-tck-reviewed-cli.log/.exit0; naming and formatting pass.
Leak-enabled native state/TAP/scheduler/flatten and main33/TAP9/hidden4/
functional20/end11/mixed15 checks pass in build/equivalent-tck-final-sanitizer-*.
The20-method sanitizer run excludes both optimized scale methods; Task48’s
known instrumented solver-budget limits remain. Full integration passes in
build/equivalent-tck-final-full.log/.exit0:40 dynamic/static suites,51 directional/
seven legacy goldens,2523 ICL334/119 rules and editor6 in178.636s. Full wall time
is1047.081s; the five optional ANTLR tests remain skipped.
Strict coverage, vendor certification and Rocky8 qualification have not been
refreshed for this change.


### Ordinary data-source kind eligibility (2026-11-14)

Elaboration now rejects clock, scan and control-only terminal kinds in ordinary
data consumers. This covers data-output and ToSelect sources, child data/select
inputs, register capture/read/write data and write enables, output enables, mux
selectors, LogicSignal operands and explicit OneHotDataGroup Port contributors.
Alias classification follows only the consumed bits and stops at actual typed
terminals. ToIRSelectPort retains its specific exception as the scalar selector
of a two-input ScanMux (§6.4.6.23(d)). PDL target eligibility is unchanged.

The check runs after flat origin/type restoration and combinational-cycle checks.
Thus an internal clock port serialized as a flat DataOutPort regains its original
kind before validation. Generated TAP route/control connections still replay;
there is no exemption based on a generated name or the presence of metadata.
This is terminal-kind eligibility under §6.3.12(d), not a complete local-versus-
child direction check. Contextual DataMux/control-source rules and special
AddressPort/WriteEnPort/ReadEnPort binding rules remain separate requirements.

The validator classifies node kinds once and memoizes referenced alias bits with
an iterative walk. With GCC16.2.1 on the local host,4096 scalar aliases in83873
ICL bytes take0.069–0.079s over three validated CLI runs (baseline0.072–0.076s),
with maximum56408KiB RSS. A65536-bit alias and4096 consumers of256 distinct bits
in177434 bytes take0.060–0.070s (baseline0.067–0.078s), with maximum61864KiB RSS.
Exact commands, input sizes and per-run results are in
build/source-kind-audit/comparison.json. These measurements do not establish a
speedup or a bound for every legal input.

Native kind/alias/restored-type checks, main35 and multiple-TAP22 CLI methods,
formatting/naming and independent source review pass. Existing fixture wiring
that treated scan/clock signals as ordinary data now uses the corresponding port
kind. The native graph-cycle test retains its assertion by explicitly constructing a
lowered cycle after legal elaboration. Update-controlled data-register fixtures
use the specified UE/zero DataMux and ToUpdateEnPort combination, preserving the
original update/restoration assertions and CLI round trips.

Full integration passes in build/data-source-confirmed-full.log/.exit0, with
907.574s wall time. The initially selected leak-enabled checks and corrected
scheduler/client/control-reservation checks all pass in
build/data-source-final-sanitizer-* and build/data-source-corrected-sanitizer-*.
The first full failure is retained in build/data-source-final-full.log/.exit2:
reservation fixtures exposed address/read enables through invalid data outputs.
Their corrected same-kind child-port fixtures retain distinct ownership identities.
A separate40-suite optimized sweep passed39 suites and found the direct-UE client
fixture; that failed aggregate remains exit1. Its corrected18-method suite and
the final complete dynamic/static regression pass. The validator itself did not
change during fixture corrections. Strict coverage, vendor certification and
Rocky8 qualification have not been refreshed for this change.

### Relative system-clock waits (2026-11-14)

`iClock` checks one ClockPort/ToClockPort bit against the applied routing state.
`iRunLoop count -sck port` then emits the corresponding root ClockPort wait,
rounding up after cumulative multiplier/divider scaling. For example, a clock
running at3/10 of its root needs7 root cycles for2 local cycles, or10 root cycles
for3 local cycles. Generated PDL names the root, so replay does not scale twice.

Supported paths use non-inverted forwarding/aliases, positive fixed frequency
factors and ClockMux entries evaluated in declaration order. Selectors must be
known and covered. Targets are scalar or explicitly indexed port bits. A wait
requires a prior successful iClock on that target, including zero-cycle waits;
queued reads/writes must be applied first. iClock itself does not flush the queue.
After applied-state changes, a changed effective root or reduced ratio requires
another iClock. A different known route with the same effective tuple is accepted;
this establishes duration equivalence, not phase continuity.

These relative waits preserve modeled storage, TAP state and pin levels. Roots
with possible non-clock fanout are rejected, including unsafe unselected branches,
inverted paths and differential-clock uses. Known held clock levels are checked
only within the pulsed root's possible fanout; unrelated stopped or unknown clocks
do not block the wait. Potentially related inactive/unknown held routes remain
conservatively rejected. No physical root period or terminal clock level is invented.
Oscillators, differential paths, overrides, absolute time, tester-period normalization
and general clock-driven data/control events remain required follow-up work.

The route walk is iterative and caches effective tuples until hardware state
changes; it does not retain a full path for each target. Fanout analysis visits
connections once with ordered-map lookup costs. Ratios and emitted cycle counts
are checked uint64 values; cross-cancellation and unsigned128 arithmetic avoid
intermediate rounding/wrap when scaling the count. Individual ICL factors use
the existing positive signed64 integer domain; a cumulative intermediate ratio
outside uint64 is rejected even if a later factor could cancel it.

Seventeen CLI methods cover five replay forms, prefixes/procedures, wildcard mux
order, changed/equivalent routes, zero/missing checks, pending operations, invalid
factors/overflow, fanout rejection and atomic artifact preservation. Native tests
cover held root/forwarded/other bits, unrelated inactive/unknown clocks and unchanged
state. Independent review found the unrelated-held-clock rejection; a native RED
case now passes, together with the existing scheduler suite. Decimal frequency
serialization fixes an initial failing flat replay. Existing main35 passes.

With local GCC16.2.1,4096 forwarding instances (219051 ICL bytes),1024 checks/waits
and five replay forms take0.982595s. A65536-bit root with256 forwarded bits,
1024 checks/waits and five forms takes0.233540s. The complete17-method run takes
3.586s (3.63s wall,94096KiB maximum RSS); /usr/bin/time evidence is in
build/system-clock-legal-fixtures.log. These are
measured inputs, not a general performance guarantee. Leak-enabled native
scheduler/flatten/netlist and main35/clock17/client18/reset12 checks pass in
build/system-clock-final-sanitizer-*. The final hierarchical clock fixtures also
pass separately in build/system-clock-legal-sanitizer.log/.exit0. Positive clock
fixtures use child ToClockPort sources under §6.3.12(e), including the long chain;
initial flat-style local-output fixtures were corrected before either clock-suite
invocation in the confirmed full run. Full integration passes in
build/system-clock-confirmed-full.log/.exit0 (993.511s):41 dynamic/static suites,
51 directional/seven legacy goldens,2523 ICL334/119 rules and editor6 in179.043s;
five optional ANTLR tests skipped.

The first full run failed its coverage-source manifest check because the new clock
source was missing. Both the source and inline-header implementation are now listed;
the seven manifest checks pass. No coverage threshold or exclusion was relaxed.

Tessent2026.2 accepts both the original3/10-frequency source and the normalized
root-relative PDL with a configured100ns root period. Its two exports and our
output have identical runtime traces: iClock root, then7 and10 root cycles.
Exact submitted files/hashes, fetched logs and comparison commands/results are
retained in build/system-clock-vendor-probe. The remote run is under the authorized
lianghuaxiao /tmp/icl-system-clocks-20261114 directory. This is a small direct
probe, not a new stored directional certificate or general clock equivalence.
Strict coverage and Rocky8 qualification have not been refreshed.

### System-clock override snapshots (2026-11-14)

`iClockOverride` changes one scalar/indexed ToClockPort's source and positive
frequency factors. Each omitted field returns to its original ICL default.
Previously checked clocks retain their duration snapshot until another `iClock`;
applied mux changes still revalidate that saved version. Current reservations
independently follow the latest routing. Introduced cycles are rejected before
publishing the override, including cycles through unselected ClockMux branches.

Normalized output contains root checks/waits and omits resolved overrides.
Reset-only output retains declarations and procedure text; reset and procedure
return preserve override/check history. A formerly unconnected ToClockPort may
receive an explicit source. Its flat declaration now uses the valid semicolon
form, sharing the existing empty DataRegister serialization rule. Oscillator
and differential targets, unsafe non-clock fanout, absolute-time waits and
general clock events remain outside this increment.

Thirty clock CLI methods pass through original/flat/generated/reset-only forms.
Native tests cover current reservations versus old durations, moved roots,
failed-cycle rollback and unchanged hardware. Eight initial override methods
fail on the prior unsupported command. Independent source review is clean.
The bounded single-version route cache completes4096 forwarding instances
(219071 ICL bytes) and1024 waits alternating old/current versions in4.32s;
the corresponding leak-enabled run takes21.4063s. The declared workload target
remains45s, with no general speedup claim. Native scheduler/flatten/netlist and
main35/clock30/client18/reset12 leak-enabled ASan/UBSan checks pass in
build/clock-override-final-sanitizer-*. Full integration passes in
build/clock-override-final-full.log/.exit0 (1050.603s):41 dynamic/static suites,
51 directional/seven legacy goldens,2523 ICL334/119 rules, editor6 in182.629s;
five optional ANTLR tests skipped.

A Tessent2026.2 probe changes both the source and factors before testing omitted
fields. Its exports restore the ICL defaults and match our wait sequence:
root10, other35, root10, root25. The source and normalized vendor exports have
identical commands; the vendor removes redundant iClock commands from our text.
Exact input hashes, fetched evidence and comparison.json are retained under
build/clock-override-vendor-probe/changed-defaults. The earlier partial-factor
probe also matches10/10/5 root waits. A separate no-option override is rejected
by Tessent as missing an option, while the IEEE optional-field interpretation
here restores all ICL defaults. The v2023.4 manual describes different omitted
source/factor behavior; it must not be substituted for these measured2026.2
results. These small probes are not formal directional certificates.
Strict coverage and Rocky8 qualification have not been refreshed.

### Embedded oscillator relative waits (2026-11-14)

A ToClockPort with a positive Period now supplies a relative clock root, including
child instances and indexed vector bits. Bare periods mean ns; attached/separated
s,ms,us,ns,ps,fs,as forms retain their units. Magnitudes use the existing positive
signed64 integer domain and serialize as decimal plus an explicit unit. The shared
frontend continues to reject Period combined with Source, frequency factors or
DifferentialInvOf. No unit conversion or physical period is inferred for external
clock inputs.

Positive descendants scale against the oscillator root once, and generated PDL
retains its hierarchical name. Equal periods do not merge root identities.
Overrides may reference an oscillator source; direct oscillator-target overrides
remain unsupported in this increment. Existing fanout/reservation/snapshot checks
continue to apply. Waits preserve pin, TAP and storage state. Absolute-time waits
and general clock events remain open.

Thirty-seven CLI methods and native source/flat held-bit, distinct-root and
unchanged-state tests pass. Four positive methods have19 failing subcases against
the previous unsupported path. The confirmed CLI run takes9.991s (10.04s wall,
94764KiB maximum RSS). Its4096-stage oscillator chain (219107 ICL bytes),1024 waits
and five replay forms take1.033372s; a65536-bit oscillator (55 ICL bytes),1024 waits
and five forms take0.231470s. The existing external-root cases remain measured in
the same run. These are workload measurements, not a general performance bound.
Independent source review, formatting and naming pass. Full integration passes
in build/oscillator-final-full.log/.exit0 (999.946s):41 dynamic/static suites,
51 directional/seven legacy goldens,2523 ICL334/119 rules, editor6 in176.843s;
five optional ANTLR tests skipped. Leak-enabled native scheduler/flatten/netlist
and main35/clock37/client18/reset12 checks all pass in
build/oscillator-final-sanitizer-*. The first CLI attempt overlapped an executable
relink and failed to launch it; that failed log is retained separately.

Tessent2026.2 accepts the child oscillator3/10-ratio probe. Both vendor exports
and our normalized output have identical command traces: iClock child.osc, then
7 and10 cycles on child.osc. Submitted input hashes match fetched copies;
commands, logs and comparison.json are under build/oscillator-vendor-probe,
with the remote run in lianghuaxiao /tmp/icl-oscillator-clocks-20261114. This
is a direct probe, not a formal directional certificate. Strict coverage and
Rocky8 qualification have not been refreshed.

### Explicit oscillator source overrides (2026-11-14)

An iClockOverride on a Period ToClockPort may supply an explicit replacement
ClockPort/ToClockPort source and positive frequency factors. Previously checked
versions retain their intrinsic oscillator duration; a new iClock follows the
replacement source. Other vector bits remain independent roots. Each omitted
factor returns to the ICL default, which is1 for a Period target. Original period
metadata remains immutable, and current reservations follow current routing.

Period targets require explicit -source on every invocation in this increment.
Omitted-source behavior was unresolved at this stage; see the later
[measured audit](#oscillator-omitted-source-audit-2026-11-17). No intrinsic
self-reference or frequency multiplication is invented. Differential targets and
absolute-time waits remain open.

Three positive methods fail against the prior unsupported guard. All41 clock
methods pass in11.277s (11.33s wall,94896KiB maximum RSS), together with native
source/flat snapshot, held-bit, rollback and unchanged-state cases. Existing4096
stages/1024 alternating-version waits take6.66197s during concurrent verification.
Naming and independent review pass. Full integration passes in
build/oscillator-override-final-full.log/.exit0 (1086.789s):41 dynamic/static
suites,51 directional/seven legacy goldens,2523 ICL334/119 rules, editor6 in181.938s;
five optional ANTLR tests skipped. Leak-enabled native scheduler/flatten/netlist
and main35/clock41/client18/reset12 checks pass in
build/oscillator-override-final-sanitizer-*. Tessent2026.2 accepts the probe: our normalized command trace exactly
matches its export, retaining two child.osc2 waits then a root1 wait after the new
check. Exact submitted hashes match fetched files; comparison.json and commands
are under build/oscillator-override-vendor-probe, with the remote run under
lianghuaxiao /tmp/icl-oscillator-overrides-20261114. No formal certificate, strict
coverage or platform refresh is claimed.

### Clock source terminal eligibility (2026-11-14)

ClockPort/ToClockPort input Sources, ClockMux choices and DifferentialInvOf
connections now require eligible positive clock terminals, ordinary data signals
or constants. The ordinary-data alternative is explicit in concat_clock_signal;
a legal data-driven clock remains distinct from an operable SystemClocks route.
Aliases are classified per bit with the existing iterative memoizer. Negative
differential ports cannot serve as positive clock terminals. This validates terminal
kinds, leaving hierarchy direction and contextual control/DataMux rules open.

DifferentialInvOf now has width-checked typed references before hierarchy
restoration. Internal differential ClockPorts retain their clock declaration in
flat ICL; explicit parent bindings use an optional ninth origin field because
ClockPort syntax cannot carry Source. Ordinary eight-field metadata remains
unchanged. Named/constant/partial bindings roundtrip, including OneHotDataGroup
sources. Invalid widths, contradictory unconnected metadata and PDL-only alias
consumers remain rejected. Alias-consumer validation runs once after restoration
so restored edges receive the same checks. Reset-reference restrictions are unchanged.

Native invalid-kind/direct/alias matrices and positive source/flat cases pass,
as do38 main CLI and41 clock methods, formatting/naming and independent review.
Review identified bound-input loss and metadata validation gaps; their corrections
have explicit roundtrip or rejection cases. The timed native suite takes0.64s,
126456KiB maximum RSS;4096 clock aliases take0.0177807s and a65536-bit source with
4096 consumers takes0.0314348s. These are measured workloads, not speedup claims.
Exact failed and corrected runs remain in build/clock-source-*. Full integration
passes in build/clock-source-final-full.log/.exit0 (1103.136s):41 dynamic/static
suites,51 directional/seven legacy goldens,2523 ICL334/119 rules, editor6 in181.921s;
five optional ANTLR tests skipped. Leak-enabled native scheduler/flatten/netlist
and main38/clock41/client18/reset12 checks pass in build/clock-source-final-sanitizer-*.
Older generators requiring eight-field entries reject the new bound differential
input metadata; ordinary eight-field output remains unchanged. Strict coverage and platform qualification
have not been refreshed.

### Child TCK input source kinds (2026-11-14)

Child TCKPort bindings now require TCKPort/ToTCKPort terminals, ordinary data,
constants or their aliases. General ClockPort/ToClockPort/ClockMux terminals
remain valid ordinary clock sources but are outside this TCK binding union.
Kind eligibility does not establish same polarity, phase or hierarchy direction.
Explicit ToTCKPort Source is an existing lowered-model extension; it remains
available without claiming its Source syntax is IEEE-defined.

Main39, native positive/negative/alias/flat/constant checks and clock41 pass.
Independent source review and naming/style pass. Two client and one native gate
fixtures previously bound ClockMux directly to TCKPort; they now use explicit
ToTCKPort Source forwarding, retaining all disabled/enabled/unknown assertions.
Their comments identify extension coverage. The first full run and scheduler
sanitizer failure remain recorded in build/tck-source-final-*; the corrected
full native scheduler passes. Confirmed full integration passes in
build/tck-source-confirmed-full.log/.exit0 (964.274s):41 dynamic/static suites,
51 directional/seven legacy goldens,2523 ICL334/119 rules, editor6 in182.718s;
five optional ANTLR tests skipped. Leak-enabled native scheduler/flatten/netlist
and main39/clock41/client18/reset12 checks pass in
build/tck-source-confirmed-sanitizer-*. The corrected client18 suite passes.
Strict coverage, vendor certification and platform qualification are not refreshed.

### Reset source terminal eligibility (2026-11-14)

ResetPort input bindings and ToResetPort Sources now require reset-family/data/
constant terminals and aliases. Ordinary-data terminals now require a direct
DataMux driver, with aliases transparent; see the direct-driver section below.
Inferred global reset remains eligible. Existing TRST-driven reset acceptance is preserved: the generic source list and specific
ResetPort prose differ, while Tessent2026.2 accepts both direct TRST-to-ResetPort
and ToResetPort connections in the recorded loading probe. That probe exercises
only a clock wait, not functional reset equivalence. Exact input hashes and fetched
comparison evidence are under build/reset-source-vendor-probe.

Kind validation leaves DataMux forcing/selection, hierarchy direction, polarity
and broader reset events as separate requirements. Register reset associations
retain their existing path. Main40, native source/flat/inferred-global, clock/client
and reset-preview checks and source review pass. A native edge-snapshot fixture
formerly encoded an invalid direct TCK reset source; it now elaborates legal ICL
then constructs the synthetic lowered event circuit explicitly. Rising/falling
snapshot/priority assertions pass without a frontend exception. First full and
scheduler sanitizer failures remain under build/reset-source-final-*. Confirmed
full integration passes in build/reset-source-confirmed-full.log/.exit0 (955.735s):
41 dynamic/static suites,51 directional/seven legacy goldens,2523 ICL334/119 rules,
editor6 in182.967s; five optional ANTLR tests skipped. Leak-enabled native
scheduler/flatten/netlist and main40/clock41/client18/reset12 checks pass in
build/reset-source-confirmed-sanitizer-*. Strict coverage, formal
vendor certification and platform qualification are not refreshed.

### Scan controls and TMS/TRST source eligibility (2026-11-14)

CaptureEn, ShiftEn and UpdateEn input bindings and To* Sources accept their own
family, ordinary data/constants and aliases. TMS/TRST follow the same family rule;
TRST also accepts the inferred asynchronous global-reset function. TMS/TRST
ordinary-data Sources additionally require a direct DataMux driver. Unrelated
control/clock/scan terminals and mismatched virtual reset functions are rejected.
Generated TAP scan controls remain ordinary data and require no exemption.

Omitted Reset/TRST/TMS bindings require one physical source bit (including indexed
scalars), repeated across grouped destinations with polarity preserved. A vector
input is multiple candidates, so implicit ordinal matching is ambiguous. Explicit
vectors retain declaration-order mapping. Scalar register reset inference now
rejects ambiguous vector sources. Preflight caps new inferred control Source
buffers at4M bits; larger real models require a compact representation and renewed
qualification. The compact65×65536 rejection takes0.000262s;65536-bit scalar
broadcast/source-flat replay takes0.298485s on the recorded native run.

Tessent2026.2 rejects vector-input inference with ICL123/124 and accepts unique
scalar-source loading/analysis for grouped destinations. The source-only clock
export agrees with ours; our output was not submitted in that run. This is no
functional control-value equivalence claim. Exact inputs/hashes/logs are under
build/inferred-control-vector-vendor-probe. The abandoned ordinal-inference trial
and first fixture failures remain recorded. Contextual forcing/gating, direction,
phase/polarity and broader event behavior remain separate acceptance work.

Full combined integration passes in build/control-source-final-full.log/.exit0,
1094.986s wall:41 dynamic/static suites,51 directional/seven legacy goldens,
2523 ICL334/119 rules, editor6 in185.044s; five optional ANTLR tests skipped.
Leak-enabled native scheduler/flatten/netlist and main43/clock41/client18/reset12
checks pass in build/control-source-final-sanitizer-*. Final source/test hashes
match the reviewed Task59 checkpoint. Naming/style and independent source/resource
reviews pass. Strict coverage, formal vendor certification and platform refresh
remain open.

### Scan source eligibility and alias-hidden register taps (2026-11-14)

Child ScanInPort bindings, ScanOutPort Sources and ScanMux choices now require
scan terminals, ordinary data/constants or their bit-specific aliases. ScanRegister
middle bits remain ordinary data in concat-scan contexts. ScanMux selectors retain
their data/ToIRSelect rules. OneHotScanGroup retains its stronger enabled-fan-in
validation. Scan terminals do not become eligible in ordinary-data consumers.

Bare ScanRegister ScanInSource instead requires a scan terminal, a rightmost
ScanRegister bit or a constant. Ordinary-data terminals do not qualify there.
Nested/inverted/sliced aliases cannot hide a middle-register tap in ScanInSource
or ScanOutPort Source. Rightmost means the declared right endpoint, including
ascending declarations. Omitted direct register indices retain the IEEE permission
in §6.4.8(u). Legitimate mux/port terminals remain boundaries; the new check does
not recursively apply downstream rules to their drivers. No generated TAP bypass
or fixture exemption was needed. Hierarchy direction and contextual mux/control
requirements remain separate acceptance work.

Tessent2026.2 loading-only probes accept constants and explicit/implicit rightmost
indices in both directions; reject non-scan data terminals and alias-hidden middle
taps with source-specific diagnostics. Seven positive and12 negative cases have
matched fetched input hashes under build/scan-terminal-qualified-vendor-probe.
The earlier SI/SO-only interface batch failed ICL101 setup and supplies no source
eligibility conclusion. No PDL, analysis or functional comparison ran in the
qualified batch. Main46/native/flat/state/TAP/client/scheduler and source review
pass;4096 aliases take0.0109507s,65536-bit/4096 consumers0.0230771s in the recorded
native run (0.80s wall,120336KiB maximum RSS). No speedup is claimed.

Full combined integration passes in build/scan-sources-final-full.log/.exit0,
1121.542s wall:41 dynamic/static suites,51 directional/seven legacy goldens,
2523 ICL334/119 rules, editor6 in198.149s; five optional ANTLR tests skipped.
Leak-enabled native scheduler/flatten/netlist/state/TAP and main46/clock41/client18/
reset12 checks pass in build/scan-sources-final-sanitizer-*. Final three source/test
hashes match the reviewed Task61 checkpoint. Naming/style and independent source
reviews pass. Strict coverage, formal vendor certification and platform refresh
remain open.

### Address, write-enable and read-enable source kinds (2026-11-14)

AddressPort, WriteEnPort and ReadEnPort bindings now require their matching parent
family, DataInPort/DataOutPort terminals or ScanRegister bits, including aliases.
Other data primitives, constants, cross-family ports and unrelated controls are
rejected. Both rightmost and intermediate register bits remain valid here. Data
ports retain their ordinary-data eligibility in existing consumers and remain
invalid bare ScanInSource terminals. Unconnected `?` binding holes have a distinct
cached classification; they remain available for contextual domain diagnostics.

The specific addressed-port clauses preserve matching parent inheritance despite
the generic instance-rule data list. Native tests retain explicit parent mappings,
AddressValue inheritance, complete logical bus order and top-level DataInPort
domains. AddressResolver checks contextual constraints when resolving addressed
operations; complete elaboration-wide direction and no-intervening-logic validation
remain open. Its logic-rejection/cache-retry unit now elaborates legal source then
constructs the lowered graph explicitly, retaining both original assertions.

Thirty-five matched-input Tessent2026.2 loading probes record17 accepted controls,
15 source-specific ICL65 rejections and three enabled-DataOut contextual ICL81
rejections. Separate unenabled DataOut forwarding cases load successfully. They
establish loading compatibility, not inherited-offset or read-enable sequencing
correctness. Evidence: build/address-port-source-{vendor, boundary-vendor,
forwarding-vendor}-probe/comparison.json (actual directories use the complete
address-port-source prefix). Original test-only constant Alias syntax failures
remain recorded; corrected literal bindings reproduce all60 invalid acceptances.

Main47 passes9.984s; native netlist/address/flat/scheduler, address CLI20 and scan-data
CLI25 pass. Netlist native0.79s/120704KiB RSS;4096 clock aliases0.00985495s and
65536-bit/4096 consumers0.0184732s. Native address0.68s/117048KiB RSS. These are
measured checks, not a speedup claim or new performance mechanism.

Full integration passes in build/address-port-source-final-full.log/.exit0,
1094.305s wall:41 dynamic/static suites,51 directional/seven legacy goldens,
2523 ICL334/119 rules, editor6 in180.025s; five optional ANTLR tests skipped.
Leak-enabled native scheduler/flatten/netlist/state/TAP/address and main47/clock41/
client18/reset12 pass in build/address-port-source-final-sanitizer-*. Four final
source/test hashes match the reviewed Task62 checkpoint. Naming/style and
independent source/fixture review pass. Strict coverage, formal vendor certification
and platform refresh remain open.

### DataMux impossible source kinds (2026-11-14)

DataMux choice inputs now reject ShiftEn, clock/TCK, scan-only, addressed-port and
ToIRSelect terminals, including aliases. The permitted superset retains ordinary
data/constants and conditional Reset/CaptureEn/UpdateEn/TMS/TRST families and To*
ports. SelectedBy remains ordinary data. This excludes impossible terminals; it
does not establish matching-output context, mux structure, forcing values or
normal-operation sensitization. Those remain required follow-up work.

Main48, native ordinary/conditional alias and flat cases, state/TAP/client/clock
checks and independent review pass. Virtual-reset metadata negatives exercise
SelectedBy, not unrepresented choice metadata. Two clock CLI cases now fail at
the frontend; lowered native cases retain both direct and unselected-branch
fanout guards. A further multi-TAP fixture uses ClockMux plus the existing explicit
ToTCK Source extension, retaining1/0/X/inverted and replay assertions.

The335-byte scale model has65536-bit data/alias/mux and10 choices (655360 source
references). Three interleaved baseline/changed source+flat runs have equal output
hashes. Source validation takes0.1369–0.1667s versus0.0595–0.1210s before this check;
changed flat replay0.1191–0.1489s, peak67344KiB RSS. Exact commands/environment and
hashes are in build/data-mux-source-union-scale/comparison.json. Native suite0.76s/
120656KiB,4096 aliases0.0101215s and65536-bit/4096 consumers0.0182843s. These
measure overhead; no speedup or new optimization mechanism is claimed.

Loading-only context experiments are under build/data-mux-context-vendor-probe,
build/data-mux-context-qualified-vendor-probe and build/data-mux-update-write-vendor-probe.
They distinguish ICL65 consumer restrictions, ICL115 capture structure, ICL64
polarity and ICL129 reset-state failures. Corrected cases accept aliases/chains,
To* outputs, TRST-to-reset and unused/partly unused mux outputs. Tessent2026.2
rejects the existing UE-to-ToUpdate plus DataRegister WriteEn fixture with ICL65,
although the inspected IEEE generic DataMux data-signal list does not explicitly
impose that gating exclusion. Keep the standard-first mixed-fanout interpretation
separate from vendor compatibility; the matching-use plan records the distinction.
No PDL or operational equivalence ran in these probes.

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

### Conditional DataMux matching control use (2026-11-14)

Observed DataMux bits carrying Reset/CaptureEn/UpdateEn/TMS/TRST inputs now require
matching control use. Per-bit masks propagate through aliases and DataMux data
choices; selectors, Enable, ordinary ports, logic, storage and groups do not grant
control context. Reset associations count as Reset/TRST uses. Unused bits and
dangling chains remain accepted. A matching use still permits existing mixed data
fanout under the documented standard-first interpretation; Tessent2026.2 rejects
some such fanout. Port-specific structure, forcing/polarity and normal-operation
sensitization remain separate required work.

The pass runs only for special-control mux inputs. Dense masks use at most16MiB
under the existing16M graph-bit limit; a deduplicated uint32 worklist is bounded
by alias/mux bits and monotone family masks. Three interleaved baseline/changed
source+flat measurements preserve output hashes:262144-bit control case changed
0.4373–0.4849s versus0.4029–0.4434s, peak126616KiB RSS;4096 aliases changed
0.0645–0.1259s versus0.0689–0.1230s. Exact evidence is in
build/data-mux-control-use-scale/comparison.json. Aggregate mux expansion and
selector dependency work still need their separate resource check.

Full integration passes in build/data-mux-control-use-final-full.log/.exit0,
1228.810s wall: dynamic/static suites,51 directional/seven legacy goldens,
2523 ICL334/119 rules and editor6 in281.764s. Five optional ANTLR tests remain
skipped. Leak-enabled native scheduler/flatten/netlist/state/TAP/address and
main49/clock41/client18/reset12 pass in build/data-mux-control-use-final-sanitizer-*.
All three source/test SHA-256 values match the independently reviewed checkpoint
and measured scale source. Naming/style and independent review pass. Strict
coverage, full vendor certification and refreshed platform checks remain open.

### Aggregate mux expansion and selector graph work (2026-11-14)

DataMux, ScanMux and ClockMux now share a4M expanded-bit limit per final netlist:
selector references once, every choice source bit and every selector-code bit,
including alternative codes. Checks precede padded-source and code allocation.
Selectors resolve against the remaining budget. String-valued selector codes are
rejected during elaboration instead of failing only when debug ICL is serialized.
The final graph check also counts generated TAP routing muxes; a source network
at the exact combined boundary retains identical flat replay. Oversized networks
previously accepted now fail with a source-located limit diagnostic, preserving
existing pattern/reset/debug artifacts. Object/width/real-graph-bit limits remain.

Wide muxes share one private selector-dependency vertex. Output data dependencies
stay bit-specific, registers still cut cycles, and selectors cannot qualify an
enabled data path. This removes output-width times selector-width traversal.
Three interleaved source/flat runs against9e602dd preserve output and flat hashes.
For65536 outputs/4096 selector bits (169 ICL bytes), source changed0.1144–0.1146s
versus8.9460–8.9966s; changed source/flat peak52728KiB. Ordinary655360-reference
and262144-bit control cases retain similar measured ranges. Exact commands,
compiler/platform and hashes are in build/mux-resource-scale/comparison.json.
These workload measurements do not establish a worst-case runtime guarantee.

The shared signal resolver also rejects target widths beyond its operand budget
before unsized padding. This prevents one-hot budget subtraction from underflowing;
width4/max4 remains accepted, width4/max3 is a failing-first regression.

Verification exposed a primary-input optimization hotspot. Symbolic references now
reuse contiguous vector slices with validated index order, inversion, knownness and
held/shift selection. Primary deviation objectives use unsigned32-bit chunks MSB
first, preserving their lexicographic order around drive-count minimization. All
goals, reservations, replay and deadlines remain unchanged. Slice-only, chunk-only
and unsigned-integer alternatives were measured; both final changes are necessary
for the tested4096-bit boundary. Native concrete equivalence and a262-bit reduction
preference check protect semantics. No diagnostic setting/output enters the product.

Full final integration passes in build/mux-resource-primary-confirmed-full.log/
.exit0,1641.867s wall with make-j1check: dynamic/static generator suites,51 directional
goldens,2523 ICL334/119 rules, LSP62 and editor6 in216.032s. Five optional ANTLR
tests are skipped. Leak-enabled native symbolic/netlist/flatten/state/TAP/address/
scheduler and main50/clock41/client18/reset12 pass in
build/mux-resource-primary-final-sanitizer-*. All8 source/test hashes match the
reviewed checkpoint. Naming/style and independent source/test review pass.

The full optimized scheduler measures1024 direct bits0.0232s and4096 bits0.3365s.
Official ASan total Apply times are0.320s/4.750s; diagnostic check-only4096 times
0.425–0.834s stay below the unchanged2s check deadline. Source/command/environment/
RSS evidence is in build/primary-combined-scale-comparison.json and
build/mux-resource-primary-final-optimized-comparison.json. The selector comparison
above measures its earlier reviewed checkpoint; the shared sizing and primary
fixes were integrated afterward. Initial solver timeouts and diagnostic failures
remain recorded, with no assertion/deadline/coverage waiver. Strict coverage, full
vendor certification and refreshed platform qualification remain open.

### CaptureEn/UpdateEn DataMux input structure (2026-11-14)

DataMux bits with actual CaptureEn/UpdateEn provenance now admit only one distinct
corresponding control input and literal zero. Aliases compose inversion before
source comparison; duplicate rows referring to the same canonical control remain
valid. Each upstream mux and each scalar vector bit is checked independently,
including unused actual-control gates. Identity-only control muxes remain valid.
The existing canonical source cache is reused, with one byte of provenance per
real graph bit and bounded expanded mux references. No serialized format changes.

This follows the standard-first interpretation of semantic inputs. Tessent2026.2
loading accepts repeated zero rows, multiple selector alternatives in one control
row and per-bit two-input vector shapes, but rejects duplicate control rows and
ordinary-data-only mux outputs used as these controls. Those differences remain
explicit: ordinary-data mux behavior is preserved here. Output inversion/polarity,
normal-operation sensitization and the remaining reset/TMS/TRST rules are still
open; loading probes do not establish operational conformance.

Three interleaved baseline/source/flat measurements preserve output and flat
hashes. The262144-bit CE case takes0.617–0.767s versus0.617–1.219s at3c9fd30,
with peak135032KiB versus126500KiB. The65536-bit UE,4096-alias and ordinary
655360-reference cases are recorded in build/capture-update-mux-scale/comparison.json,
including commands, input/binary hashes, environment and memory. These are
workload measurements under concurrent late editor checks, not a speedup claim.
The final binary is byte-identical to the measured binary after a braces-only
naming correction. Independent source review and focused checks pass.

Full integration passes in build/capture-update-mux-final-full.log/.exit0,
1774.796s wall with make-j1check: dynamic/static generator suites,51 directional
goldens,2523 ICL334/119 rules, LSP62 and editor6 in255.589s. Five optional ANTLR
tests are skipped. Native7 leak-enabled ASan/UBSan and main51/clock41/client18/
reset12 pass. All four final source/test hashes match the reviewed checkpoint.
Strict coverage, full vendor certification and refreshed platform checks remain open.

### Direct data drivers of reset/TMS/TRST (2026-11-14)

Ordinary-data bits bound to ResetPort/TMSPort/TRSTPort inputs or used in the
corresponding To* Sources must be directly driven by DataMux. Aliases preserve
canonical driver identity and inversion; ordinary ports, logic, storage and groups
remain boundaries. Check every bit, including mixed vectors. Typed forwarding,
TRST-to-reset compatibility, literals, virtual resets and binding holes retain
their existing eligibility. Previously accepted bare data drivers now fail with a
source-located diagnostic before replacing pattern/reset/debug artifacts.

This enforces the direct-driver part of IEEE1687 clauses6.4.6.13/.14/.15/.16/.21/.22.
It does not establish mux input shape, hierarchy direction, forcing/polarity,
normal-operation sensitization or TRST-assertion selection. Those remain required
work. In standalone models without any actual or inferred TRST function, retaining
a locally asserted selector after global reset is distinct from TRST assertion.
The global branch must still be selected for downstream normal operations. A
source-less ToResetPort alone does not create a TRST/power-up function.

Connected frontend fixtures now use real global/reset mux paths and preserve
release ordering, common updates, reservations and source/flat/generated replay.
Three deliberately arbitrary feedback/history cases moved to explicit native
lowered-netlist checks, retaining reset values, CaptureSource and failed-Apply
storage rollback. Their former source/flat/generated and CLI artifact coverage is
not claimed for that migration. Other CLI artifact checks remain enabled.

Tessent2026.2 loading rejects18 direct/aliased/logic source cases with ICL65. Typed
and mux controls load. Three initial constant-input models lacked top ports;
corrected inputs load in all three families, preserving literal eligibility.
Submitted/fetched model and runner hashes match in
build/direct-control-driver-{vendor,constant-vendor}-probe. No PDL or operational
conformance claim follows from these probes.

### Known binary mux values and reset solver work (2026-11-14)

When a one-bit selector is provably known and exactly two single-code rows0/1
cover it, symbolic evaluation selects the matching row directly. Either row order
works, branch widths must match the declared mux, and identical knowledge masks
are reused. Unknown selectors and overlapping, alternative, incomplete or wider
code sets keep the existing consensus behavior. Enable and held/shift handling
remain shared. There is no public API, metadata, scheduling-policy or limit change.

This avoids redundant unknownness terms in cyclic reset snapshots. Diagnostic
physical32 checks retain65 passes and34 objectives while reducing assertions
4228/4227 to2148/2147; measured checks change from1629ms SAT/2005ms unknown
to73ms/141ms SAT. Final three interleaved repetitions cover32 registers/64 storage bits and32
forcing muxes with implicit, physical and reversed-row roots. All27 changed
source/flat/generated runs pass; successful baseline output/flat hashes match.
Changed source0.617–0.767s and flat0.566–0.668s leave the existing2s check and15s
scheduling limits intact. Baseline failures remain recorded, not treated as passes.

Variable-selector proofs,243 ternary/inverted/reversed vector combinations,
malformed lowered widths and512/4096-stage data/selector chains protect the change.
Final4096 selector construction0.026–0.031s/check0.003–0.004s, data construction
0.132–0.139s/check0.473–0.942s; peak138856KiB. Old selector4096 did not complete
within a60s process-group bound, leaving no completed measurement or peak-RSS claim. These are
component/workload measurements, not a worst-case complexity or optimality claim.

Direct-driver loading overhead is separately measured with empty PDL:262144 reset
bits (290 ICL bytes) changed0.566–1.269s versus0.516–1.067s, peak134528KiB versus
134780KiB;4096 aliases (83940 bytes) changed0.114–0.165s; ordinary655360 references
changed0.165–0.166s. Source/flat hashes match. Exact inputs, source/binary hashes,
commands, environment, wall and RSS are in build/direct-control-driver-
{known-mux-confirmed-scale,deep-mux-confirmed-scale,loading-scale,scan-diagnostic-confirmed}.
Independent source/fixture review, naming/style and focused proofs pass. All7
native and main52/clock41/client18/reset12 leak-enabled ASan/UBSan checks pass in
build/direct-control-driver-final-sanitizer-*.

Full serial make-j1check passes in build/direct-control-driver-final-full.log/
.exit0,1808.878s wall:41 dynamic and41 static generator suites,51 directional
goldens,2523 ICL334/119 rules, LSP62 and editor6 in220.236s. Five optional ANTLR
tests are skipped. All29 final source/test hashes match the reviewed checkpoint.
Strict coverage, full vendor certification and refreshed platform checks remain open.

### Reset/TMS/TRST DataMux input shape (2026-11-14)

Each mux bit directly supplying these six input/To* Source contexts requires a
corresponding control path and at most two canonical data-input identities.
Aliases preserve inversion; equivalent inverted literals share identity. Repeated
selector rows are not extra input wires. Required control-bearing mux branches
are checked independently. An ordinary data mux used only to compute a forcing
value and an unused reset/TMS/TRST mux remain outside these port-source checks.
Reset destinations retain the qualified TRST-family compatibility. No public API,
serialized format or solver/scheduling limit changes.

Identity gates, canonical duplicate rows and two matching controls retain the
explicit semantic compatibility interpretation. The primary text's two-input
rule rejects a third distinct source, even though Tessent2026.2 loads it. Of72
matched-hash loading cases,30 load,12 missing-control/constant-only cases fail
ICL65 and30 duplicate/special-selection cases fail ICL115. Single-row identities,
control chains and ordinary-data forcing chains load. These restrictions differ
from the standard-first input-identity rule; no PDL or operation was qualified.
Evidence: build/reset-mux-structure-vendor-probe/comparison.json.

Three interleaved repetitions cover five workloads and source/flat replay: all60
runs pass with matching pattern and flat hashes against e156f88. For262144 reset
bits (290 ICL bytes), source runs with debug output take0.667–0.868s versus0.566–0.617s;
source/flat peak137084KiB versus134836KiB. The4096 shared mux/consumer model
(392920 bytes) takes0.265–0.366s source and0.215–0.266s flat;4096 aliases and655360
ordinary references are recorded separately. Native512/4096-stage source/flat
identity checks take0.035/0.361s. Exact inputs, binaries/source hashes, commands,
platform/compiler, wall/RSS and repetitions are in
build/reset-mux-structure-loading-scale/comparison.json. Empty PDL measures loading;
these are supported-scale observations, not worst-case or optimality claims.

Local connected native7 and main53/clock41/client18/reset12/multiple-TAP22/feedback10,
style/naming and independent review pass. A brace-only style correction produces
byte-identical CLI/native/object files. Root native7 and main53/clock41/client18/
reset12 leak-enabled ASan/UBSan pass. Final serial make-j1check passes in
build/reset-mux-structure-final-full.log/.exit0,1618.418s:41 dynamic and41 static
generator suites,51 directional goldens,2523 ICL334/119 rules, LSP62 and editor6
in211.728s. Optional ANTLR5 skipped; all3 source/test hashes match. Early Rocky8 native7/dynamic41 results belong to the retained pre-scale/pre-format
snapshot. Its individual checks and completed archive pass despite SSH transport
exit124; this is not final platform/install/static qualification.
Hierarchy direction, forcing/polarity, normal-operation/TRST selection, strict
coverage and remaining model/runtime/vendor/platform requirements stay open.

### Effective typed-control Source inversion (2026-11-14)

Direct typed TCK/TMS/TRST/CaptureEn/ShiftEn/UpdateEn Sources now require zero
effective inversion, including aliases and child To* output references. Every
bit is checked. Ordinary-data expressions retain their previous eligibility;
Reset/Scan inversion, TCK used as a clock_signal, literal sources and inferred
active-low virtual TRST remain unchanged. Explicit ToTCK Source remains the
existing lowered-model extension. Mux/output polarity, hierarchy direction and
operational sensitization remain separate requirements.

This follows IEEE1687 6.3.12's typed-control productions and6.4.7's child binding
rules. It is effective inversion validation, not full literal grammar enforcement:
even alias cancellation remains a documented semantic compatibility interpretation.
The generic typed-alias permission and narrower6.5.2(e) list remain distinct.
Tessent2026.2 accepts most typed inversions despite that grammar, while inverted
TRST fails polarity checks. Both cancellation forms load. All64 submitted/fetched
model and runner hashes match;51 loading cases pass and13 fail ICL64/65. No PDL or
operational equivalence is inferred. Evidence:
build/typed-control-inversion-vendor-probe/comparison.json.

Native direct/child-output, hidden alias, cancellation and mixed-vector checks
plus44 CLI negatives preserve pattern/reset/debug files. Existing standalone idle
and multi-TAP inverted-phase cases retain backend coverage through the established
ToTCK/ClockMux extension, with their original assertions and artifact checks.
All48 interleaved source/flat loading runs match baseline14236ce pattern and flat
hashes:65536 TCK bits (146 bytes), six8192-bit control families (585 bytes),4096
cancelling aliases (88008 bytes) and655360 ordinary references (335 bytes).
Wide TCK runs take0.064–0.115s, peak53008KiB versus53080KiB; other measured ranges,
commands, environment and source/binary hashes are retained in
build/typed-control-inversion-loading-scale/comparison.json. Empty PDL and debug
output measure whole loading runs, not guard-only time or an optimization claim.

Local connected native7/main54/clock41/client18/reset12/multiple-TAP22/feedback10,
style/naming and independent review pass. Required leak-enabled native7 and
main54/clock41/client18/reset12 pass. Final serial make-j1check passes in
build/typed-control-inversion-final-full.log/.exit0,1381.252s:41 dynamic and41 static
generator suites,51 directional goldens,2523 ICL334/119 rules, LSP62 and editor6
in173.740s. Optional ANTLR5 skipped; all5 reviewed source/test hashes match.
Rocky8 native7/dynamic41 individual results and archive also pass despite SSH
transport124; production/Python sources match, native-test formatting differs.
Full vendor, strict coverage, remaining models/runtime and final platform/install/
static qualification remain open.

### Explicit reset-port polarity (2026-11-15)

An explicitly connected ResetPort with omitted ActivePolarity inherits its typed
ResetPort/ToResetPort/TRSTPort/ToTRSTPort driver's effective polarity, including
alias inversion, under IEEE1687 6.4.6.13(a,b). All dictating bits in a grouped port
must agree. Data/mux/literal/unbound bits do not dictate a separate default; when
no typed driver dictates it, the default remains high. ToResetPort keeps its
independent high default; its explicit typed Source must match. Implied connections
continue automatic polarity conversion. Flat replay preserves and validates the
resolved flags, including fixed-low internal TRST metadata.

Native hierarchy/vector/alias/implicit/flat checks and main56 exercise inherited-
low reset/write/read, source/flat/generated replay and pattern/reset/debug atomic
rejection. The TRST metadata review fix has RED/GREEN evidence. Connected native7
and selected CLI suites, format/naming, independent review and all required
leak-enabled native7/main56/clock41/client18/reset12 pass.48 interleaved loading
runs preserve baseline525f1b6 output/flat hashes:65536 reset bits (156 bytes),4096
aliases (88004 bytes),4096 child scopes (99383 bytes),655360 ordinary references
(335 bytes). Wide source0.065–0.115s, peak54128KiB versus52916KiB. Commands,
environment/compiler, hashes, repetitions and RSS are retained under
build/reset-polarity-loading-scale. These whole empty-PDL CLI/debug measurements
do not establish isolated guard cost or a speedup.

Tessent2026.2 keeps omitted explicit-binding polarity high, differing from the
primary inheritance exception.82 matched loading cases yield37accepted/45rejected
(31ICL64 and14 existing grammar/property checks); no PDL/operation certificate.
Evidence: build/reset-polarity-vendor-probe/comparison.json. The manual archive
fetch succeeds; original transport124 and wrapper parse failure remain explicit.
Current-source Rocky8 build/native/main56 pass with matched hashes and transport0.

Final serial make-j1check passes in build/reset-polarity-final-full.log/.exit0,
1339.731s,41 dynamic/static suites,51 directional goldens,2523 ICL334/119 rules,
LSP62 and editor6 in181.382s. Optional ANTLR5 skipped; all3 production/test hashes
match the reviewed snapshot. Mux-output polarity/forcing and reset operation,
strict coverage, full vendor, remaining models/runtime and refreshed all-tools/
platform/static/install qualification remain mandatory.

### Possible Reset/TRST mux path polarity (2026-11-15)

A ResetPort/ToResetPort driven by a DataMux needs a possible typed Reset/TRST path
with matching effective polarity. TRSTPort/ToTRSTPort need a possible typed TRST-
low path; an ordinary Reset forcing input cannot substitute. Alias, input-edge
and output inversion compose, including nested double cancellation. Two typed
control alternatives may provide different possible polarities under the retained
forcing interpretation. This check does not establish associated-reset ownership,
selector reachability, normal-operation selection or TRST assertion behavior.
A mux does not dictate an omitted ResetPort default; a direct typed bit elsewhere
in the same grouped port can still dictate its single polarity.

The bounded origins array now uses uint16_t, adding at most16MiB at the existing
16M-bit bound. It keeps separate Reset/TRST polarity pairs, family masks and an
independent unknown sentinel. The implementation adds no graph traversal or public
metadata; prior shape diagnostics retain precedence. Native truth tables and
main58 preserve source/flat positives and pattern/reset/debug atomic rejection.
Connected native7/main58/clock41/client18/reset12/multiple-TAP22/feedback10, final
child native rerun, style/naming and independent combined review pass. All72 valid
loading comparisons preserve output/flat hashes versus5714c7c. Full/leak integration
checks now pass.

88 matched vendor loading probes yield36loaded/36ICL64 polarity failures and16
source-context/special-input restrictions (ICL65/130/115). The legal child-ToTRST
follow-up distinguishes hierarchy direction from parity. No PDL/operation proof
is claimed. Evidence is under build/reset-mux-polarity-vendor-probe,
build/trst-mux-polarity-vendor-probe and build/trst-mux-child-vendor-probe.

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

### Possible capture/update/TMS identity paths (2026-11-15)

A consumed CaptureEn/UpdateEn/TMS DataMux with corresponding typed ancestry needs
a possible non-inverted path to that typed control after alias and mux inversions.
This is a necessary transparent-normal-path interpretation of the relevant enable
and TMS sensitization rules. Nested cancellation is retained; raw inverted inputs
are not banned. A compensated path can invert a zero forcing branch to one, so
passing this check does not establish zero suppression or selector operation.
All-data enable scope, unused muxes and opaque ports/storage/logic remain unchanged.

The existing uint16_t origins mask holds three additional pairs with unknown32768;
there is no allocation-width increase or new graph traversal. CE/UE-only networks
reach the shared Source check without receiving Reset/TMS/TRST shape rules.
Native parity/alias/producer/nested/vector/shared tests and main59 preserve flat
replay and atomic pattern/reset/debug rejection. Canonical inverse-arm fixtures
balance output parity while retaining every prior row and alias lookup order;
their suppression behavior remains outside this identity assertion.

All72 matched Tessent2026.2 loading probes pass, including odd paths, differing
from the derived standard-first normal-path condition. No vendor PDL equivalence
is claimed. Remote72 interleaved loading comparisons preserve output/flat hashes:
three65536-bit control families0.418–0.468s, CE-only262144 bits0.569–0.620s at
103732KiB both,4096 shared gates0.268–0.319s,4096 aliases0.118s,ordinary655360
references0.168s. Whole empty-PDL CLI/debug runs are measured, not isolated guard
cost or speedup. Exact environment/compiler, commands, hashes/time/RSS remain in
the retained remote scale archive. Final local connected/scale/style checks and independent current code/test
review pass; required full/leak integration now passes.

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

Queued ordinary accesses can now prepare modeled child client routes before
raw replay in either source order. Immutable client metadata and operands are
checked when queued; exact active child geometry/contiguity is checked after
preparation. Existing disjoint child batches, local stable proofs and parent/
surrounding reservation masks remain enforced. Default strict native normalization
stays strict. Nested preparations that use symbolic fallback now invalidate the
old native route cache before following raw accesses. A child with no ordinary
preparation still requires an already accessible route. The strict vendor forward
certificate passed; Tessent2026.2 rejected reverse source at its documented
internal-ScanInterface limitation, including within a leaf-scoped procedure.
No reverse certificate is claimed. See the mixed-child preparation plan for
actual scale, source/call/owner/cancellation and validation evidence.

Verified Task107: ordinary preparation now supports qualified modeled child raw clients in either source order and disjoint batches. Existing immutable client/pin checks factored into RawClientPorts; deferred mode checks root/child metadata and operands, exact current route/length remains strict at Apply. Default strict native normalization unchanged; bool parameter rename preserves signature. Removed root-only source guard without introducing a solver/evaluator/clock/reset engine. Child-only raw authored footprint preserves surrounding/parent SIB protection, exact ownership refresh, owner return and local stable/overlap proofs. Initial RED native/5CLI14failures; first GREEN exposed stale canceled-geometry expectations at plain/default-chain plus inherited native root-only expectation, replaced with positive canceled cases and actual Apply/type rejection. Dedicated cache RED reproduced successful nested goals with stale warm Root1 versus cold Root10. Public ordinary Apply now invalidates dynamic route/segment caches after success as well as before; failed native attempt's valid pre-state cache remains on failure. New native hot/cold nested Root10/stable child9 and assignment/failure/retry/all-endpoint tests pass. Final and necessary delta reviews no unresolved Critical/Important/Minor; runtime evidence declined by reviewer and verified by executor. GREEN2 all15 fresh statuses0 and actual192 inputs verified; native scheduler0/main69/modeled77 in48.263s remote and34.027s local. Local20 connected0 (scheduler/modeled source-identical standalone checks reused), style/tidy0, corpus2557/zero duplicates/26 reviewed repeated-module groups0. Local32/remote32 matched-option scale commands0, complete candidate source/flat/generated hashes equal saved Task106 explicit-Apply references:1M route .25s remote,1024 child interfaces/512 sharedaliases .73-.74s,128 reopen/close .04s,1Mpartial32 held bits .52-.53s; memory/timing/commands and artifacts retained. Genuine Tessent2026.2 FORWARD run/strict record/check0, all source/jobinputs/artifacts/console hashes verified, local replay0,7 schema3 snapshot files imported. REVERSE vendor1 because child.local is internal ScanInterface unsupported in tested context unless inferred active for ISTG; leaf-scoped procedure also rejected. Original console retained, no reverse certificate/trace waiver. All68offline0. Frozen22 leak-enabled SAN/native7/main69/modeled77/resources/data/state/clock/reset/trace/68offline all0, fullserial make check0 (elapsed build/mixed-child-raw-final-full.wall). Final remote54 all0, actual199 hashes match;14 source-matched functional groups reused with192 input verification/explicit registry+README exclusions, fresh offline covers registry. Commit22, fast-forward master and delete completed branch; preserve raw TAP IR work/recovery artifacts. No push. Pure inaccessible-child execution without ordinary preparation, opaque/IR/TAP/parallel/merge and broader reset/clock/ownership/strict coverage/release acceptance remain open.

### Raw IR on a bound root TAP

`iScan -ir jtag 4 -si 3 -so 0bxx01; iApply` performs one instruction
CSU on a direct scalar root client-TAP backed by one BSDL TAP. An unambiguous
inferred interface is named `unnamed`; `-chain default` is accepted. SI must be
known binary after numeric normalization. SO is an observer mask, independent of
instruction selection. `-stable` applies to fixed IR length. Ordinary queued
actions execute before the raw IR transaction, even when queued later. Reset
cancels queued scans; overwritten/canceled width checks defer to Apply.

The scheduler reuses its ordinary instruction planner on a copy and adopts it
only after proving one exact IR transaction, with the requested SI/SO, physical
endpoints, and no additional assignments. This preserves subsequent data access,
partial instruction fill, reservations and generated/flat/reset replay. Unknown
SI, raw DR, multi-TAP, parallel/child-TAP raw scans and raw merge remain open.
Measured supported examples cover 4/32/256-bit IRs, 1024 modeled registers, and
128 repetitions. A 4096-bit IR reaches the inherited scan-event resource guard
in both the previous ordinary planner and this raw path; no budgets are raised.

Verified Task108: known-binary iScan -ir on a direct scalar root client-TAP bound to one BSDL TAP, explicit or inferred unnamed. Shared numeric/default-chain parsing retains leading-ir canonical PDL; source queue offsets account for IR target/payload. Reuses ordinary IR Apply on a copied scheduler, adopts full state and publishes assigned IR only after exact one-CSU/endpoints/serial/mask/load-write-read proof. Unknown SI, raw DR, multi-TAP/parallel/child-TAP and raw merge stay unsupported. Native literal LSB-first/final-exit/N+6-cycle/all-endpoint/idle/state/transaction checks, CLI7 source/flat/generated/reset/owner/partial/queue/cancel/call/scope groups and trace17 pass. Intended native/CLI RED precedes implementation; separate trace numeric/default-chain RED observed. Initial single-lane preflight assertion and private-field compile issue corrected; no unresolved product failure. Independent final review no Critical/Important/Minor; pending runtime gates declined and verified by executor. Format/tidy0. Full serial make check0 in1435.091s, editor6 in195.861s, corpus2558/zero duplicate files/26 reviewed repeated-module groups; full checkpoint included68 directional certificates. New registry and15 genuine snapshot files then checked freshly:70 offline locally and leak-enabled SAN, oracle/golden/roundtrip harness checks all0. Original22 SAN gates all0; initial additional offline attempt under ptrace failed for LeakSanitizer environment, approved untraced rerun passes. Final remote60 statuses0, actual3903 input hashes match;53 functional/performance checks reused from all0 GREEN56 with3888 exact unchanged runtime/test/benchmark inputs and explicit registry exclusion, new certificates/registry fresh. Remote extra harness checks initially lacked formatter/inventory executables; archived failures retained, built prerequisites and reran affected checks0 without source edits. Genuine Tessent2026.2 BOTH directions run/strict record/check0; all source/job-input/artifact/console hashes verified before importing15 schema3 files. Local and remote32 matched-option scale commands each:30 successes with complete source/flat/generated byte identity and canonical whole-IR ordinary equivalence,2 expected inherited4096 scan-event rejections, no guard increase. Remote4bit .05s,32bit .09-.11s,256bit2.68-2.71s,1024nodes .54s,128repeats3.32s; copy/proof overhead measured, no speedup or1M IR promise. Original frozen17 inputs unchanged throughout full/SAN/remote checks; later changes only documentation and separately verified certificates/registry. Commit34, fast-forward master and delete completed branch, retain recoverable worktree/backups/stashes. No push. Pure inaccessible child, opaque child, raw DR/TAP/parallel/multi-TAP/merge, broader reset/clock/ownership, strict coverage and final release acceptance remain open.

### Raw data on a bound root TAP

After selecting an instrument instruction, `iScan jtag 8 -si 0x96 -so 0;
iApply` supports a complete known modeled register path on one direct scalar root
client-TAP. Ordinary preparation may select the instruction before the raw CSU
in either source order. SI must be known binary. The same copied ordinary
planner and exact-one-CSU proof used for raw IR checks the data transaction.
The selected instruction and all unrelated state retain the existing hardware
semantics. `-stable` requires a known complete post-route of unchanged total length. Raw IR and data kinds retain separate queue keys and reject mixed raw
groups explicitly. Unknown selection/SI, multi-TAP, parallel and child-TAP raw scans remain unsupported.

Verified Task109: known-binary raw DR on one active full instrument ScanRegister on one bound scalar root TAP. Shared qualification/normalization/private TAP request and exact-one-CSU transaction proof with raw IR; read-only ActiveDataRegisters reuses hardware selection. IR/data queue keys are separate; ordinary preparation runs first in either source order. Builtin DRs remain explicitly unsupported because ordinary synthetic PDL targets are forbidden; next task addresses them through an internal-only exact-ID allowance. Native/CLI intended RED and independent trace spelling RED precede implementation. Literal TMS100/LSB-first/final-exit/N+5 clocks, all endpoints/state/idle and assigned/failure/retry checks pass. Fixture capture was made explicit after an uncontrolled-capture test mismatch. Independent review found one Important stable post-route disconnect; new native rollback and CLI constant/alternate/inverted post-input regressions RED then GREEN. Existing ActivePath now optionally requires bound TDI and compatible parity, plus exact post register/width; default callers unchanged. No new walker/FSM/solver. Final native/DR8/IR7/trace17/style checks pass. Fresh23 leak-enabled SAN gates all0; full serial make check0 in1664.5059401610051 seconds on original frozen21 inputs. Local64 and corrected remote64 matched-option DR/IR scale commands each60 successes and4 expected inherited4096 resource rejections; complete source/flat/generated bytes and canonical prior ordinary outputs match. Remote DR8 .07s,32 .08s,2561.24s,1024nodes1.00s,128repeats3.26s; no speedup or1M TAP promise. Initial remote57 statuses0 retained; after review fix, fresh GREEN2 all58 statuses0 and actual3910 inputs match. Genuine vendor initial reverse0, forward strict mismatch because vendor adds initial IR capture0001. Strengthened only the source fixture with explicit iRead tap.INSTRUCTION1; original frozen source remained immutable through full completion. Recertified BOTH Tessent2026.2 directions run/strict record/check0 with all source/job-input/artifact/console hashes verified,15 schema3 files imported. Fixture-only amendment qualified by fresh optimized/static/SAN DR8, all72 SAN/optimized offline snapshots and harness tests0; original snapshot and amendment manifest retained. Final remote61 statuses0 and3925 actual hashes match;54 source-matched functional/performance checks reused with explicit registry/fixture exclusions, affected DR8/new metadata fresh. Commit37, fast-forward master and delete completed branch, preserve3 builtin-task drafts/recovery artifacts. No push. Pure inaccessible/opaque child, builtin/multiple-register/multi-TAP/parallel/rawmerge, broader reset/clock/ownership, strict coverage and final release acceptance remain open.

### Raw builtin TAP data registers

A reset-selected IDCODE32 or selected BYPASS1 may be scanned through the root
client-TAP using the same raw data syntax, including default-chain and stable
checks. The qualified request grants the private ordinary planner access to only
that one internal register for the duration of the call. Public synthetic TAP
`iWrite`/`iRead` targets remain forbidden before and after raw scans and copies.
Internal proof accesses are retained only for verification and emitted as raw
`iScan`; no synthetic target names appear in the generated PDL. The existing
exact-one-CSU proof rejects instruction retargeting or additional data accesses.
BYPASS/IDCODE pin timing and capture are independently exercised; broader private
BSDL capture/register associations still need separate qualification.

Verified Task110: raw IDCODE/BYPASS reuse the existing one-data-CSU planner and proof on one bound scalar root TAP. A private call-local optional exact register ID flows through Apply/Solve/FilterTapData; normal public calls stay nullopt, public synthetic iWrite/iRead remain forbidden and no permission is stored/copied. Internal proof accesses never appear in generated PDL. No new solver/walker/FSM/public API. Native/CLI RED reaches prior unsupported builtin boundary. GREEN native independent IDCODE32/BYPASS1 captures, LSB-first/N+5 clocks/all endpoints/idle/owned IR and assigned/copy/failure checks0; CLI4, DR8 and IR7 all0. Independent review found no production defect; Important test gap addressed with valid-width failed privileged apply (incompatible protected IR), reserved-context proof, all-state/assigned rollback, original/copied BOTH-node public rejection and successful retry. Existing feature RED retained; no fabricated behavior fix. Style0. Local and remote88 benchmark commands each84 successes/4 expected inherited4096 guard rejections, source/flat/generated bytes agree. Builtin24 candidate-only measurements explicitly lack a prior comparable CLI baseline because prior CLI rejects them; no speedup claim. Remote IDCODE32 .09s/BYPASS1 .08s,1024model nodes .57/1.01s,128repeats6.37/3.37s; existing IR/DR matched-option ordinary equivalence retained. Full serial make check0 in1669.6238344970043 seconds on original frozen17, fresh24 leak-enabled SAN gates0 and GREEN remote60 statuses0 with actual3932 inputs verified. Initial vendor reverse strict0, forward strict1 only because Tessent drops -stable flags; original diff/archive retained. Vendor-only PDL fixture omits flags, while native/CLI/bench inline sources still test stable. BOTH Tessent2026.2 portable fixture directions run/strict record/check0, source/job-input/artifact/console hashes verified before15 schema3 files imported. Original freeze immutable through full; fixture-only amendment qualified freshly on optimized/static/SAN builtin4, all74 offline snapshots/harness checks0. Final remote63 statuses0, actual3947 hashes match;56 exact source-matched functional/performance groups reused with explicit registry/fixture exclusions and affected builtin4/metadata fresh. Commit33/ff master/delete branch, preserve2 multi-register design drafts/recovery artifacts; no push. Multi-register/parallel/multi-TAP/opaque child/rawmerge and broader clock/reset/ownership/strict coverage/final release acceptance remain open.

### Ordered full TAP data paths

A raw root TAP data packet may span several unequal registers and internal serial
aliases/inversion. The packet is printed MSB-left: its prefix belongs toward TDI
and its suffix toward TDO. A direct a3/b5 chain loaded with SI96 stores a4/b22.
SI and SO parity are handled separately; generated whole-register goals and
expected masks must prove exactly the requested physical stream and one data CSU.

The shared concrete traversal preserves ordinary replay and implicit-selection
behavior. Raw qualification also checks enables, known selection, complete
registers and the bound physical input. Assigned ownership follows the pre-CSU
visible path; hardware replay accounts for side effects on other registers
sharing its controls. Stable permits representable parity changes with unchanged
total length, and rejects disconnected or alternate-input post-routes.

The measured scale includes64 one-bit registers and16 sixteen-bit registers,
1024 unrelated registers,512 serial aliases and128 repeated packets. The
combined64 four-bit-register geometry remains an inherited performance limit:
the prior ordinary planner exceeded120 seconds and the candidate probe exceeded
15 seconds. These timeouts are not successful budget rejections. Larger4096-bit
paths retain the existing explicit event-work guard; budgets are unchanged.

Verified Task111: full known ordered raw TAP data paths span unequal modeled registers and serial aliases/inversion. One shared TraceSerialPath preserves ordinary Replay enable behavior while raw qualification requires known enables, complete register boundaries and bound TDI. Per-register SI uses total XOR prefix parity; SO uses prefix parity with X masks preserved. One copied ordinary Apply must prove exact single data CSU/maps/endpoints/serial streams before full state adoption; assigned set remains pre-CSU path. Stable permits parity changes at unchanged complete length and rejects disconnected/alternate input. Builtin exact-ID privilege and public synthetic restrictions unchanged. Native/CLI intended RED precedes implementation; independent literal16 parity/index cases and dynamic length/protected-side-effect/failure/retry/assigned tests pass, CLI4 including16 parity subcases, DR8/builtin4/IR7. Independent review no Critical/Important/Minor; runtime judgments verified by executor. Format/tidy0. Original frozen16 unchanged through full serial make check0 in1501.9609203200089s/editor6 in170.949s; corpus2561, zero duplicate-file groups,26 reviewed repeated-module groups, five optional ANTLR tests skipped. Fresh25 leak-enabled SAN gates0. Local/remote132 benchmark commands each126 successes and6 expected4096 event-work guard rejections, exact source/flat/generated bytes and equivalent ordinary canonical outputs agree. Remote64x1 .85s/16x16=256bits4.06s/1024unrelated1.03s/128repeats4.4s/512aliases1.4s; no budget increase. Combined64x4 ordinary reference exceeded120s and candidate probe15s; original failures retained and explicit inherited fragmented-wide limit stays open for next native TAP optimization task. Initial vendor reverse strict0, forward run0/record1 only retained a4/b22 observations in partial-write group; trace diff preserved. Fixture explicitly retains those reads; comparator/runtime unchanged. BOTH Tessent2026.2 directions recertified run/strict record/check0, all source/job-input/artifact/console hashes verified before15 schema3 files imported. Fixture-only amendment after immutable full qualified freshly on dynamic/static/SAN CLI4, all76 offline certificates (optimized/SAN) and oracle23/golden17/roundtrip7 harness tests0. Final remote65 statuses0,3969 actual hashes match;58 source-matched functional/performance checks reused with explicit registry/fixture exclusions and affected CLI/metadata fresh. Original16 and qualified32 snapshots/recovery evidence retained. Integrate into master and delete completed task branch, preserving only next performance design/plan; no push. Pure inaccessible/opaque child, parallel/multi-TAP/rawmerge, broader event/reset/clock/ownership, strict coverage and final release acceptance remain open.

### Native replay of selected TAP data

An already selected instrument data path with known full load values can use a
concrete candidate before symbolic event planning. The scheduler reuses its
ordered path and TAP cycle replay, checks canonical phase/clock forwarding and
proves selection from held IR independently of varying pins/storage. Captured
masks, actual updates and every reserved edge are checked on a complete copy.
Unsupported controls, reset/fill histories or observations return to joint
planning. Builtin/IR privileges, raw exact-one-CSU checks and work budgets stay.
An authored SO comparison differing from modeled capture retains existing
symbolic generation behavior; it is not rejected as a cached-state assertion.

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

Verified Task113: a reset-closed modeled child now receives every required parent CSU before its final physical raw packet. Existing local RawRoute and combined prefix-only TryGreedy planning are reused; strict native defaults and existing SI/X rules remain. Preparation plus final geometry/overlap/stable/ownership/replay proof runs on a complete copy and publishes state, controlling fill, assigned IDs and prefix outputs only together. Every prefix is rendered through existing output flow; no target dummy CSU, new engine/dependency/RTL or raised guard. True native/source RED preceded implementation. Scale RED exposed unnecessary general reset/event work; qualified immutable inactive resets and existing native-first ordinary execution resolve it without changing default planner callers. Independent final review found no Critical/Important/Minor issues. Native literal original/flat checks, output/state rollback and retry, ownership, nested/disjoint paths,16 parity/index/alias subcases and source/flat/generated/reset replays pass; automatic CLI11, modeled77, opaque13 and style checks pass. All88 local and88 remote matched-option benchmark commands succeed with exact output/artifact hashes; saved reference fingerprints match Task112. Automatic source1M physical route1.17-1.19s local/1.90s remote,1024 parents with512 aliases .77-.78s/1.17-1.18s,97 levels .33-.34s/.62-.63s,reopen128 .37-.40s/.79-.87s; peak source216536/211988KiB. Original16-file Root freeze stayed byte-identical through full serial make check0 in1538.4046997200348s/editor6 in175.756s,27 fresh leak-enabled SAN gates0,remote62 gates0 and3976 actual input hashes. Parser334/334 and scanner119/119 rules reached; five optional ANTLR integration tests skipped. Initial Tessent2026.2 reverse rejects internal ScanInterface; forward executes0 but adds a parent reset-value comparison, so strict record fails and its trace remains unqualified. A unique unknown-capture parent reusing the existing leaf is separately certified by genuine forward run/strict record/check0, with all source/job-input/artifact/console hashes and measured binary fingerprint verified before8 schema3 files are imported. Reverse still rejects the internal interface. This data-only amendment receives fresh corpus2562/zero duplicate-file groups/26 reviewed repeated-module groups,77 optimized and SAN offline certificates, focused static certificate and all oracle/golden/roundtrip harness checks0; final remote6 gates0 and3985 actual inputs match. Original and qualified snapshots/recovery evidence are retained. Integrate into master and delete the completed task branch; no push. Known self-capture vendor comparison differences, bounded select-only/unknown-parent preparation, opaque/parallel/multi-TAP/rawmerge, broader events/reset/clock/ownership, strict coverage and final release acceptance remain open.

### Literal unknown input on qualified raw TAP paths

A bound scalar single-root TAP may shift literal X input on qualified IR,
modeled full DR and existing builtin paths. The emitted packet retains X, and
actual three-valued hardware replay receives X. Only known addressed SI bits
refresh controlling fill; a later partial ordinary write retains earlier fill
for X positions. Established protected X assignments remain errors.

The ordinary planner validates target privileges, access groups, end-state
requirements and one exact compatible CSU on a disposable copy. Its binary
choices are discarded. Existing TAP cycle replay executes the literal packet
from the original execution copy, with per-edge preservation checks and actual
final-load/end-state/stable validation before publication. All-binary behavior
is unchanged. SO remains an authored comparison, including values differing
from cached capture. No new FSM, solver, walker, dependency or raised guard.

This profile requires canonical phase/clock forwarding, held-IR-only selection
and inactive immutable resets. Unsupported gated/control/reset histories fail
explicitly. Unknown IR values retain the existing instruction-alternative and
event-work limits; a large unknown opcode mask is not a supported-scale claim.

Verified Task114: qualified scalar single-root raw TAP IR, complete modeled DR
and existing builtin paths replay literal unknown SI. The ordinary single-CSU
proof is disposable; shared ReplayTapStep/Cycle alone supplies adopted literal
hardware/execution state. Known SI positions alone refresh controlling fill;
protected X assignments reject, with per-edge reservations and actual final
load/end-state/stable checks before state and both caller outputs publish.
Existing binary dispatch, permissions, source-family validation and event/IR
bounds remain; no new engine/dependency/RTL or raised limit. True source2/native
runtime RED preceded implementation. Original/flat literal endpoint and clock
checks, post-replay stable rollback/retry, ownership/fill/end-state/builtin cases,
11 source tests including16 parity/index/alias subcases and32-bit compact IR
masks pass;4096 event-work rejection remains. A proposed serial-selector review
finding was withdrawn after actual elaboration rejected both source-family
variants and Read(false) was verified to use held update storage. Corrected
review has no Critical/Important/Minor findings; no speculative guard was added.
Format/tidy/build/native and all7 local qualification gates pass. Both platforms
52 artifact-verified measurements pass (32 literal-X,20 known binary; no completing
old X baseline or speedup claim). Literal64x4 .50-.55s local/.85s remote,512aliases
.79-.80s/1.47s,1024 unrelated .30-.31s/.55s,IR4repeat16 .53s/.94s;128 wide literal
packets21.49-22.15s/52.53-52.67s, so no15s whole-program claim. Peak literal
261724/261860KiB. Existing known-native128 packets10.67-10.71s/25.97-26.07s.
Original15-file freeze remains byte-identical through full serial make check0
in1564.5019128859858s/editor6 in174.330s and28 fresh leak-enabled SAN gates0. Five optional ANTLR integration tests were skipped. Existing77
directional certificates pass. Initial remote63 statuses include two faulty test
expectations (compact IR mask and8-bit benchmark value);60 unchanged passing
gates are reused with exactly2 script exclusions, correctedCLI/benchmark/verify/
aggregate4 gates freshly pass and all3991 current input hashes match. Original
failure evidence is retained. Fresh Tessent2026.2 IR/DR forward/reverse runs all
exit1 explicitly rejecting SI containing X; source/job-input/console hashes,
version and absence of results are verified. No new certificate or registry
entry is created, and no Tessent equivalence is claimed for literal-X export.
Only verified documentation changes follow the original freeze. Integrate into
master/delete the finished branch; no push. Opaque/parallel/multi-TAP/rawmerge,
noncanonical X profiles, broader event/reset/clock/ownership, strict coverage
and final release acceptance remain open.

### Native ordinary TAP instructions

Pure known instruction-register goals can use the existing concrete TAP replay
path before joint event planning. The current serial tracer proves the complete
IR path and bound input. All data-interface phase/clock forwarding is checked;
data SelectPorts may be inactive because DR phases remain inactive during IR.
Partial writes use existing controlling fill, and unknown fill or unsupported
controls/reset/groups retain joint planning. Data-mode qualification is unchanged.

The shared N+6 cycle replay checks every reserved edge and actual final IR value
on a complete copy. IR controller synchronization does not emit register update
events, so the native path uses actual controller replay rather than inventing
an event. Actual captured comparison mismatch returns to the existing symbolic
planner, preserving authored comparison behavior. No FSM, walker, solver, public
API, dependency or work-limit change is introduced.

Fresh IR-only64x4/512alias baseline used252-262 thousandKiB RSS; the first two
native profiles use48-51 thousandKiB with identical saved-reference/source/flat/
generated bytes. Broader measurements and full qualification are recorded below
when complete; this is not a1M IR or whole-program optimality claim.

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


## Secondary clock input bindings (2026-11-18)

Task129 accepts a named secondary ClockPort from the parent or secondary
ToClockPort from a direct child as the Source of a negative ClockPort input.
The exception requires that destination, its DifferentialInvOf association,
the Source role, and a secondary clock terminal after alias resolution.
Existing scope, direction, alias ownership and restored flat-origin checks
still apply. Ordinary positive inputs, ClockMux choices, ToClockPort Sources,
data outputs and DifferentialInvOf references retain their previous rules.
No new command, global clock classification, dependency or event model is added.

This is a measured Tessent2026.2 extension to the literal clock_signal lists in
IEEE1687-2014 6.3.12(e)/6.4.7(h). Source eligibility does not prove a waveform.
The current relative-duration backend checks a traced negative input against
its declared positive partner using the existing root, ratio and phase-origin
proof. Partial unbound lanes, unrelated routes and wrong polarity remain errors
when used; duration waits do not simulate clock events.

The maintained source-kind regression failed before the change. Native tests
now preserve direct/aliased/child-output Source identity and original input
kind through flattening. The clock CLI suite has63 methods, including nested
and permuted vectors, partial bindings, source/scope rejection and phase
mismatches, each valid case using source/flat/generated/reset replay. Existing
rejection tests for other source contexts remain unchanged.

Fresh Tessent2026.2 runs distinguish three directions for direct, aliased and
shared-divider models. All three original source programs and all three native
normalized programs elaborate, execute and export successfully. Their six
complete ordered iClock/iRunLoop traces match the native result; the divider
maps four child cycles to twelve root cycles. All twelve native replays of
those exports against original and flattened models pass. All27 submitted
probe-input hashes match their fetched copies.

All three flattened models instead fail Tessent read_icl with ICL59-1: the
negative ClockPort refers to a positive partner serialized as DataOutPort.
Native flat-origin restoration handles that representation, but it is not a
vendor-portable model. Separately, the negative-target control elaborates and
then fails at iClock; the positive-target control succeeds. IEEE7.9.15(a)
permits any valid ClockPort/ToClockPort target, so native negative-target
support is retained. These results do not establish vendor support for those
commands or the earlier inverted-positive input spelling. The existing formal
certificate runner lacks this clock setup; no registry certificate was added.
Evidence is retained under build/secondary-clock-bindings-20261118/.


Qualification for Task129 passes the six changed-C++ format/tidy/strict17 gates,
whole native source-kind checks,63 clock methods per dynamic/static variant,
and leak-enabled ASan/UBSan native netlist/clock-snapshot,63 clock and71 main CLI
checks. The3994 runtime input hashes remain frozen; only the shared netlist
source and its two maintained test files differ from Task128.

Full serial normal integration passes in1833.155s:
1916 regression cases,2180 conformance cases,334 parser/119 scanner rules across
2549 ICL files, and six editor tests in198.376s. Five optional ANTLR checks are
skipped. Fresh instrumented testing takes1304.924s;
all58 Python groups/955 cases pass, including51 generator groups/793 cases.
The unchanged strict gate still fails: 17376/18005 lines(96.5%),
29350/46789 raw branches(62.7%). All54 authored reports and19 instrumented
products remain included; no exception-edge filtering or threshold waiver.

The additional named-secondary scale fixture has4096 pairs,4096 shared mux
stages,605109 input bytes,1024 waits and five replay forms.
It completes in1.551698s dynamic/1.534427s static, peak RSS125452/126072KiB.
The parent rejects these named inputs, so there is no completing baseline or
speedup ratio. Existing4096-stage/65536-bit clock fixtures and the parent/current
5000-instance netlist benchmark pass; commands, environment and wall/RSS logs
are retained. No general throughput or clock-event claim is made.

All seven current Rocky Linux compiler/native/dynamic/static/ELF gates pass;
both variants run51 generator groups/793 cases and all3994 actual input hashes
match. All14 remote and12 local archive/install gates pass for five commands
per variant, LSP behavior, static ELF checks and uninstall sentinel preservation.
Only the two generator binaries change locally and remotely; the other eight
products remain identical to Task128. The clean41ee126 archive
ships4229 tracked files plus six generated/prebuilt files;
its local dynamic build runs with Bison/Flex disabled. All4013 non-documentation files in the archive
are byte-identical to the current remote source, allowing reuse of that current compile
and suite evidence. This is not a fresh remote archive rebuild. Later doc-only
archive refreshes preserve the qualified runtime bytes and original provenance.

Independent Task129 whole-branch review at986a1db approves the scoped merge with no Critical/Important findings. The reviewer selected dynamic63/static63/native-netlist checks, all passing through a disclosed read-only Root proxy because its tool host was unavailable. Broad artifact reproduction remains executor-owned; the recorded verifier checks hashes, counts and statuses. One optional wording minor is deferred:4013 denotes non-documentation files, although the guide currently calls them bytes. No runtime fix pass or second review was needed. Broader release and vendor acceptance remain open.


## Typed flat clock declarations (2026-11-18)

Task130 serializes internal ClockPort inputs as ToClockPort outputs. Positive
forwarding declarations keep Source and negative declarations keep only
DifferentialInvOf. Root clock ports and other port families retain their previous
declaration kinds. This corrects the earlier association from a negative
ClockPort to a positive DataOutPort. A shared private kind selector keeps the
serializer and loader aligned.

Version1 flat-origin metadata still preserves original input kind, scope,
physical input bindings, ranges, inversion and missing inputs. The loader accepts
both previous and new declaration encodings; older flat files now canonicalize
to the new encoding. A converted output declaration cannot give an original
clock input frequency multiplier/divider or oscillator Period properties.
Genuine ToClockPort frequency and oscillator declarations remain supported.
No Source-scope rule, clock execution, public API or metadata version changed.

New native declaration/origin assertions cover nested, vector, partial
and unbound inputs and migration of legacy encodings. A metadata frequency test
failed after admitting the new declaration kind, then passed with the guard.
A new CLI method checks three legacy direct/alias/divider models with existing
source/flat/generated/reset replay plus legacy source/generated replay. All64
clock methods pass dynamically, statically and with leak-enabled ASan/UBSan.
Existing whole flatten/netlist and clock-snapshot native leak checks,71 main CLI
sanitizer methods and six changed-C++ format/tidy/strict17 gates also pass.

Six fresh Tessent2026.2 flat probes use encoded port names in their actual clock
setup and program. Direct and aliased models run successfully with both the flat
positive clock target and the normalized root clock: four complete exports
contain the expected two ordered clock/wait operations. Mapping physical names
through the recorded origin metadata gives the expected root trace. Eight
native replays of these origin-mapped programs against source/flat models pass.

The eight untranslated native export replays instead fail with exit2 because
origin restoration exposes original names, while the export uses physical flat
names. No raw native export replay success is claimed. Both divider probes now
pass read_icl but fail set_current_design with ICL130-2: local output ports feed
other output Sources. This is separate from the repaired ICL59-1 declaration
kind failure. General output-source flattening and native flat-name lookup
remain open; the existing source-scope policy is retained.

The six earlier source/normalized vendor directions from Task129 are reused
only after verifying exact model, input-program and current native-output
identity for all three cases. Fresh flat probes verify all18 submitted/fetched
input hashes. No formal registry certificate is added and no general clock
event, vendor equivalence or final release acceptance is claimed.
Evidence is retained under build/flat-differential-clocks-20261118/.
The earlier Task129 file-count wording minor is clarified:4013 is the count of
non-documentation files whose individual bytes match, not a byte count.


Task130 qualification passes the whole native flatten and netlist runners,
clock snapshots,64 clock methods per dynamic/static build and leak sanitizer,
71 main CLI sanitizer methods, and six changed-C++ style/strict17 gates.
All3994 runtime input hashes remain frozen; only pattern_flatten.cc and its
native/clock CLI tests differ from Task129. Full serial normal integration0
takes1744.420s, with1916 regression cases,2180 conformance cases,
334 parser/119 scanner rules across2549 ICL files and six editor tests in190.081s.
Five optional ANTLR checks are skipped.

Fresh instrumented testing takes1312.593s. All58 Python groups/
956 cases pass (51 generator groups/794 cases), along with native/regression/
conformance checks. The strict gate still fails at17386/18015 lines(96.5%) and
29399/46869 raw branches(62.7%). All54 authored reports and19 instrumented products
remain included, with no exception-edge filtering or threshold waiver.

The605109-byte named-secondary fixture retains4096 pairs,4096 shared stages,
1024 waits and five replay forms. Parent/current dynamic/static runs take
1.877934/1.692213/1.796047s, with peak RSS
125444/127896/125244KiB; all three emit identical
PDL hashes. Existing4096-stage/65536-bit checks and parent/current5000-instance
netlist benchmarks pass. Commands, source/product fingerprints and environment
are retained; no speedup or general throughput claim is made.

All eight current Rocky compiler/native/dynamic/static/ELF gates pass; both
variants run51 generator groups/794 cases and all3994 submitted input hashes
match. All14 remote and12 local archive/install gates pass for five commands
per variant, LSP behavior, static ELF and uninstall sentinel preservation.
Only the two generator products change; the other eight local/remote tools
retain Task129 fingerprints. The clean daacad0 archive ships
4231 tracked files plus six generated/prebuilt files and builds
locally with Bison/Flex disabled. Its4013 non-documentation files match the
current qualified remote source byte-for-byte. That current compiler/suite
evidence is reused by exact identity; no fresh remote archive rebuild is claimed.
Later doc-only archive refreshes preserve the tested runtime bytes and original
provenance. Broader source, event, vendor and release acceptance remains open.

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

Historical Task136 evidence follows. Task143's current
[invocation-sensitive fallback](#local-reset-output) supersedes its
identical-signature rejection; the measurements below retain their original scope.

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


## Physical reset-pattern replay

A vendor pattern export may express the bits shifted into the scan path rather
than a final instrument value. For example, loading `core.b.value = 3` while
the same update asserts its local reset can leave the register at zero.
Ordinary `iWrite` scheduling correctly rejects that conflicting final objective.

Replay such a physical export explicitly:

```sh
icl-pattern-generation --physical_pdl --icl model.icl \
  --bsdl AccessProbe model.bsdl --pdl exported.pdl --output physical.pdl --debug
icl-pattern-generation --physical_pdl --icl debug/Top.icl \
  --bsdl AccessProbe model.bsdl --pdl physical.pdl --output replay.pdl
```

Replace `Top` with the selected module name. Physical mode performs one exact
capture/shift/update sequence per nonempty apply. It checks the outgoing serial
bits before updating, simulates local reset on actual TAP edges, and records the
actual retained state and fill only after success. Output writes preserve the
shifted values even when reset masks them. Use `--physical_pdl` for subsequent
replay; this interpretation is opt-in and does not alter ordinary PDL.

The supported scope is scan-only storage (no DataRegister declarations), one
reset-initialized bound root TAP, a known current scan route and direct common
capture/shift/update/clock controls. Explicit writes
must be binary; reads may contain X comparison masks. Partial and omitted loads
use the known retained fill, which local reset events update. The existing
1,048,576 event-work bound applies. Resource preservation is checked on every
clock; an unrepresentable route or failed compare leaves output artifacts
untouched.

Physical mode adds no setup scans and does not merge applies. It rejects mixed
data access, AccessTogether aliases, iApplyEndState defaults, raw scans,
iState, waits, clocks and source-preserving `--local_reset`. Use ordinary mode
to schedule instrument goals and generate reset prerequisites first.

Initial Task137 qualification before independent review: all26 focused reset tests pass with dynamic and static
binaries. Full normal regression passes (1916 regression cases,2180 conformance
cases,334 parser and119 scanner rules across2549 files, plus6 editor tests).
ASan/UBSan with leak detection passes the native scheduler/runtime and all
focused reset/source/main suites. Rocky Linux passes51 CLI suites and818 cases
per binary variant, strict C++17 and native checks, and14 fresh installation
gates. Four unchanged recorded vendor export/model combinations now replay
natively with the explicit physical option; all13 explicit loads and7 apply
boundaries per export are preserved, and outputs are identical and idempotent.
A repeated256-sequence probe executes1792 applies in0.748/0.741s on source/flat
models (66524/66600KiB RSS on a shared host); no speedup is claimed.

Fresh raw coverage runs58 suites/980 cases successfully, but the strict100%
line/90% branch gate remains failed:17588/18193 lines and29777/47293 branches.
All54 authored reports,19 instrumented binaries and missing edges are retained;
12347 missing throw edges and5169 other missing edges are not filtered.
This is scoped reset replay qualification, not whole-project release acceptance.

Independent Task137 review at4964653 found two Important retained-fill defects.
The one fix pass at83a8eb8 reconciles every scan register's physical fill to its
actual final stored value after both a complete CSU and a complete iReset.
This preserves unreset storage across repeated asynchronous/synchronous resets
and invalidates stale fill after a bypassed register updates to a partial-X
value. Such unknown fill now requires an explicit binary payload.
Both reproductions were observed RED then GREEN, including original/flat
models, idempotent output and preserved artifacts on failure. Ordinary reset
and iWrite conventions remain unchanged.

Post-review qualification supersedes the initial totals above:28 reset tests
pass for dynamic/static/SAN binaries; full native, normal regression and all
sanitizer suites pass. Full normal check exits0 in1750.319s; native scheduler
141.570s and sanitizer native335.318s. Rocky51 suites/820 cases per variant,
11 platform gates,12 local package gates and14 remote package gates pass.
The archive contains4245 tracked files and6 generated extras; all4013 non-doc
bytes and ten qualified products match remote installation evidence.
Four archived vendor export/model replays remain identical/idempotent with
all13 explicit loads and7 apply boundaries preserved. The final256-sequence
probe performs1792 applies in0.679/0.678s (source/flat), with66292/66432KiB RSS;
these single runs establish no speedup or general throughput guarantee.

Fresh post-fix coverage runs58 suites/982 cases successfully but retains the
failed raw100/90 gate:17589/18193 lines and29784/47295 branches,
12347 missing throw edges and5164 other missing edges across54 reports.
No reports or edges were filtered. One Minor remains deferred: physical replay
currently reports a generic CSU failure instead of the concrete comparison,
reservation or routing cause. No re-review was requested after the one fix pass.
All four subjects the reviewer declined to judge have explicit executor
rulings: whole-release/strict-coverage acceptance stays open; excluded physical
features stay unsupported; invalid control-pin data selectors remain rejected;
and native replay of archived exports supplies no fresh vendor/hardware or
general equivalence signoff.

Task137 integration: merged into master at267e0d1 and pushed. Both merged
28-case reset suites and the native runtime pass; all3994 runtime input hashes
and ten product hashes match the qualified worktree. The completed task branch
is deleted. This closes the scoped reset replay task; raw100/90 release
acceptance and the explicitly excluded physical features remain open.


## Task139: physical replay failure causes

The explicit physical replay path now preserves routing and reservation errors
and reports mismatched serial bit positions with expected/observed values.
Masked comparisons do not evaluate their source. Unknown retained scan loads
name their register and bits. Failed commands preserve state and published
artifacts; ordinary scheduling still prepares closed routes through fallback.

All30 focused cases pass for dynamic/static and leak-enabled sanitizer builds.
Full native scheduler and all51 generator suites/822 cases per variant pass;
full sanitizer native, focused30 and main71 checks pass. Both changed C++ files
pass format, clang-tidy and strict C++17 checks. Source-distribution checks pass.
Evidence: build/physical-replay-diagnostics-20261120/. No coverage closure,
fresh vendor run or general physical-replay equivalence is claimed.


## Task140: BSDL model/default qualification

The eight audited BSDL requirement groups now have native model and replay
coverage: private omissions, per-instruction captures, alternate capture lists,
repeated opcode descriptions, implicit BYPASS, shared widths, semantic case
folding and compatible shared-opcode masks. See the Task140 section in
[model acceptance](pattern-model-coverage.md#task140-audited-bsdl-modeldefault-forms)
for conservative-mask semantics, migration/reset fixes, measured resource
bounds, exact qualification records and Tessent2026.2 differences. The initial
matrix passed830 cases per variant; the post-review record in that section
supersedes it. No coverage closure or general physical/vendor equivalence is
claimed.

## Task141: flat clock overrides, sparse history and child-port locks

Associated flattened clock names use the same checked name mapping for both
iClockOverride targets and sources as ordinary clock configuration and waits.
Literal-name collisions retain their existing priority, and indexed clocks,
saved checks, override versions and default restoration keep their validation.

Never-read register/output bits return unknown history rather than requiring a
device sample. Explicit sparse history preserves unknown untouched bits.
A successfully validated apply invalidates its read history until observations are
supplied; expected comparisons never become observations. Status is zero while
no cared comparison has been applied, including all-masked raw comparisons;
unknown actual comparison results still reject, and reset does not clear them.
This does not add a hardware transport, live observations or chain history.

Modeled child raw scans accept compatible port reservations and check their
actual capture/shift/update edges and final state. An incompatible selection,
clock edge or derived output still rejects atomically. Root raw effects retain
their existing port handling. Scope return and ordinary follow-up writes work.
A single-stream iRead does not assign a controlling lock (IEEE1687-2014
7.9.19, printed178/PDF192); the native preservation API tests actual clock and
output locks.

Qualification after independent review and one necessary fix pass passes all13
native pattern suites,51 generator suites/852 cases per dynamic/static variant,
focused72 clock +30 history +14 child methods per variant, three native
leak-enabled ASan/UBSan suites, sanitizer clock72/history30/child14/BSDL20/main71
methods,30 changed-hunk format/tidy/C++17 gates and source-distribution checks.

Status follows effective per-bit expectations after controlling state and
overlapping reads, plus surviving normalized interface/IR raw requests.
Grouped X masks retain the same meaning as unspaced masks. Static target/value/
apply-option validation precedes history commit, including discarded operands;
failed applies preserve histories, status and previously published artifacts.

Child raw replay proves held ports invariant across intermediate serial states
before bulk shifting. Otherwise it checks each edge within the unchanged
one-million event-work budget and reports exhaustion explicitly. A65536-bit
child scan with five held ports uses the proof path in0.422371s in one local
measurement; no general speedup is claimed. Cared child port locks add one
linear netlist snapshot; unprotected/root-exempt paths retain existing behavior.

Exact commands, exits, timings, hashes and review disposition are retained in
build/runtime-clock-ownership-20261120/review-fix/; the original pre-review and
failed-draft evidence is preserved separately. Two Minor review findings remain:
the design/top-name constructor without a netlist conservatively rejects an
explicit all-masked ordinary read's zero status; eager validation identifies the
apply location rather than the queued operand's origin. CLI elaborated history
and original operand errors remain covered. Full remaining items5–7, excluded
coverage1 and general vendor/physical equivalence are not claimed.

## Task142: typed vector-port functions

Repeated indexed input/output port functions retain their own types, attributes,
enums, sources and clock properties (IEEE1687-2014 6.4.1g/h, printed80/PDF94).
For example:

```icl
Module PortFunctions {
 DataInPort data[1];
 DataInPort data[0];
 DataOutPort value[1:0] { Source data; }
}
```

A whole `data` reference uses descending bit order when all functions have one
index. Nontrivial ranges determine ascending/descending order, independent of
declaration order. Source and PDL references, aliases and whole/sliced child
InputPort bindings select actual function bits. Binding holes and wholly absent
bindings retain their different meanings. Mixed function kinds and disjoint
input/output functions do not gain each other's access permissions.

A selected function retains its RefEnum; a whole heterogeneous group cannot
borrow the first function's table. An alias with its own RefEnum remains valid.
LogicSignal enum comparisons retain the standard's single-object/no-subrange
restriction (6.4.10c/k). Overlapping/gapped/inconsistently directed functions,
scalar/non-port duplicates and aggregate width exhaustion reject.

Associated-flat models retain real typed functions and canonical original
identities in existing metadata. Restoration checks actual bounds, type,
membership and namespace conflicts; it creates no synthetic hardware node.
Grouped clock names retain literal collision priority and checked override
snapshots. An encoded physical fragment cannot select another fragment's bit.
Scalar generated TAP pins cannot shadow a bus, and generated routes reserve
logical bus names. TAP construction checks the selected scan-input bit before
adding wiring, rejecting explicit sources while allowing unrelated bound
functions on the same bus.

The existing generic primary limits remain256 functions/4096 bits plus bounded
solver time. Actual128-function scheduling takes0.0208102s in one native run;
a single128-bit control takes0.00944873s. The equivalent CLI runs take0.1768s/
84084KiB dynamic and0.1600s/83644KiB static.4096-function loading/history formatting
takes0.1307s/56848KiB; this is not a4096-function scheduling result. The512-function
guard and a parent4096-bit bounded-query failure remain explicit item10 evidence.
These single measurements do not establish a general speedup.

Nine bounded port models have nine successful fresh normalized Tessent2026.2
runs,72 nonempty local source/flat/generated/vendor replays and18 exact
directional canonical comparisons. Submitted and returned input hashes match.
They are exploratory evidence, not newly maintained golden certificates or
universal vendor equivalence. Eleven corpus models now pass the former duplicate
boundary and reach the separately tracked interface-event guard; they are not
counted as end-to-end generation successes.

Full post-fix qualification passes13 native suites and52 generator suites/869 cases per
dynamic/static variant. Leak-enabled ASan/UBSan passes eight native suites,
port17/clock72/history30/child14/BSDL20 and main71 CLI methods. All39 changed-code
format/tidy/C++17 gates and source-distribution checks pass. Independent
whole-branch review found no Critical/Minor and two Important blockers, fixed
in one pass: native scan endpoints now use exactly one selected index, and
default-bearing child DataInPorts remain legal DataInPorts in associated-flat
ICL. Existing version1 ninth-field binding metadata preserves their parent
sources, inversions and holes; wholly unbound defaults retain eight fields.
Conventional single-port defaults receive the same shared fix. Native source/flat
single/multiple-lane endpoints, default/source identity, canonical reserialization,
six forged-metadata guards and CLI source/flat/generated/reset controls pass.
All72 vendor-related replays were refreshed after the fixes; submitted payloads
are unchanged from the recorded nine successful vendor runs. No re-review or
new vendor certificate is claimed. Exact commands, exits, timings and frozen
source/product identities are retained under build/segmented-ports-20261121/. The broader source/corpus, event/topology and
excluded coverage requirements keep their tracker status.

## Task143: invocation-sensitive reset callers

The same literal caller can now need different release preambles even when its
evaluated arguments are identical. The existing source trace supplies all
invocations, including empty and caught-error calls. A final fallback uses
interned argument/preamble records and a compact per-site invocation sequence;
original procedure definitions remain unchanged. Strict replay preserves already
valid sources without redundant insertions. See [local reset output](#local-reset-output)
for namespace, invocation and atomic publication contracts.

Forty caller methods cover direct writes and procedures, argument failure
and side effects, empty/partial-error returns, prefixes/vector literals, ordered
files/continued EOF, level0 restoration, helper cleanup/interference, rejected
missing/extra invocations and source/flat/generated/reset idempotence. Existing
queue/reservation/internal-reassertion negatives remain. Native runtime final
level controls and12 changed-code format/tidy/C++17 gates pass.

Nine post-fix single-run matched-input measurements at64/256/512 calls preserve
exact canonical outputs. Plain parent0.273/0.826/1.473s versus current0.240/0.769/
1.482s; repair0.628/2.313/4.571s and61120/69948/71276KiB peak RSS. Rewritten source
is2212/2982/4006 bytes, including exact immutable-table validation. These are measurements, without a speedup claim or raised
runtime/solver budget. Commands, environment, source and product hashes are in
build/occurrence-reset-20261121/review-fix/scale/results.json.

The invocation wrapper and a manually ordered reference both run successfully
through Tessent2026.2 on the existing bound-TAP reset model. Their full exports
are byte-identical. All submitted/returned input hashes match; eight nonempty
source/flat/generated physical replays match canonically. This is bounded fresh
evidence, without a new maintained golden certificate or universal equivalence
claim. These are fresh post-fix runs because the submitted wrapper changed.

One independent review found a Critical joined-source validation gap and four
Important issues: skipped cleanup after return, caught guard failures, deletion
of unexpected array data, and a stale reservation-test rejection. One fix pass
adds shared fresh-source/final-level validation, independent cleanup attestation,
a persistent mismatch flag and exact array checks. The corrected reservation
test requires byte-identical unchanged source for explicit user releases and
retains a genuinely impossible automatic-release negative with protected outputs.
All40 caller/resource35/compound12 focused methods pass. No re-review is claimed.

Post-fix15 qualification steps pass:52x879 dynamic/static, fresh native sanitizer
runtime, eight reset/resource sanitizer suites and main71,12 format/tidy/C++17
gates and source package. The actual prior full13 native pass is reused through
3992 unchanged shared inputs and13 unchanged binaries. Exact commands, exits,
input/product hashes, failed pre-fix receipts, review and costs are retained under
build/occurrence-reset-20261121/, with final evidence under review-fix/.
Coverage1 remains excluded and other inventory items keep their own status.

## Bounded DataRegister feedback (Task144, 2026-11-21)

Feedback components share one immutable dependency plan between native replay
and primary/mixed symbolic execution. Acyclic cascades retain their name order
and single pass. Cyclic members evaluate simultaneously from the preceding
snapshot until bits and known masks stop changing. A self hold retains its
actual old value; an algebraic solution cannot invent another value. Capture
observes storage before Update, and settling adds no clock edges.

Every changed feedback snapshot checks held resource bits, including computed
outputs. NetworkState input/clock failures restore pins, shift/update storage
and event history; ScanScheduler retains the outer TAP-controller transaction.
Scoped checks are not stored. The last definite known write/reset controls fill,
so an unknown self hold does not create a known history entry and a self-disabling
enable does not erase an earlier write. Both solver paths protect pending reads
at each delta. Primary frames retain driven reset/control pins.

OneHot payload dependencies include resolved read address/enable controls, so
mixed scheduling can discover the required controls automatically. Explicit
autonomous feedback goals remain in mixed output when physical scans alone
would lose their cared-bit/default-fill intent on replay. In this digital model,
an explicitly emitted root-pin write, including an unchanged value, can supply
a parallel-apply event. The legacy acyclic constant-source reachability
restriction remains separate item10 work.

A feedback-derived reset of external storage must retain its level and known
mask through every delta. Changing external reset events reject atomically;
masked/invariant paths qualify. General event execution remains item3, and
DataRegister PDL callbacks remain item4 work. At most 32 delta rounds and 4 Mi
feedback-bit evaluations per native event or symbolic query are allowed.
Existing register, dependency, primary-input and solver limits are unchanged.
A convergence-limit failure does not prove unreachable hardware or physical
instability. No new vendor-equivalence or timing-signoff claim is made.

One independent review found zero Critical, three Important and zero Minor
issues. One tested fix pass resolves unknown mixed feedback history, primary
pending-read intermediates and implicit OneHot control discovery. Its multi-bit
self-disable control also verifies generated-PDL goal/fill intent. Fourteen
focused CLI methods cover source, associated-flat and generated-PDL replay,
capture ordering, known/unknown convergence, ownership, resets, rollback/retry
and protected negative artifacts.

All 41 post-fix qualification steps pass: 13 native and 13 native sanitizer
suites; 53 suites/893 tests for each dynamic/static generator; nine affected
sanitizer CLI suites plus main71; 39 format/tidy/strict C++17 gates; and source
packaging. Hash checks verify 3,997 inputs, 29 products and the benchmark binary.
Evidence is under build/data-register-feedback-20261121/review-fix/. Pre-review
receipts remain under final/, including earlier failed focused runs. No second
review is claimed.

The scale check records 54 actual processes: three repeats of source/flat/
generated forms for 16/128 groups, with matched parent/current DAG controls.
At 128 groups/512 bits, median three-replay wall times are 1.047 s for the parent
DAG, 0.946 s for the current DAG and 3.606 s for feedback; peak RSS is
124108/124332/124812 KiB. Environment: GCC16.2.1, Z3 4.13.3, Linux WSL2 x86_64.
Inputs, exports, commands and hashes are in
build/data-register-feedback-20261121/scale-review-fix/. These measurements make
no speedup or global-optimality claim. Coverage item1 remains excluded.

## Task145: typed DataRegister callback declarations

Historical declaration checkpoint. [Task153](#task153-explicit-dataregister-callback-execution)
adds explicit callback-only execution; the invocation restrictions below describe
Task145's earlier implementation.

ReadCallBack and WriteCallBack declarations now retain resolved namespace and
procedure names, with distinct data/register placeholders and literal arguments.
Each declaration resolves parameters in its own elaborated instance. Numbers use
the shared constant evaluator, retaining widths and unknown bits; integer
expressions keep its existing checked 64-bit limits. Identifier parameters must
be scalar identifier strings. Type/cardinality validation remains in the shared
ICL parser. No duplicate type checker or metadata version was introduced.

Flat ICL writes canonical callback properties using existing literal quoting and
restores the original declaration scopes through the existing origin metadata.
Literal strings containing <D>, <R>, Tcl characters, quotes or backslashes remain
literal values. Procedure definitions need not be loaded for model serialization.
For example, this declaration can be exported with the normal --debug model
output without defining a PDL procedure:

```icl
Module Example {
  Parameter owner = "Example";
  DataRegister value[3:0] {
    WriteCallBack $owner store <D> <R> 4'hA;
  }
}
```

Use an access-free PDL input such as `iNote {Inspect callback model}` to inspect
that output. An iWrite/iRead of the callback register still requires the future
invocation implementation; defining an iProc does not enable that access yet.

Seven focused CLI methods cover source/flat/generated model replay, namespace
forms, instance specializations, invalid names/type mixtures and preservation of
existing output after errors. Native checks assert typed argument values and
canonical replay explicitly.

Applied direct/aliased reads and writes that require a declared callback reject
at iApply until procedure invocation exists. This prevents an ordinary observable
route from silently replacing a required callback. Reset-canceled queues,
event-free assumptions/history and explicit output comparisons remain valid.
The callback-free path avoids the pending-callback scan.

This increment models declarations only. Direct/implicit callback invocation,
queue ordering/cancellation, read observation and sourced-value execution remain
item4 work. The unquoted module-symbol parameter spelling in the standard's
callback example remains separate source-audit work; quoted identifier parameters
are the supported form here. No new vendor execution/equivalence claim is made. The retained 2026.2 writer
probe and a fresh read-only callback probe each reject their callback property at
read_icl with ICL20-1 before PDL execution. Neither provides a callback export to
compare. The fresh reader receipt, input hashes and logs are under
build/callback-execution-discovery-20261122/read-callback/.

The scale record contains 54 actual processes: three repeats of source/flat/
generated forms at 256 and 2048 instances (512 and 4096 registers), with matched
parent/current controls. At 2048 instances, median three-replay wall times are
0.394 s parent control, 0.395 s current control and 0.444 s callback model; peak RSS is
78348/79120/83320 KiB. These are measured model/serialization costs, without a
speedup or global-optimality claim. Environment: GCC 16.2.1, Z3 4.13.3, Linux WSL2
x86_64. Inputs, commands, output hashes and the parent executable snapshot are
under build/data-register-callback-model-20261122/scale/.

Post-guard qualification passes all 33 steps: 13 native suites, 13 leak-enabled
ASan/UBSan native suites, 54 suites/900 tests per dynamic/static generator,
callback-model 7 and main 71 sanitizer methods, 21 changed-code format/tidy/strict
C++17 gates and source packaging. Hash checks verify 3998 source/test/build
inputs, 29 products and the measured executable. Evidence is under
build/data-register-callback-model-20261122/final/. The pre-guard receipts are
retained separately and are not the final result. The exact original scalar
callback-read bypass now rejects with the existing output preserved.

Technical accuracy was checked against the implementation, native assertions
and recorded processes. The guide example and its unsupported-access outcome
were executed; the reader walkthrough is an author self-review, separately
recorded in doc-reader-author-check.json. No independent usability reader is
claimed. Coverage item 1 remains excluded with unchanged thresholds.

One fresh independent code review reports zero Critical, Important or Minor
findings and approves this increment. The reviewer's command host was unavailable;
the root agent relayed actual source/grammar/tests and executed the reviewer's
18 independently specified probes. All met their expectations, including mixed
queue rollback/history and ordinary-only versus callback-only slices of mixed
and inverted aliases. This is independent judgment with proxied tool execution,
not an independent rerun of the qualification gates or a usability-reader claim.
No second review is claimed. Report and raw probes are retained in the evidence.

The review also isolates an existing unrelated value-loss defect: a trailing
iWrite -ignore_when_value_already_present off/on emits0 for a requested1 in
matched parent/current primary-pin controls. It was tracked under item10 and is fixed by Task146 below; the original
reproduction evidence is retained.

## Task146: optioned iWrite payloads

A validated trailing -ignore_when_value_already_present option no longer replaces
the requested value with an X mask. Runtime now records the existing internal
target/value pair after shared option validation. Original evaluated source
arguments remain available for source rewriting; procedure bodies and once-only
Tcl evaluation remain unchanged. History, merge, reset planning, reservations
and generation receive the same value as an unflagged write.

For example, with a writable scalar data port, this requests value 1:

```tcl
iWrite data 1 -ignore_when_value_already_present off
iApply
```

The auto/off/on forms retain the native generator's existing explicit-load
policy. The v2023.4 Shell Reference Manual iWrite topic describes on as permission
to omit a load when the value is already present, rather than a prohibition on
loading. This fix introduces no skip-load optimizer or global retargeting-option
command. Broader scheduling optimization remains item 10. These are native PDL
usage examples; Tessent command mode/context requirements remain its own.

Invalid numeric widths and missing enum values still reject before publication,
even if a later reset cancels the queue. Six focused methods cover primary pins,
scan and parallel storage, aliases/inversions/enums/X masks, merged calls,
once-only Tcl evaluation, cancellation and source/flat/generated replay. Native
assertions check normalized operation pairs, original traced arguments and
validation inside a queue containing reads. The saved parent fails 16 subcases;
the fixed executable passes all six methods.

Four fresh Tessent 2026.2 cases (baseline/auto/off/on) complete successfully in
patterns -ijtag / analysis with an open pattern set. All four exports are equal
and explicitly write 0 then 1 and compare 1. Sixteen native forms (source, flat,
generated and vendor output with its exported procedure invoked) agree exactly.
This is bounded changed-value PIO payload agreement, without a physical-import,
general load-elision, timing or hardware-equivalence claim. Raw inputs, commands,
logs, hashes and exports are under build/write-ignore-value-20261122/vendor/.

The scale record contains 54 actual processes: three repeats of source/flat/
generated forms at 16/128 applies to a 32-bit pin. Matched parent/current unflagged
controls and current mixed-mode flagged input have identical outputs per size.
At 128 applies, median three-form wall times are 0.495/0.494/0.495 s and peak RSS is
75524/75592/75604 KiB. Environment: GCC 16.2.1, Z3 4.13.3, Linux WSL2 x86_64.
These measured costs do not establish a speedup or global runtime guarantee.
Receipts are under build/write-ignore-value-20261122/scale/.

All33 final qualification steps pass:13 native and13 leak-enabled ASan/UBSan
native suites,55 suites/906 tests per dynamic/static generator, six focused and71
main sanitizer methods, six changed-code format/tidy/strict C++17 gates and source
packaging. Frozen checks verify3999 source/test/build inputs and29 products.
Evidence is under build/write-ignore-value-20261122/final/. Strict coverage item1
remains excluded with unchanged thresholds.

Technical accuracy was checked against source, native assertions and recorded
processes. The guide example above was executed and emits value1. Its reader
walkthrough is an author self-review, recorded in doc-reader-author-check.json;
no independent usability-reader claim is made.

One fresh independent code reviewer approves this increment with zero Critical,
Important or Minor findings. Its command host could not start, so the reviewer
assessed actual source/parser/test excerpts and qualification output relayed by
the root agent. No independent test rerun or full-checkout inspection is claimed.
General load elimination, physical observations and wider vendor equivalence
remain outside this payload fix. The report is retained in the evidence.

## Task147: actual primary events and autonomous DAG goals

The primary-only scheduler now counts an enabled DataRegister update caused by
an actual emitted primary-pin event, matching the existing mixed and feedback
paths. The pin may be outside that register's data/control dependency cone.
The generated write is a real input event in the native model; an empty or
read-only group cannot create one. No clock or observed value is invented.

For a two-bit value with constant WriteDataSource 2'b11 and a writable trigger,
this reaches value3 from an assumed zero through an explicit trigger write:

```tcl
iState trigger 0
iState value 0
iWrite value 3
iApply
```

This uses the existing accepted native DataRegister and input-event semantics.
It does not extend ICL source acceptance or establish physical-interface timing.
Both paths retain a DataRegister goal when no emitted controller write carries
its intent. This includes autonomous DAGs and unchanged protected sources.
Goal markers preserve replay intent; concrete storage still follows real events.

Seven focused methods cover constant/cascaded/feedback sources, inverted aliases,
partial masks, event-free reads, reset cancellation, mixed scan replay and atomic
failure for no writable event source, disabled/unknown enables, an active reset
and protected side effects. Negative assertions require scheduling failure,
rather than allowing invalid setup to count as proof. The original five-method
parent run fails five subcases. Two further methods
cover the connected mixed and source-ordered primary replay defects found
during the final fix pass, including full/masked values and together applies.
A native test independently applies every emitted pin event
to NetworkState on original and serialized models and compares concrete storage.
Existing state11 and feedback14 methods pass, as do six code-quality gates.

The54-process scale record uses16/128 two-bit registers and three repeats of
source/flat/generated forms. Parent/current writable-source controls have equal
outputs; candidate constant-source forms agree across replay. At128 registers,
three-form medians are0.495/0.496s for matched controls and0.344s for the new
constant-source case, with88104/92044/91916KiB peak RSS. GCC16.2.1,Z34.13.3,
LinuxWSL2x86_64. Cases differ in data-source model, and no speedup is claimed.
Receipts under build/primary-data-events-20261122/scale/ record actual commands,
input/output hashes, binary identities and costs. The inherited benchmark key
flagged means constant sources; its meaning is explicit in summary.json.

Broader item10 optimization and the other open inventory capabilities remain.

All33 post-fix qualification steps pass:13 native/13 leak-enabled ASan/UBSan native
suites,56 suites/913 tests per dynamic/static generator, seven focused/71 main
sanitizer methods, six changed-code format/tidy/strict C++17 gates and source
packaging. Frozen checks verify4000 source/test/build inputs and29 products.
Evidence is under build/primary-data-events-20261122/final/. Coverage item1
remains excluded with unchanged thresholds.

Technical source/test checks are separate from the reader walkthrough. The actual
guide example above executes and emits the trigger write; its author self-review
is recorded in doc-reader-author-check.json. No independent usability reader or
live hardware observation is claimed.

One fresh independent review approved the pre-fix scope and isolated a related
pre-existing mixed replay discrepancy. Root regraded the lost-goal family
Important; source-order probes also reproduced it in the primary path. One
verified fix pass shares goal retention across both paths. The sixth and seventh
methods each have four semantic RED cases; all seven methods pass afterward.
The native oracle applies only real pin events, then checks goal markers against
independently reached storage. No marker assigns the oracle state.

The reviewer used relayed source/results and independently selected root-executed
probes because its command host was unavailable. No independent gate rerun,
post-fix rereview, usability reader or physical scan-phase reconstruction is
claimed. All33 post-fix gates run freshly on the final frozen code. Pre-review
receipts are under pre-review-final/ and pre-review-scale/; the interrupted
mixed-only run is under intermediate-mixed-fix-final/ and is not a full pass.

An additional54-process mixed scale record compares matched already-held3
controls and candidate requested0-to3 goals under a protected source3, at16/128
registers. Source/flat/generated bytes agree per family; parent/current controls
are equal. At128 registers the three-form medians are0.194/0.193/0.193s and RSS
is74524/78368/78752KiB. Goals differ between controls and the new case; no speedup
is claimed. Together the final records contain108 successful processes. The
initial proposed control exposed the known parent replay discrepancy and remains
archived separately under mixed-scale-initial-control/, outside the final record.

## Task148: modeled combinational PIO readback

Use --model_readback when a PDL computation needs the modeled value of an
applied output read. This mode obtains concrete combinational PIO observations
from the scheduler; expected read values remain comparison data. Diagnostics
and generated PDL identify modeled execution explicitly.

Save this as model.icl:

~~~icl
Module ModelTop {
 DataInPort p[3:0];
 DataOutPort q[3:0] {Source p;}
}
~~~

Save observe.pdl as follows. The expected zero deliberately disagrees with
the assumed input 15, so all four compared bits fail:

~~~tcl
iPDLLevel 1
iState p 15
iRead q 0; iApply
iNote [iGetReadData q -hex]
iNote [iGetMiscompares q -bin]
iNote [iGetStatus -clear]
iNote [iGetStatus]
~~~

~~~sh
icl-pattern-generation --icl model.icl --pdl observe.pdl \
  --top_module ModelTop --model_readback --output result.pdl
~~~

The notes contain 0xf, 0b1111, 4 and 0, in order. The clear query returns the old
global failure count 4; the next query sees 0. These are generation-time model
results. Replaying generated PDL preserves the single mode note and resolved
notes; original and associated-flat inputs also agree.

Only networks without storage nodes or bound TAPs are eligible for newly needed
model observations. Supported operation histories contain ordinary reads/writes/
applies, state assumptions, resets, reservations and notes. Raw scans, merge
groups and clock operations remain outside this increment. Output/alias access
obeys existing observability checks. Existing manual-history and known-zero
queries retain their behavior. Enabling the flag without a model query does not
initialize an additional scheduler.

History is per physical bit, with sparse and inverted aliases supported.
Repeated reads replace expected bits; X removes comparison care but still
observes the bit. An unknown actual value remains x. A cared comparison against
unknown data makes accumulated status unknown; status then fails without
returning or clearing an invented count. Failure counts accumulate over applies;
reset cancels queues without clearing the count. Never-read bits stay x.
Manual history flags retain existing storage/root-input target rules and do not
assign observable output history in this bounded mode.

The adapter consumes newly appended normalized operations once, batches concrete
reads and delivers only newly observed history bits. It does not reexecute Tcl
arguments or procedures. Local-reset rewriting preserves source and procedure
bytes and identifies modeled execution in diagnostics.

The runnable example above was executed as an author walkthrough, separately
from technical regression checks. Its receipt is under
build/modeled-pio-readback-20261122/author-example/. An independent usability
reader is not claimed. Full verification and final review results follow below.

One independent direct review found0Critical/2Important/1Minor. A verified
fix pass captures each bit at its actual selected concrete replay frame,
including access groups that need multiple frames; status clear immediately
caches known zero; label filtering copies history only when needed. The two
new CLI methods first reproduce three failing subcases, then all17 pass per
variant. A native oracle independently applies emitted pin writes and checks
observations against the resulting concrete output values. Expected values
never assign oracle state. The superseded final-state batch getter is removed.
There is no post-fix rereview or independent post-fix approval claim.

The final scale record contains108 successful processes:64/128 applies over
32-bit targets and8 applies over1024-bit targets with two sparse read bits.
Each uses four families, three repeats and source/flat/generated forms. Raw
generated bytes agree within each family; parent/current default controls match
and unused/queried modeled outputs match. Commands, environment, hashes, time
and peak RSS are under build/modeled-pio-readback-20261122/scale/.

| Family |128 applies,32 bits: three-form median(s) | Peak RSS(KiB) |8 applies,1024 bits: three-form median(s) | Peak RSS(KiB) |
|---|---:|---:|---:|---:|
|parent-default|0.646|75888|4.460|117152|
|current-default|0.596|75764|4.509|116900|
|current-unused|0.645|75836|4.459|117208|
|current-queried|0.946|110076|7.068|150504|

Queried runs do additional model work on source/flat inputs; generated replay
contains resolved commands/notes. These measurements establish compatibility and
observed cost at these sizes, without a speedup or larger-scale claim.
Pre-review receipts are retained under pre-review-scale/ and pre-review-partial/;
the interrupted pre-review broader run is not a full qualification pass.

All35 post-fix qualification steps pass:14 native and14 leak-enabled ASan/UBSan
native suites,57 suites/930 tests for each dynamic/static generator,17 focused/
71 main sanitizer methods,30 changed-code format/tidy/strict C++17 gates and
source packaging. Frozen hashes verify4005 source/test/build inputs and31
products. Evidence is under build/modeled-pio-readback-20261122/final/.
The final108 scale processes use the same qualified generator binary.
Coverage item1 remains excluded with unchanged thresholds.

## Task149: modeled DataRegister observations through PIO

--model_readback now includes DataRegisters reached through existing PIO routes.
It uses the same concrete scheduler and event model as ordinary generation,
capturing the first actual observation at its selected frame. ScanRegisters,
bound TAPs, clock/raw/merge histories requiring new observations, callback
execution and device transport remain outside this increment.

Save data.icl:

~~~icl
Module ModelData {
 DataInPort data[3:0]; DataInPort enable;
 DataRegister value[3:0] {
  WriteDataSource data; WriteEnSource enable; ResetValue 0;
 }
 DataOutPort q[3:0] {Source value;}
}
~~~

Save observe.pdl:

~~~tcl
iPDLLevel 1
iReset; iState enable 0
iWrite value 9; iApply
iRead value 0; iWrite value 6; iApply
iNote [iGetReadData value -hex]
iNote [iGetStatus]
iState value 15 -LastReadValue
iNote [iGetReadData value -dec]
iRead q 6; iApply
iNote [iGetReadData q -hex]
iNote [iGetStatus -clear]
iNote [iGetStatus]
~~~

~~~sh
icl-pattern-generation --icl data.icl --pdl observe.pdl --top_module ModelData --model_readback --output result.pdl
~~~

The notes contain 0x9, 2, 15, 0x6, 2 and 0. Modeled pin events first store 9.
The next apply observes that old value before storing 6; its expected 0 produces
two failing bits. Manual LastReadValue 15 changes only the history plane, while a
later applied output read still observes actual storage 6. Status clear returns
the old count 2, then leaves known zero. Model results use declared ICL state and
events; the mode has no connection to a device.

A DataRegister's manual read/miscompare history can overlap pending model
observations. The lazy adapter suppresses only older undelivered bits of that
plane; later applied observations supersede both planes. It preserves physical
bit identity, sparse/inverted/ascending aliases, unknown values and independent
failure counts. Unknown status cannot be returned or cleared numerically.
Controlling state still rewrites queued access values; history-only state does
not alter hardware or queues. Reset cancels pending accesses without clearing
history/counts, and a later applied read can observe reset storage.

Writes retain the scheduler's existing reset requirements. The retained
unreset read/write control fails under its unknown reset state; this feature
does not relax it. DAG/feedback, ownership and routing remain governed by the
existing concrete model and reachability/resource checks.

The Task149 mode note was modeled PIO/DataRegister readback; no live device.
Input containing the former combinational mode note migrates to one current
note; subsequent original/flat/generated replay is stable. Default mode leaves
ordinary notes unchanged. Tcl arguments remain once-only; local-reset rewriting
preserves procedure bytes.

The example was executed as an author walkthrough, with receipt under
build/modeled-data-readback-20261122/author-example/. Technical checks and the
independent code review are separate; no independent usability reader is claimed.
Full qualification, scale and review evidence is recorded below.

The 108-process scale record at behavioral head2641965 compares matched
parent/current defaults and current unused/queried mode. It uses8/16 applies
over one16-bit DataRegister and8 applies over32 four-bit registers with two
sparse read bits, three repeats and three replay forms. Default controls match;
unused/queried current outputs match; each family's three forms agree.
Commands, input sizes, environment, binary hashes, wall time and RSS are under
build/modeled-data-readback-20261122/scale/. No speedup or larger-scale claim.

| Family |16 applies,16 bits: three-form median(s) | Peak RSS(KiB) |32 registers,two sparse bits: three-form median(s) | Peak RSS(KiB) |
|---|---:|---:|---:|---:|
|parent-default|0.445|75676|3.355|88200|
|current-default|0.445|75684|3.306|88252|
|current-unused|0.395|75716|3.356|88228|
|current-queried|0.646|109676|6.166|122924|

Full qualification at2641965 passes36 steps:14 native/14 leak-enabled native
ASan/UBSan suites,58 suites/946 tests per dynamic/static generator,16 new/17
existing readback/71 main sanitizer methods,15 C++ quality gates and source
packaging. One fresh independent review found0Critical/0Important/2Minor:
stale CLI help scope and unordered partial-history assertions. Both are fixed
in one verified pass; no rereview or independent post-fix approval is claimed.

That pass changes only the CLI help literal and stronger ordered/help test
assertions. It passes13 focused checks covering312 dynamic/static/sanitizer
methods,15 C++ quality gates and fresh packaging. All28 native binaries and4004
other inputs retain their full-baseline hashes. Final manifests verify4006
inputs/31 products. The full baseline and its three generator snapshots remain
under final/; final help/assertion checks and manifests are under
final/post-review/. Scale measurements use the immutable2641965 binary; later
help/assertion changes do not alter model or scheduling code.

The reviewer assessed relayed actual source/diffs/docs/tests because its command
host was unavailable. Its independently selected partial-unknown probe passed
both getter orders and original/flat/generated forms (six root-executed jobs):
read10x1, mismatch1x01, status2 then0 after clear. No independent full gate rerun,
checkout inspection, live transport or independent usability reader is claimed.
Coverage1 remains excluded with unchanged thresholds.

## Task150: modeled standalone scan observations

--model_readback now includes ScanRegisters and DataRegisters read through
existing standalone or client-controlled serial routes. The concrete replay
samples the actual return wire immediately before each shift edge, decodes
chain order and inversion, and maps capture cells back to their logical data
targets. The qualified native shortcut retains batched shifting: its fixed
route and uninterrupted-shift checks prove captured shift storage equals the
decoded return, so it exports that actual storage after capture. Speculative
read shortcuts publish observations only after accepting the trial.
Readback comes from the first selected read in an Apply, before its
subsequent update; expected data only controls mismatch masks and counts.

Save scan.icl:

~~~icl
Module ScanModel {
 ScanInPort si;
 ScanRegister r[3:0] {ScanInSource si; CaptureSource r; ResetValue 0;}
 ScanOutPort so {Source r[0];}
}
~~~

Save observe.pdl:

~~~tcl
iPDLLevel 1
iReset; iWrite r 9; iApply
iRead r 0; iWrite r 6; iApply
iNote [iGetReadData r -dec]
iNote [iGetMiscompares r -bin]
iNote [iGetStatus -clear]
iRead r 6; iApply
iNote [iGetReadData r -dec]
iNote [iGetStatus]
~~~

~~~sh
icl-pattern-generation --icl scan.icl --pdl observe.pdl --top_module ScanModel --model_readback --output result.pdl
~~~

The notes are9,0b1001,2,6,0. The first read sees stored9 before the
same Apply writes6. Two bits differ from expected0; clearing returns2 and
leaves zero. A later read sees6. Sparse/inverted aliases, unknown captures,
manual history planes and source-order rules retain their existing semantics.
Unknown cared-for data makes the numeric failure count unavailable.

Supported disjoint unequal lanes sample from tick zero and ignore trailing
return padding. Existing scan clocks, route restrictions, reset/update
semantics and resource checks still apply. Reset-held update cells continue
shifting under the existing model; this extension does not change cell timing.

The current note is modeled register/PIO readback; no live device. Both older
combinational and PIO/DataRegister notes migrate to one current note. Without
a getter query the adapter remains lazy; ordinary output differs only by the
opt-in model note. Bound TAPs, raw/clock/merge histories, applied callbacks,
interface/chain histories and real-device transport remain outside this
increment. Applied histories requiring unsupported operations reject before
publishing observations and preserve an existing output file.

The author walkthrough and qualification records are retained under
build/modeled-scan-readback-20261122/. They are model/replay evidence, not
physical timing signoff or an independent usability-reader assessment.


Task150 final qualification passes37 recorded gates:14 native and14
leak-enabled ASAN/UBSAN native suites,59 suites/962 methods for each dynamic
and static generator,16 scan/16 data/17 PIO/71 CLI sanitizer methods,18 affected
format/tidy/strict C++17 gates and source packaging. All4007 qualified input
hashes and31 executable hashes are verified. Coverage item1 stays excluded
with unchanged thresholds.

The fresh independent reviewer inspected source and ran probes directly:
0Critical,2Important,0Minor, no declined-to-judge cases. Both missing-observer
paths (native ordinary scans and speculative regular reads) are resolved in
one test-first verified fix pass. Native captures retain batched shifting;
speculative observations remain private until accepted. Subscriber exceptions
propagate outside fallback catches. The full results above are after fixes.
No rereview or independent usability-reader assessment is claimed.
The interrupted pre-review run is retained as partial evidence.

Scale evidence contains144 parent/current processes: three repeats of each
original/flat/generated form, with default, unused-mode and queried-mode
controls. Inputs cover8/16 Applies on16-bit storage,32 chained four-bit
registers with sparse reads, and four read/write Applies plus route setup on a4096-bit native chain with
two requested bits. Three-replay queried medians are0.244–0.444 seconds;
the4096-bit native case is0.294 seconds, with maximum queried RSS112324 KiB
across these cases on the recorded WSL2/GCC16.2.1/Z3 4.13.3 environment.
Matched ordinary outputs and replay forms agree. These short, noisy process
measurements establish the tested scale, not a global speedup or a million-bit
modeled-readback qualification. Commands, input sizes, hashes and timings are
retained in build/modeled-scan-readback-20261122/scale/.

## Task151: modeled bound-TAP observations

--model_readback now includes IR/DR register observations through existing
bound TAP routes, captured DataRegister reads and common-phase multiple TAPs.
It samples the actual modeled TDO before each shift edge and retains the first
requested observation within an Apply. Expected bits control masks and counts;
they never supply observed values. Capture-IR metadata, unknown bits and
capture/shift/update separation come from the existing BSDL/ICL controller.

Save tap.icl:

~~~icl
Module Instrument {
 ScanInPort si; ScanOutPort so {Source r[0];}
 SelectPort sel; CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue; TCKPort ck;
 ScanRegister r[7:0] {ScanInSource si; CaptureSource r; ResetValue 0;}
 ScanInterface client {Port si; Port so; Port sel; Port ce; Port se; Port ue; Port ck;}
}
Module TapModel {
 Instance core Of Instrument;
 AccessLink tap Of STD_1149_1_2001 {
  BSDLEntity AccessProbe;
  IJTAG_ON {ScanInterface {core.client;}}
  IJTAG_OFF {ScanInterface {core.client;}}
 }
}
~~~

Save observe.pdl:

~~~tcl
iPDLLevel 1
iReset
iWrite tap.INSTRUCTION 3; iApply
iWrite core.r 9; iApply
iRead core.r 0; iWrite core.r 6; iApply
iNote [iGetReadData core.r -dec]
iNote [iGetMiscompares core.r -bin]
iNote [iGetStatus -clear]
iRead core.r 6; iApply
iNote [iGetReadData core.r -dec]
iNote [iGetStatus]
~~~

From the repository root, use its self-constructed BSDL test fixture:

~~~sh
icl-pattern-generation --icl tap.icl --bsdl AccessProbe tests/cases/ours/pattern_generation/access_link/simple.bsdl --pdl observe.pdl --top_module TapModel --model_readback --physical_pdl --output result.pdl
~~~

The notes are9,0b00001001,2,6,0. The first read sees old storage9
before the same CSU updates6; its two mismatching bits accumulate and clear
returns2. A later read sees6. The explicit IR setup is needed for exact
physical replay. Removing --physical_pdl uses ordinary retargeting; both forms
of this author walkthrough have been executed. The fixture is an ICL/BSDL
model, not a certificate for a physical device.

Runtime observations and emitted generation use matching observer-aware
scheduling. Modeled miscompares remain data to count, including physical
replay, rather than causing a different verification fallback. Default mode
retains its existing physical comparison diagnostics. The physical-mode
readback factory uses the same exact-CSU/reset mode as output generation.
Implicit IR setup is not added to explicit source-read history or failure
counts. Accepted native results own observations across route/state changes;
subscriber errors propagate without retrying an accepted scan.

Sparse/inverted/ascending aliases, unknown status, manual history planes,
reservations and reset cancellation retain their existing semantics.
Prefix/file ordering and once-only Tcl evaluation remain intact, and local
reset rewriting preserves procedure bytes. Existing unsupported topology,
reset, binary-load, event-work and physical-shape guards stay in force.
Direct accesses to synthetic TAP data registers remain forbidden; their raw
scan models do not become ordinary PDL targets.

Raw/interface/chain histories, independently phased unsupported TAP networks,
timed histories, applied callbacks and device transport remain open. This
increment neither inserts RTL nor signs off physical timing. The mode note
remains modeled register/PIO readback; no live device.

Final qualification, independent review and measured scale evidence follow. The author walkthrough is not an independent usability
reader assessment.


Task151 final qualification passes 38 recorded gates: all 14 native and 14
leak-enabled ASAN/UBSAN native suites, 60 suites/978 methods for each dynamic
and static generator, 16 TAP/16 standalone/16 data/17 PIO/71 CLI sanitizer
methods, 18 affected format/tidy/strict C++17 checks and source packaging.
All 4008 input hashes and 31 executable hashes are verified. Coverage item1
remains excluded with unchanged thresholds.

The fresh independent reviewer used direct source/tool access and independently
ran the 16 new dynamic methods, native readback and selected inversion/IR-lane
probes. Verdict: 0 Critical, 1 Important, 2 Minor. The Important subscriber-error
finding is fixed in one test-first verified pass. Public Apply now commits the
complete state and finishes route invalidation before external notification.
A subscriber exception leaves that completed transaction committed; a following
read keeps its data and raw normalization uses the new route. Tests cover native
standalone, regular/general and ordinary/physical TAP paths. Both stale-fill
and stale-route regressions failed before their fixes. Interrupted earlier runs
are retained as partial evidence, never a full pass; no rereview is claimed.

Two Minor findings are deferred: compiled help does not explicitly explain
modeled physical comparison behavior, and the walkthrough numeric prose lacks
spaces in several places. The guide explains the behavior and its executed
examples produce the five correct notes. These are not runtime failures.

The final 144-process scale matrix repeats original/flat/generated forms three
times for parent/current, default/unused/queried controls. It covers prepared
32-bit and 512-bit single-TAP chains, two unequal TAP lanes, and physical 32-bit
replay. Matching ordinary outputs and replay forms agree. Queried three-replay
medians are single32=0.294s, single512=0.545s, multiple=0.947s, physical32=0.294s. Commands, sizes, RSS, hashes and environment are
recorded under build/modeled-tap-readback-20261122/scale-qualified/.
Short process timings are noisy; no global speedup is claimed.
The initial implicit-setup 512-bit parent invocation reaches the existing
solver limit; explicit IR preparation passes matched parent/current controls.
No limit was raised and the prepared-chain measurement does not qualify every
wide startup sequence. Hardware timing, device transport, vendor equivalence
and an independent usability reader are not established by these checks.

## Task152: raw interface and chain readback

Use --model_readback with existing native scalar root/child scans and bound
single-TAP raw IR/DR scans. iGetReadData and iGetMiscompares now accept their
client interface and, for named opaque parallel chains, -chain NAME. Binary,
hexadecimal and decimal formatting matches register getters. A raw getter
requires a preceding applied iScan to establish its width. IR and DR share
the latest applied history for that interface; register histories remain separate.

For example, save this model as native.icl:

~~~icl
Module RawModel {
 ScanInPort si; ScanOutPort so {Source r[0];}
 SelectPort sel; CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue; TCKPort tck;
 ScanRegister r[3:0] {ScanInSource si; CaptureSource r; ResetValue 0;}
 ScanInterface client {Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck;}
}
~~~

Save these commands as native.pdl:

~~~tcl
iPDLLevel 1
iReset; iWrite r 9; iApply
iScan client 4 -si 6 -so 0; iApply
iNote [iGetReadData client -dec]
iNote [iGetMiscompares client -bin]
iNote [iGetStatus -clear]; iNote [iGetStatus]
~~~

Run:

~~~sh
icl-pattern-generation --icl native.icl --pdl native.pdl --top_module RawModel --model_readback --output result.pdl
~~~

The four notes are 9, 0b1001, 2 and 0. Capture returns the old value 9 before
loading 6. Comparing those four returned bits with zero produces two failures;
-clear returns that old count and then clears it. Expected data and scan inputs
are never treated as observations.

Opaque storage has no declared behavior. Its actual returned data is x even
after reset, a DefaultLoadValue declaration, or a preceding known scan input.
For example, save this as opaque.icl:

~~~icl
Module OpaqueModel {
 ScanInPort si[1:0]; ScanOutPort so[1:0]; SelectPort sel;
 ScanInterface client {
  Port sel;
  Chain first {Port si[1]; Port so[1]; DefaultLoadValue 4'b0;}
  Chain second {Port si[0]; Port so[0]; DefaultLoadValue 7'b0;}
 }
}
~~~

Save these commands as opaque.pdl:

~~~tcl
iPDLLevel 1
iReset
iScan client -chain first 4 -si 9 -so 0 -stable
iScan client -chain second 7 -si 5 -stable
iApply
iNote [iGetReadData client -chain first -hex]
iNote [iGetReadData client -chain second -bin]
iNote [iGetMiscompares client -chain second -dec]
if {[catch {iGetStatus -clear}]} {iNote {unknown failure count}}
~~~

Run:

~~~sh
icl-pattern-generation --icl opaque.icl --pdl opaque.pdl --top_module OpaqueModel --model_readback --output result.pdl
~~~

The notes are 0xX, 0bxxxxxxx, 0 and unknown failure count. The first chain's
cared comparison is unknown. The second chain has no cared expected bits, so
its miscompare mask is zero. Clearing an unknown count does not make it known.

Named parallel requests require an explicit valid chain. Each chain keeps its
own variable width and last queued request. Shared controls execute every chain:
after explicit reset, an omitted chain uses its sized default; later it can
reuse the last input declared -stable. Implicit reused lanes compare no bits
and add no source history. Omitting a chain whose next length is unknown
rejects the access. Empty Apply performs no opaque scan. The total executed
payload, including implicit lanes, is limited to 1048576 bits.

Only latest applied source requests supply raw histories and failure counts.
Reset cancels pending requests and preserves prior observations and counts.
Scalar -chain default remains the existing unnamed-chain alias; a declared
parallel chain named default is a distinct named chain. Prefixes qualify the
interface name. Values substituted by Tcl execute once, including across PDL
input files. A subscriber exception propagates after an accepted transaction
commits; its assigned outputs and state remain committed.

Physical PDL continues to reject raw iScan. Existing ordinary physical modeled
comparisons still count actual failures. Unsupported modeled parallel/multiple-
TAP raw routes, opaque children, raw merge, timed histories, applied callbacks
and device transport remain open. The existing mode note is retained for
generated-file compatibility. The prior Task151 compiled-help omission is now
covered by explicit raw-history and physical-comparison help assertions;
the earlier walkthrough's numeric-prose Minor finding remains deferred.

The two author examples have been executed; they are not an independent reader
assessment. Qualification and measured scale results follow below.


Task152 final qualification passes all 40 recorded gates: 14 native and
14 leak-enabled ASAN/UBSAN native suites, 62 suites/1008 tests for each
dynamic and static generator, 18 raw/12 opaque/16 TAP/16 standalone/16
data/17 PIO/71 CLI sanitizer tests, 30 affected format/tidy/strict C++17
checks and source packaging. All 4011 input hashes and 31 executable
hashes match. Qualified code is e36f0dc. Evidence is under
build/modeled-raw-readback-20261122/final/. Coverage item1 remains excluded
with unchanged thresholds.

The fresh independent review found 0 Critical, 2 Important and 2 Minor
issues. One test-first fix pass resolves both Important issues: invalid
IR/mixed-mode provider requests reject before publishing widths or changing
prior histories, and opaque resets record default availability without
expanding unused payloads. An 800-reset scalar/parallel test with a one-megabit
unused default failed at 1.356/1.301s before the fix and passed at
0.0558/0.0547s afterwards, with identical output. Existing diagnostic wording
is retained. All full post-fix checks pass; interrupted earlier runs remain
partial evidence. No rereview is claimed.

Deferred Minor findings are the binary TAP completeness check occurring
after inversion (no supported route misses a sample), an older Task151
sentence in inventory item6 that still says raw/interface/chain histories
remain open, and the prior Task151 walkthrough's numeric-prose spacing.
Current Task152 support and remaining topology limits are stated above.

The final 57-process scale matrix repeats matched parent/current
default/unused/queried controls three times. It covers 32/512-bit roots,
a 512-bit child interval inside a 520-bit root, a prepared 512-bit TAP
route, and 32 opaque chains of4096 bits each. Valid schedules agree after
removing observation notes. Queried CLI medians are root32=0.1393s,
root512=0.1177s, child512=0.1082s, TAP512=0.3688s and opaque32x4096=0.0823s.
Peak queried RSS ranges from68492 to124608KiB. Inputs, apply counts, commands,
environment, hashes and preserved executables are under
build/modeled-raw-readback-20261122/scale-final/. Reproduce with
tests/src/pattern/benchmark_modeled_raw_readback.py, providing current and
parent executable paths. Short process timings are noisy; no global speedup
or wider startup/topology qualification is claimed. Hardware timing, live
transport, vendor equivalence and independent reader usability remain unverified.

Task152 is integrated into master. Seven integration checks pass against all
4011 qualified source inputs and16 copied native products; the31-product
qualification manifest remains verified. The merged task branch is deleted,
and all117 stashes are preserved. Integration receipts are under
build/modeled-raw-readback-20261122/merged/.

## Task153: explicit DataRegister callback execution

Explicit direct or aliased DataRegister accesses with a matching ReadCallBack or
WriteCallBack now invoke their prewritten PDL procedure at iApply. The procedure
queues and applies real endpoint operations through the existing runtime. It may
use ordinary endpoint calls, merges, reservations and timing commands within
their existing supported limits. A pending body tail is applied before return.

For example, save this model as example.icl:

~~~icl
Module Example {
  DataInPort p[3:0];
  DataOutPort q[3:0] {Source p;}
  DataRegister value[3:0] {WriteCallBack Example store <D> <R>;}
  DataRegister observed[3:0] {
    ReadCallBack Example load <D> <R>;
    ReadDataSource p;
  }
}
~~~

Save these procedures and requests as example.pdl:

~~~tcl
iPDLLevel 1
iProcsForModule Example
iProc store {data reg} {iWrite p $data}
iProc load {data reg} {iRead q $data}
iWrite value 9; iApply
iRead observed 9; iApply
~~~

Run from the repository root after building the generator:

~~~sh
build/icl-pattern-generation --icl example.icl --pdl example.pdl \
  --top_module Example --output out/example.pdl --debug
~~~

The output drives p to 0b1001, applies it, records the declared logical
postcondition with iState value 0b1001, then compares q against 0b1001.
The postcondition is an assumption about callback-controlled storage outside ICL
connectivity. It does not prove that a user-written procedure implements its
declared protocol. Read expectations never become actual observations; callback
readback histories remain unsupported.

<D> receives a full declaration-order binary mask, with x for unrequested bits.
Repeated requests coalesce once per register and read/write mode; latest values
win by resolved bit identity, including inversion through aliases. An all-x read
still invokes its procedure. <R> is the register name relative to the actual
ancestor module instance named by the callback namespace, without indices.
A namespace may include a PDL subnamespace; module parameters come from that
ancestor instance. Quoted literal "<D>" and "<R>" remain literal arguments.
Procedure definitions are looked up at invocation time, so ordered replacement
and loading between files work.

No procedure runs when a request is queued. iReset cancels the pending requests,
including ones whose procedure has not been loaded. Every binding and current procedure argument count is preflighted
before the first callback in a group. An invocation error preserves completed
effects but blocks iApply and Finish until iReset discards unfinished endpoint
requests; retry by requeueing after that reset. Preflight errors can be corrected
and retried without reset because no procedure has run. Recursive register/mode
callback cycles reject with their existing output artifacts preserved. Implicit callback tails also
share the existing 128-level procedure budget. Reset recovery closes unwound
merge boundaries in the internal operation stream, preserving completed work
while reset discards the unfinished queue.

Caller Apply groups must contain only callback requests. Mixed callback/ordinary
or raw groups, aliases spanning both kinds, AccessTogether callback aliases, -together, and a callback Apply
inside an active iMerge reject before invocation. Logical callback storage
cannot be taken or released, including by a whole-instance reservation; ordinary
endpoint resource checks still apply. Automatic callbacks needed while solving
downstream access goals remain open.

--model_readback supports actual ordinary endpoint observations during explicit
write callbacks, as described below. Invoked read callbacks and --physical_pdl
callbacks still reject before artifact publication. Reset-canceled requests remain
valid. Native applications can supply actual readback providers; no hardware
transport is certified by modeled execution.

--debug preserves callback declarations and original module scopes in flat ICL.
Generated PDL contains endpoint operations and declared write postconditions.
--local_reset hoists actual endpoint reset prerequisites before the whole
original callback request/caller group, preserving procedure bytes, prefixes and
once-only Tcl arguments. The emitted pattern follows the same verified release
order as the rewritten source. Source, flat ICL, generated PDL and reset-source
replay are compared in the focused tests.

The maintained scale driver is tests/src/pattern/benchmark_callback_execution.py.
It measures 64/256/1024 four-bit targets with four Apply groups, matched
parent/current callback-free and unused-declaration controls, and flat/generated
replay. Commands, elapsed time, peak RSS, inputs and output hashes are recorded
under build/callback-execution-20261123/. Existing runtime limits remain
unchanged; no vendor-equivalence, live-device or timing-signoff claim is made.


The scale run passes 51 processes. At 64/256/1024 callback targets with four
Apply groups, median source wall times are 0.2523/2.0236/25.9599 s and peak RSS
is 77656/83520/107056 KiB. Matched parent/current callback-free controls at1024
targets take0.0535/0.0550 s; unused-declaration controls take0.0634/0.0643 s.
The controls each access one four-bit endpoint per Apply. The driver's
operand_bits_per_apply/applies fields describe the original callback fixture;
the known Minor labeling issue remains deferred, and generated replay has its
own expanded Apply structure. Input/binary/output hashes and exact commands
are retained in scale/results.json.

Task153 checkpoint; [Task154](#task154-primary-solver-state-in-callback-heavy-models)
subsequently improves the same primary-scheduling case.
These measurements demonstrate1024 targets/four callback groups within the
existing limits. End-to-end PIO scheduling grows faster than target count in
this workload; no linear runtime or speedup is claimed. Larger routine batches
or a tighter throughput requirement call for measured scheduler work under item10
before extending this demonstrated scale.

A separate maintained benchmark, tests/src/pattern/benchmark_callback_alias.py,
covers one wide alias spanning4096/16384 one-bit callback targets. Before the
review fix,16384 targets use366828 KiB RSS and fail its256 MiB budget. Requests
now retain source indices into the immutable pending snapshot instead of copying
the full operand for each target. Six corrected runs pass: at16384 targets,
median wall time is0.3322 s and peak RSS163376 KiB. This isolates request/storage
cost with empty callback bodies and does not measure hardware protocol work.
Receipts are in wide-alias-test-red/ and wide-alias-green/.

The fresh whole-branch review found0 Critical,7 Important and1 Minor findings.
The single fix pass adds regressions for preflight arity, AccessTogether
obligations, callback invocation context across inner resets, unwind/retry of
open merges, fully qualified ancestor binding, wide-alias memory and implicit
tail recursion limits. Forty focused callback methods and native runtime/readback
checks pass. All43 post-fix qualification gates pass:14 native and14
leak-enabled ASan/UBSan native suites,63 suites/1048 tests per dynamic/static
generator,10 affected sanitizer CLI suites,24 format/tidy/strict C++17 checks,
and source-distribution packaging. Hash checks verify4014 inputs and31 products,
including the exact executable used by both scale drivers. The runnable guide
example passes source/flat/generated byte replay; this is author verification,
not an independent reader test. The interrupted pre-review run is retained as
partial evidence, not full qualification. Receipts are in final/.


Task153 is integrated into master. Seven integration checks pass against4014
matching source inputs and16 copied qualified native executables, covering
Runtime/readback, dynamic/static callbacks, typed callback declarations, raw
histories and the71-method CLI suite. All117 stashes are preserved. Merge
receipts and executable hashes are in
build/callback-execution-20261123/merged/receipt.json. Broad item4 remains pending
for the callback cases listed above; this increment does not close that item.


## Task154: primary solver state in callback-heavy models

Repeated endpoint Applies in a model with many unused callback DataRegisters
previously rebuilt symbolic values for all of those registers. Task154 keeps a
conservative topology index so primary-input solves omit irrelevant held callback
storage from their symbolic snapshots. Concrete state, callback postconditions,
manual assumptions and histories stay complete.

Only unreferenced callback storage without WriteDataSource, reset association or
ResetValue is eligible. Aliases, logic/mux/one-hot inputs, reset consumers and
interface/TAP references keep their state. Explicit goals, reads, end-state
requirements and reservations also retain the values they need. Values are read
fresh for each solve, including after assumptions, resets and scheduler copies.
The existing solver objectives, event/feedback/reset settlement, concrete replay
and actual-observation paths are reused. Model and runtime limits stay unchanged.

On the same GCC16.2.1/Linux WSL2 environment, three matched source repeats with
four caller Apply groups and four-bit targets give:

| Callback targets | Parent median s | Current median s | Speed ratio | Current peak RSS KiB |
|---|---:|---:|---:|---:|
| 64 | 0.2754 | 0.1659 | 1.66x | 76236 |
| 256 | 2.0898 | 0.5743 | 3.64x | 81688 |
| 1024 | 26.2120 | 2.6123 | 10.03x | 106920 |

The1024-target case meets the<=10s,>=2x parent speed and256MiB budget. The report
contains60 processes: parent/current plain, unused-declaration and callback source
controls plus current flat/generated callback replay. Every pattern matches its
parent control exactly. A20-process alternating256-target follow-up also retains
exact hashes; plain medians are0.0598/0.0701s and unused-declaration medians
0.0381/0.0354s (parent/current). These small control timings vary, so the speed
claim applies to the measured callback workload above.

The maintained driver reuses the existing callback input builder and records
mode-specific requested bits, actual emitted Apply/write counts, environment,
commands and input/output/executable hashes. Run from the repository root:

~~~sh
python3 tests/src/pattern/benchmark_primary_callback_state.py \
  --binary build/icl-pattern-generation --parent /path/to/parent/icl-pattern-generation \
  --output build/primary-callback-state-scale --repeats 3
~~~

The original wide-alias memory budget and guide example also pass. Nine compatibility methods plus a benchmark-repeatability regression
 cover fresh postconditions, known/unknown held data,
aliases, logic/mux/one-hot consumers, reset relationships, ordinary ownership,
end-state requirements and nested/flat identities. A native scheduler test checks
complete concrete state, copy independence, fresh assumptions, reset behavior,
preserved masks and actual observations.

This increment does not optimize referenced/aliased/resettable storage or add
automatic callbacks, new scheduling topologies or a hardware transport. Other
solver/network validation costs remain. Broad item10 and the other pending
inventory requirements stay open.

The fresh independent review found no scheduler correctness blocker and verified
30 additional multi-route parent/current cases. Its one Important finding was
a benchmark false-green on stale output. A regression first proved that a
successful no-output candidate could reuse old artifacts; the single fix pass
gives every measured process a fresh retained output/debug directory and requires
fresh pattern and flat model files. The corrected60-process run passes with the
same qualified executable. Original valid measurements remain in scale/; final
fresh-directory measurements and exact commands are in scale-final/results.json.

All46 qualification gates pass:14 native/14 leak-enabled ASan/UBSan native suites,
64 suites/1058 tests per generator,13 affected sanitizer CLI modules, six changed
C++ format/tidy/strict C++17 checks and source-distribution packaging. Hash checks
verify4016 source inputs and31 products, including the benchmark executable.
The new10-method primary-state module, including stale-output rejection, passes
on dynamic/static and sanitizer generators. Guide source/flat/generated replay,
six wide-alias budget processes and20 alternating control processes also pass.

One review Minor remains deferred: the ownership negative test checks failure and
preserved output, but could also pass on a future parser/setup failure; the
reviewer confirmed its current intended diagnostic is reserved storage =0011.
Full-suite execution is author-run evidence; performance outside this workload
retains its separate acceptance criteria. Qualification, review dispositions and
hash receipts are under build/primary-callback-state-20261123/final/.


Task154 is integrated into master. Seven integration checks pass against4016
matching inputs and16 copied qualified native executables: scheduler state/copy
snapshots, native readback, dynamic/static primary-state tests, explicit
callbacks, observation state and the71-method CLI suite. All117 stashes remain.
Integration receipts are in
build/primary-callback-state-20261123/merged/receipt.json. Broad item10 remains open.


## Task155: modeled parallel raw scans

One explicit root Client ScanInterface can now execute named fixed disjoint
ScanRegister chains under common direct root controls or their existing
virtual defaults. Common selects may bind indexed bits or multiple direct
root SelectPorts; unused vector bits retain their state. Each group emits one iScan per field and one iApply. Names,
including a chain literally named default, stay independent when requests are
replaced or spread across PDL files.

Save the following as model.icl:

~~~icl
Module Chains {
 ScanInPort si[3:2];
 ScanRegister a[3:0] {ScanInSource si[3]; CaptureSource a; ResetValue 0;}
 ScanRegister b[1:0] {ScanInSource si[2]; CaptureSource b; ResetValue 0;}
 ScanOutPort so[5:4] {Source a[0], b[0];}
 SelectPort sel; CaptureEnPort ce; ShiftEnPort se; UpdateEnPort ue; TCKPort ck;
 ScanInterface client {
  Port sel; Port ce; Port se; Port ue; Port ck;
  Chain first {Port si[3]; Port so[5];}
  Chain second {Port si[2]; Port so[4];}
 }
}
~~~

Save this as source.pdl:

~~~tcl
iPDLLevel 1
iReset
iScan client -chain first 4 -si 9 -so 15
iScan client -chain second 2 -si 2 -so 3
iApply -together
iNote [iGetReadData client -chain first -dec]
iNote [iGetReadData client -chain second -dec]
iNote [iGetStatus -clear]
iScan client -chain first 4 -si 6 -so 0
iScan client -chain second 2 -si 1 -so 0
iApply
iNote [iGetReadData client -chain first -dec]
iNote [iGetReadData client -chain second -dec]
iNote [iGetStatus]
~~~

Run the modeled endpoint:

~~~sh
icl-pattern-generation --icl model.icl --pdl source.pdl \
  --top_module Chains --model_readback --debug --output result.pdl
~~~

The six numeric notes are 0, 0, 6, 9, 2, 3. The first capture returns the real
reset values despite the deliberately different expected values15/3. The
second capture returns the preceding loaded9/2, while the update loads6/1.
The status counts cared mismatching bits and -clear resets that count.
These are concrete model observations; this example does not access a device.

Unequal fields share capture, the maximum field length's shift clocks and
update. Shorter fields have leading zero shift-in clocks and ignored trailing
shift-out clocks, following the ordinary native lane convention. Actual
returns are the first field-length samples after capture. Input/output
inversion, indexed ports, ascending registers, multiple registers per field,
constant/unknown capture data and static aliases retain their native meaning.
The implementation uses the existing concrete capture and complete-shift
engine; it does not allocate a field-count by maximum-length matrix.

When a fixed field is omitted, generated output reuses its current controlling
fill, clears expected comparisons and marks its static input stable. This
preserves its storage. Only explicit requests create source raw history or
claim assigned storage; implicit output commands do not fabricate source
observations. Raw histories and ordinary register histories remain separate.
Known explicit assignments refresh the existing held-storage value policy;
unknown assignments cannot replace protected numeric storage. Existing root
port side effects and mixed ordinary preparation remain: mixed preparation
with iApply -together rejects.

All fields validate before a raw group changes state, fill or assigned storage.
A failed group publishes no observation; a subscriber exception after commit
retains the completed hardware state. Numeric operands validate even for
replaced/canceled requests. iReset can cancel a valid queued length before
replay; an executed field must match its modeled physical length, and actual
raw replay requires explicit iReset first. Effective aggregate explicit and
implicit fields are limited to1,048,576 bits before payload completion.

This increment requires every modeled ScanRegister to belong to exactly one
static field, unique SI/SO endpoints, inactive direct/virtual resets and
unconditional common controls. It rejects shared or hidden scan storage,
muxed/gated/dynamic routes, hierarchical/child/TAP clients, event-driven
DataRegisters and iApplyEndState. Held/event-free data and combinational
capture sources are supported. Physical raw PDL, modeled merged/timed
readback and live transport retain their existing guards. Wider item6/8
topologies and the rest of the unfinished inventory remain pending.

The vector C++ API ApplyRawScans returns canonical scan commands only; its
caller emits the original Apply. The previous single-result APIs reject named
parallel requests before execution. Scalar/child/TAP calls delegate to the
existing scalar implementation.

The dedicated CLI module has16 methods, with native per-clock differential,
rollback, copy, implicit-fill, ownership and postcommit callback checks.
Ninety-three fresh measured processes cover8/32/64 chains at32/512/4096 bits and a
skewed262,144-bit field with63 short fields, each in source/flat/generated
forms with scalar and opaque controls and an additional skewed omitted-field case. The prior binary rejects modeled
parallel raw, and is recorded as a negative control. Commands, input/product
hashes, time and RSS live under build/modeled-parallel-raw-20261123/scale.
The driver enforces45s/process and512MiB for these representative cases.

All47 qualification gates pass:14 native/14 leak-enabled sanitizer suites,
65 suites and1,074 tests per generator,14 sanitizer CLI modules,15 changed-code
format/tidy/strict-C++17 checks and source packaging. All4,018 input and31
product hashes are verified. Mechanical braces/constant-name fixes leave
all16 optimized products byte-identical to the fully tested and measured
post-fix executables; changed debug sanitizer products received fresh checks.
The single independent review's four Important findings are fixed in one
test-first pass; its two Minor findings remain deferred, with no rereview.

The independent final review found four Important issues: retained oversized
implicit operand buffers, obsolete native replacement lengths checked too
early, unbound AccessLink acceptance, and rejected supported common-select
bits. All four have RED→GREEN native coverage and pass the expanded CLI
module in one fix pass. Two Minor findings remain deferred: implicit
controlling fill can resolve previously unknown storage, and equal-width
benchmark lanes share the same payload, so that scale oracle does not prove
absence of lane permutations. The native capacity regression covers omitted
short fields beside a wide explicit field. Full post-fix qualification passes; no rereview is used.

The corrected executable's93 scale/control processes peak at
0.2239s and
143040KiB. Its preserved1,024-target/four-group callback source
median is2.5877s against the original reference's26.5808s,
within10s;60 callback replay processes,20 alternating controls and six
wide-alias processes also pass, for179 measured processes total. The wide
alias peak163108KiB remains below256MiB. This proves the recorded workloads,
not broader topology or performance acceptance. Qualification receipts are in
build/modeled-parallel-raw-20261123/final; integration remains pending.

Task155 is integrated into master. Seven integration checks pass against4018
matching inputs and16 copied qualified executables: native parallel/copy/
ownership and readback, dynamic/static parallel CLI, scalar raw histories,
resource reservations and the71-method main CLI suite. All117 stashes remain.
The receipt is build/modeled-parallel-raw-20261123/merged/receipt.json.
Broader inventory items6/8 remain open.


## Task156: default model and source round-trip audit

Bare control-output declarations such as ToSelectPort forwarded; now retain
a valid semicolon form when their bodies are empty. Attributes and explicit
or inferred Sources retain their bodies, and other empty-port spellings
remain unchanged. Native cases cover supported To* kinds in root and child
scopes. The following previously failed while producing associated flat ICL.

Save model.icl:

~~~icl
Module Chains {
 ScanInPort si;
 ScanRegister r[3:0] {ScanInSource si; CaptureSource r; ResetValue 0;}
 ScanOutPort so {Source r[0];}
 ToSelectPort forwarded;
}
~~~

Save source.pdl and run it through the concrete model endpoint:

~~~tcl
iPDLLevel 1
iReset
iWrite r 9; iApply
iRead r 9; iWrite r 6; iApply
iNote [iGetReadData r -dec]
iRead r 6; iApply
iNote [iGetReadData r -dec]
~~~

~~~sh
icl-pattern-generation --icl model.icl --pdl source.pdl \
  --top_module Chains --model_readback --debug --output result.pdl
~~~

The notes are the real captured9 and6, with three nonempty Apply groups.
Original, associated-flat and generated-PDL replay produce identical pattern
bytes. This is author-verified modeled execution; no device was accessed.

Undefined RefEnum declarations now identify the missing scoped name through
the existing source/node diagnostic, for example unknown RefEnum Missing.
A missing table does not escape as map::at. Valid forward/scoped/specialized
enum tables, symbolic reset/default values and existing kind/width errors
retain their behavior. Failed generation preserves the previous output file.

### Run the maintained corpus model audit

~~~sh
make model-corpus-check
~~~

This builds the existing shared-frontend corpus inventory and the new public
model inventory. It writes build/model_source_corpus.json and
build/model_source_audit.json. BUILD_DIR selects both helper executables;
corpuscheck includes this target and the related producer/protocol/build
selection regression tests.

The current inventory is2562 ICL files, with1228 parser-rejected inputs still
checked for byte provenance. The1334 parser-accepted files contain1765 module
declarations and1763 effective default module identities after existing
per-file ordered overrides. Of these default models:

| Actual outcome | Count | Meaning |
|---|---:|---|
| Required canonical roundtrip | 1016 | Real SerializeIcl, flat load/elaboration and repeated SerializeIcl bytes agree |
| Explicitly unsupported serialization | 56 | 44 unbound AccessLinks, seven PortGroups and five LaunchEdge properties |
| Original default elaboration failure | 691 | Reported actual errors; many fixtures are intentionally incomplete or require dependencies/parameters |

PASS requires every pinned canonical model, exact unsupported boundaries,
complete current file/module coverage and no internal map::at diagnostics.
It does not assert that all parser-accepted grammar examples are runnable
models, or prove physical/vendor equivalence. Unbound AccessLinks are checked
before external model/BSDL binding; the report never invents a binding,
dependency or parameter value. Valid instance specializations reached by a
selected module use the existing elaborator.

Each registry identity combines corpus-relative file and effective qualified
module key, preserving namespaces and ordered overrides. Parser fixture
module_keys supplement the existing unchanged definition hashes. Source
collection and hash validation include rejected files and the same generated
tree exclusion as the parser audit. A changed, missing or newly added input,
omitted/duplicated backend result, failed/empty process, canonical failure or
changed unsupported boundary fails the audit. Failure writes a fresh failed
report, replacing any old success. One batch native process loads each file
once and visits its effective default tops; it does not launch per module.

The explicit registry is tests/cases/model_source_acceptance.json. Update
required/unsupported identities deliberately after proving a newly supported
model and its canonical replay. An unsupported case becoming canonical also
requests a registry update, so acceptance changes are reviewable.

### Sanitizer audit and measured scope

For a separate sanitizer build, use the same build directory and compiler
flags for building and running both helpers:

~~~sh
make -j2 BUILD_DIR=build/model-audit-san CXX=clang++ \
  CXXFLAGS='-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -MMD -MP' \
  build/model-audit-san/icl_corpus_inventory \
  build/model-audit-san/pattern_netlist_inventory
ASAN_OPTIONS=detect_leaks=1:allocator_release_to_os_interval_ms=500 \
UBSAN_OPTIONS=halt_on_error=1 \
make BUILD_DIR=build/model-audit-san CXX=clang++ \
  CXXFLAGS='-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -MMD -MP' \
  model-corpus-check
~~~

The500ms setting releases unused allocator pages to the OS. The installed
runtime still retains its256MiB global and1024KiB thread quarantine, redzones,
leak detection and undefined-behavior checks. The default5,000ms release
interval retains about579MiB across this roughly4s batch; it is not the
qualified512MiB configuration. Releasing the original Netlist early had no
measured peak benefit, so that trial was removed.

Three fresh normal and three configured leak-sanitizer batch runs pass the
same10s/512MiB bounds. Post-fix normal peak is104436KiB/max0.9734s;
sanitizer peak is485176KiB/max4.3360s. The input size, all input/registry/helper/report hashes,
commands, runtime flags and platform/compiler versions are recorded in
build/model-source-audit-20261123/batch-scale/results.json. These measurements
qualify the maintained1334-file/1763-module workload; broader inputs require
their own evidence.

Eight new generator methods, native source/flatten/default checks, eight
producer and13 protocol methods pass, including direct parent RED
evidence for12 broken serializations and14 internal enum messages. All52 final qualification gates pass:14 native/14 leak-enabled sanitizer
suites,66x1082 methods per dynamic/static generator,15 C++ quality checks,
normal/selected-sanitizer corpus and producer/protocol checks, packaging and
4024 input/36 product hashes. One final independent review found five
Important issues, all fixed in one test-first pass; one Minor is deferred. Remaining source
specializations, model semantics, binding/event support and wider vendor
round-trips still belong to unfinished inventory11/12 and related items.


Task156 independent review found five Important audit-evidence gaps. The
single fix pass now invalidates reports before prerequisite builds/parser/
policy, rejects malformed registry objects and inconsistent parser declaration
metadata, rejects contradictory success/error records, matches complete
unsupported diagnostics, selects both test-runner helpers by build variant,
and checks source file sets/digests plus the helper digest before and after
execution. A failed or changing run cannot keep a prior success under these
handled paths. Post-fix full qualification passes; no rereview is used.

One Minor remains deferred: the standalone native helper checks its output
stream before its final buffered flush, so a late write failure can still
exit0. The Python audit rejects incomplete records. The example was checked
by its author; independent reader usability and general peak-live-application
memory were not measured. Allocator exit totals explain this workload's
retention pattern without establishing a live-heap bound. Broader physical/
vendor equivalence and inventory11/12 completion remain pending.

Post-fix selected-parser producer8/protocol13 and normal corpus/three Make
lifecycle/selection tests pass. Their exact source/tool/log hashes are retained
in review-fix/focused-receipt.json so the final executor can replace earlier
pre-fix gates without pretending they covered the corrected audit. All52 final
gates and six post-fix corpus performance measurements now pass.

The preserved callback budget also passes: the1,024-target/four-group source
median is2.5622s against the original reference's26.2754s,
within10s. Sixty callback scale/replay and20 alternating controls retain exact
patterns; six wide-alias runs peak162912KiB, below256MiB.
Together with six corpus batches these are92 qualified measured processes.
The unchanged C++ full suites and exact post-fix corpus/protocol/producer gates
are bound to actual current source/tool/log hashes; earlier gates are retained
separately. Receipts are in build/model-source-audit-20261123/final.
Integration and branch cleanup remain pending.

Task156 is integrated into master. Nine integration checks pass on4024
matching inputs and19 copied qualified native tools/suites: netlist/flatten/
state, dynamic/static source-audit CLI, complete corpus policy/default audit
and producer8/protocol13/Make3, source literals/typed ports and71-method main
CLI. All117 stashes remain. Receipt:
build/model-source-audit-20261123/merged/receipt.json. Broader11/12 remains open.

### Accepted legacy scan-output launch declarations

Task157 preserves the existing compatibility form in canonical flat ICL:

    ScanOutPort so { Source r[0]; LaunchEdge Falling; }

The frontend still accepts only Falling on ScanOutPort. Flattening retains
the property, source bit order/inversion, Enable, attributes and original
instance ownership, including repeated children. Source, flat and generated
PDL replay use the existing native model; preserving this declaration does
not add a physical launch-edge timing model or certify hardware timing.

The full maintained default audit now has 1017 canonical identities, 55
explicit unsupported serializations (44 unbound AccessLinks and 11 PortGroups)
and 691 original elaboration failures across 1334 accepted files and 1763
effective modules. One former LaunchEdge-only model is newly canonical.
Four legacy SIB models advance to their existing PortGroup boundary and are
still unsupported. The expectation registry records each exact outcome.
Broader inventory items 11/12 remain open.

Task157 qualification passes all 53 checks after the single Important review
fix: 14 native/14 sanitizer native suites, 67 suites/1085 test methods per
generator, selected normal/SAN corpus and producer8/protocol13/Make3 checks,
six C++ quality checks and source packaging. Exact 4025 source inputs and 36
products are verified in build/legacy-launch-edge-20261123/final/receipt.json.

Six fresh corpus measurements pass the unchanged 10s/512MiB limits. Normal
maximum: 1.0726s/104396KiB. Configured leak-SAN
maximum: 4.3357s/485176KiB, with the established
500ms unused-page release and unchanged quarantine/leak/UB checks. Commands,
environment and hashes: build/legacy-launch-edge-20261123/batch-scale/results.json.

The independent review reported 0 Critical, 1 Important and 2 Minor findings.
The Important silent-loss case is fixed test-first before the full fresh run;
specialized native serializers now reject an illegal property while legitimate
Enum members named LaunchEdge remain valid. Deferred Minors: the earlier
operator table retains Task156 counts, and the native fixed-point loop reparses
the same canonical text. Physical timing/vendor equivalence remains unverified;
coverage item1 remains excluded. Master integration is pending.

Task157 is integrated into master. Eleven integration checks pass on 4025
matching input hashes and 19 copied qualified native products. All 117 stashes
remain. Receipt: build/legacy-launch-edge-20261123/merged/receipt.json.

### Legacy named PortGroup annotations

Task158 resolves the frontend's existing legacy compatibility form:

    Module Example {
      Parameter Width = 4;
      PortGroup pins {p[$Width-1:0], out}
      DataInPort p[3:0];
      DataOutPort out[3:0] {Source p;}
    }

Use the existing generator command and installation described above; no new
option or dependency is needed. For example, queue the following PDL:

    iWrite p 9; iApply; iRead out 9; iApply

The existing corpuscheck target runs the maintained model audit. Run focused
replay cases with:

    python3 tests/src/pattern/test_legacy_port_groups.py build/icl-pattern-generation

A PortGroup lists actual local ports, with optional index/ranges. Forward
declarations, comments, keyword names, reversed ranges and repeated members
retain their bit order. Segmented port functions keep their existing typed
behavior. Flattening rewrites references to physical flat port declarations;
origin restoration recovers member/group instance paths and port kinds.
Named legacy PortGroups are distinct from the implementation's segmented
port-function groups.

These declarations annotate membership. They do not drive data, own storage,
create scan interfaces or add PDL access/reservation targets. Undefined or
nonport members, invalid selections and malformed native/origin metadata fail
with source/group context. No missing port or binding is invented. A list is
bounded by the existing 1,048,576-bit resolver limit, with a 4,194,304-bit
aggregate member-expansion limit across all legacy groups.

The current complete default audit has 1024 canonical identities, 44 unsupported
unbound AccessLinks and 695 original elaboration failures, across 1334 accepted
files and 1763 effective module tops. Seven models are newly canonical: six
former PortGroup serialization failures plus the commented LSP fixture, whose
members were previously mistaken for properties. Five grammar-only fixtures
with undefined members now report original elaboration errors. Earlier Task156
counts describe that task's baseline. This is source/model acceptance evidence;
broader inventory11/12 and physical timing/vendor equivalence remain open.

Task158 passes all 54 post-fix qualification checks: 14 native/14 sanitizer
native suites, 68 suites/1094 test methods per generator, nine focused CLI
methods on all variants, normal/selected-SAN corpus and producer8/protocol13/
Make3 checks, 21 C++ quality checks and source packaging. Exact 4026 input
hashes and 36 products: build/legacy-port-groups-20261123/final/receipt.json.

The independent review reported 0 Critical, 3 Important and 0 Minor findings.
One test-first pass fixes all three: valid annotations preserve scalar/parallel
opaque and modeled raw scans and their actual/unknown histories; incompatible
native hardware payloads reject before register collection or metadata;
unindexed member ports require canonical scalar bounds/index zero. Full fresh
qualification passed after those fixes; no rereview was used.

All108 fresh final scale processes pass10s/512MiB at1/64/1024groups x64memberbits,
vector/segmented modes, three repeats each on normal/SAN tools, with identical
control/source/flat/generated output. Max normal: 0.1161s/80032KiB;
max leak-SAN: 0.3157s/159224KiB. Six fresh full-corpus runs pass
the same limits: normal 1.1241s/104352KiB, configured leak-SAN
5.2867s/473028KiB with the established500ms unused-page release and
unchanged quarantine/leak/UB checks. Exact commands, environment/input/tool/
output hashes and RSS are recorded in scale/results.json and
batch-scale/results.json under build/legacy-port-groups-20261123/.
Pre-review measurements are retained separately and are not final evidence.
The guide example has three equal nonempty replays with the final binary;
it is author-verified, with no independent reader claim. Physical timing,
vendor equivalence and whole11/12 closure remain pending; coverage1 is excluded.
Master integration is pending.

Task158 is integrated into master. Thirteen integration checks pass on 4026
matching input hashes and 19 copied qualified native products. All 117 stashes
remain. Receipt: build/legacy-port-groups-20261123/merged/receipt.json.

## Actual endpoint observations in write callbacks

With --model_readback, explicit WriteCallBack procedures can read actual modeled
ordinary endpoints, including reads inside the procedure. Expected iRead data
only selects comparisons; it never supplies the returned value. For example:

    Module Chains {
      DataInPort p[3:0];
      DataOutPort q[3:0] {Source p;}
      DataRegister value[3:0] {WriteCallBack Chains store <D>;}
    }

    iPDLLevel 1
    iProcsForModule Chains
    iProc store {data} {iWrite p $data}
    iWrite value 9; iApply
    iRead q 0; iApply
    iNote [iGetReadData q -dec]
    iNote [iGetMiscompares q -bin]
    iNote [iGetStatus -clear]; iNote [iGetStatus]

Run the existing executable with --icl model.icl --pdl source.pdl
--top_module Chains --output result.pdl --debug --model_readback.
The notes are 9, 0b1001, 2, and 0. The global status counts the actual
ordinary endpoint comparisons once, including comparisons in write procedures.
PIO, selectable DataRegister and serial ScanRegister endpoints use their existing
observation model, scopes, reservations, masks and history rules.

Callback registers represent abstract logical storage outside ICL connectivity.
Their requested <D> and generated iState postconditions are useful to goal
generation, but remain unknown in the independent observation model. A no-op
procedure cannot fabricate observed data. Outputs sourced from callback storage
stay unknown, even after a declared ResetValue or an iState assignment through
a mixed or inverted alias. Reset metadata is masked in a private observation
network so asynchronous reset propagation during ordinary frames also remains
unknown; the goal-generation network keeps its declared reset semantics.
This also changes older declaration-only modeled histories: callback-typed
storage reports unknown after a logical iState or ResetValue assignment.
Explicit manual -LastReadValue/-LastMiscompareValue assumptions keep their
existing meaning; these are supplied assumptions.

Invoked ReadCallBack histories remain unsupported, including invocation inside
a write callback or a read body that resets itself. A read request canceled by
iReset before invocation is allowed, including cancellation inside a write body.
An ordinary endpoint iRead in a write body is supported. Unexpanded logical
callback data accesses are rejected by the native observer.

Source, flat-model, generated and split-file replay use the same callback
metadata to mask abstract storage. --local_reset verifies rewritten source with
the same observer and preserves the modeled/no-live-device label in the final
pattern. Physical callbacks, timed histories, automatic downstream dispatch,
mixed callback/ordinary caller groups, together and merged callback dispatch
retain their existing limits. A model-backend error remains terminal for that
observer; procedure failures retain the existing reset-and-requeue recovery.

Reproduce the bounded observation benchmark with:

    python3 tests/src/pattern/benchmark_callback_endpoint_readback.py \
      --binary build/icl-pattern-generation --parent /path/to/parent-executable \
      --sanitized /path/to/sanitized-executable --output /tmp/callback-observations

Generated measurements replace the four literal result notes with fresh getters
at the corresponding group boundaries; their canonical output must still match
the source result. This forces the lazy observer to execute on generated
transactions. It checks actual four-group values, fresh source/flat/generated output, three
repeats at 64/256/1024 targets and sanitizer64, and matched callback-free, unused
and observer-off controls. Each run must stay within10s and512MiB.

Task159 passes55 post-fix qualification gates:14 native/14 leak-enabled sanitizer
native suites,69 suites/1117 methods for each normal/static generator,23 new
endpoint-observation methods in each variant and sanitizer,18 changed-C++ checks,
normal/selected-sanitizer corpus and producer/protocol/Make checks, source
packaging,4028 maintained-input hashes and36 qualified products. The corpus
retains1024 canonical/44 declared-unsupported/695 original elaboration failures
among1763 effective modules; this increment does not claim wider source support.

Independent review found0 Critical,5 Important and0 Minor issues. One test-first
pass fixes all five: reset-frame abstract-state contamination, missing native
read invocation before body/getters/errors, outer queued-read cancellation across
an inner write Apply, duplicate labels after generated-prefix reset repair, and
a generated benchmark that formerly replayed literal notes without observing.
All pre-fix qualification/performance is archived separately.
Updating seven historical Python expectations changed only that test file.
Seventeen completed gates and52 generator modules are retained with an explicit
hash proof covering all36 unchanged products and every other4027 input; the
updated module, remaining modules, whole static generator and sanitizer checks
run against the current expectations.

All90 corrected observation/control measurements,60 preserved callback runs,
six wide-alias runs and six normal/configured-sanitizer corpus batches pass.
Maximum observed normal runtime/RSS: 6.5251s/148876KiB; sanitizer64: 1.3415s/513792KiB.
Generated measurements execute four fresh getters; source/flat/generated bytes
and matched parent/current controls agree. The16K alias peak is163676KiB within256MiB.
Measurements include ordinary background build/test activity; no new speedup is
claimed. Exact commands, binary/input/output hashes and environment are retained
under build/callback-endpoint-readback-20261123/{final,scale,callback-scale,
wide-alias,batch-scale}/. Strict coverage item1 and broader inventory limits stay
unchanged. The exact guide example passes three matching source/flat/generated invocations
with9/0b1001/2/0 result notes; this is author verification, not an independent
reader study.

Task159 is merged into master;17 integration checks pass on4028 identical input
hashes/19 copied qualified native products, with117 stashes preserved. Receipt:
build/callback-endpoint-readback-20261123/merged/receipt.json.


## One fixed-width opaque child on a scalar scan route

One opaque leaf can sit between ordinary ScanRegisters on a complete static
scalar root route. Its scalar ScanInterface must declare a numeric, explicitly
sized DefaultLoadValue, and each request must use that declared width. Pure
scalar wrappers are supported. The root controls must describe a supported
native capture/shift/update route; child selection must follow the root
SelectPort. Unrelated unused opaque branches keep their previous backend.

Save this as opaque-child.icl:

~~~icl
Module ChildLeaf {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 ScanOutPort so;
 ScanInterface local { Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port reset; DefaultLoadValue 8'b0; }
}
Module Top {
 ScanInPort si; SelectPort sel; CaptureEnPort ce; ShiftEnPort se;
 UpdateEnPort ue; TCKPort tck; ResetPort reset;
 ScanRegister head[2:0] { ScanInSource si; CaptureSource head; ResetValue 3'b0; }
 Instance child Of ChildLeaf {
  InputPort si = head[0]; InputPort sel = sel; InputPort ce = ce;
  InputPort se = se; InputPort ue = ue; InputPort tck = tck; InputPort reset = reset;
 }
 ScanRegister tail[4:0] { ScanInSource child.so; CaptureSource tail; ResetValue 5'b0; }
 ScanOutPort so { Source tail[0]; }
 ScanInterface client {
  Port si; Port so; Port sel; Port ce; Port se; Port ue; Port tck; Port reset;
  DefaultLoadValue 16'b0;
 }
}
~~~

Save this as opaque-child.pdl:

~~~tcl
iPDLLevel 1
iProcsForModule Top
iReset
iWrite head 5; iWrite tail 19; iApply
iScan child.local 8 -si 0xa5 -so 0x3c -stable; iApply
iNote [iGetReadData child.local -bin]
iRead head 5; iRead tail 19; iApply
iNote [iGetReadData head -dec]; iNote [iGetReadData tail -dec]
~~~

Run the source, exported flat model and generated PDL:

~~~sh
build/icl-pattern-generation --icl opaque-child.icl --pdl opaque-child.pdl --top_module Top --output example/source.pdl --debug --model_readback
build/icl-pattern-generation --icl example/debug/Top.icl --pdl opaque-child.pdl --top_module Top --output example/flat.pdl --model_readback
build/icl-pattern-generation --icl opaque-child.icl --pdl example/source.pdl --top_module Top --output example/generated.pdl --model_readback
~~~

All three outputs contain result notes 0bxxxxxxxx, 5 and 19. The leaf's
expected 0x3c comparison and requested 0xa5 input do not supply observed data:
the opaque return stays unknown, while the surrounding registers retain their
actual native values. This example was author-checked in all three forms.

A nonempty ordinary parent access physically traverses the opaque interval.
Every preparation CSU therefore needs its prior stable payload or a reset
default. A current explicit child/full-root request is consumed only on its raw
CSU, even when ordinary writes share its iApply. After a non-stable raw request,
later implicit traversal rejects until another explicit stable request or iReset
establishes length knowledge. Merely queuing a new request cannot authorize an
earlier preparation CSU. Empty iApply adds no CSU. A full-root stable request
also establishes the corresponding opaque interval's controlling fill.

Private shift storage is never an exported instrument, public target, logical
history or observation. --model_readback supplies actual parent capture bits and
unknown opaque return bits for explicitly requested raw targets. Implicit
traversal does not create raw history; child and root histories remain separate.
Default/stable payloads control shift input only. Existing sparse comparisons,
unknown failed-bit counts, reservations, reset cancellation and atomic output
publication retain their meanings. Getters still require --model_readback or
a real supplied transport.

This support excludes multiple/dynamic-width/parallel opaque children, gated or
muxed child selection, TAP routes, clock commands/overrides/idle or time waits,
raw merge, iApplyEndState constraints and physical PDL. Existing complete-route,
control, reset, ownership and aggregate 1MiBit limits still apply. It does not
certify a live instrument, a vendor opaque export or all remaining raw/hierarchy
configurations.

The reproducible benchmark is:

~~~sh
python3 tests/src/pattern/benchmark_opaque_child_raw.py --binary build/icl-pattern-generation --parent /path/to/parent/icl-pattern-generation --sanitized /path/to/sanitized/icl-pattern-generation --output /tmp/opaque-child-measurements
~~~

It measures normal widths8/64/256/1024 and sanitizer widths8/64, each with four
probe groups and three repeats. Source, flat and generated runs verify real
root getters: three head bits, the opaque interval as x, and five tail bits.
Generated probes replace all four literal notes with fresh getters. Visible
capture-x and root-only opaque controls report their different parent-storage
dimensions separately; each receipt distinguishes the requested grid width,
actual opaque width, modeled storage, emitted CSU count and total shifted bits.
An8-bit child has nine16-bit CSUs(144shifted bits); its root-only control has
four16-bit CSUs(64shifted bits) and a16-bit opaque interval. Their output is not
claimed identical to the child configuration. The acceptance bound is10s/512MiB per process. No speedup is
claimed.


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


## Mixed ordinary and callback caller groups

An Apply can contain separate ordinary requests and explicitly requested
DataRegister callbacks. Ordinary requests stay in the hardware queue while
the procedure runs. Existing explicit and implicit callback tail Applies keep
their order; reset cancels unexecuted hardware requests. Known ordinary caller
write goals stay protected until the selected callback group returns.

Save this as mixed-callback.icl:

~~~icl
Module Chains {
 DataInPort spare[3:0]; DataOutPort spare_out[3:0] {Source spare;}
 DataInPort p[3:0]; DataOutPort q[3:0] {Source p;}
 DataRegister value[3:0] {WriteCallBack Chains store <D>;}
}
~~~

Save this as mixed-callback.pdl:

~~~tcl
iPDLLevel 1
iProcsForModule Chains
iProc store {data} {iWrite p $data}
iReset
iWrite spare 3; iWrite value 9; iApply
iRead spare_out 0; iRead q 0; iApply
iNote [iGetReadData spare_out -dec]
iNote [iGetReadData q -dec]
~~~

Run source, exported flat ICL and generated PDL:

~~~sh
build/icl-pattern-generation --icl mixed-callback.icl --pdl mixed-callback.pdl --top_module Chains --output example/source.pdl --debug --model_readback
build/icl-pattern-generation --icl example/debug/Chains.icl --pdl mixed-callback.pdl --top_module Chains --output example/flat.pdl --model_readback
build/icl-pattern-generation --icl mixed-callback.icl --pdl example/source.pdl --top_module Chains --output example/generated.pdl --model_readback
~~~

The result notes are3 and9 in each form. The zero comparisons do not supply
observed data: spare_out and q reflect actual native endpoint values. Abstract
callback storage remains unknown. This is author verification, not an
independent reader study; final qualification records the executed invocations.

For example, iWrite p3 together with iWrite value9 rejects because store
would replace the ordinary caller's3 with9. A compatible p9/value9 group is
allowed. Direct conflicting or unknown endpoint writes and controlling state
assumptions reject before those commands are recorded. Indirect effects, such
as writing q and thereby driving p, are checked by native preservation at
frame/state boundaries. Native checks cover callback waits as well; no extra
writes, assumed observations or preparation cycles are added to enforce goals.

Static targets, data, callback bindings, argument counts and cycles are checked
before procedures execute. A body can have earlier committed inner Applies
before a later error; they are not fictitiously rolled back. Existing Tcl
callback failure poisoning and reset recovery remain. A body reset clears
pending ordinary requests and caller write constraints; the already selected
callback batch retains its existing iteration semantics. A canceled ordinary
read keeps its prior history, and outer completion does not invalidate a read
already completed by an inner Apply. Native/model execution failures retain the
existing fail-closed observation rules.

The ordinary request can use an ordinary-only alias, and callbacks can use
callback-only aliases, masks, namespace/prefix and split inputs. One alias
combining ordinary and callback bits remains unsupported, as do automatic
downstream invocation, invoking callbacks within an active iMerge,
iApply -together, mixed raw caller requests, physical callback output and actual
invoked ReadCallBack histories. ReadCallBack generation keeps its previous
non-model behavior. Existing ordinary endpoint merges within a callback body
retain their normal pending-queue rules. Unsupported or conflicting programs
preserve previously published output/debug/reset artifacts on failure.

The benchmark command is:

~~~sh
python3 tests/src/pattern/benchmark_mixed_callback_groups.py --binary build/icl-pattern-generation --parent /path/to/parent/icl-pattern-generation --sanitized /path/to/sanitized/icl-pattern-generation --output /tmp/mixed-callback-measurements
~~~

The grid uses64/256/1024 normal targets and64 sanitizer targets, four probe
groups and three source/flat/generated repeats, bounded by10s/512MiB per
process. Every generated observation executes a fresh spare_out getter.
Ordinary, unused-callback and observer-off controls report their different
request/declared-storage/postcondition dimensions, not an identical workload
or a speedup claim.

These are primary-I/O models: scan storage, scan CSUs and shifted bits are
zero. For N invoked callbacks, the output has4*(N+2) Applies, including
4*(N+1) nonempty PIO frames and four empty outer Applies, with4*N endpoint
writes and callback postconditions. The extra Applies are existing implicit
callback tail completions. Ordinary/unused controls have eight nonempty PIO
Applies and four endpoint writes; all modes have eight spare writes. Receipts
measure these counts from the actual output and distinguish original versus
generated input operand counts.


Task161 passes59 final qualification gates:15 normal and15 leak-enabled sanitizer
native suites,71 suites/1153 methods for each normal/static generator,17 mixed
caller CLI methods in each variant and selected sanitizer,30 changed C++
format/tidy/C++17 checks, corpus/producer/protocol/Make checks and source packaging.
The30 quality checks retain exact unchanged-source and passing-log provenance;
whole generators, changed native products and packaging executed current code.
Exact4033 maintained-input hashes and38 qualified products are bound in
build/mixed-callback-groups-20261123/final/receipt.json.

One independent review found1Critical/1Important/3Minor. One test-first pass
fixes the blockers: complete concrete caller-goal proof with rollback before
observation delivery, established state protection despite queued reassertions,
and surviving mask snapshots after nested callback reset. Three Minors remain
deferred: the unused split-reset helper second-file path duplicates incorporated
input; benchmark zero scan counts are expected dimensions rather than independent
scan-output counters; retained caller masks cost caller width times annotated
boundaries. The measured caller width is four bits through1024 callback targets;
wider caller-width scaling remains unmeasured. Full rulings and concrete fixes:
build/mixed-callback-groups-20261123/rulings.json. No second review was run.

All81 fresh mixed/control processes,60 preserved callback controls, six wide
aliases and six normal/configured-sanitizer corpus batches meet their bounds.
Three exact guide invocations pass, for156 measured/example processes. Mixed
grid maximum runtime/RSS: 9.5725s/520176KiB within10s/512MiB;16K callback alias peak:
165244KiB within256MiB. Each observed/generated mixed run executes four fresh getters.
Receipts disclose actual emitted Applies, PIO frames, empty Applies, endpoint
writes and postconditions; no scan performance or identical-workload speedup is
claimed. Commands, input/output/product hashes and runtime environments are under
build/mixed-callback-groups-20261123/{final,scale-final,final-preserved,batch-scale,
example}/. Pre-fix measurements remain separately archived with an explicit path
mapping and are not final-product measurements.

The corpus retains1024 canonical/44 declared-unsupported/695 original elaboration
failures among1763 effective modules. General automatic callbacks, mixed-bit
aliases, broader histories/topologies and other remaining inventory work stay
open. Coverage item1 remains excluded.

Task161 is merged into master;22 integration checks pass on4033 identical input
hashes/20 copied qualified native products, preserving117 stashes. Receipt:
build/mixed-callback-groups-20261123/merged/receipt.json. Broader original inventory
items remain open as tracked; execution continues after this increment.


## Mixed ordinary and callback alias bits

One alias request can combine ordinary data with explicit callback registers.
Ordinary bits retain their native access; each selected callback register invokes
its procedure once with its own data and name. Inversion, partial ranges,
segmented ports and duplicate references keep their resolved bit identities.
Duplicates use the last specified bit; unselected ordinary bits and prior read
histories stay untouched.

Save this complete model as `mixed-alias.icl`:

```icl
Module Chains {
 DataInPort spare[3:0]; DataOutPort spare_out[3:0] {Source spare;}
 DataInPort p[3:0]; DataOutPort q[3:0] {Source p;}
 DataRegister value[3:0] {WriteCallBack Chains store <D> <R>;}
 Alias mixed[7:0]=spare,value;
}
```

Save this program as `mixed-alias.pdl`:

```tcl
iPDLLevel 1
iProcsForModule Chains
iProc store {data reg} {iWrite p $data}
iReset
iWrite mixed 0x39
iApply
iRead spare_out 0; iRead q 0; iApply
iNote [iGetReadData spare_out -dec]
iNote [iGetReadData q -dec]
```

Run the source, flattened model and generated program:

```sh
build/icl-pattern-generation --icl mixed-alias.icl --pdl mixed-alias.pdl --top_module Chains --output mixed-source.pdl --model_readback --debug
build/icl-pattern-generation --icl debug/Chains.icl --pdl mixed-alias.pdl --top_module Chains --output mixed-flat.pdl --model_readback
build/icl-pattern-generation --icl mixed-alias.icl --pdl mixed-source.pdl --top_module Chains --output mixed-generated.pdl --model_readback
```

The source and flat runs evaluate the getters and contain `iNote 3` and
`iNote 9` from ordinary observations. The generated input already contains
these literal notes; its replay reproduces them without reevaluating getters.
The high alias bits request `spare=3`; its low bits invoke `store` with `value=9`,
which writes `p`. Abstract callback register storage stays unknown.

A write selects WriteCallBack members; a read selects ReadCallBack members.
Mixed reads require observable ordinary members; a DataInPort is a write target.
Generation without `--model_readback` retains ordinary reads alongside a
ReadCallBack procedure. Actually invoked read-callback histories still reject
in modeled mode. A mixed read canceled before invocation preserves prior
ordinary history and does not invoke a callback.

Existing callback Apply/reset rules apply. A body Apply can flush ordinary
requests; reset cancels still-pending ordinary requests and caller goals.
Conflicting known caller/procedure values reject: a caller request for `p=3`
cannot silently become the procedure's `p=9`. Caller-only local-reset rewriting
retains the original iProc bytes, including mixed-alias callers.

AccessTogether, `iApply -together`, callback dispatch inside an active iMerge,
mixed raw requests, automatic downstream invocation and physical callback output
retain their guards. Ordinary endpoint merges inside a procedure keep their
existing behavior. Broader items4/6 and excluded coverage1 stay as tracked in
[remaining work](remaining-work.md). Final qualification and measured limits
are recorded after verification.


Task162 qualifies mixed ordinary/callback alias bits while preserving the
original request and compact ordinary runs. Full, partial, sparse, inverted,
nested, duplicate and ascending segmented selections retain actual ordinary
history and once-only callback dispatch. Invoked read-callback modeled histories,
active callback iMerge, automatic downstream invocation and physical callbacks
retain their existing limits.

All61 final gates pass:15 normal/15 leak-enabled sanitizer native suites,
72 suites/1169 methods per normal/static generator,16 mixed-alias CLI methods
per build and selected sanitizer, nine C++ format/tidy/C++17 checks, complete
corpus/producer/protocol checks and source packaging. Exact4036 input hashes and
38 products are recorded in build/mixed-callback-aliases-20261123/final/receipt.json.
All changed native products, whole generators and packaging executed fresh;
nine quality checks retain exact source and raw-log provenance.

One independent review found0Critical/2Important/1Minor. One test-first pass
fixes both payload-copy costs and clarifies the guide. The512-bit sparse write
changes from258 full-payload allocations to2, matching the contiguous control's2;
canceled reset-only controls remain0. A maintained dynamic allocation
probe covers four processes. Mixed reads retain register-local invocation
payloads and immutable original requests; native1/16/64-callback fixtures enforce
a2KiB total argument budget and source provenance. Pure-callback stream behavior
stays unchanged. No new Minor is deferred and no second review ran. Rulings:
build/mixed-callback-aliases-20261123/rulings.json.

The243-process matrix passes:81 new mixed/control, six4096/16384-bit mixed
aliases,81 preserved mixed-group,60 callback, six pure-wide-alias, six corpus
batches and three guide invocations. New regular-grid maximum: 8.2675s/520192KiB
within10s/512MiB; mixed-wide peak:114324KiB within10s/256MiB. Normal target counts
are64/256/1024; sanitizer target count64, with leak detection and UBSAN halt.
The smaller unit receipt smoke grid does not replace these measurements.
Each observed generated benchmark uses fresh getters. The guide's generated
replay instead reproduces existing literal notes, as explicitly documented.

Separate and unused parent/current controls omit the PDL-only mixed alias
declaration; receipts disclose declared_aliases and actual command/operand/frame
counts. Hardware output matches separately queued mixed controls; no identical
source-workload speedup is claimed. The scheduler retains aliases reached from
hardware, explicit requests and reservations without rooting PDL-only aliases.
Before/after identical-input/output evidence for the measured sanitizer state
index fix is retained in index-performance-proof.json; ASAN settings and caps
are unchanged. Wider caller masks and broader inventory cases remain open.

One separate1024 control first measured10.8987s. Alternating exact-input
diagnostics measured current8.1817–9.0434s and parent8.2896–10.1874s with
identical outputs, showing timing variability and no demonstrated regression.
The final grid resumed only unrun rows after validating existing binary,
environment and artifact hashes; caps stayed unchanged. The failed measurement
and six diagnostic runs remain separately recorded under scale-timing-first-attempt/
and control-diagnose/. Results establish the measured runs, not a
hard wall-time guarantee on a noisy host. Dynamic allocation instrumentation
uses LD_PRELOAD; the fully static ELF cannot load it, so static allocation
counts are unmeasured. The complete static behavioral suite passed independently.
Coverage item1 remains excluded. Corpus:1024 canonical/44 declared unsupported/
695 original elaboration failures among1763 effective modules.


## Transparent downstream write callbacks

A write to a selectable DataRegister can require a WriteCallBack on its
WriteDataSource. The generator traces selected transparent Source/Alias and
selectable WriteDataSource paths, combines effective masks, invokes the necessary
procedures and then schedules the dependent ordinary requests. Callback
arguments retain ranges, inversions and instance-relative names.

Save this model as `downstream.icl`:

```icl
Module Chains {
 DataInPort en;
 DataInPort p[3:0];
 DataRegister value[3:0] {WriteCallBack Chains store <D> <R>;}
 DataRegister dest[3:0] {WriteDataSource value; WriteEnSource en;}
}
```

Save this program as `downstream.pdl`:

```tcl
iPDLLevel 1
iProcsForModule Chains
iProc store {data reg} {
 iNote "$reg=$data"
 iWrite en 0
 iWrite p $data
}
iReset
iWrite dest 9
iApply
```

Generate from the source, its flattened model and the expanded program:

```sh
build/icl-pattern-generation --icl downstream.icl --pdl downstream.pdl --top_module Chains --output source.pdl --debug
build/icl-pattern-generation --icl debug/Chains.icl --pdl downstream.pdl --top_module Chains --output flat.pdl
build/icl-pattern-generation --icl downstream.icl --pdl source.pdl --top_module Chains --output generated.pdl
```

The source and flat runs call `store` with `value` and `0b1001`. Output writes
`p=0b1001` while `en=0`, records the logical callback postcondition, then writes
`en=1` for the ordinary destination. The generated replay reproduces expanded
writes and the existing literal note; it does not invoke the procedure again.
The note records the requested argument, rather than an observed device value.

Effective same-target writes are last-wins. Compatible prerequisites through
different targets share one callback invocation; contradictory masks reject
before procedure effects. A matching established callback postcondition or
controlling `iState` avoids a second inferred invocation. Explicit callback
accesses still execute. The private declaration history is conservative and is
invalidated by `iReset`; read/miscompare histories do not seed it.

Dependent requests wait for their prerequisites. Other ordinary caller goals
keep their existing protection during callback frames. A callback body reset
cancels pending dependent requests without resurrecting them after return;
already completed frames remain completed. Namespace binding, selected-bit
ordering, once-only expressions, split files and caller-local reset rewriting
retain their existing contracts.

This support follows transparent write-data paths. Logic/mux-selected callback
data, callback-dependent enables or addresses, automatic serial prerequisites
and general cyclic staged scheduling remain open. Native proof still rejects
shared-enable collateral unknowns when the matched explicit sequence also
cannot execute. Plain expected reads do not request controlling callback writes.
Actually invoked automatic callback histories reject `--model_readback`;
physical callback output and active caller `iMerge` retain their guards.

Task163 core checkpoint867dc8d passed all Runtime cases in normal/static native builds,
eight initial CLI methods plus compatibility coverage totaling346 methods, and
15 C++ format/tidy/C++17 checks. All16 current CLI methods pass normal/static and
leak-enabled sanitizer builds, including a small benchmark receipt grid. A real
preliminary grid of17 processes plus512/1024-bit payloads passes with a maximum
4.2300s/373680KiB; it does not replace the full repeated matrix. Final review,
whole regressions and integration remain open.
