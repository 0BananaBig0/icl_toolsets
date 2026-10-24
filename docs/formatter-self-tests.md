# Formatter self-tests and saved expected output

For an input file `A`, let `B = F(A)` and `C = F(B)`, using the same formatting
options each time. The tests enforce two separate properties:

1. **Idempotence:** `C == B`, compared byte for byte. The first output is the
   expectation for the second pass; no second `.expected` file is needed.
2. **Golden regression:** `B == G`, where `G` is the previously reviewed saved
   output. This detects changes across program versions, even if the new
   behavior is itself stable.

The CLI golden runner checks filename and stdin input agreement, exit status,
timeouts, and the second formatting pass. With `--server`, it also checks:

- Opening `A` reports no syntax diagnostics and formats to `B`.
- Sending `B` through `didChange` reports no syntax diagnostics and returns no
  formatting edit. This checks the editor update path as well as stability.
- Whole-document edit ranges match the source, including UTF-16 positions.

`make goldencheck`, `make check`, `make check-dynamic`, and `make coverage`
include both CLI/LSP pairs. Standalone use remains available:

```sh
make goldencheck BISON=/usr/bin/bison
python3 tests/golden_test.py --binary build/icl-format --server build/icl-lsp
python3 tests/golden_test.py --language pdl \
  --binary build/pdl-format --server build/pdl-lsp
# Focus on one group of cases:
python3 tests/golden_test.py --language pdl --case 'nested*' \
  --server build/pdl-lsp
```

There are currently 13 ICL pairs under `tests/golden/` and six PDL pairs under
`tests/cases/ours/pdl/golden/`. Inputs use `.icl` or `.pdl`; saved expected bytes
use `.expected`. Existing corpus, conformance, and property tests provide
broader idempotence checks. The golden suite intentionally pins selected layout
contracts rather than storing another copy of the entire generated corpus.

## Updating the baseline

Ordinary regression never creates or changes expected files. A missing
expectation or mismatching output is a failure. After an intentional formatting
change, review its output and independent syntax/content tests, then explicitly
update the affected expectations:

```sh
python3 tests/golden_test.py --language pdl --case 'nested*' \
  --server build/pdl-lsp --update
git diff -- tests/cases/ours/pdl/golden/
make check-dynamic BISON=/usr/bin/bison -j4
```

An update checks filename/stdin agreement, second-pass stability, and any
requested LSP checks for **all selected cases before writing any expectation**.
Validation failure leaves them unchanged. Writes are ordinary file writes;
filesystem failures during the write phase are reported but are not a
multi-file transaction. Update only after reviewing the reason for a change;
a failed comparison is not itself a reason to accept new output.

Only intentionally maintained `.expected` files belong in Git. Temporary
formatted files, logs, generated corpora, parser/scanner output and binaries
remain untracked/ignored. This change does not regenerate any golden files.

## What these checks cannot prove

A formatter that consistently deletes a declaration can still be idempotent.
Recording that bad output as a golden would preserve the bug. Keep independent
syntax acceptance/rejection, token/literal preservation, standards-based cases,
and reference comparisons. Likewise, formatting checks do not establish LSP
definition/reference/completion correctness; those have separate protocol and
semantic tests. Collected PDL is parsed and formatted, never evaluated.

## Validation of this change

The new runner tests first failed on the previous runner. All 19 runner tests
now pass, including unstable second output, crashes/timeouts, wrong-but-stable
formatting, LSP diagnostic/response/range mismatches, and update failure paths.
All 13 ICL / 6 PDL golden cases pass through both dynamic and static CLI/LSP
pairs. Independent review found no actionable issues.

Full `make check-dynamic BISON=/usr/bin/bison -j4` passed, including 334/334
grammar productions, 119/119 scanner rules, and both Vim and Neovim checking
38,488 ICL / 728 PDL files. The run used the existing local Honza reference
checkout and reference virtual environment. Diff/artifact checks passed and
no expected output files were modified.

Coverage's functional suites passed. Its unchanged strict gate remains below
100% lines / 90% branches: 4,308/4,418 lines (97.5%) and 7,382/11,965 branches
(61.7%), identical to the preceding review. Logs remain ignored at
`build/self-test-regression.log` and `build/self-test-coverage.log`.
