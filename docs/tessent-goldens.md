# Tessent-qualified offline regression

Generator qualification uses `tests/tessent_roundtrip.py`; formatter qualification
uses `tests/tessent_golden.py`. Tessent runs only when explicitly creating a
standard. Normal `make check` compares local output with stored standards without
Tessent, SSH or a license service.

## What each test establishes

| Tool | Qualification with Tessent | Ordinary offline check |
| --- | --- | --- |
| `icl-pattern-generation`, forward | Local generator(A.icl, A.pdl) → B; Tessent(A.icl, B) → C; compare B/C by ordered apply groups. | Regenerate B exactly; compare with the stored vendor trace and check local idempotency. |
| `icl-pattern-generation`, reverse | Tessent(A.icl, A.pdl) → B; local generator(A.icl, B) → C; compare B/C by ordered apply groups. | Replay stored B locally, require the certified C bytes and matching groups, then check local idempotency. |
| `icl-format` | Original ICL/source PDL versus formatted ICL/same PDL: compare exported PDL and complete Verilog testbench bundles. | Exact formatted bytes and idempotency. |
| `pdl-format` | Source ICL/original PDL versus same ICL/formatted PDL: compare exported PDL and complete Verilog testbench bundles. | Exact formatted bytes and idempotency. |

Each direction is recorded separately. Either successful direction qualifies the
case under the requested rule. Independently generated local/vendor schedules
from the original PDL **need not match**. This supersedes the earlier generator
policy preserved in legacy schema-1/2 certificates below.

Forward qualification feeds the **flat output**, never `--local_reset` output,
to Tessent. It can stage source cases requiring automatic local-reset release.
Reverse qualification feeds original source PDL to Tessent; cases marked as
requiring local-reset insertion remain blocked in that direction because Tessent
does not supply those releases. No missing prerequisites or TAP operations are
silently removed to obtain a match.

A round trip checks schedule stability for the particular model, initial state,
inputs and vendor version. It is not a proof of all requested target values,
complete retargeting support or hardware timing. Source-intent tests and concrete
state replay remain separate regression obligations.

### Ordered apply groups

`pdl_pattern_trace --apply-groups TOP --icl FILE ...` uses the shared PDL parser,
ICL elaboration, target/alias resolution and numeric conversion, without executing
Tcl or running the scheduler. Independent writes within one group may reorder;
whole-register and bitwise spellings resolve to the same bits. Overlapping writes
retain their last effective assignments (an X write retains an earlier queued
controlling value). Reads and state assumptions remain explicit.

Group order, apply options/boundaries, resets, waits and other executable commands
remain significant. A new apply cannot be removed merely because the final state
matches. Dynamic/unhandled commands, pending actions at a non-apply boundary,
unknown targets and AccessTogether aliases fail qualification. AccessTogether
needs explicit group-obligation support before it can be canonicalized safely.
Repeated state assumptions retain their order, including known-to-X history.
Expansion is bounded to 256 Ki bits per trace;
canonicalization uses ordered maps, O(expanded bits × log distinct bits), with no
schedule enumeration.

A TDR must be selected before its update. Its controlling SIB may close on that
same update. The scheduler regression uses the existing ResourceAccessTop
TAP/SIB fixture to verify this transition and reopening before a later write.

Schema-3 snapshots under `tests/golden/tessent_roundtrip/` preserve the direction,
comparison policy, original source hashes, staged inputs/scripts, raw vendor
export, local output, binary/version provenance and compared trace. Ordinary
checks never refresh them. The six directional standards cover ScalarPins,
EqualWidthProbe_0096 and WidthProbe_xx96 in both directions, using unchanged,
hash-verified Tessent Shell 2026.2 evidence from the protected-scheduling rerun.
No new licensed run was needed for these exact inputs. These are pin/vector
probes, not vendor qualification of every SIB/controller topology.

## Comparison and immutable standards

Formatter qualification also exports `result.v` with
`write_patterns result.v -verilog -pattern_sets oracle` in both independent jobs.
The relative basename is identical. Every `result.v*` artifact must be present
and match, including `.0.vec`, `.cfg` and `.info.dict` sidecars observed with
Tessent 2026.2. Added, omitted, empty-main, linked or changed artifacts fail
qualification. Export hashing is streamed in 1 MiB blocks.

