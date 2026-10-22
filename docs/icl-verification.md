# ICL verification and limits — 2026-10-22

The block-by-block IEEE/Tessent audit and its compatibility rulings are in
[icl-conformance-review.md](icl-conformance-review.md). The upstream inventory is
in [honza-icl-comparison.md](honza-icl-comparison.md). This report records actual
verification; it does not claim complete hardware elaboration or formal proof of
all possible inputs.

## Clang-methods follow-up

The subsequent [handwritten syntax-edge corpus](../tests/cases/ours/syntax_edges/README.md)
adds 72 conformance fixtures (33 accept, 39 reject) across 15 blocks. Its separate
parameter-width overflow reproduction is an explicit expected failure in the
LSP diagnostics suite: `$WIDTH'H1_FF` is accepted with `WIDTH` resolving to 8.
This confirms the parameter elaboration limitation below; it is not counted as a
passing conformance fixture. No production behavior changed in this addition.

`PYTHONPATH=build/reference/python ICL_REF_DIR=/tmp/icl-reference-honza-20261021
make check-dynamic formatcheck namingcheck BISON=/usr/bin/bison -j4` exited 0:
2,015 conformance inputs, 29,177 metamorphic variants across 45 blocks (138 seeds
excluded), and the existing 5,568 identifier/context cases. The live diagnostics
suite ran seven tests with one expected failure for parameter-width overflow.
The six real ANTLR adapter tests ran without dependency skips. Grammar traces
reached 334/334 productions and 119/119 scanner rules over 2,284 corpus files.
The whole-test-tree audit covered 2,297 ICL files with no duplicate-file groups
and no new unreviewed module repetition. These figures extend the earlier
functional evidence; handwritten branch coverage was not remeasured.

Independent review verified the manifest and fixture expectations. Its transport
failure injection exposed an overly broad expected-failure annotation; an
unmarked companion test now checks publication for that same input. Repeating
the injected timeout fails the suite, and supplying the future width diagnostic
reports an unexpected success. The final review has no residual findings.

The [Clang-methods report](clang-testing-and-icl-status.md) records the preceding
1,943-case conformance suite, 5,568 identifier/context cases, 27,364 syntax
properties and a fresh real ANTLR comparison. These functional counts supersede
the earlier runs below; the coverage limitations remain unchanged.

## Independent syntax and diagnostic follow-up

The [new syntax-property report](icl-syntax-properties.md) records 25,938
IEEE-derived transformations across 45 blocks, corpus-wide live LSP validation
of 1,885 cases, precise/versioned diagnostics and additional EOF/CRLF regressions.
Its newer functional counts supersede the earlier run below; the handwritten
line/branch coverage limitations in this document remain unchanged.

## Functional checks

`make check-dynamic BISON=/usr/bin/bison` passed: 1757 classified conformance
inputs, 570 regression assertions, 714 block cases, 13 golden cases, 74 CLI and
62 original LSP assertions, 42 semantic LSP tests, 3 protocol tests with malformed
input subcases, 54 C++ unit assertions, whitespace/fuzz/language-separation checks,
and coverage-reader/build-graph tests. Follow-up cases cover PortGroup comments
and CaptureSource enum precedence and signal fallback. The renewed audit also
covers keyword identifiers and namespaces in complete/incomplete completion contexts.

`make gramcov` reached **334/334 Bison productions** and **119/119 Flex rules**
over **2035 files**. Production IDs are counted separately even if their actions
share a source line. Generated Bison/Flex implementation coverage is excluded.
All fixtures remain classified under `tests/cases`, with generated scalability
inputs reproducible under `tests/generated/ours/performance`.

The real Honza ANTLR adapter passed 6/6 tests in the prepared reference environment.
The ordinary system-Python run explicitly skips integration tests without ANTLR.
The refreshed 2035-input comparison has 1642 agreements, 258 local-only accepts
and 135 reference-only accepts; all 135 are adjudicated expected negatives. Its difference
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
| All handwritten sources and headers | 2247/2286 (98.3%) | 3829/6452 (59.3%) |
| `src/icl_parser.yy` | 418/418 (100%) | 843/1582 (53.3%) |
| `src/icl_lexer.l` | 205/207 (99.0%) | 77/138 (55.8%) |

The final coverage run included all 42 semantic LSP tests. Its nonzero status is
the threshold failure, not a failed functional assertion.

Remaining mapped line misses include the static property-map initializer in
`icl_driver.cc`, closing braces with cleanup edges, the semantic walk lambda
definition, and two lines at the `icl_scan_generic` function entry. The scanner
function is called and its body executes; gcov maps entry/reset counts differently.
These are retained in the denominator. Branch gaps also include defensive guards
and valid input combinations not independently exercised. The enum-member fallback return is now covered by the CaptureSource tests. Rule coverage is not
evidence that every semantic-action branch executed.

