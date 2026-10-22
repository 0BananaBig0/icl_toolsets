# ICL verification and limits — 2026-10-22

The block-by-block IEEE/Tessent audit and its compatibility rulings are in
[icl-conformance-review.md](icl-conformance-review.md). The upstream inventory is
in [honza-icl-comparison.md](honza-icl-comparison.md). This report records actual
verification; it does not claim complete hardware elaboration or formal proof of
all possible inputs.

## Functional checks

`make check-dynamic BISON=/usr/bin/bison` passed: 1744 classified conformance
inputs, 570 regression assertions, 714 block cases, 13 golden cases, 74 CLI and
62 original LSP assertions, 35 semantic LSP tests, 3 protocol tests with malformed
input subcases, 54 C++ unit assertions, whitespace/fuzz/language-separation checks,
and coverage-reader/build-graph tests. Two additional semantic tests for empty-file
completion and missing enum members passed in the focused suite, now **37/37**.

`make gramcov` reached **334/334 Bison productions** and **119/119 Flex rules**
over **2018 files**. Production IDs are counted separately even if their actions
share a source line. Generated Bison/Flex implementation coverage is excluded.
All fixtures remain classified under `tests/cases`, with generated scalability
inputs reproducible under `tests/generated/ours/performance`.

The real Honza ANTLR adapter passed 6/6 tests in the prepared reference environment.
The ordinary system-Python run explicitly skips integration tests without ANTLR.
The 2007-input comparison has 1625 agreements, 253 local-only accepts and 129
reference-only accepts; all 129 are adjudicated expected negatives. Its difference
exit code is intentional, not a regression failure.

An independent reviewer reproduced four LSP defects: contextual keyword names,
parameters misrouted to enum lookup, broadcast interface target scope, and omitted
ActiveSignals references. Persisted reproductions failed 14 assertions before the
shared semantic-layer fixes. The reviewer independently reran the four focused
tests after correction, all passing, with no remaining finding in those fixes.

`COC_ROOT=/tmp/icl-coc-20261022 COC_PLUGIN=/home/banana/.vim/plugged/coc.nvim
COC_VIM=/usr/bin/vim bash tests/run_coc_check.sh` passed with Vim 9.2 and coc.nvim
0.0.82: diagnostics, formatting, invalid-buffer preservation, definition, hover,
symbols and completion. The editor used isolated configuration/data and no user
vimrc/viminfo. The optional coc-diagnostic CLI adapter is schema-checked against
its upstream documentation; it was not installed or tested end to end. The
separate Neovim built-in-client test was not run.

## Handwritten coverage

`make coverage BISON=/usr/bin/bison` uses GCC/gcov 16.2.1 at `-O0` and merges
unit, parser, CLI and LSP executions. It includes inline handwritten headers and
the actions/prologues of `.yy` and `.l`. Zero/starred counts, never-executed
branches, repeated source lines and missing reports have reader regression tests.

The strict gate retains **100% line / 90% branch** thresholds and currently fails.
The previous 100%/92.6% README figures omitted relevant counts and are withdrawn.
Exact final measurements appear below; raw branches include compiler-emitted
exception and cleanup edges and are not a count of handwritten decisions alone.

| Measured source | Lines | Raw branches |
|---|---:|---:|
| All handwritten sources and headers | 2229/2269 (98.2%) | 3756/6406 (58.6%) |
| `src/icl_parser.yy` | 418/418 (100%) | 827/1582 (52.3%) |
| `src/icl_lexer.l` | 206/208 (99.0%) | 74/140 (52.9%) |

The final coverage run included all 37 semantic LSP tests. Its nonzero status is
the threshold failure, not a failed functional assertion.

Remaining mapped line misses include the static property-map initializer in
`icl_driver.cc`, closing braces with cleanup edges, the semantic walk lambda
definition, and two lines at the `icl_scan_generic` function entry. The scanner
function is called and its body executes; gcov maps entry/reset counts differently.
These are retained in the denominator. Branch gaps also include defensive guards
and valid input combinations not independently exercised. The final enum-member
fallback return also remains uncovered. Rule coverage is not
evidence that every semantic-action branch executed.

## Performance

