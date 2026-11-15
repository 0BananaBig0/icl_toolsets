# Pattern generation

`icl-pattern-generation` is an experimental executable. It currently expands
PDL procedures and Tcl computations and schedules single-path scans, shared
output observations, simultaneous named client chains, independently selected
standalone clients, indexed vector inputs and common-phase TAP instruction/data scans.
It also schedules supported primary-pin and mixed primary/scan DataRegister operations. It is
**not yet the complete retargeter described in the requirements**.

## Current verification

On 2026-11-14, reset source terminal eligibility passed the complete
make -j2 --jobserver-style=pipe check command, with exit0 recorded in
build/reset-source-confirmed-full.log/.exit. This includes all41 generator CLI suites
under dynamic/static builds, native tests,51 directional and seven legacy Tessent
standards, all334 parser and119 scanner rules across2523 ICL files, and six
Vim/Neovim tests (182.967s). Full wall time was955.735s. Five optional ANTLR
integration tests were skipped because ICL_REF_DIR was unset.

Leak-enabled ASan/UBSan checks passed for native scheduler/flatten/netlist,
main40, clock41, client-control18 and reset-preview12 methods. Formatting, naming
and independent source review pass. See
[the scope and evidence](#reset-source-terminal-eligibility-2026-11-14).
The previous multiple-TAP sanitizer scale limit remains: instrumented16/32-TAP
cases reached the unchanged2s solver budget. This clock work does not claim a
fresh sanitizer pass for those scale cases; all optimized suites pass.

Strict authored-code coverage remains unmet. A fresh complete instrumented run
including Tasks40–45 reaches 14,623/15,208 lines (96.2%) and 24,524/39,581 raw
branches (62.0%), against required 100%/90%. All instrumented tests completed;
the final strict gate failed with make exit 2. Reports are preserved under
.worktrees/coverage/2026-11-14-alias-boundary, with full evidence in
build/alias-boundary-full-coverage.log/.exit. The later help-only wording change
does not alter the measured runtime logic. No coverage threshold was weakened.

Rocky8 source/dynamic and transferred-static qualification covers the implementation
through473bc9e, with39 CLI suites (514 tests) passing per build on2026-11-14.
See [the exact platform scope](#rocky-linux-8-refresh-2026-11-14).
These results do not establish complete retargeting, arbitrary
event/clock support or general vendor equivalence. The
[model ledger](pattern-model-coverage.md#current-evidence-and-remaining-work)
records the remaining acceptance work.

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
a limit, not proven unreachability. Cyclic/self-referential data updates and
callbacks are not yet supported. Mixed primary-drive/scan DataRegister access
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
| `iReset` | Explicit top-level reset, including `-sync` for a single TAP. |
| `iRunLoop` | Literal TCK waits on standalone clients or a reset single TAP; zero count is a no-op. |
| `iNote` | Retain literal notes in generated PDL. |
| `iMerge` | Flatten nested blocks; order direct notes first; batch compatible register and connected/computed data/control-port accesses, including slices/aliases/X masks, using disjoint dependency ownership and serial fallback. |
| `iTake`, `iRelease` | ScanRegisters, DataRegisters, DataInPort/DataOutPort, AddressPort/ReadEnPort/WriteEnPort, their indexed/alias resources and instance members; bit-level protection after applied writes or `iState`, inherited ownership and scoped release. All declared port kinds support membership; Tessent default-instance and persistent takes are supported within the input stream. Assignment/frequency and broader pattern-set lifecycle remain unsupported. |
| System-clock commands | Execution/scheduling remains unfinished and reports an error. |
| Observation getters | `iGetReadData`/`iGetMiscompares` evaluate fully supplied explicit history; applied reads invalidate their target bits. Live observations, `iGetStatus` and interface/chain history remain unavailable. |

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
  scan-interface control ports still require the supported single-TAP backend.
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
  and explicit/implicit scan-interface controls. Each complete scan must operate
  its observed storage without also clocking bypassed registers. Every accepted
  transaction executes through the native TAP controller and child event model.
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
  on standalone clients or a single TAP after explicit reset. Standalone clients
  need no TAP or initial reset. `iRunLoop COUNT` defaults to TCK;
  `COUNT -tck` and `-tck COUNT` are accepted and emitted as decimal
  `COUNT -tck`. Counts must fit an unsigned 64-bit integer and contain no X.
  Queued reads/writes must first be applied. Zero cycles need no TAP or reset.
  Native idle replay includes any explicitly enabled storage events; it does
  not infer BIST completion or live hardware readback.

The backend accepts one physical scan path and at most one TAP, or standalone
client interfaces selected one at a time, each with one path or disjoint named
chains. Multiple scalar
or vector output bits may observe that path through transparent forwarding,
including inversion. Equivalent ungated observations use the first sorted
output; gated alternatives retain their enables and require a known-enabled
path. All declared controls are still checked. A path may use an indexed vector input
and an indexed one-hot return lane. Explicit or inferred standalone client
interfaces support bound select/capture/shift/update/clock controls. The fast
path proves complete-scan equivalence; bounded event planning also tracks hidden
capture/shift/update storage when that proof does not hold. Unbound vector control lanes retain
their state, and parallel input frames do not pulse scan controls.
It rejects
multiple TAPs, overlapping paths within a selected chain group, intermediate serial taps,
general serial- or phase-dependent reset events,
system-clock/absolute-time waits, unsupported reservation types and other unsupported
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
at most three candidates, using the same scheduler and solver bounds. If both
early-insertion candidates fail, a Tcl apply/uplevel wrapper evaluates the
original command words once, releases prerequisites, then executes that command
in its caller scope. This repairs argument substitutions that reset state before
a write or call. Parsed PDL vector ranges remain literal while their variable
indices and real Tcl substitutions retain their original evaluation order.
Wrappers introduce no persistent helper commands or procedure-body edits.
After patching the source, a fresh runtime executes the actual
rewritten text and compares its ordered commands and resolved arguments with
the planned stream. All analysis and replay must succeed before publication.

Remaining limits: a procedure that reasserts reset internally and then needs
another release cannot always be repaired from its caller without changing the
procedure. Cases that none of these caller-site strategies can repair remain
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
Omitted-source restoration/factor-only semantics remain unresolved; no intrinsic
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
