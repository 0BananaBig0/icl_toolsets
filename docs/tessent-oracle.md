# Tessent PDL differential tests

## Protected scheduling rerun (2026-10-27)

Fresh serial Tessent Shell **2026.2** runs against generator commit `ea7f5d1`
reproduced **36/36 expected reference outcomes** (17 exports, 19 expected errors).
The maintained comparison manifest still has 30 generator preparation failures,
one expected vendor failure (`SerialWrite`, ICL71-1), five differing operation
traces and **zero exact operation matches**. Expected negative cases remain in
these counts. This is not a full equivalence pass.

The three new authored reservation probes are in
`tests/cases/ours/pattern_generation/resources/oracle_cases.json`. They use the
existing AccessProbe BSDL, a two-bit SIB control and a select-qualified child
instrument. They cover persistent takes, default-instance takes with iPrefix,
and explicit nested release. Tessent consumes each generator-produced
`--local_reset` artifact; the generator consumes the original source PDL.
All three export successfully, but strict comparison reports differences:

| Probe | Observed comparison |
| --- | --- |
| Persistent/default take | Both write `gate=11` then `leaf.value=10100101`, preserving `gate=11`. Tessent also emits explicit TAP instruction write/read/apply and splits iState into individual bits. |
| Explicit release | Both open the gate and write the target. Our final gate is `00`; Tessent retains `01`. This is a real final-state difference. |

The trace helper now retains literal `iState` commands and operands, rather than
rejecting them as unsupported. It does not discard assumptions, turn them into
hardware writes, or accept dynamic values. The new tests distinguish changed or
missing assumptions. These three probes remain unqualified as equivalence goldens.
An initial local probe with unqualified downstream updates was rejected by the
scheduler; the maintained fixture explicitly qualifies the child select by the
SIB. This does not add support for arbitrary bypassed-register update behavior.

The transitive-local-reset probe was also rerun successfully. Its strict trace
still differs, including the previously documented final SIB state.

Golden requalification ran 12 source/candidate jobs with Verilog testbench
exports, plus two dedicated ScalarPins PDL fixed-point jobs. All six formatter
standards passed both PDL and complete testbench-bundle comparison. ScalarPins
passed its existing PDL-only generator certification policy. Fresh artifacts,
traces and applicable normalized testbench hashes match **all seven existing
standards**, so the committed standards were not replaced.

The additional ScalarPins generator testbench check fails the strict bundle
policy solely because `.vec` comments describe `observed=1` in the source and
`data=1, enable=1` in the flattened input. That stronger check remains a failure;
its result was not normalized away. The two vector-port generator cases still
fail the literal PDL fixed-point criterion. See [golden policy](tessent-goldens.md).

Across the six completed bundles, all **307 input/script hashes**, **40 PDL
export hashes** and **48 testbench-file hashes** verified after retrieval.
Prepared bundle metadata also matched the local staged originals. After the
parser initialization fix below, fresh preparation reproduced all staged input
hashes for the six maintained, one reset and three reservation comparison jobs;
the seven freshly qualified standards also passed with the rebuilt tools.
The 60 vendor
executions comprise 40 successful exports and 20 expected tool errors. The
30 failed generator preparations never launched a vendor job.

Remote evidence is retained in:

```text
/data/workspaces/lianghuaxiao/test/tessent_ijtag/test_icl_toolsets/test_protected_scheduling/
```

The completed bundles are `tessent-reservations-reference`, `-comparison`,
`-goldens`, `-local-reset`, `-qualified-resources` and `-pattern-fixed-point`.
The earlier `-resources` preparation failure is retained separately. Local
retrieved bundles are ignored under `build/tessent-reservations-*-evidence/`.
Hash and certification summaries are in `build/tessent-reservations-verification.json`,
`-certification.json` and `-standards-comparison.json`; fresh certificates stay
under `build/tessent-reservations-certified/`.

During verification, the instrumented trace helper exposed a shared PDL parser
initialization crash in Tcl's allocator on vector tokens. Passing the existing
lazy parser interpreter into Tcl_ParseCommand initializes it before parsing;
no Tcl_Init or script evaluation is added. After this fix, full `make check`
passed, including dynamic/static generator checks, all grammar/scanner rules,
seven offline goldens and Vim/Neovim corpus checks. Four trace tests and eighteen
resource tests passed ASan/UBSan with leak detection enabled. Independent review
found no blocking issues. Strict authored-C++ coverage was not rerun.
Final verification evidence is recorded in the [rerun plan](superpowers/plans/2026-10-27-tessent-protected-rerun.md).

## Transitive local-reset rewrite (2026-10-27)

The maintained case at
`tests/cases/ours/pattern_generation/local_reset/oracle_cases.json` exercises
two reset-control stages before a preserved `iCall`, behind a BSDL-backed TAP.
Our generator receives `vendor.pdl`; Tessent receives its `--local_reset`
artifact. Tessent Shell 2026.2 completed pattern export successfully in
`test_icl_toolsets/test_local_reset_explicit/test_transitive_releases`.
Local evidence is ignored under `build/local-reset-explicit-evidence/`.

