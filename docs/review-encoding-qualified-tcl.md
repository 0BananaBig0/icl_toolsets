# Encoding, qualified Tcl, and workspace-event review fixes

The four approved findings were fixed in the shared LSP server and PDL
document analyzer. No ICL grammar, CLI byte-encoding policy, or editor settings
were changed.

## Changes

- Disk indexing rejects invalid UTF-8 before parsing and logs the percent-encoded
  file URI on stderr. Invalid files cannot inject malformed UTF-8 into LSP JSON.
  JSON input and disk text use the same Unicode-scalar validation helper.
  Valid editor overlays remain available; watcher refreshes remove stale disk
  symbols and recover when the disk file is repaired.
- Supported global Tcl builtin spellings, such as `::proc` and `::if`, now
  receive the same nested syntax checks and declaration analysis as their
  unqualified forms. Formatting retains source spelling; user namespace
  commands retain their lookup names. PDL commands are not normalized into
  Tcl builtins, and PDL level-0 restrictions remain enforced.
- Disk-event URI keys are decoded and re-encoded consistently. Equivalent
  percent escapes now remove the correct entry after deletion, including a
  deleted symlink whose target still exists. Open-buffer URIs stay unchanged;
  normalization does not collapse symlinks or symlink-sensitive `..` paths.
- Closing a document refreshes each necessary identity once. An original
  identity can still be restored after an alias is retargeted. The attempted
  identity is tracked independently of successful parsing, so invalid UTF-8
  cannot trigger a duplicate fallback read.

## Regression evidence

New tests first reproduced malformed protocol responses, missed qualified-Tcl
diagnostics/navigation, stale symbols, and duplicate reads on the original
binaries. Independent review found the invalid-UTF-8 fallback variant; an
additional failing read-count test reproduced it before the correction.
The reviewer subsequently verified symlink retarget restoration inside and
outside a workspace for both servers and reported no remaining findings.

The maintained regression tests are:

- `tests/src/test_lsp_server.py`: 12 tests, parameterized across both servers,
  including invalid Unicode encodings, watcher recovery, dirty overlays,
  percent-escape equivalence, deleted aliases, and bounded close-read volume.
- `tests/src/test_pdl_tools.py`: 51 tests, including qualified builtin syntax,
  nested scripts/expressions, level-0 rejection, namespace/procedure/variable
  navigation, and formatting stability.

Dynamic/static builds and Google formatting/naming checks passed. Static
verification passed the shared-server tests, 51 PDL tool tests, 45 ICL LSP
tests, 13 ICL and 6 PDL golden cases, and 32,000-element reference-lookup
checks. The complete final `make check-dynamic` run exited 0, including parser,
formatter, LSP, property/fuzz, reference comparison, grammar/scanner, and editor
tests. All 334 grammar productions and 119 scanner rules were exercised. Both
Vim and Neovim passed all 38,488 ICL and 728 PDL corpus files, followed by
standalone/vim-plug installation and large-input plugin checks. `git diff
--check` and the source-tree generated-artifact check also passed.

Commands (from the repository root):

```sh
make dynamic static formatcheck namingcheck BISON=/usr/bin/bison -j4
ICL_REF_DIR=/tmp/icl-reference-honza-20261021 \
  PATH="$PWD/build/combined-audit/reference-venv/bin:$PATH" \
  make check-dynamic BISON=/usr/bin/bison -j4
make coverage BISON=/usr/bin/bison -j4
git diff --check
python3 tests/tree_clean_test.py
```

Logs are ignored under `build/review-final-*.log`; generated artifacts are not
included in the change.

## Read-volume measurement

On Linux 6.18.33.2 WSL2 with GCC 16.2.1, dynamic `-O2` builds, the shared-server
test reads `/proc/PID/io` before closing the editor overlay and after a
`workspace/symbol` barrier. Each file has 16,000 ports or assignments.

| File | Disk bytes | Before: bytes read | After: bytes read |
| --- | ---: | ---: | ---: |
| ICL | 292,907 | 586,077 | 293,170 |
| PDL | 249,780 | 499,823 | 250,043 |

Appending one invalid UTF-8 byte also results in one disk read: 293,171 bytes
for ICL and 250,044 for PDL. Protocol input accounts for the extra 263 bytes.
The test permits a bounded 16 KiB protocol allowance and is Linux-specific;
it makes no timing-speedup assertion.

## Coverage limitation

The strict coverage command exits unsuccessfully because the existing
100% line / 90% branch thresholds are not met. Its functional suites pass.
Final measured coverage is **4,275/4,385 lines (97.5%)** and
**7,325/11,911 branches (61.5%)**. The preceding review baseline was already
below the gate at 97.5% lines / 61.4% branches. Thresholds were not weakened.
The new UTF-8 helper is explicitly included in the report and has 23/23 lines
and 35/36 branches covered; its out-of-range caller guard is the missing edge.

These fixes and tests do not establish complete ICL/PDL conformance or close
the repository-wide coverage gap. No commit, push, or installation was done.
