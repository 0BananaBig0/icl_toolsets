# Tessent PDL differential tests

`tests/tessent_oracle.py` prepares isolated, serial Tessent runs and records
input SHA-256 hashes, exit codes, timeouts, tool banners and expected diagnostics.
It exports **PDL** through `write_patterns -pdl`, not STIL or Verilog patterns.
No normal regression requires a Tessent license or SSH connection.

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

# Once the generator and local-reset output exist:
python3 tests/tessent_oracle.py prepare \
  --generator build/icl-pattern-generation --output build/oracle-comparison
```

Copy the prepared bundle and `tests/tessent_oracle.py` into a new directory
under the authorized remote experiment area. On lianghuaxiao's EDA host:

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
`/data/workspaces/lianghuaxiao/test/tessent_ijtag/icl_toolsets_oracle_20261025_HygZyT`.
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
transactions; its separate local-reset rewrite remains unimplemented. The old
blocked count is not a current generator result; rerun preparation and compare
reviewed pairs before reporting vendor equivalence.
End-to-end comparison, local-reset vendor cases and independent state-equivalence
checks remain requirements; the reference results do not complete them.

## Harness checks

`make tessent-oracle-check BISON=/usr/bin/bison` runs 15 Python harness tests and
three static-trace tests (including multiple rejection subcases). Checks cover
original/rewrite routing, expected-error enforcement, timeout handling, stale
outputs, altered scripts/rewrites, inventory accounting, exact source module
reconstruction (including namespace resets), unsafe Tcl substitution and
ambiguous procedure entries.
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
`/data/workspaces/lianghuaxiao/test/tessent_ijtag/icl_toolsets_address_defaults_20261025_Mwxclr`:

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
our generator, which still lacks its executable and scheduler.

## Addressed register and instance contributors

`tests/cases/ours/pattern_generation/addressed_contributors.icl` supplies complete
self-constructed direct-register and nested-instance probes for IEEE 1687-2014
§6.4.15(b,c). Additional modules exercise mixed source widths and split logical
data buses in local tests. Tessent Shell 2026.2 ran the eight maintained cases in
`/data/workspaces/lianghuaxiao/test/tessent_ijtag/icl_toolsets_contributors_20261025_BN5L5k`.

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
`/data/workspaces/lianghuaxiao/test/tessent_ijtag/icl_toolsets_domains_20261025_DEq8hc`.
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
path per case, matching Tessent's `read_icl -bsdl_file` option. The harness stages
it as `model.bsdl`, checks its hash along with the ICL/PDL/script, and forwards
`--bsdl model.bsdl` to the proposed generator input interface in comparison mode.
Inventory includes explicitly declared inputs regardless of filename extension
(including `.vhd` BSDL files). A missing, changed or out-of-repository BSDL does
not silently disappear. The generator's BSDL loader and TAP construction remain
unimplemented, so comparison is still blocked when the generator is absent.

The maintained runner reproduced the positive outcome, all staged input/script
hashes match, and the shared PDL parser confirms the five operations above.
Evidence: `build/access-link-oracle-evidence/`, from the isolated remote
`icl_toolsets_domains_20261025_DEq8hc` directory. The manifest now has 30 cases;
this is a vendor reference test, not generator equivalence or HDL simulation.

The final inventory after this addition records 35 manifest-assigned source
files, 36,588 generated syntax inputs, 88 syntax-negative files and 2,529 unpaired
files (39,240 total, now including BSDL inputs). Running comparison preparation
for the AccessLink case returns `blocked_generator`, as expected while the real
executable is absent. The 17 harness tests and three shared-parser trace tests
pass; they do not substitute for the blocked generator comparison.

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