Measured on Linux 6.18.33.2 WSL2 x86-64, glibc 2.43, GCC 16.2.1, GNU Bison 3.8.2,
with `-O2`. Baseline commit `44fc3db` was built separately with the same compiler
and flags. Wall times are medians of three executions including process startup;
RSS is the maximum observed KiB from `/usr/bin/time`. No coverage instrumentation
was used. Measurements ran without concurrent builds.

| Input | Bytes | Current seconds / RSS KiB | Baseline seconds / RSS KiB |
|---|---:|---:|---:|
| Real merged ICL | 141671 | 0.00927 / 7636 | 0.00705 / 5516 |
| Second real merged ICL | 137377 | 0.01018 / 7684 | 0.00696 / 5516 |
| Merged repeated ×4 | 566684 | 0.03114 / 17016 | 0.01547 / 9504 |
| Merged repeated ×16 | 2266736 | 0.11931 / 54540 | 0.05173 / 25716 |
| 16000 integer terms | 48047 | 0.01437 / 9684 | 0.09408 / 4748 |
| 16000 decimal digits | 16040 | 0.00886 / 4812 | 0.00292 / 4272 |
| 1024 nested parentheses | 2080 | 0.00369 / 5152 | 0.00296 / 4520 |

Stricter validation and the shared semantic model cost time and memory on merged
files: this is not an overall speedup. At the measured 2.27 MB scale the CLI takes
about 119 ms and 54 MB RSS. Long integer expressions previously copied right-hand
tails quadratically: 4000/16000 terms took 0.0324/0.3998 s before correction.
Left-recursive moved accumulation preserves source spelling and reduces the
current scaling check to 0.0040/0.0130 s, ratio 3.24 for four times the input.

The existing limb-based decimal conversion still has quadratic worst-case work
near width boundaries; 16000 digits measured 9 ms. If production inputs require
substantially larger decimal literals or a tighter latency budget, use these
benchmarks to justify changing that algorithm. No such larger scale is certified.

Real stdio LSP measurements use one document open and five repeated requests:

| Bytes | Open/parse seconds | Cached formatting median | Symbols median |
|---:|---:|---:|---:|
| 141671 | 0.00695 | 0.00150 | 0.00352 |
| 2266736 | 0.13154 | 0.02029 | 0.05800 |

Reproduce after building the baseline into a separate directory:

```sh
python3 tests/src/benchmark_icl.py --binary build/icl-format \
  --baseline /path/to/baseline/icl-format --output build/performance-final.json
python3 tests/src/benchmark_icl_lsp.py --binary build/icl-lsp \
  --output build/performance-lsp.json
python3 tests/src/test_integer_scaling.py --binary build/icl-format
```

## Build and language boundaries

`make static BISON=/usr/bin/bison` fails on this host because static libc, libm
and libstdc++ are absent. Dynamic binaries are the deliverables installed here.
Grouped generator outputs require GNU Make 4.3 or newer. Source distribution
creation needs only the dynamic build and excludes Python bytecode caches.

ICL and PDL remain different languages. No PDL Flex/Bison parser is implemented.
Source-backed extensions are documented individually; generic AccessLink payloads
are opaque, and there is no full cross-instance width/connectivity elaborator.
Implementation navigation returns ICL Module bodies, not inferred HDL targets.
Rename is not advertised. Parsing is serial because the scanner is not reentrant.
The transport rejects malformed JSON/UTF-8, limits JSON nesting to 256, headers
to 8192 bytes and payloads to 64 MiB. Invalid frames terminate the stream;
invalid JSON within a valid frame does not consume the next frame.


## Installed artifacts

`make install-dynamic BISON=/usr/bin/bison` installed the pair under
`/home/banana/.local/bin` and project headers under
`/home/banana/.local/include/icl-format`. Installed bytes match the verified build.
The installed formatter passed1744/1744 conformance inputs and installed LSP
passed37/37 semantic tests. The source archive was extracted under `/tmp` and
successfully built with `BISON=false FLEX=false`; its formatter accepted the
review fixture. No personal editor configuration was modified.


A final tracking audit found four Python bytecode files (including two inherited
from the baseline) and the generated differential log in Git. They were removed
from the tracked tree; the log moved under ignored `build/reference/`. The guard
now rejects bytecode, nested `__pycache__`, tool workflow state, worktrees and
generated benchmark inputs. Its regression test failed seven cases before the
fix and passed afterward. Maintained input fixtures and adjudication expectations
remain versioned; automatically generated build/analysis outputs do not.