The strict comparison reports `different_operations_requires_review`.
Both outputs write leaf values `1010` and `0011` after releasing `core.root`
and `core.branch.release`. Tessent additionally exports TAP instruction
write/read operations and an intermediate read of the reset controller, uses
explicit vector ranges, and keeps `core.gate=1` at the final apply. Our output
keeps instruction selection internal and ends with `core.gate=0`. The latter
is a real final control-state difference, not just formatting. No equivalence
golden was recorded, and these observations do not establish full state or
testbench equivalence.

Earlier probes are retained in descriptively named `test_local_reset_rewrite`,
`test_local_reset_ports` and `test_local_reset_tap` directories. Vendor DRC
identified an internal-register hierarchy reference, reset-source restrictions,
a missing scan interface and ambiguous reset binding. The final fixture uses
explicit ToResetPort forwarding, DataMux reset gating, an AccessLink interface
and the BSDL's TRST pin. Failed probes were not relabeled as passing runs.

Reproduce preparation locally (choose a new output directory for each run):

```sh
python3 tests/tessent_oracle.py prepare \
  --manifest tests/cases/ours/pattern_generation/local_reset/oracle_cases.json \
  --generator build/icl-pattern-generation --output build/local-reset-oracle-new
```

Use the existing remote runner and compare command described below. Preparation
must use comparison mode so Tessent consumes the reset-only rewrite.

`tests/tessent_oracle.py` prepares isolated, serial Tessent runs and records
input SHA-256 hashes, exit codes, timeouts, tool banners and expected diagnostics.
Its default export is **PDL** through `write_patterns -pdl`. Golden qualification
also requests `write_patterns result.v -verilog -pattern_sets oracle` and records
the complete `result.v*` testbench bundle. Missing exports fail qualification.
No normal regression requires a Tessent license or SSH connection.

The [Tessent-qualified golden harness](tessent-goldens.md) additionally checks
flat-pattern re-ingestion and source/formatted ICL/PDL pairs (including matching
Verilog testbench bundles for formatter standards), then stores
explicitly certified outputs for offline regression. It never substitutes
local-reset output for the flattened pattern.

The generator executable, BSDL loader and single-TAP scheduler now exist.
Comparison uses the generator-produced `--local_reset` artifact, including
the supported transitive local-reset rewrite. The dated sections below preserve historical measurements; their
then-current implementation limitations are not the current support matrix.
See [the generator guide](pattern-generation.md) for current capabilities.

## Primary-input rerun (2026-10-27)

The primary-input implementation makes three previously blocked maintained
cases generate PDL: `EqualWidthProbe_0096`, `WidthProbe_xx96` and
`AscendingWidthProbe_xx96`. The other six Port-width cases are intentionally
invalid writes to unknown padding and now fail with a requirement-justification
error; they remain negative cases, not missing valid pattern support.

Fresh serial Tessent Shell 2026.2 execution reproduced **36/36 expected reference
outcomes: 17 exports and 19 expected errors**. Six comparison jobs ran using
generator-produced `local_reset.pdl`, preserving source bytes. The strict
operation comparison reports:

| Result | Cases |
| --- | ---: |
| Blocked at generator preparation (including negative cases) | 30 |
| Blocked by vendor ICL71-1 (`SerialWrite`) | 1 |
| Different operation sequences | 5 |
| Exact operation matches | 0 |

The three new differences are vector grouping: our `iWrite data 0b10010110`
is represented by Tessent as `data[7]` through `data[0]` writes with those same
bits. Both write `enable=1` and use one `iApply`. An explicit expansion of
these three fixed 8-bit fixtures verified the pin transaction. This check is
limited to those fixtures; it is not a general hardware-equivalence checker
and does not change the strict trace status. The other two differences are
the previously documented TAP traces. Full Tessent equivalence remains unmet.

The reference runner returned 0. The comparison runner and comparison command
returned 1 because unresolved cases remain. All **236 input/script hashes**
and **22 export hashes** were verified after retrieval (127/17 reference and
109/5 comparison). A fresh preparation with the final reviewed binary produced
identical input hashes for all six remote comparison jobs, including generated
PDL, reset PDL and Tcl; no remote input needed replacement after review fixes.

Remote evidence is preserved under:

```text
/data/workspaces/lianghuaxiao/test/tessent_ijtag/test_icl_toolsets/test_primary_input_writes/
  tessent-primary-reference/test_<case-id>/
  tessent-primary-comparison/test_<case-id>/
```

Ignored local records are `build/tessent-primary-{reference,comparison}-evidence/`,
`build/tessent-primary-verification.json`,
`build/tessent-primary-port-transactions.json`, and
`build/primary-write-{remote,comparison}.log`. Generated vendor outputs and
compiled artifacts are not committed.

## Prefixed-layout rerun (2026-10-27)

The requested repeat run against commit `a39fc7b` matched **36/36 expected
Tessent outcomes: 17 PDL exports and 19 expected errors**. All 36 reference cases' input
and Tcl hashes, and their outcomes, match the previously reviewed inputs. The
runner naming change has not changed the vendor workload.

Real comparison preparation produced **33 generator failures and three ready
cases**. Tessent consumed the three generator-produced `local_reset.pdl` files;
each preserves its original PDL bytes. The comparison report remains:

| Result | Cases |
| --- | ---: |
| Blocked by generator/model limitations | 33 |
| Blocked by vendor ICL71-1 (`SerialWrite`) | 1 |
| Different operation sequences (the two TAP cases) | 2 |
| Exact operation matches | 0 |

The 33 generator failures have these recorded first causes:

