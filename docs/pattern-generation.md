# Pattern generation

`icl-pattern-generation` is an experimental executable. It currently expands
PDL procedures and Tcl computations and schedules single-input,
single-output scan networks, including single-TAP instruction/data scans. It is
**not yet the complete retargeter described in the requirements**.

## Vendor round-trip acceptance

The [directional qualification harness](tessent-goldens.md) accepts either local
flat output re-exported unchanged by Tessent, or Tessent source output replayed
unchanged by the local generator. Compare ordered apply groups, allowing
independent writes inside a group to reorder. Independent source schedules need
not match. Stored directional pin/vector, TAP, reservation and reset standards pass offline.
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
primary-input path described below. OneHotScanGroup, primary-port AccessTogether
groups and mixed scan/parallel scheduling remain unsupported and report errors.
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
a limit, not proven unreachability. Cyclic/self-referential data updates, callbacks,
mixed primary-drive/scan transactions and DataRegister reservations are not yet
supported. Ordinary write-only groups of at least four addressed data goals on
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
| `iState` | Numeric assumptions for scan registers and top-level DataInPort/SelectPort pins, including aliases and slices; default or `-LastWrittenValue`; no device transaction. |
| `iReset` | Explicit top-level reset, including `-sync` for a single TAP. |
| `iRunLoop` | Literal TCK waits on a reset single TAP; zero count needs no TAP reset. |
| `iNote` | Retain literal notes in generated PDL. |
| `iMerge` | Flatten nested blocks; order direct notes first; batch compatible disjoint full-register single-apply calls, with serial fallback. |
| `iTake`, `iRelease` | Full named ScanRegisters and instance ScanRegister storage; protection after applied writes or `iState`, inherited ownership and scoped release. Tessent default-instance and persistent takes are supported within the input stream; port/alias/slice reservation names remain unsupported. |
| System-clock commands | Execution/scheduling remains unfinished and reports an error. |
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
- `AccessTogether` on a referenced ICL Alias keeps its full supported register group
  in a single operation, including nested aliases and partial references.
  Writes require simultaneous updates after reset release; reads compare their
  requested bits together after any prerequisite scans. Unrelated accesses may
  use separate scans. ScanRegister and writable DataRegister members are supported;
  unsupported port members are rejected.
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
  reservation. Mixed primary/scan execution remains unsupported.
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
  on a single TAP after explicit reset. `iRunLoop COUNT` defaults to TCK;
  `COUNT -tck` and `-tck COUNT` are accepted and emitted as decimal
  `COUNT -tck`. Counts must fit an unsigned 64-bit integer and contain no X.
  Queued reads/writes must first be applied. Zero cycles need no TAP or reset.
  Native idle replay includes any explicitly enabled storage events; it does
  not infer BIST completion or live hardware readback.

The backend accepts one SI/SO boundary and at most one TAP. It rejects
multiple TAPs, multiple outputs/shared chains, intermediate serial taps,
OneHotScanGroup transactions, feedback reset scheduling,
system-clock/absolute-time waits, unsupported reservation types and other unsupported
operations. Transactions needing shadow-register effects or independently gated
interface events are outside the current complete-scan model; inability to
schedule them is not proof of hardware unreachability.

### Local-reset output

`--local_reset PATH` preserves source text, including procedure definitions, and
inserts reset-control writes before main-level writes, calls or whole merge
blocks. Releases are never inserted between a merge's begin/end markers. Multiple PDL
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
write early. After patching the source, a fresh runtime executes the actual
rewritten text and compares its ordered commands and resolved arguments with
the planned stream. All analysis and replay must succeed before publication.

Remaining limits: a procedure that reasserts reset internally and then needs
another release cannot always be repaired from its caller without changing the
procedure. Repeated source sites needing different preambles, insertions that
split a repeated pending queue, nonliteral/procedure-body insertion sites, and
effectful substitutions that change execution order are rejected. Unresolved
reset sources, forwarding cycles, feedback scheduling and unsupported controller
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
PDL parser. It combines consecutive calls only when each contains a single
plain apply with fully known, full-width direct ScanRegister writes, and their
register sets do not overlap. It does not merge across outside apply boundaries.
Multi-apply calls, aliases, partial/X writes, reads, notes within calls, state
changes, `-together`, and clock waits retain their serial barriers. Nested blocks
are processed recursively. Calls containing resource commands remain serialized.

The combined stream uses the existing scheduler and concrete replay. A failed
attempt is discarded and retried serially with a fresh scheduler; artifacts are
published only after success. This is a conservative heuristic, not a proof of
the globally shortest pattern. A shared apply may still need multiple scans.
The merge pass uses ordered-set conflict checks, not interleaving enumeration.

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

Supported resources are full named ScanRegisters and ScanRegister storage in
an instance subtree. Alias/slice **writes** to a taken register are supported;
alias/slice **reservation names** and direct ports are rejected.
Taking an instance does not add support for port or other unsupported register
transactions. Calls inherit ownership; returning drops only acquisitions made
in that call. Explicit release propagates to inherited ownership, following
§7.9.20(e) and Tessent IJTAG User's Manual v2023.4, “How to Run iCalls in
Parallel”. Overlapping parent/child takes do not lose the parent's protection
on return.

Tessent's `iTake [resource] [-persistent]` extension is supported for this
ScanRegister subset. Missing or empty resources select the effective instance
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

The scheduler constrains protected values at each scan state and checks them
during concrete clock replay. Explicit user writes may replace a protected
value with their requested value; automatic reset-only preambles cannot.
Failed schedules or idle replays do not publish partial state or files. Taking
a resource after an earlier apply does not retroactively protect that write.
Reset-only rewriting also rejects insertion of an explicit prerequisite write
to a taken-but-unwritten resource: executing that file would otherwise acquire
a lock that the original source never acquired.
Read reservations do not authorize concurrent interleaving: calls using
reservations remain serialized.

The fully generated pattern resolves reservations into concrete writes/applies,
without retaining procedure scopes or resource directives. Generated-PDL and
flattened-ICL replay are tested. Source-preserving `--local_reset` output retains
the original resource commands and is separately re-executed before publication.

Large write-only applies can use dependency-guided scheduling with reservations;
reads and incompatible prerequisite configurations use the bounded solver.
`iReset` with active protected values reports an explicit unsupported error;
release first. TCK waits must preserve protected values. Full resource/model support and
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
partial AccessTogether port-alias regression first failed and now explicitly
rejects unsupported whole-group access. Existing elaboration already rejects
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
