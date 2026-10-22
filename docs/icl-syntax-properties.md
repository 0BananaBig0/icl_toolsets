# Independent syntax properties and live diagnostics — 2026-10-22

## Why another method

Production and scanner-rule coverage shows that tests reached our implementation;
it cannot show that a missing or incorrect rule matches IEEE 1687. The new
`make syntaxcheck` uses **metamorphic testing**: transform a known valid input in
ways the standard permits, and require it to remain accepted. Expected results
come from those permissions, not from parsing our `.yy` or `.l` files. Unlike the
crash fuzzer, a clean rejection of a valid transformation fails this check.

## Normative basis and transformations

The reference is IEEE 1687-2014, printed pages 59–62 (PDF pages 73–76):
§6.3.2 tokens, §6.3.4(11–12) grouping and whitespace, §6.3.6 whitespace,
§6.3.7 comments, and §6.3.8 numeric expressions. For each eligible positive
seed, in manifest/block order, the runner:

1. Inserts a block comment and a line comment at each existing whitespace gap.
2. Replaces whitespace runs with CRLF or newline-plus-tab separators.
3. Wraps literal numeric indices/range endpoints in parentheses to depths 1, 4
   and 16, exercising recursive numeric expressions.
4. Inserts a forbidden nested block comment and requires rejection.

Quoted strings and existing comment contents are protected. Line-comment newline
termination is retained. This checks existing token boundaries; it does not claim
that comments may be inserted inside identifiers or between an apostrophe and a
base letter. Unit tests check these transformations independently of the parser.

The maintained script is `tests/src/test_icl_metamorphic.py`. Generated inputs and
`results.json` stay under ignored `tests/generated/ours/metamorphic/`. The report
maps seed indices to conformance entries, including original source attribution,
and records counts, exclusions and unexpected results. The runner passes batches
of 128 paths to the existing inventory helper instead of starting a parser process
for each input. `--block enum` selects a block; unknown or empty selections fail.

## Scope and adjudication

Positive conformance seeds longer than 4096 characters, the `eda-corpus` block,
files named for Generic AccessLink payloads, and Origen `#include` examples are
excluded. This bounds generated storage and avoids applying ICL token rules to
opaque vendor payloads. Their original acceptance expectations remain covered by
the corpus and live LSP suites. Generated variants are checked for acceptance,
not formatter output equivalence.

The first whitespace/comment run found two rejections: comments inserted inside
Origen `#include` syntax. That directive is not an IEEE construct. It was excluded
from this IEEE oracle; no relaxation of the grammar was justified. The expanded
run also covers nested expressions and forbidden comments. No IEEE mismatch has
been reproduced by this campaign; this is not proof of full language conformance.

## Live syntax checking

`tests/src/test_icl_diagnostics.py` sends every classified conformance case as a
versioned edit over real stdio LSP transport. Its expected acceptance is maintained
in the manifest, rather than copied from formatter output. Dedicated fixtures
cover unexpected parser tokens, illegal characters, NUL, EOF (including the Generic AccessLink scanner state), Unicode before an
error, semantic error ranges in CRLF files, stale edits, correction, close and
reopen. The LSP retains the shared parser's first error, including byte locations,
and converts locations to UTF-16 for editor clients. Semantic errors that supply
only a line number retain a whole-line range, excluding CRLF.

Diagnostics are published on open/edit, carry the tracked document version, clear
on correction/close, and reset version tracking when reopened. Checking a valid
document no longer renders formatted text just to obtain a success status. The
shared parsed document and AST still serve both executables; no separate LSP
parser or PDL tokens were introduced.

## Reproduction and limits

```sh
make syntaxcheck BISON=/usr/bin/bison
python3 tests/src/test_icl_diagnostics.py --binary build/icl-lsp
make check-dynamic formatcheck namingcheck BISON=/usr/bin/bison -j4
```

See the verification record below for final counts and measured runtime. Finite
transformations cannot prove all recursive inputs or catch every missing syntax
family. Existing normative block review, upstream comparison, golden tests and
coverage gates remain necessary. This work does not implement multi-error parser
recovery, full parameter/width elaboration or vendor connectivity DRC.

## Verification record

- `make check-dynamic formatcheck namingcheck BISON=/usr/bin/bison -j4`:
  the functional, corpus, style and naming suites pass. The manifest contains
  1,885 acceptance/rejection cases, all also tested through live LSP edits.
- `make syntaxcheck`: **25,938 variants across 45 blocks**, from 649 positive
  seeds; 136 seeds excluded; **zero unexpected results**. Four oracle tests and
  five diagnostic test methods pass (the latter include the entire manifest).
- Grammar/scanner rule coverage remains **334/334 productions and 119/119 rules**.
  This is rule reachability, not full handwritten line/branch coverage; the
  previously documented coverage gate remains open.
- Independent review found a Generic AccessLink EOF range bypassing the default
  scanner EOF adjustment. Both newline/no-newline regressions failed before the
  fix and passed afterward; the reviewer confirmed the fix with no remaining
  findings. A semantic CRLF range regression also failed before correction.

On Linux 6.18.33.2 WSL2 x86_64, the standalone variant campaign took **3.24 s**
wall time and **20,860 KiB** peak RSS (`/usr/bin/time -f '%e %M'`, using the already
built inventory helper and existing generated directory). This measures the
runner and its subprocess maximum, not summed memory across all processes.

`python3 tests/src/benchmark_icl_lsp.py --binary build/icl-lsp --output
build/syntax-lsp-performance.json` observed open-to-diagnostic latency of 7.3 ms
for the 141,671-byte merged Tessent input and 106 ms for its 2,266,736-byte 16-fold
repeat. Five formatting requests had medians of 1.2 ms and 20.4 ms respectively.
An existing main-worktree binary observed 8.6/166 ms opens in a separate smoke
run. These single open samples ran alongside other checks and do not establish
a statistically meaningful speedup or a larger-workspace bound.

The final full suite retained rule reachability over 2,153 corpus files. Five
ANTLR integration tests were skipped by the ordinary Python environment; that
optional reference runtime was not revalidated during this task. Static runtime
builds and the separate Neovim built-in client were not run.

`make dist BISON=/usr/bin/bison` followed by an isolated archive build with
`dynamic BISON=false FLEX=false -j4` passed. Both archive executables passed
1,885 conformance inputs and the five live diagnostic tests. `make
install-dynamic BISON=/usr/bin/bison` installed both dynamic commands into
`~/.local/bin`; the same conformance/diagnostic checks passed against those
installed programs. Installation printed both coc.nvim configurations and did
not change personal editor settings.

A real Vim 9.2/coc.nvim run passed diagnostics, clean-buffer checks, formatting,
invalid-buffer preservation, definitions, hover, symbols and completion. It used
an isolated `/tmp/icl-coc-syntax-20261022` config/data/test tree, populated with
`docs/editor/coc-lsp.json` and `tests/cases/ours/editor/*.icl`, then:

```sh
COC_ROOT=/tmp/icl-coc-syntax-20261022 \
COC_PLUGIN=/home/banana/.vim/plugged/coc.nvim COC_VIM=/usr/bin/vim \
bash tests/run_coc_check.sh
```

Initial attempts before populating that test directory failed because settings
and fixture files were absent; those were setup failures, not passing checks.
The optional coc-diagnostic CLI adapter remains untested end to end.