The `leading-date-comment-v1` policy normalizes only a precisely matched Date
line in the initial contiguous comment header (`//`, or `#` for `.info.dict`).
Everything else remains byte-sensitive, including executable content, vectors,
timing, filenames, other comments and `Idstamp`. Raw files remain preserved as
`.expected` artifacts. Schema-2 certificates store raw SHA-256 hashes, compared
hashes and the policy; offline checks validate the exact inventory and hashes.
Legacy PDL-only formatter certificates require explicit recertification. The
existing schema-1 generator standard retains its separate PDL criterion.

Matching generated testbenches strengthens evidence for these selected cases;
it does not execute the testbenches or establish RTL simulation equivalence.

Legacy schema-1/2 and formatter pattern comparison reuses `pdl_pattern_trace` and the shared PDL parser.
It compares module binding and ordered executable commands, including operands,
bit/range spelling and apply boundaries. It ignores comments, `iNote`, PDL level
declarations and the exported zero-argument `oracle` wrapper. It never executes
PDL while comparing traces. Unsupported/dynamic traces fail qualification.

That legacy policy has no regrouping, numeric normalization or reordering.
A behaviorally equivalent alternative schedule can therefore fail this strict
check. Such a failure is not automatically a product bug and is never silently
accepted as a new standard.

Offline checks use **exact bytes** for the local tool artifacts. Each snapshot
contains those artifacts and the original/candidate vendor PDL exports with
`.expected` suffixes, plus a
`snapshot.json` recording the case definition, source hashes, artifact hashes,
ordered vendor input/script hashes, executable hash, vendor versions, console
hashes and compared operation trace. The current case manifest must still agree
with the snapshot. Changing a source, its order, the top or a staged Tcl script
requires explicit recertification.

Recording rechecks input/export hashes, immutable preparation metadata, console
completion/error status and version, both vendor traces and, for formatters, complete testbench bundles before publishing.
A failed validation leaves no snapshot. Existing destinations, including dangling
symlinks, are refused. Recording stages a complete directory before publishing;
use a fresh destination and a single maintainer for snapshot publication.
These are reproducible evidence records, not cryptographic signatures.

The user explicitly requested stored generated standards. They live under
`tests/golden/tessent/` and `tests/golden/tessent_roundtrip/`; repeated vendor wrappers/design text there are intentional
expected-output fixtures. Their `.expected` suffix keeps them separate from the
source corpus without weakening its duplicate audit. Binaries, generated parsers,
logs and remote workspaces
remain ignored and are not committed. No ordinary test updates a standard.

## Commands

Run the offline regression:

```sh
make tessent-golden-check BISON=/usr/bin/bison
# Included by make check and make check-dynamic.
```

Prepare one direction in a new bundle:

```sh
make dynamic build/pdl_pattern_trace BISON=/usr/bin/bison
python3 tests/tessent_roundtrip.py prepare --direction forward \
  --output build/forward-roundtrip
python3 tests/tessent_roundtrip.py prepare --direction reverse \
  --output build/reverse-roundtrip
```

The default manifest is `tests/cases/ours/tessent_golden/cases.json`. Each bundle
has one job per case. Reverse replay invokes the exported zero-argument `oracle`
procedure explicitly. Original source order, top and companion BSDL are retained.
The runner uses serial jobs with bounded subprocess timeouts.

Copy a prepared bundle and `tests/tessent_oracle.py` into a fresh descriptive
`test_icl_toolsets/test_<topic>` directory on the authorized EDA host. Run:

```sh
python3 tessent_oracle.py run --output forward-roundtrip \
  --tessent /data/eda/mentor/tessent_2026/tessent_2026.2/bin/tessent
```

Retrieve the results, then record a passing direction to a **new** destination:

```sh
python3 tests/tessent_roundtrip.py record \
  --bundle build/forward-roundtrip-evidence \
  --case ScalarPins --direction forward \
  --output build/reviewed-scalar-forward
```