| First diagnostic | Cases | Interpretation |
| --- | ---: | --- |
| Target requires ScanRegister or Alias | 24 | Addressed/one-hot target execution is unsupported. |
| Interface controls require event scheduling | 3 | Required control-event scheduling is unsupported. |
| Reset artifact requires one PDL source | 1 | Safe multiple-file reset output is unfinished. |
| Parent logical DataIn width too small | 1 | This is an intentionally invalid contributor-width fixture. |
| Explicit TAP reset required | 4 | These source procedures omit reset; the generator refuses unknown TAP state. |

These counts identify the first blocker, not every missing feature. Some cases
are deliberately negative vendor tests and cannot become positive pattern
comparisons without changing their purpose. Keep these expectations separate
from the work needed to support valid networks.

The TAP traces are unchanged from the reset-passthrough comparison below,
including Tessent's explicit instruction-register write/readback/apply. The
comparison command returned exit 1, correctly retaining these unresolved
outcomes. The reference runner returned 0; the comparison runner returned 1
because its report retains the 33 failed generator preparations, even though
all three ready jobs matched their expected vendor outcomes. **This is not a
full equivalence pass.** General reset rewriting, the other generator/model
limitations and independent state/observation equivalence remain unfinished.

All **227 staged input/script hashes** and **19 successful export hashes** were
verified after retrieval (127/17 in the reference bundle, 100/2 in comparison).
The shared trace helper parsed all 19 exports, including repeat cases. Remote
execution used Tessent Shell 2026.2 and serial jobs in:

```text
/data/workspaces/lianghuaxiao/test/tessent_ijtag/test_icl_toolsets/test_equivalence/
  tessent-repeat-reference/test_<case-id>/
  tessent-repeat-comparison/test_<case-id>/
```

Local evidence remains ignored:
`build/tessent-repeat-{reference,comparison}-evidence/`,
`build/tessent-repeat-verification.json`, `build/tessent-repeat-blockers.json`
and `build/tessent-repeat-remote.log`. All 19 exported static traces are
unchanged from the prior run; the two TAP `operation_comparison.json` files are
byte-identical. Earlier `tessent-prefixed-*` evidence remains intact. No result
was replaced or accepted solely because it matched an expected filename.

