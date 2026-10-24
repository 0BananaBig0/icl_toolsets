# Symbol identity and namespace scope review

Baseline: `c0a18f6`. This pass traced PDL symbol indexing and namespace scopes,
and inspected shared formatter file output and Vim runtime setup. The confirmed
defects were in PDL navigation; this is not a new exhaustive syntax audit.

## Findings and corrections

- **Distinct Tcl procedures could resolve to the same definition.** Index keys
  concatenated binding, namespace, and name with newline separators. Tcl permits
  literal newlines in names: namespace `a\nb` / procedure `c` collided with
  namespace `a` / procedure `b\nc`. The latter declaration overwrote the former
  in the per-document index, producing wrong definitions and merged references.
  A tuple key now preserves component boundaries in both index construction and
  lookup. This removes the serialization helper and follows the existing
  variable/member index design. Same-document redefinitions and cross-document
  ambiguity keep their existing behavior.
- **Namespace bodies inherited procedure-local variable ownership.** A
  `namespace eval N` nested in a procedure incorrectly bound its `set x` and
  `$x` to that procedure's local `x`. Namespace evaluation now creates a
  namespace variable frame. Unknown namespace names remain unresolved instead
  of inventing local targets. Parent-word substitutions still use the parent
  frame.
- Independent review caught a completion side effect of that scope correction:
  the unfinished-procedure context fallback also covered substituted namespace
  bodies. Restricting it to `proc`/`iProc` preserves completion of `x` in
  `namespace eval N "puts $x"` inside a procedure with argument `x`.

Four tests in `tests/src/test_pdl_tools.py` cover distinct multiline names,
definition/reference separation, known and unknown namespace scopes, and
parent-frame substitution completion. The original-defect tests failed before
their fixes; the completion test failed before the review correction. No
collected PDL fixture was evaluated.

## Design and performance

The existing shared PDL document/AST remains the source for formatting and LSP.
ICL and PDL grammar/token definitions remain separate. Tuple keys retain
logarithmic map lookup and avoid concatenating key fields; no runtime or memory
speedup is claimed without comparative measurements. No new dependency, cache,
or parallel semantic model was introduced. Unrelated formatter copy changes
and broader refactoring were not justified by this pass.

## Verification

The final `make check-dynamic` completed with exit 0, including parser,
formatter, LSP, corpus, conformance, and differential/property regressions.
All 334 grammar productions and 119 scanner rules were exercised. Both Vim and
Neovim passed 38,488 ICL and 728 PDL files, plus standalone/vim-plug installation
checks. Independent review reported no remaining actionable findings after the
completion correction. Diff and generated-artifact checks passed.

The final dynamic/static PDL suite passed all 58 tests, including the four new
regressions. Static focused checks also passed the 13 shared CLI tests, 14 shared
LSP tests, 45 ICL LSP tests, and 13 ICL / 6 PDL golden cases. The existing
32,000-variable and 32,000-ICL-object reference scaling tests passed their
two-second bounds (approximately 0.013 s / 0.005 s in the static run; inputs
926,670 / 1,001,811 bytes). These are bounded regression checks, not a
before/after performance comparison. Google formatting and naming checks passed.

Coverage's functional tests passed. The unchanged strict gate still fails:
**4,308/4,418 lines (97.5%)** and **7,382/11,965 branches (61.7%)**, versus the
required 100% / 90%. The repository-wide coverage shortfall remains open.
Static linking also retains the existing glibc/Tcl NSS linker warnings; the
local static tests do not establish portability to different glibc systems.

```sh
make dynamic static unitcheck formatcheck namingcheck BISON=/usr/bin/bison -j4
ICL_REF_DIR=/tmp/icl-reference-honza-20261021 \
  PATH="$PWD/build/combined-audit/reference-venv/bin:$PATH" \
  make check-dynamic BISON=/usr/bin/bison -j4
make coverage BISON=/usr/bin/bison -j4
python3 tests/src/test_pdl_tools.py \
  --formatter build/pdl-format-static --server build/pdl-lsp-static
python3 tests/src/test_pdl_document.py --server build/pdl-lsp-static
git diff --check
python3 tests/tree_clean_test.py
```

Logs and build artifacts stay ignored under `build/symbol-review-*` and
`.worktree/coverage`. No commit, push, installation, or live coc.nvim session was
performed. The final full-run log is `build/symbol-review-final-regression.log`;
the final coverage log is `build/symbol-review-coverage.log`.