For reverse, select the reverse bundle and `--direction reverse`. Existing
unchanged `vendor_goldens` bundles can also be certified with this command;
forward uses their generator candidate job, reverse their original source job.
The unrelated job need not pass. Source/input/export hashes and console status
are rechecked before publication.

Formatter staging and certification retain their existing commands:

```sh
python3 tests/tessent_golden.py prepare --output build/new-formatter-goldens
python3 tests/tessent_golden.py record \
  --bundle build/new-formatter-goldens-evidence \
  --case ScalarPins --tool icl-format --output build/reviewed-scalar-format
```

Review the record and file diff before adding it under `tests/golden/tessent/`.
To replace a previous standard after an intentional change, record to a fresh
scratch directory, review the differences, and explicitly replace the previous
snapshot in Git. Do not remove a failure by blindly regenerating goldens.

## Historical strict-policy qualification on 2026-10-27

The vector rejections in this historical record are superseded by the six
ordered-apply directional standards described above; legacy files are retained.

Tessent Shell **2026.2** successfully exported all **12 original/candidate jobs**.
All 39 staged input/script hashes and 12 export hashes verified after retrieval.
The source fixtures are the self-constructed scalar pin/procedure example and
the maintained `EqualWidthProbe_0096` / `WidthProbe_xx96` cases. Seven standards
were certified and committed:

| Case | Pattern generator | ICL formatter | PDL formatter |
| --- | --- | --- | --- |
| ScalarPins | Certified | Certified | Certified |
| EqualWidthProbe_0096 | Rejected: vector/bit trace difference | Certified | Certified |
| WidthProbe_xx96 | Rejected: vector/bit trace difference | Certified | Certified |

For the rejected fixed points, ours writes `data 0b10010110`; Tessent writes
`data[7]` through `data[0]` individually. The original and candidate vendor runs
agree, but the direct candidate/re-export comparison differs. No generator
golden was created for either case, and the comparison was not weakened.

Remote evidence is preserved in:

```text
/data/workspaces/lianghuaxiao/test/tessent_ijtag/test_icl_toolsets/test_vendor_goldens/tessent-goldens/
```

### Formatter testbench recertification on 2026-10-27

A fresh Tessent Shell 2026.2 run passed all 12 original/candidate export jobs.
All staged inputs, PDL exports and testbench file hashes were verified after
retrieval. All six formatter standards passed both PDL and testbench comparison
and were explicitly replaced with schema-2 records. Their formatted output bytes
and PDL operation traces are unchanged. The scalar generator standard remains
unchanged; vector generator rejections above still apply.

The complete raw exports are preserved remotely in:

```text
/data/workspaces/lianghuaxiao/test/tessent_ijtag/test_icl_toolsets/test_testbench_goldens/tessent-testbench-goldens/
```

Local evidence is ignored under `build/tessent-testbench-goldens-evidence/`;
`build/testbench-goldens-remote.log` records the licensed run. The committed
formatter snapshots preserve both raw bundles. Seventeen golden-harness tests
and 23 oracle tests cover failed exports, missing/changed testbenches, sidecar
inventory tampering, narrow Date normalization, legacy formatter rejection,
source/context changes and the existing fixed-point/overwrite protections.
Unit tests simulate only the licensed process boundary; actual certification
uses the retrieved remote vendor exports.

## Leak detection

The sanitizer suite was rerun with `ASAN_OPTIONS=detect_leaks=1`, not disabled.
Inside the sandbox, LeakSanitizer failed with its ptrace/environment fatal error.
The same command outside the sandbox passed the native symbolic/scheduler tests
and all 19 generator CLI tests with AddressSanitizer, UndefinedBehaviorSanitizer
and leak detection enabled. External Z3 remains uninstrumented.

The commands and results are in `build/tessent-goldens-leak-check.log` and
`build/tessent-goldens-leak-check-unsandboxed.log`. This establishes an actual
environment limitation and its successful workaround; it does not retroactively
turn earlier leak-disabled runs into leak checks.

