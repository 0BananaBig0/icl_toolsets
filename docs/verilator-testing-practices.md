# Practices adapted from Verilator

Reference inspected on 2026-10-24: [verilator/verilator](https://github.com/verilator/verilator)
at `63685a23525c7022e17c0b4e5fea6226c42b0306`. The downloaded files remain
under ignored `build/verilator-review/`; nothing from Verilator is compiled,
executed or copied into our source/tests.

## What was inspected

- [test_regress/driver.py](https://github.com/verilator/verilator/blob/63685a23525c7022e17c0b4e5fea6226c42b0306/test_regress/driver.py):
  scenario selection, per-test output directories, expected-file comparison,
  explicit golden updates, individual test selection and CPU-time limits.
- [t_lint_width_bad.py](https://github.com/verilator/verilator/blob/63685a23525c7022e17c0b4e5fea6226c42b0306/test_regress/t/t_lint_width_bad.py)
  and its adjacent `.out`: a small driver explicitly expects a failure and
  compares the diagnostic output with maintained expectations.
- [docs/internals.rst](https://github.com/verilator/verilator/blob/63685a23525c7022e17c0b4e5fea6226c42b0306/docs/internals.rst):
  parsing into an owned AST, separate linking/elaboration/optimization passes,
  and Visitor-based algorithms separated from node representation.

## Adaptation decisions

| Practice | Decision for this repository |
|---|---|
| Small source/expectation pairs | Extend the existing ICL golden runner to PDL; retain `.expected` naming. Six new handwritten PDL pairs pin layout and literal preservation. |
| Shared driver, explicit scenarios | One runner selects `--language icl` or `pdl` and checks filename/stdin scenarios. No second PDL-only driver. |
| Focused reruns | Add `--case 'pattern'` matching case stems; no matches is an error. |
| Bounded tests | Use a configurable ten-second wall-clock timeout per formatter call. This also catches blocked I/O; it is not Verilator's inherited CPU limit implementation. |
| Reviewable expectations | Byte comparison decides success; unified diffs explain failures. Normal regression never updates expectations. Explicit `--update` requires successful, equal filename/stdin outputs. |
| Negative test diagnostics | Already provided by PDL block expectations and ICL diagnostic fixtures, including live LSP ranges. Keep these domain-specific runners. |
| Isolated build artifacts | Already provided by ignored build directories and temporary workspaces. New reference downloads/logs stay under build; maintained expectations stay beside inputs. |
| Compiler AST passes | Already reflected in shared ICL AST/Visitor, semantic indexing and formatting layers. Keep PDL's separate Tcl-backed tree. No new compiler pipeline or elaborator is needed for these tools. |
| Compiler-scale scheduler and simulator matrix | Not adopted: our new golden suite has 19 small cases and 38 formatter invocations; multiprocessing/framework dependencies would add complexity without an established bottleneck. |
| Verilator source naming/layout | Not copied: keep Google-style names, `src/`, provenance-classified `tests/cases/`, developer `docs/`, and the independently installable Vim runtime. |

This improves test organization rather than moving working production files.
The runner now also checks second-pass CLI stability and both LSP formatting
passes; see [formatter self-tests](formatter-self-tests.md) for the current
commands and baseline-update safeguards. The counts below describe the original
Verilator-inspired change.
Source/expected pairs change together. The reusable runner owns execution and
comparison; its self-tests live in `tests/tests/test_golden_test.py`. Language
fixtures retain separate roots, and both formatter/LSP pairs still share their
existing per-language document and formatting code. No production code changed.

## New coverage

The [PDL golden fixtures](../tests/cases/ours/pdl/golden/README.md) cover recursive
iProc/if/foreach bodies; namespace/proc/try handlers; multiline braced data;
quoted text and substitutions; constant/dynamic argument expansion; and comments
adjacent to commands. Expected bytes were written and reviewed as formatting
contracts, not recorded from the current executable. The unchanged formatter
passed them. All six also enter `pdl_blocks.json` for diagnostics, idempotence
and CLI/LSP parity; the editor corpus runner discovers them automatically.

Runner self-tests check exact-byte failures despite stable token content,
both input modes, explicit update safeguards, empty discovery, unmatched filters,
missing expectations, nonzero/signal exits, and a real sleeping process killed
by timeout. The timeout protects direct formatter children; this is not a general
process-tree supervisor for arbitrary programs that spawn background children.

```sh
make goldencheck BISON=/usr/bin/bison
python3 tests/golden_test.py --language pdl --case 'nested*'
python3 tests/golden_test.py --binary build/icl-format-static
python3 tests/golden_test.py --language pdl --binary build/pdl-format-static
# For slow instrumented builds, explicitly raise the per-invocation limit:
python3 tests/golden_test.py --language pdl --timeout 30
```

`make check`, `make check-dynamic` and `make coverage` include both-language
goldens and runner self-tests. Existing ICL commands remain compatible; the
default language/binary is ICL. Regressions remain offline and never evaluate PDL.

## Validation record

Before implementation, the new runner tests reported eight failures against the
old runner. After implementation, all 11 self-tests, 13 existing ICL goldens,
six new PDL goldens and six new live LSP block cases passed.

| Check | Result |
|---|---|
| `make check-dynamic BISON=/usr/bin/bison -j4` | Passed, exit 0, including 207 PDL block cases and 438 PDL syntax-property cases. |
| Grammar/scanner reachability within full regression | 334/334 ICL grammar productions and 119/119 scanner rules reached across 2,325 files. This is rule reachability, not proof of complete language support. |
| Vim and Neovim corpus checks within full regression | Each checked 38,488 ICL files (449,354 lines) and 728 PDL files (76,607 lines), with no errors. All six editor tests passed, including standalone and vim-plug loading. |
| Static executable checks | All 13 ICL and six PDL goldens, four shared LSP protocol tests, and six new PDL block cases passed. |
| `make formatcheck namingcheck BISON=/usr/bin/bison` | Passed. |
| Independent change review | No actionable findings; reviewer independently reran the 11 runner tests and both golden suites successfully. |
| `make coverage BISON=/usr/bin/bison -j4` | All functional checks passed, including the newly integrated runner/golden tests. The strict gate failed (exit 2): 4,141/4,248 lines (97.5%) and 7,002/11,401 branches (61.4%), below the unchanged 100%/90% thresholds. |
| `git diff --check` and `python3 tests/tree_clean_test.py` | Passed; no generated files among the 2,805 tracked files. New files are maintained tests, expectations and documentation. |

Full regression used the existing local Honza reference checkout through
`ICL_REF_DIR=/tmp/icl-reference-honza-20261021` and the existing reference virtual
environment at `build/combined-audit/reference-venv/bin` on `PATH`. Logs remain
under ignored `build/verilator-review/`.

The coverage failure was checked against the pre-change
[runtime review baseline](runtime-corpus-review.md): both covered/total counts
are identical. It is an existing coverage gap, not a newly failing functional
case. No production source, coverage exclusions or thresholds changed. These
new tests strengthen formatting contracts and test-runner failure handling;
they do not establish full ICL/PDL syntax support or close the strict coverage
requirement. Static verification was focused; the complete functional suite
ran against the dynamic binaries.
