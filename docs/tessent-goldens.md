# Tessent-qualified offline regression

The requested method is implemented by `tests/tessent_golden.py`. Tessent is
used when explicitly creating a standard. Normal `make check` runs local tools
and compares their outputs against committed standards without invoking Tessent,
SSH, or a license service.

## What each test establishes

| Tool | Qualification with Tessent | Ordinary offline check |
| --- | --- | --- |
| `icl-pattern-generation` | Run original ICL/source PDL through Tessent; separately feed our **flat generated PDL** to Tessent with the same ICL/BSDL. Require agreement with the original request and a fixed point between the flat pattern and its re-export. | Compare newly generated flat PDL byte-for-byte with the certified flat artifact; regenerate it again and require idempotency. |
| `icl-format` | Tessent receives original ICL + source PDL, then formatted ICL + the same source PDL, in separate fresh runs. Compare the exported patterns. | Compare formatted ICL byte-for-byte with its certified formatted artifact; format it again and require idempotency. |
| `pdl-format` | Tessent receives source ICL + original PDL, then the same ICL + formatted PDL, in separate fresh runs. Compare the exported patterns. | Compare formatted PDL byte-for-byte with its certified formatted artifact; format it again and require idempotency. |

The generator qualification **does not use `--local_reset` output**. Only
successful source cases marked `requires_local_reset: false` are accepted by
this harness. Source PDL must contain any necessary reset actions explicitly.
The older local-reset-aware differential harness remains available separately.

A fixed point alone is insufficient: a generator could emit the wrong writes
and still reach a stable result. Requiring agreement with Tessent's original
source run catches that failure. Even both checks establish agreement only for
the selected inputs, model, initial conditions, options and vendor version.
They do not prove complete language support, physical timing, or correctness
for every possible instrument state.

## Comparison and immutable standards

Vendor pattern comparison reuses `pdl_pattern_trace` and the shared PDL parser.
It compares module binding and ordered executable commands, including operands,
bit/range spelling and apply boundaries. It ignores comments, `iNote`, PDL level
declarations and the exported zero-argument `oracle` wrapper. It never executes
PDL while comparing traces. Unsupported/dynamic traces fail qualification.

There is no regrouping of vector/bit writes, numeric normalization or reordering.
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
completion/error status and version, and both vendor traces before publishing.
A failed validation leaves no snapshot. Existing destinations, including dangling
symlinks, are refused. Recording stages a complete directory before publishing;
use a fresh destination and a single maintainer for snapshot publication.
These are reproducible evidence records, not cryptographic signatures.

The user explicitly requested stored generated standards. They live only under
`tests/golden/tessent/`; repeated vendor wrappers/design text there are intentional
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

For explicit qualification, build the tools and prepare a fresh bundle locally:

```sh
make dynamic build/pdl_pattern_trace BISON=/usr/bin/bison
python3 tests/tessent_golden.py prepare --output build/new-vendor-goldens
```

The default manifest is `tests/cases/ours/tessent_golden/cases.json`. Each source
case creates four jobs: original, flat generated PDL, formatted ICL and formatted
PDL. Existing paired corpus/origin and BSDL staging uses the original oracle
helpers. Local transformations have 120-second subprocess timeouts. Vendor
jobs run serially; no license-parallel execution is introduced.

Copy the bundle and `tests/tessent_oracle.py` into a fresh descriptive directory
under the authorized remote `test_icl_toolsets/test_<topic>` area. On the EDA host:

```sh
python3 tessent_oracle.py run --output new-vendor-goldens \
  --tessent /data/eda/mentor/tessent_2026/tessent_2026.2/bin/tessent
```

The reused Tcl script loads ICL/BSDL in `patterns -ijtag / setup`, changes to
analysis, opens a pattern set with `-no_initial_ireset`, executes the selected PDL,
and exports it using `write_patterns -pdl`. Original and candidate jobs have
independent tool processes, the same top, source order and companion inputs.

Retrieve the result bundle, then explicitly record one passing case/tool into
a **new** destination:

```sh
python3 tests/tessent_golden.py record \
  --bundle build/new-vendor-goldens-evidence \
  --case ScalarPins --tool icl-pattern-generation \
  --output build/reviewed-scalar-standard
```

Review the record and file diff before adding it under `tests/golden/tessent/`.
To replace a previous standard after an intentional change, record to a fresh
scratch directory, review the differences, and explicitly replace the previous
snapshot in Git. Do not remove a failure by blindly regenerating goldens.

## Qualification on 2026-10-27

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

Local evidence is ignored under `build/tessent-goldens-evidence/` and
`build/tessent-goldens-{remote,certification,offline,final-focused}.log/json` as
applicable. Committed snapshots preserve the checked exports and compact
provenance. Eleven local harness tests exercise real tools/trace parsing and
simulate only the licensed process boundary for failure injection. They cover
wrong original semantics despite a fixed point, operand changes, failed consoles,
changed preparation/context, stale sources/manifest, tampered files and overwrite
protection. Actual qualification uses the remote vendor exports, not those
simulated unit-test results.

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

The seven offline snapshots took 0.29 seconds and 55,512 KiB maximum resident
memory in one Linux x86_64 run using `/usr/bin/time -f 'elapsed_seconds=%e
max_rss_kib=%M' python3 tests/tessent_golden.py check`. The measurement includes
local process startup, transformation, hashing and idempotency, with existing
GCC 16.2.1 `-O2` product binaries. Evidence is
`build/tessent-goldens-offline-performance.log`. This small corpus measurement
is not a throughput claim for large designs or vendor qualification.



## Final regression

`make check BISON=/usr/bin/bison -j4` passed with the established reference
Python environment and `ICL_REF_DIR=/tmp/icl-reference-honza-20261021`. It includes
all seven offline snapshots, the 11 new harness tests, the existing 22 oracle
and three trace tests, dynamic/static generator checks, all 334 grammar and
119 scanner rules, and both Vim/Neovim corpus checks (38,634 ICL and 806 PDL
files per editor). The first duplicate-artifact failure and the final passing
run remain in `build/tessent-goldens-regression.log` and
`build/tessent-goldens-final-regression.log` respectively.

No product C++ changed. C++ coverage was not remeasured for this Python test
harness; the previous strict coverage result remains 95.3% lines / 60.5% branches,
below the unchanged 100%/90% requirement. Full pattern-retargeting acceptance
remains separate from this new regression capability.
