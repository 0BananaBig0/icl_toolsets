# Expression, navigation, and LSP position fixes

This change addresses the three reproduced findings from the review of
`5c064fc`. ICL and PDL continue to use separate syntax and semantics; the new
shared component only converts source byte offsets to UTF-16 positions.

## Correctness changes

- Literal `if`, `elseif`, `while`, and `for` expressions receive Tcl syntax
  checking whether braced, quoted, or bare. Dynamic words remain unevaluated.
- The parser explicitly distinguishes a PDL vector suffix from a genuine Tcl
  literal. Independent review caught a false rejection of `if 1[0] {}` when
  command `0` supplies part of the expression. Tests reproduced that failure
  before correcting control-flow and `expr` handling.
- PDL's ICL member index excludes `EnumMember`, matching ICL's ordinary member
  lookup. Registers and instances are no longer made ambiguous by enum labels
  with the same name. Duplicate ordinary objects remain ambiguous, and enum
  labels alone do not become object navigation targets.

## Position lookup and measurements

Each immutable document owns a UTF-16 prefix-count index with one checkpoint
per 256 source bytes. A lookup scans at most 255 bytes at each endpoint instead
of repeatedly scanning an entire line. Stored counts use about one `size_t`
per 256 bytes, roughly 3.1% of source size on this 64-bit platform. Building the
index is linear in input size. Document replacement constructs a fresh index.

The existing byte-prefix convention is retained even inside a multibyte code
point. Tests cover every byte prefix across two-, three-, and four-byte UTF-8,
CRLF, empty input, exact checkpoint boundaries, long Unicode lines, and edits.

Measured on WSL2 Linux 6.18.33.2, GCC 16.2.1, dynamic `-O2` binaries. Each query
ran after parsing completed, with three repetitions and unchanged reference
counts. These are medians for the same single-line inputs used during review:

| Input | Source bytes | Reference locations | Before | After |
| --- | ---: | ---: | ---: | ---: |
| ICL: one port and 16,000 output sources | 516,916 | 16,001 | 3.2841 s | 0.0627 s |
| PDL: one variable and 32,000 uses | 288,010 | 32,001 | 3.6478 s | 0.1197 s |

Equivalent multiline inputs remained fast: post-change medians were 0.0609 s
and 0.1610 s. The maintained long-line regression also inserts Unicode and
requires exact reference ranges and query latency below two seconds. It is in
`tests/src/test_lsp_server.py`; parse time is excluded from that query bound.

## Regression failure investigation

The first full regression failed in `gramcov` with a segmentation fault on an
existing enum case. Its `format_job.o` was built against the old document
layout while `icl_semantics.o` had been rebuilt against the new layout.
Grammar-coverage recipes lacked header dependency files.

The repair enables `-MMD -MP`, includes the generated dependency files, and
rebuilds legacy grammar-coverage objects when the Makefile changes. Unit-test
prerequisites also include both shared text headers. Maintained Makefile tests
first reproduced missing invalidation and then passed; independent review
verified the corrected build graph. No test threshold was weakened.

## Verification

Dynamic/static builds, Google formatting/naming checks, 60 C++ unit checks,
53 PDL tool tests, 13 shared-server tests, and dependency checks pass. Static
checks also passed 45 ICL LSP tests, 13 ICL and 6 PDL golden cases, and
32,000-element variable/object lookup cases. The complete final
`make check-dynamic` rerun exited 0: all 334 grammar productions and 119 scanner
rules were exercised across 2,325 grammar-coverage files, and both Vim and
Neovim passed 38,488 ICL and 728 PDL files. Standalone/vim-plug installation and
large-input plugin checks also passed. Final style, `git diff --check`, and
source-tree generated-artifact checks passed.

```sh
make dynamic static unitcheck formatcheck namingcheck BISON=/usr/bin/bison -j4
ICL_REF_DIR=/tmp/icl-reference-honza-20261021 \
  PATH="$PWD/build/combined-audit/reference-venv/bin:$PATH" \
  make check-dynamic BISON=/usr/bin/bison -j4
make coverage BISON=/usr/bin/bison -j4
```

The strict coverage command's functional suites passed, but its existing gate
still fails: **4,295/4,405 lines (97.5%)** and **7,344/11,927 branches (61.6%)**,
below the required 100%/90%. The new position header is included and has 18/18
lines covered; its remaining three branch edges are allocation exception paths.
Logs and generated outputs remain ignored under `build/` and `.worktree/`.
No commit, push, user installation, or live coc.nvim session is included.