This repeat changes documentation only. The 22 harness tests and three shared
trace tests were rerun and passed (`build/tessent-repeat-focused.log`). The latest
complete regression remains the successful check for `a39fc7b`, including all
334 grammar / 119 scanner rules and both editor corpus runs (38,633 ICL / 805
PDL per editor). The complete regression, sanitizers and strict C++ coverage
were not repeated for this documentation update. The last measured coverage
gate remains unmet at 95.2% lines / 60.5% raw branches. See the [verification
record](pattern-generation.md#tessent-harness-directory-naming-2026-10-27).

## Reset-passthrough comparison (2026-10-27)

The maintained manifest now has **36 cases**. A fresh 36-case reference run
initially produced two unexpected failures: the two new plain serial examples
lacked Tessent's required scan-interface controls (ICL71-1). Adding an empty
ScanInterface would not supply a valid client/host/TAP interface, so that attempted
fixture change was discarded. `SerialWrite` now explicitly records the measured
vendor restriction, and the procedure case uses the existing complete TAP model.
The old 33 cases were unchanged and all matched their expected outcomes; a fresh
run of the three final cases also matched all expectations. Together these
reviewed inputs give **17 successful exports and 19 expected vendor errors**.
An expected error is a compatibility result, not an equivalence pass.

The real generator now emits a byte-preserving `--local_reset` artifact for one
PDL source when the reset ancestry proves no local-reset TDR release is needed.
Tessent consumed **that artifact only** in comparison mode. It did not receive a
substituted source file or a fabricated rewrite. All three new comparison jobs
ran, and the static comparison returned exit 1:

| Case | Result |
| --- | --- |
| `SerialWrite` | Generator succeeded; Tessent rejected the incomplete vendor interface with ICL71-1; comparison blocked by reference. |
| `AccessLink_ExplicitReset` | Both exported PDL; literal operation sequences differ. |
| `AccessLink_ProcedureReset` | Both exported PDL; literal operation sequences differ; the reset artifact preserves the original procedure bytes. |

For both TAP cases Tessent emits `iWrite tap.INSTRUCTION[3:0] 0b0011`,
`iRead tap.INSTRUCTION[3:0] 0b0001`, and an `iApply` before the core writes.
Our output leaves synthetic instruction-register selection inside retargeting.
Tessent also spells `core.r` as `core.r[7:0]`. The core write values agree:
`0b10010110`, followed by `0b00000101` in the procedure case. These observations
do **not** justify discarding instruction readback or claiming hardware-state
equivalence. There are **zero exact-operation matches** and no new proven full
Tessent-equivalence result. Independent state/observation comparison remains work.

The original 33 comparison preparations still fail, now with specific model or
execution limitations: addressed/one-hot targets, interface-event scheduling,
multiple PDL inputs, an invalid contributor width, or missing explicit TAP reset.
They no longer all fail on an unimplemented option. General local-reset releases
and safe multi-file rewriting are still required.

All retrieved input/script and successful-output hashes match the prepared
bundles. Verification covered 126 inputs / 16 exports in the initial bundle,
11 inputs / 2 exports in the corrected reference subset, and 17 inputs / 2
exports in the comparison subset. All 20 exports across these bundles parsed
with the shared static trace helper; counts include repeated cases. Evidence:

- `build/tessent-reset-initial-reference-evidence/`: initial run, including both
  unexpected failures rather than hiding them.
- `build/tessent-reset-reviewed-reference-evidence/` and
  `build/tessent-reset-reviewed-comparison-evidence/`: final three-case inputs,
  logs, exports, hashes and operation comparisons.
- `build/reset-passthrough-evidence-verification.json`: hash/trace/count checks.
- `build/tessent-identity-20261027-comparison/`: original 33 generator failures.

Remote work lives in `test_icl_toolsets/test_reset_passthrough`, under the
workspace described below. Tessent Shell is **2026.2**, banner dated
21 May 2026; jobs ran serially. Evidence and generated files remain ignored.
The complete functional regression passed after fixtures were frozen, including
18 CLI tests and both editors over 38,633 ICL / 805 PDL files. Sanitizers and
style checks passed; strict coverage remains below its unchanged threshold.
See [the verification record](pattern-generation.md#local-reset-passthrough-verification-2026-10-27).

## Reference rerun before reset passthrough (2026-10-27)

The fresh **Tessent Shell 2026.2** run matched **33/33 expected outcomes**:
**15 successful PDL exports** and **18 expected tool errors**. All 116 staged
input/script hashes and all 15 generated-output hashes were verified after
retrieval. The shared-parser static trace helper accepted all 15 exports.

**Zero generator-equivalence comparisons completed.** Fresh comparison
preparation using the real `build/icl-pattern-generation` executable failed
all 33 cases with:

```text
error: --local_reset rewrite integration is not implemented
```

The harness continues to require the generator's original-input output and
its separate local-reset rewrite; Tessent must consume that rewrite in
comparison mode. Reference-only success does not satisfy this contract. The
rerun did not substitute source PDL or fabricated rewrites for missing generator
output, nor does parsing exported PDL prove hardware-state equivalence.

During preparation, the harness's obsolete `--bsdl FILE` invocation was fixed to
use the current `--bsdl ENTITY FILE` CLI. BSDL cases now declare and validate
`bsdl_entity` before creating their run directory. Failing argument-contract
and invalid-entity tests reproduced the gap; all 18 harness tests and three
shared-parser trace tests pass after the correction. The four maintained BSDL
cases identify the actual entity `AccessProbe`.

The reference bundle was prepared before this comparison-only CLI correction;
its ICL/PDL/BSDL and Tcl script bytes are unchanged by the fix. The isolated
remote directory is:

```text
/data/workspaces/lianghuaxiao/test/tessent_ijtag/test_icl_toolsets/test_equivalence
```

Tool banner: `Tessent Shell 2026.2 Thu May 21 21:09:45 GMT 2026`;
remote Python: 3.8.16. The jobs ran serially, and original external projects
were not modified. Local evidence is ignored build output:

- `build/tessent-rerun-20261027-evidence/`: retrieved `results.json`, vendor
  logs/PDL, verified traces and `verification-summary.json`.
- `build/tessent-rerun-20261027-comparison/`: current generator commands,
  per-case failure logs and `bundle.json`.
- `build/tessent-rerun-20261027-inventory.json`: 39,435 inputs with hashes and
  dispositions: 38 manifest-assigned, 36,588 generated syntax fixtures,
  88 syntax-negative and 2,721 unpaired files.
- `build/tessent-rerun-{remote,focused,comparison-prepare,regression}.log`:
  run, harness and repository-check output.

Full `make check BISON=/usr/bin/bison -j4` passed after the harness correction:
static/dynamic checks, all 334 grammar and 119 scanner rules, and both editor
corpus runs over 38,632 ICL and 802 PDL files each. Independent code and evidence
review found no remaining issues. C++ production code is unchanged by this
rerun; sanitizers and strict C++ coverage were not rerun. The latest measured
coverage gate remains unmet at 95.2% lines / 60.5% raw branches against unchanged
100%/90% thresholds, as recorded in [the generator guide](pattern-generation.md).
Implementing and validating local-reset rewriting remains necessary before
this harness can establish any end-to-end generator comparison.

## Remote test organization

The local mount `/home/banana/remote_eda/lianghuaxiao/test/tessent_ijtag`
corresponds to `/data/workspaces/lianghuaxiao/test/tessent_ijtag` on
`lianghuaxiao@192.168.6.20`. Following `test_Local_Reset`, all eight agent-created
experiment directories have been moved under one parent, with descriptive names:

```text
test_icl_toolsets/
  README.md
  migration.json
  test_oracle/
  test_contributors/
  test_domains/
  test_address_defaults/
  test_tap_signals/
  test_equivalence/
  test_reset_passthrough/
  test_connections/
```

Moves preserved each directory inode and its contents; no existing destination
was overwritten. The original `test_Local_Reset` was not modified. The paths in
this guide point to the new locations. `migration.json` retains the old-to-new
mapping; historical logs may still mention their original paths. Existing
bundle case IDs, relative input paths and hashes were preserved. New topic
directories must use `test_` and meaningful names, without opaque random
suffixes. Retain each run in a fresh descriptive subdirectory; never replace
old evidence to rerun a case. Generated evidence stays outside Git.

The separately named `icl-toolsets-connections.h7xp5L` experiment is now
`test_icl_toolsets/test_connections`; its directory inode and contents were
preserved, and its Tcl probes use relative inputs. The mapping is recorded in
the parent `migration.json`.

New harness bundles record `case_directory_prefix: "test_"` and place each case
in `test_<case-id>`, keeping logical case IDs unchanged. For example:

```text
test_equivalence/
  tessent-prefixed-reference/
    bundle.json
    test_AccessLink_ExplicitReset/
      design_0.icl
      model.bsdl
      source_0.pdl
      run.tcl
```

The runner and comparison command use this explicit layout marker. Archived
bundles without it retain their original case paths; existing evidence does not
need renaming or rehashing. Unsupported prefixes, escaping case IDs and case
symlinks are rejected. The naming change does not alter the prepared Tcl,
source bytes, case IDs, or expected vendor outcomes.

## Input contract

In comparison mode the generator receives the original ordered ICL and PDL
inputs, with `--output ours.pdl --local_reset local_reset.pdl`. Tessent receives
the same ICL and **only `local_reset.pdl`**. This preserves the user's requirement
that Tessent need not discover/release local-reset TDRs itself. Missing generator,
failed execution or missing rewrite cannot pass as a successful comparison.

Reference-only mode is explicitly separate. It accepts original PDL only for
cases whose manifest declares that they need no local-reset rewrite. Such runs
measure vendor behavior; they do not compare our generator. The inspected
existing-network cases have global reset connections and no local-reset TDRs.
An explicit source `iReset` supplies initial state where needed; pattern sets
open with `-no_initial_ireset` so the harness does not secretly add a reset.

`tests/cases/ours/pattern_generation/oracle_cases.json` lists reviewed pairings,
top modules, ordered sources, reset policy and expected outcomes. Original EDA
ICL sources are reconstructed from their deduplicated module manifest using
`icl_corpus_inventory` and its shared parser. No second ICL parser or duplicate
vendor module library is introduced. The adapted iWrite sequence lives in
`tests/cases/tessent/pattern_generation/`, with attribution to the user's
`test/tessent_ijtag/test_iWrite/test_iWrite.tcl`. Byte-for-byte vendor imports
remain in `tests/cases/pdl_eda/`; that directory's integrity manifest is unchanged.

Module reconstruction emits namespace directives only at actual context changes,
including resets back to the global namespace. Tessent 2026.2 rejected an
intermediate version that emitted redundant `NameSpace;` directives in the two
global-only network cases (`Name spaces for ICL Modules are not yet supported`).
The corrected preparation produces byte-identical inputs/scripts to the eleven
successful expected-outcome reference runs, verified by their stored hashes.
Namespaced inputs remain intact and may expose this vendor limitation; our IEEE
parser's namespace support is not restricted to match the vendor.

## Commands

Run from the repository root; choose a fresh output directory for each run:

```sh
make tessent-oracle-check BISON=/usr/bin/bison
python3 tests/tessent_oracle.py inventory --output build/oracle-inventory.json
python3 tests/tessent_oracle.py prepare --reference-only --output build/oracle-reference

# Comparison requires generator-produced local-reset output; unsupported cases fail:
python3 tests/tessent_oracle.py prepare \
  --generator build/icl-pattern-generation --output build/oracle-comparison
```

Copy the prepared bundle and `tests/tessent_oracle.py` into a new directory
under `test_icl_toolsets/test_<topic>` in the authorized remote experiment area.
Use descriptive topic/run names without random suffixes and never overwrite a
previous run. On lianghuaxiao's EDA host:

```sh
python3 tessent_oracle.py run --output oracle-reference \
  --tessent /data/eda/mentor/tessent_2026/tessent_2026.2/bin/tessent
```

After copying a comparison bundle's results back:

```sh
python3 tests/tessent_oracle.py compare --output build/oracle-comparison
```

The static trace helper `build/pdl_pattern_trace` uses `pdl::Document`; it never
executes Tcl. It compares a flattened main sequence with Tessent's zero-argument
`oracle` procedure, preserving command order and operand spelling. Comments,
PDL-level declarations and notes are omitted. Dynamic substitution, variables,
unresolved calls/merges and ambiguous entries are unsupported, not silently
accepted. Different vector grouping or scheduling needs review. Equal static
operations alone do not establish hardware equivalence or shortest patterns.

## Measured results, 2026-10-25

Remote experiments ran in the newly created directory
`/data/workspaces/lianghuaxiao/test/tessent_ijtag/test_icl_toolsets/test_oracle`.
Existing external cases were read, not changed. The tool banner was
`Tessent Shell 2026.2 Thu May 21 21:09:45 GMT 2026`; the runner also works with
the host's Python 3.8.16.

| Cases | Expected and observed outcome |
| --- | --- |
| Equal-width Port contribution, `0x0096` | PDL exported successfully |
| 16-bit descending/ascending group, `0bXXXXXXXX10010110` | Both exported PDL successfully; low eight source bits driven and enable asserted |
| Both 16-bit groups, `0x0096`, `0xff96`, `0b10010110XXXXXXXX` | All six rejected writes to bits fixed to unknown |
| Existing `test_iWrite` and `test_multi_from_so_sib` networks | Both exported PDL with explicit scan-register/SIB configuration |

All **11 expected outcomes matched**. All five successful exports were accepted
by the shared-parser static trace helper. These results establish right alignment
with unknown leading bits for the tested OneHotDataGroup **Port** contribution.
They do not establish DataRegister/Instance contribution behavior or padding of
a short numeric value written directly to an ordinary ScanRegister.

The corpus inventory contains 39,223 ICL/PDL files: 19 files used by manifest
cases, 36,588 generated syntax cases, 88 syntax-negative files and 2,528 unpaired
files. Every file has a disposition and hash. Unpaired does not mean invalid:
it means no reviewed complete design/procedure binding has been provided yet.
Syntax-only and negative fixtures cannot safely be treated as executable vendor
scripts. Coverage of all retargetable pairs remains unfinished.

One local inventory run over all 39,223 files, using Python 3.14 on this Linux
workspace while regression was active, took 1.59 seconds and 116,088 KiB peak
RSS (`/usr/bin/time -f 'elapsed=%e peak_kib=%M' python3 tests/tessent_oracle.py
inventory --output build/fresh-inventory.json`). This is a scale observation,
not a comparative speedup claim. Vendor jobs run serially with a 120-second
per-case timeout by default; the observed successful jobs took about 6.6 seconds.

Actual logs, generated PDL and reports are ignored build artifacts in
`build/tessent-reference-final-results-20261025/`. They are not committed golden files.
That historical preparation run reported **11 blocked_generator** cases before
the executable existed. The generator now supports serial and single-TAP
transactions; general local-reset release insertion remains unimplemented. The old
blocked count is not a current generator result; rerun preparation and compare
reviewed pairs before reporting vendor equivalence.
End-to-end comparison, local-reset vendor cases and independent state-equivalence
checks remain requirements; the reference results do not complete them.

## Harness checks

`make tessent-oracle-check BISON=/usr/bin/bison` runs 22 Python harness tests and
three static-trace tests (including multiple rejection subcases). Checks cover
original/rewrite routing, expected-error enforcement, timeout handling, stale
outputs, altered scripts/rewrites, inventory accounting, exact source module
reconstruction (including namespace resets), unsafe Tcl substitution and
ambiguous procedure entries, prefixed and legacy bundle layouts, and unsafe
case paths.
Subprocess protocol fixtures simulate the tools only in unit tests; they are
not evidence of generator/vendor equivalence. Native generator verification is
recorded separately in [the generator guide](pattern-generation.md).

Independent review exposed mixed vector/command substitutions and escaped words
that could become empty operands in a trace, and an unexpected vendor success
that could pass comparison. New failing regressions reproduced both problems;
the trace now requires literal words and comparison requires a successful
reference expectation plus unchanged hashed inputs/outputs. The C++ helper
passes Google clang-format and clang-tidy checks.

The first full regression caught an adapted fixture placed in the exact-byte
vendor-import directory. Moving it to the source-attributed derived-fixture
directory fixed the integration error; the original import manifest and its
integrity checks were kept intact.

After all code corrections, the complete regression exited **0**:

```sh
ICL_REF_DIR=/tmp/icl-reference-honza-20261021 \
PATH="$PWD/build/combined-audit/reference-venv/bin:$PATH" \
make check BISON=/usr/bin/bison -j4
```

The log is `build/tessent-oracle-delivery-regression.log`. It includes static and
dynamic tests, all 334 grammar productions and 119 scanner rules, plus Vim and
Neovim checks over 38,489 ICL and 734 PDL files each. Strict handwritten-code
coverage was not rerun for this test-harness addition; its previously documented
unmet thresholds remain unchanged. A passing regression is not full generator
conformance or completed vendor comparison coverage.

## Addressed-instance data defaults

The additional fixture `tests/cases/tessent/pattern_generation/address_defaults.icl`
is a completed adaptation of the *Tessent Shell Reference Manual v2023.4*,
`Instance`, Example 2. The exact local topic is
`htmldocs/tshell_ref/topics/Command_Instance_id9dd6ea02.html` under the manual
root. `tests/cases/ours/pattern_generation/address_partial_data.icl` supplies
independent explicit-binding, split-bus and insufficient-width variants. All
five use `write_address_child.pdl`; none requires a local-reset TDR release.
They are included in the maintained oracle manifest.

The topology probe `tests/src/tessent_address_defaults.tcl` loads both fixture
files beside the script. Set `ICL_TOOLSETS_PROBE_TOP` to one of the five top
module names below (default `AddressDefaultsTop`). Run in a fresh directory:

```sh
ICL_TOOLSETS_PROBE_TOP=SplitDefaultTop tessent -shell \
  -dofile tessent_address_defaults.tcl -logfile tessent.log
```

It uses `patterns -ijtag`, queries fan-in after `set_current_design` in setup,
then enters analysis to export PDL. Expected-error probes exit nonzero; that is
not a successful pattern export. Use the oracle harness for automated outcome
classification. Tool-generated logs/PDL belong under ignored `build/` locally.

Observed with **Tessent Shell 2026.2** in the isolated remote directory
`/data/workspaces/lianghuaxiao/test/tessent_ijtag/test_icl_toolsets/test_address_defaults`:

| Top module | Observed data connectivity and pattern outcome |
| --- | --- |
| `AddressDefaultsTop` | Child `data[7:0]` maps to parent `data[15:8]`; `iWrite child.r 0x96` exports PDL driving address 5 (instance 4 plus register 1) |
| `SplitDefaultTop` | Child `upper[3:0]` maps to `{zfirst[1:0], asecond[0:1]}`, and `lower[0:3]` maps to `asecond[2:5]`; PDL export succeeds |
| `PartialDataTop` | Explicit child `data[7:4]` maps to `override_data[3:0]`; remaining child data bits have no fan-in; the requested write has no solution |
| `ExplicitDataTop` | Explicit `upper` maps to `override_data`; the entire `lower` port has no fan-in; the requested write has no solution |
| `ShortDataTop` | Elaboration fails with ICL48-1: parent logical data width 4 is smaller than required child width 8 |

IEEE 1687-2014 §6.4.6.6(c), printed p.87, defines logical data ordering:
ports top-to-bottom, each vector left-to-right. §6.4.7(v), printed p.112,
requires addressed-instance data defaults. The narrower-child leading-bit
mapping and suppression of the **entire child logical data bus** by any explicit
data binding are measured vendor behavior, not additional quoted IEEE rules.

The probe also shows that child AddressPort and WriteEnPort/ReadEnPort fan-in
includes address arithmetic/selection: child `address[1]` depends on parent
`address[1]` and `address[0]`; enables depend on the full parent address. Thus
these controls cannot be implemented as ordinary direct wire defaults. This
checkpoint adds only the verified DataInPort defaults; addressed control/model
semantics and end-to-end generator comparisons remain open.

The maintained runner reproduced all **5/5 expected outcomes**, with input and
script hashes matching the staged sources. Both successful exports passed the
shared-parser static trace helper. Retrieved evidence is in
`build/address-defaults-evidence/address-defaults-oracle/`; generated artifacts
are not tracked. The manifest now has 16 cases in total. No run here compares
our generator; the executable and scheduler were unfinished at that checkpoint.

## Addressed register and instance contributors

`tests/cases/ours/pattern_generation/addressed_contributors.icl` supplies complete
self-constructed direct-register and nested-instance probes for IEEE 1687-2014
§6.4.15(b,c). Additional modules exercise mixed source widths and split logical
data buses in local tests. Tessent Shell 2026.2 ran the eight maintained cases in
`/data/workspaces/lianghuaxiao/test/tessent_ijtag/test_icl_toolsets/test_contributors`.

Both tested groups have a 16-bit output supplied by an 8-bit contributor; the
instance case reverses the group/output vector direction. Observed results:

| Operation, for each contributor type | Tessent result |
| --- | --- |
| `iWrite observed 0x0096; iApply` | Rejects the leading output bits as fixed to unknown |
| `iWrite observed 0bXXXXXXXX10010110; iApply` | No solution; writing through this output does not automatically establish the addressed storage value in these probes |
| `iWrite target 0x96; iApply; iRead observed 0bXXXXXXXX10010110; iApply` | Exports PDL successfully |
| Same direct write, then `iRead observed 0x0096; iApply` | Also exports PDL, including comparisons of the high output bits |

`target` is an alias for the actual register. The successful direct writes drive
address 1 in the direct-register case and 17 (instance 16 plus register 1) in the
instance case. The failed writes identify the leading bits as structurally
unknown. A successful `iRead` on a primary output records an expected comparison;
it does **not** prove that the ICL model supplies those expected values or that
hardware would pass the test. In particular, the second read result must not be
interpreted as zero padding. No pattern simulation or hardware-equivalence check
was performed.

The first experimental manifests incorrectly predicted automatic output-write
justification and rejection of expected high read values. Those experimental
runs failed those predictions; their logs are retained under ignored `build/`.
The final manifest records the measured distinctions and reproduced **8/8
expected outcomes**. All four successful PDL exports parsed with the shared trace
helper, and all retrieved input/script hashes match the staged source bytes.
Evidence: `build/address-contributors-final-evidence/`. The maintained manifest
now contains 24 cases. End-to-end comparisons remain blocked by the unfinished
generator/scheduler/local-reset implementation.

## Address-domain probes (2026-10-25)

The self-constructed `address_domains/` fixtures separate each top into its own
input file: Tessent validates modules while loading, so an invalid unused module
can contaminate another probe. Tessent Shell **2026.2** ran these cases in
`/data/workspaces/lianghuaxiao/test/tessent_ijtag/test_icl_toolsets/test_domains`.
The corrected isolated run matched **5/5 measured expectations**:

| Top | Observed result |
| --- | --- |
| `ExplicitParentAddress` | ICL114-1 rejects explicit AddressPort input binding on an AddressValue instance, even when it is the complete parent address bus |
| `ExplicitScanAddress` | ICL128-1 rejects an addressable instance without parent AddressPort definitions; diagnostic explicitly says direct address bindings are currently unsupported |
| `ScanAddressBoundary` | An ordinary bank instance driven by one ScanRegister is accepted; writing `bank.child.r` drives register address 1 plus instance offset 4, write enable, and data through `control[17:0]` |
| `AddressOverflow` | Parent address width 2, instance offset 4, register address 1: exports root address **1**, truncating the mathematical sum 5 |
| `WideZeroAddress` | Direct register address `8'b00000001` under a 2-bit logical address is accepted and exports address 1 |

The bank's scan register declares `ResetValue 0`, required by Tessent ICL79-1
for its ReadEnPort source's implicit `iApplyEndState 0`. The runner still opens
with `-no_initial_ireset`; this declaration does not execute a reset. Earlier
unsuccessful probes lacked that declaration or overrode an unsigned parameter
with a sized value (ICL36-1), so they did not establish address behavior.

IEEE 1687-2014 §6.4.9(r,s,u), printed p.121, requires additive computed addresses
and sufficient logicalAddress width. Our address resolver rejects sum 5 on a
2-bit bus; Tessent's measured truncation is recorded as a vendor difference,
not adopted as a standard rule. Explicit parent bindings remain supported by the
resolver under §6.4.7(s)'s explicit-connection exception, despite the Tessent
limitation. These two cases are intentional compatibility differences and
must not become claims of identical generator/vendor output.

Generated evidence resides in `build/address-domains-corrected-evidence/`.
The maintained manifest contains 29 cases. Three successful exports have been
checked by the shared PDL trace parser; no hardware simulation or completed
pattern-generator comparison is claimed.

## AccessLink with external BSDL

`AccessLink_ActiveSignal` is a complete self-constructed ICL/BSDL/PDL input set
under `tests/cases/ours/pattern_generation/access_link/`. Its BSDL defines a
four-bit TAP instruction register and distinct IJTAG_ON/IJTAG_OFF opcodes. Both
instructions select the instrument interface; only IJTAG_ON activates
`tap.long_setup`, which selects the instrument register in a ScanMux.

Tessent Shell **2026.2** successfully loaded the BSDL, elaborated the network and
exported these operations (no initial reset):

```tcl
iWrite tap.INSTRUCTION[3:0] 0b0011
iRead tap.INSTRUCTION[3:0] 0b0001
iApply
iWrite core.r[7:0] 0b10010110
iApply
```

The initial exploratory fixture included a ResetPort and ResetValue; Tessent
rejected ambiguous reset inference (ICL124-1) because the generated TAP has
multiple ToResetPorts. The final fixture omits reset behavior to isolate
instruction selection; it does not assume the register starts at zero. The
handwritten exploratory runner is `tests/src/tessent_access_link.tcl`.
Generated expanded ICL stays under ignored `build/`; it parses with our shared
ICL frontend but is not a verified independent model of the TAP.

The maintained oracle manifest accepts an optional **single** `bsdl` repository
path per case, matching Tessent's `read_icl -bsdl_file` option. Each such case
also requires `bsdl_entity`, naming the entity explicitly (`AccessProbe` in the
maintained fixtures). The harness stages the file as `model.bsdl`, checks its
hash along with the ICL/PDL/script, and forwards
`--bsdl ENTITY model.bsdl` to the generator in comparison mode.
Inventory includes explicitly declared inputs regardless of filename extension
(including `.vhd` BSDL files). A missing, changed or out-of-repository BSDL does
not silently disappear. The historical run preceded generator BSDL/TAP support;
current comparison preparation reaches the unimplemented local-reset rewrite.

The maintained runner reproduced the positive outcome, all staged input/script
hashes match, and the shared PDL parser confirms the five operations above.
Evidence: `build/access-link-oracle-evidence/`, from the isolated remote
`test_icl_toolsets/test_domains` directory. The manifest now has 30 cases;
this is a vendor reference test, not generator equivalence or HDL simulation.

The final inventory after this addition records 35 manifest-assigned source
files, 36,588 generated syntax inputs, 88 syntax-negative files and 2,529 unpaired
files (39,240 total, now including BSDL inputs). Running comparison preparation
for the AccessLink case returned `blocked_generator` at that checkpoint,
when the executable was absent. Its 17 harness tests and three shared-parser
trace tests passed; they did not substitute for generator comparison.

After the final inventory correction, the complete repository regression exited
0 (`build/address-accesslink-final-regression.log`), including all 17 oracle
harness tests, three trace tests, 334/334 grammar productions, 119/119 scanner
rules, and Vim/Neovim checks over 38,500 ICL / 739 PDL files each. The separate
strict C++ coverage gate remains below its unchanged thresholds; the most recent
measurement is documented in [model coverage](pattern-model-coverage.md).
## Indexed ActiveSignal probes (2026-10-25)

Three self-constructed variants under
`tests/cases/ours/pattern_generation/access_link/` exercise descending `[3:1]`,
ascending `[1:3]` and single-bit `[3]` ActiveSignals, each connecting an indexed
bit to the instrument's mode input. The variants load `simple.icl` first and
replace only `AccessTop`; the common instrument definition is not duplicated.
The maintained manifest records all three as expected vendor syntax errors.

Tessent **2026.2** rejected each ActiveSignals bracket with **ICL1-1**,
`Incorrect character sequence`, before pattern generation. The initial hypothesis
that these variants would run was false; its failed results remain in ignored
`build/tap-signals-evidence`. A repeat with explicit vendor-error expectations
passed all three expected outcomes and input/script hash checks in
`build/tap-signals-confirmed-evidence`. The post-deduplication repeat also matched
all three expected syntax errors, with all staged input/script hashes verified,
in `build/tap-signals-dedup-evidence`. No PDL pattern comparison passed in these runs.

IEEE 1687-2014 §6.4.17's grammar permits `reg_port_signal_id` for ActiveSignals.
The repo parser therefore continues to accept these indexed forms. The native instruction component now models indexed outputs using the shared
ICL range and parameter evaluator; full TAP integration remains unfinished.
The observed vendor restriction does not remove the IEEE support requirement
or establish a successful pattern-equivalence comparison.
