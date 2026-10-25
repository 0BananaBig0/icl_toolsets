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
Normal preparation currently reports **11 blocked_generator** cases because
`icl-pattern-generation` and its local-reset rewrite are not implemented yet.
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
not evidence that the generator exists or performs retargeting.

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