## Condition coverage follow-up

A separate GCC 16.2.1 probe uses native masking MC/DC to locate missing condition
outcomes. Thirteen maintained inputs improve `icl_parser.yy` from93/106 to105/106
and `icl_lexer.l` from23/26 to25/26. After the renewed audit, the scanner
measures24/24 because its duplicate cleanup guard was removed; generated Flex cleanup already accepts null. Parser coverage
remains105/106, with the outstanding condition mapped to aggregate construction
at current line653, which has no explicit source condition. This mapping remains
in the denominator; no artificial exclusion or lowered gate.

Reproduce in an isolated build directory (requires GCC supporting the flag):

```sh
make dynamic BUILD_DIR=build/conditions BISON=/usr/bin/bison \
  CXXFLAGS='-O0 -g --coverage -fcondition-coverage' -j4
python3 tests/src/test_icl_conformance.py --binary build/conditions/icl-format
python3 tests/regression.py --binary build/conditions/icl-format
mkdir -p build/conditions/reports
(cd build/conditions/reports && gcov -j -g -b -o .. ../icl_parser.o ../icl_lexer.o)
```

Read each file's `lines[].conditions` in the native JSON reports. Multiple
condition groups on a single line are distinct; totals above include every group
for the exact `.yy`/`.l` file. These diagnostic measurements supplement raw
line/branch coverage; they neither replace its gate nor prove all possible inputs.
All emitted reports and build products stay in ignored `build/`.

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
The installed formatter passed1757/1757 conformance inputs and installed LSP
passed42/42 semantic tests. Previously, the source archive was extracted under `/tmp` and
successfully built with `BISON=false FLEX=false`; its formatter accepted the
review fixture. No personal editor configuration was modified.


A final tracking audit found four Python bytecode files (including two inherited
from the baseline) and the generated differential log in Git. They were removed
from the tracked tree; the log moved under ignored `build/reference/`. The guard
now rejects bytecode, nested `__pycache__`, tool workflow state, worktrees and
generated benchmark inputs. Its regression test failed seven cases before the
fix and passed afterward. Maintained input fixtures and adjudication expectations
remain versioned; automatically generated build/analysis outputs do not.

Previous follow-up verification at f0a4725: full `make check-dynamic` passed, grammar/scanner rules
remain334/334 and119/119 over2033 files, and independent review found no functional
issue in the two shared semantic-layer fixes. It corrected standard citations to
§6.4.8(t), §6.4.9 and §6.4.10. Both new LSP tests also passed independently.
The fresh coc run initially lacked its temporary configuration; rerunning with
the prepared isolated configuration passed all editor checks. Reinstalled binaries
match the final build and pass1757 conformance cases and42 semantic LSP tests.
Static linking and the optional coc-diagnostic adapter were not re-tested in this
follow-up; their previously documented limitations remain.

## Renewed audit after3999c65

All45 classified blocks were revalidated in the established order. Completion
now distinguishes keyword identifiers from Instance type introductions, supports
a namespace named Of, and handles incomplete namespace-qualified headers.
The new protocol tests initially failed three assertions. Independent review then
found that `Source Instance, Of` was still misclassified; concatenation and Boolean
operand regressions failed before the final header-context fix. The resulting
42-test semantic suite and full `make check-dynamic` pass. Real Vim/coc checks pass.

GCC LTO diagnosed a scanner return-type mismatch across translation units:
`-flto -Werror=odr` failed before the signature fix, then the dynamic LTO build
passed along with570 formatter regression assertions. Removed an unused comment
layout computation and delegated null-buffer handling to Flex's existing cleanup.
No generated code was edited or tracked. Compiler/source coverage details above
are the final measurements; the strict gate still exits nonzero.

The two uncovered scanner source mappings (lines369–370) coexist with gcov's
`icl_scan_generic` report: called191 times, returned100%, blocks executed100%.
The subsequent mode/depth assignments also execute191 times. The ambiguous
mapped lines remain in the denominator; no coverage exclusion was introduced.

Installed verification: `make install-dynamic BISON=/usr/bin/bison` succeeded;
the installed formatter passed1757/1757 conformance inputs and installed LSP
passed42/42 semantic tests. Both installed executables match the tested build
byte-for-byte. The installation command printed both coc configurations; user
editor settings were not modified. Static linking, optional coc-diagnostic and
performance benchmarks were not re-run in this renewed audit; no new performance
claim is made. The prior documented limits still apply.