After testbench recertification, the seven offline snapshots took 0.21 seconds
and 55,516 KiB maximum resident
memory in one Linux x86_64 run using `/usr/bin/time -f 'elapsed_seconds=%e
max_rss_kib=%M' python3 tests/tessent_golden.py check`. The measurement includes
local process startup, transformation, hashing and idempotency, with existing
GCC 16.2.1 `-O2` product binaries. Evidence is
`build/testbench-goldens-offline-performance.log`. This small corpus measurement
is not a throughput claim for large designs or vendor qualification.



## Final regression

`make check BISON=/usr/bin/bison -j4` passed with the established reference
Python environment and `ICL_REF_DIR=/tmp/icl-reference-honza-20261021`. It includes
all seven offline snapshots, the 17 golden-harness tests, the 23 oracle
and three trace tests, dynamic/static generator checks, all 334 grammar and
119 scanner rules, and both Vim/Neovim corpus checks (38,634 ICL and 806 PDL
files per editor). The complete run exited successfully; its log is
`build/testbench-goldens-regression.log`. Vim/Neovim plugin checks completed
in 196.982 seconds. The log heading `FAIL: invalid ICL must be rejected` labels
expected-negative inputs; the regression reports `PASS=1916 FAIL=0`.

No product C++ changed. C++ coverage was not remeasured for this Python test
harness; the previous strict coverage result remains 95.3% lines / 60.5% branches,
below the unchanged 100%/90% requirement. Full pattern-retargeting acceptance
remains separate from this new regression capability.

## Protected scheduling requalification (2026-10-27)

The [fresh rerun](tessent-oracle.md#protected-scheduling-rerun-2026-10-27)
requalified all seven stored standards under their original policies: six
schema-2 formatter standards with PDL and full testbench comparison, and the
schema-1 ScalarPins generator standard with PDL fixed-point comparison. Their
artifacts, traces and applicable normalized testbench hashes remain unchanged;
no standard was overwritten.

The optional stronger ScalarPins generator testbench comparison failed because
vector-file comments identify the original `observed` write versus flattened
`data`/`enable` writes. The strict testbench policy retains those comments, so
this result remains a failure. A separate fresh two-job run using the existing
PDL-only generator policy passed. Vector-port generator fixed points remain
unqualified. The initial 12-job certification outcomes and all raw vendor files
are retained under ignored `build/tessent-reservations-*` paths.


## Directional regression evidence (2026-10-27)

The corrected policy has six schema-3 certificates: forward and reverse for each
of ScalarPins, EqualWidthProbe_0096 and WidthProbe_xx96. Inputs and vendor exports
were reverified from `build/tessent-reservations-goldens-evidence`; the licensed
Tessent 2026.2 executions are the earlier protected-scheduling run, not new runs.
The unchanged seven legacy standards also pass.

Focused validation includes 13 trace tests, seven directional harness tests,
17 legacy certificate tests, 23 oracle tests, and the concrete scheduler suite.
Independent review found two unsafe canonicalizations; new failing regressions
reproduced both before fixes: known-to-X state history (also through a repeated-bit
alias), and erased AccessTogether obligations. The comparator now preserves that
history and rejects the unsupported obligation.

ASan/UBSan validation kept `ASAN_OPTIONS=detect_leaks=1` enabled for all 13 trace
tests, the scheduler suite and six offline directional certificates. The log is
`build/apply-group-sanitizer-tests.log`. A local GCC optimized-build run of the
trace suite, including 65,536-bit whole/partitioned inputs and an over-budget
trace, took 0.81 seconds and 87,696 KiB peak RSS under `/usr/bin/time`; this is a
single-run scale check, not a speedup claim. Logs remain ignored.

Final `make check BISON=/usr/bin/bison -j4` passed using
`ICL_REF_DIR=/tmp/icl-reference-honza-20261021` and the existing reference virtual
environment. The log is `build/apply-group-final-regression.log`. It includes
all 334 grammar alternatives, 119 scanner rules, dynamic/static checks and
38,645 ICL plus 815 PDL files per Vim/Neovim. The editor suite completed in
169.133 seconds. Strict authored-code line/branch coverage was not remeasured;
rule reachability is a different metric. No new vendor execution or RTL
simulation is claimed by this run.
