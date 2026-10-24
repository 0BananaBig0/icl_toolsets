# CLI, Tcl namespace, and source ownership review

Reviewed baseline: `7271823`. This pass focused on shared CLI/LSP boundaries,
PDL declaration/use resolution, and avoidable work in the existing shared
processing path. The existing ICL/Vim implementations were retained; no broad
refactor or speculative deletion was justified by these findings.

## Confirmed findings and changes

1. Both formatters accepted `--indent 4junk` and `--indent 4294967297` because
   `atoi` ignored trailing text and could overflow. Checked `from_chars`
   parsing now rejects malformed/out-of-range/nonpositive values before any
   file write. Existing `+4`, `04`, and the documented indentation cap remain
   supported. Tests also check that rejected `-w` calls preserve source files.
2. PDL navigation failed for equivalent Tcl command/variable namespace paths,
   including `::n::::target`, `:::n::target`, and a `::::n` namespace. Tcl
   qualification now collapses separator runs consistently, including global
   fallback. Single colons within names are retained. This is confined to Tcl
   name resolution; IEEE PDL identifier validation and source spelling are
   unchanged. Definition/reference tests cover both procedures and variables.
3. Shared LSP Open accepted a temporary owned string by reference, then copied
   the entire source again into the parser. It now takes ownership and moves
   the text into the existing language adapter. Large open/change tests verify
   source retention, formatting, symbol updates, and stale-version rejection.

The bug tests failed on the original binaries and passed after correction.
Independent review found no additional issues; its Tcl/LSP probes also checked
relative-name global fallback from a different namespace.

## Memory measurement

Environment: WSL2 Linux 6.18.33.2, GCC 16.2.1, dynamic `-O2` binaries. Each
sample used a fresh server, initialized it, opened a 4,194,319-byte file, waited
for a `workspace/symbol` response, and read `/proc/PID/status` VmHWM. Input was
a 4 MiB comment followed by one module or procedure. Three repetitions:

| Server | Before peak RSS (KiB) | After peak RSS (KiB) |
| --- | --- | --- |
| ICL | 33,780 / 33,596 / 33,464 | 29,688 / 29,684 / 29,688 |
| PDL | 26,268 / 26,196 / 26,400 | 22,420 / 22,436 / 22,236 |

This is consistent with eliminating one approximately 4 MiB temporary copy.
Open/barrier timings overlapped (ICL about 0.055–0.068 s; PDL about
0.033–0.039 s), so no runtime speedup is claimed. Regression does not impose a
platform-dependent RSS threshold.

## Verification

Dynamic/static builds and Google formatting/naming checks passed. Focused
dynamic checks passed 13 shared CLI tests, 54 PDL tool tests, and 14 shared LSP
tests. Complete `make check-dynamic` exited 0, including all 334 grammar
productions and 119 scanner rules. Vim and Neovim each passed the 38,488 ICL /
728 PDL corpus and standalone/vim-plug installation checks. Static verification
passed the same focused CLI/PDL/shared-server suites, 45 ICL LSP tests, and
13 ICL / 6 PDL golden cases. Diff and generated-artifact checks passed.

Coverage's functional suites passed, but the unchanged strict 100% line / 90%
branch gate still fails: **4,309/4,419 lines (97.5%)** and **7,377/11,969 branches
(61.6%)**. This is the existing repository-wide coverage shortfall, not a
functional regression introduced by the cleanup.

```sh
make dynamic static formatcheck namingcheck BISON=/usr/bin/bison -j4
ICL_REF_DIR=/tmp/icl-reference-honza-20261021 \
  PATH="$PWD/build/combined-audit/reference-venv/bin:$PATH" \
  make check-dynamic BISON=/usr/bin/bison -j4
make coverage BISON=/usr/bin/bison -j4
```

Logs are ignored under `build/cleanup-*.log`. Generated artifacts are not
included. No commit/push, user installation, or live coc.nvim session is part
of this work. This review does not establish complete language conformance.
