# PDL tools: architecture, builds and limits

`pdl-format` and `pdl-lsp` edit IEEE 1687-2014 PDL. They never execute source,
retarget procedures, apply vectors or access hardware. User commands and editor
setup are in the [README](../README.md).

## Separate language pipeline

```text
PDL/Tcl source -> Tcl parsing API -> pdl::Document -> analysis/formatting
                                                    |          |
                                               pdl-lsp     pdl-format
```

Both tools consume the same source-preserving command/word tree, spans,
declarations, references and formatter in `src/pdl_document.{h,cc}`. PDL does
not reuse ICL grammar, tokens or AST definitions. `format_cli.h` and
`lsp_server.h` share file handling and protocol behavior across language adapters.
PDL workspace navigation reads ICL through the existing ICL parser and projects
its declarations into navigation records; it does not parse ICL with Tcl.
Imported ICL buffers are explicitly ineligible for PDL formatting, preventing
an empty PDL tree from replacing their contents. Both language servers ignore
nonregular watched paths (including directories and FIFOs), discard stale disk
symbols when a file becomes unreadable, and preserve open-buffer precedence.
Checked stream reading is shared with the formatter CLI in `text_input.h`.

The frontend calls `Tcl_ParseCommand`, `Tcl_ParseExpr` and `Tcl_SplitList`.
Expression parsing requires interpreter stack storage, so it creates an
uninitialized interpreter per thread. It never calls `Tcl_Init` or any evaluation
API. A regression using an `exec touch` substitution verifies non-execution.

Recognized braced bodies include `iProc`, `proc`, `if`, `foreach`, `lmap`, `for`,
`while`, `catch`, `try` and `namespace eval`. Literal expressions are checked;
`switch` pattern/body lists are validated while retaining their original spelling.
Other braced words remain data. Command substitutions are checked and retained
verbatim. Source NUL bytes and inputs exceeding the Tcl parser's supported count
are rejected; recursive script parsing is limited to 128 levels. An iterative
delimiter preflight enforces this limit before Tcl recursively parses command
and array substitutions, including substitutions in expressions. Escaped
brackets, comments and opaque braced data do not consume substitution depth.
Excessive nesting produces a diagnostic in both the formatter and language
server; no PDL is evaluated.

## Builds

Dynamic builds require Tcl development headers and its real shared library,
discovered with `pkg-config tcl`. Override `TCL_CFLAGS` and `TCL_LIBS` for another
installation. Tcl 8.6 and Tcl 9 count types are handled, but this delivery was
tested with Tcl 9.0.2 only.

```sh
make pdl-dynamic BISON=/usr/bin/bison
make pdlcheck BISON=/usr/bin/bison
```

Fully static builds require static C/C++ runtimes and a real Tcl archive built
for the selected headers. A stub archive cannot supply parsing. If the system
does not provide the archive, build upstream Tcl in the ignored build tree:

```sh
mkdir -p build/deps
curl -L https://codeload.github.com/tcltk/tcl/tar.gz/refs/tags/core-9-0-2 \
  -o build/deps/tcl9.0.2.tar.gz
tar xzf build/deps/tcl9.0.2.tar.gz -C build/deps
mkdir -p build/deps/tcl-static
cd build/deps/tcl-static
ac_cv_header_zlib_h=no ../tcl-core-9-0-2/unix/configure \
  --disable-shared --disable-load --without-system-libtommath
make -j4
cd ../../..
make static TCL_STATIC_LIB=build/deps/tcl-static/libtcl9.0.a
```

This configuration bundles Tcl's compression/math dependencies. Other Tcl builds
may require additional archives; override `TCL_STATIC_LIBS` for their complete
link flags. Dependency sources and generated binaries stay out of Git. Tcl keeps
its own upstream license; source distributions of this project do not bundle Tcl.
Installation includes [third-party notices](third-party-notices.md) for the
documented Tcl build. Alternate Tcl builds retain their corresponding notices.

Static ELF binaries have no interpreter or `NEEDED` shared-library entries.
GNU ld nevertheless warns about optional Tcl NSS calls in static glibc. The tools
do not evaluate Tcl commands, but the presence of those functions means universal
glibc-independent portability is not claimed. Validate the actual target host.
Dynamic/static installation uses the same four command names; choose one variant
per prefix, or install the other variant into a separate prefix.

## Validation and static-analysis limits

Normative references: IEEE 1687-2014 clauses 7.5.3 (Tcl compatibility and vector
indices), 7.6 (level-0), 7.7–7.8 (PDL tokens/numbers), 7.9 (commands), and clause 8
(level-1). These rules differ from ICL clause 6. Sized ICL literals such as
`4'hF` are rejected as PDL values; PDL supports decimal, `0x`/`0b` forms, unknown
digits, argument references and permitted enum names. `iScan` data can span lines.

The local validator checks command arities, recognized options, procedure
argument lists, literal numeric values and explicit level-0 command restrictions.
Compatibility choices: missing `iPDLLevel` permits level-1 Tcl/PDL fragments;
abbreviated `iPDLLevel 0` / `1` declarations are accepted. Full portable declarations
should include `-version STD_1687_2014`. `iProc` needs a preceding module binding.

The LSP supplies syntax diagnostics, formatting, completion, definition,
declaration, implementation, references, hover and symbols. For procedures,
implementation navigation returns their source definition. Statically named
Tcl namespaces and PDL module/procedure namespaces are separate. Duplicate
procedure definitions in one stream use the last definition; ambiguous definitions
across streams remain unresolved. Literal ICL module/object references and
instance prefixes support cross-language navigation.

Static array writes and reads resolve to the same base variable, including
literal bases with dynamic indices. Literal `variable` declarations in Tcl
procedures bind local names to namespace variables. Completion retains lexical
Tcl namespaces, PDL module/call namespaces and procedure arguments, including
unfinished procedure bodies; empty completed bodies do not leak arguments into
the enclosing scope. These records are shared by navigation and completion in
the PDL document, without evaluating Tcl.

This is conservative static analysis, not complete Tcl/PDL semantic evaluation:

- Unknown commands and dynamically constructed script bodies remain
  opaque. Runtime command names, aliases, `eval` and dataflow are not inferred.
- ICL width overflow, enum membership and full connectivity are not checked for
  PDL values. Timing and command scheduling require a retargeting environment.
- Completion covers keywords, statically visible procedures, local and namespace
  variables, and explicit namespace bindings;
  contextual ICL object completion is not yet supplied.
- Formatting preserves word spelling and data, normalizes command boundaries,
  and removes most blank lines; it does not reflow quoted/dynamic script bodies.
- The first diagnostic is reported; this is not a full PDL conformance claim.
  Runtime-dependent command restrictions require a retargeting environment.

The [block review](pdl-block-review.md) records the subsequent IEEE clauses 7–8
audit, literal object/procedure path checks, quoted-body validation, Tcl
continuation fixes and maintained positive/negative command fixtures. Known
quoted bodies are analyzed while their spelling is preserved in output.

The [independent syntax checks](pdl-syntax-oracles.md) add a bounded Tcl oracle,
IEEE command/lexical properties, nested inputs and CLI/LSP agreement checks.
Literal multiword `expr` arguments are concatenated before syntax checking;
dynamic expression construction is deferred. These checks do not execute PDL.

## Reproducible verification

```sh
make check-dynamic formatcheck namingcheck BISON=/usr/bin/bison
make pdl-syntaxcheck BISON=/usr/bin/bison
python3 tests/src/test_pdl_tools.py \
  --formatter build/pdl-format-static --server build/pdl-lsp-static
readelf -l build/pdl-lsp-static
readelf -d build/pdl-lsp-static
python3 tests/src/benchmark_pdl.py
python3 tests/src/test_lsp_server.py --icl build/icl-lsp --pdl build/pdl-lsp
python3 tests/src/test_pdl_document.py --server build/pdl-lsp --benchmark
make coverage BISON=/usr/bin/bison
```

The PDL regression requires Python 3 and `tclsh` for harmless Tcl execution
equivalence checks (the formatter itself never evaluates source).
`tests/src/test_pdl_tools.py` exercises real executable boundaries and stdio LSP,
including nested scripts versus data, comments, substitutions, source preservation,
invalid-input preservation, non-execution, numbers, options, module bindings,
qualified Tcl namespaces, ICL bracket navigation, correction and incomplete input.
Fixtures are classified in [test sources](../tests/cases/SOURCES.md).

The benchmark generates inputs in memory, reports actual input size, median wall
time across three processes and maximum child peak RSS via GNU time. Expected
workload is interactive editing of roughly 100 KB–1 MB procedure files. Parsing
and rendering traverse commands/words. Variable resolution uses a map keyed by
scope, namespace and base name instead of scanning every declaration for every
use. Other symbol kinds can still require declaration scans; runtime Tcl behavior
is outside this benchmark's scope.
Coverage includes the shared template headers and PDL sources; generated Tcl,
Flex and Bison internals are excluded. The strict 100% line / 90% branch acceptance
gate remains enabled even when unmet. Dated results are recorded below.

### 2026-10-24 reference-lookup regression

`test_pdl_document.py` checks exact reference locations for 32,000 declarations
and 32,000 reads (926,670 bytes), with a two-second optimized-build request budget.
On Linux/WSL2 with GCC 16 and `-O2`, the same request took 3.398 seconds before
indexed variable lookup and 0.013 seconds after it. Three request timings at
2,000 / 4,000 / 8,000 variables had medians of 1.258 / 2.054 / 3.403 ms after the
change, versus approximately 12.8 / 52 / 219 ms before it. These timings cover
reference requests after parsing, not startup, parsing or peak memory. They
demonstrate this workload's scaling, not a bound for all workspace operations.

The final dynamic regression and focused static regressions passed, including
46 PDL executable/LSP tests and both servers' directory/FIFO replacement tests.
Five optional ANTLR checks were skipped without `ICL_REF_DIR`. The instrumented
functional tests also passed, but `make coverage` failed its unchanged strict
gate: 4,125/4,234 lines (97.4%) and 6,959/11,331 branches (61.4%). The preceding
baseline was 97.5% lines / 61.4% branches and already failed that gate. The new
`text_input.h` reached 6/6 lines and 18/24 branches; unreadable-regular-file paths
in the shared LSP reader remain uncovered. Functional regression success does
not establish full error-path coverage.

The subsequent [PDL block-review results](pdl-block-review.md#verification-and-remaining-limits)
supersede the historical verification snapshots below. They include 133
source-attributed syntax cases, the final independent review fixes, rebuilt and
installed dynamic/static checks, and the current unmet coverage gate.


## Verification on 2026-10-23

Host: x86-64 WSL2 Linux 6.18.33.2, glibc 2.43, system Tcl 9.0.2,
C++17 `g++ -O2`; static Tcl built from upstream tag `core-9-0-2` as above.

| Check | Result |
|---|---|
| `make dynamic static BISON=/usr/bin/bison -j4` | All eight executables built. |
| `make check-dynamic formatcheck namingcheck BISON=/usr/bin/bison -j4` | Passed; five optional reference-ANTLR integration tests skipped because `ICL_REF_DIR` was unset. |
| PDL suite on build and installed dynamic/static executables | 21/21 test methods passed per variant, with multiple subcases and all four PDL fixtures. |
| Existing installed ICL protocol/semantic suites | 6/6 and 42/42 passed. |
| ICL production/scanner rule coverage | 334/334 productions and 119/119 scanner rules reached across 2,324 files. |
| Static PDL ELF checks | No `INTERP` or shared-library `NEEDED` entries. |
| `python3 tests/tests/test_source_distribution.py` | Actual archive built with `BISON=false FLEX=false`, no static Tcl archive; all four installed versions and the PDL suite passed. Uninstall preserved an unrelated sentinel file. |
| `make coverage BISON=/usr/bin/bison -j4` | **Failed strict gate:** 3,476/3,603 lines (96.5%), 5,530/9,431 branches (58.6%). Requirements remain 100%/90%; thresholds unchanged. |

Coverage includes compiler-emitted exception/cleanup edges. PDL document source
reached 691/767 lines (90.1%); ICL grammar actions 563/563 (100%), scanner actions
211/213 (99.1%). Rule reachability is distinct from action line/branch coverage.
No complete language-conformance claim follows from these measurements.

Independent review found and regression-tested fixes for catch options-variable
corruption, switch option-operand corruption, PDL definition/call namespace
mixing, Tcl namespace-variable collisions and valid iScan line continuations.
Level-0 command-substitution rejection was also added. Full runtime/vendor PDL
conformance, Tcl 8.6, target-host static portability and a real PDL coc.nvim session
remain unverified. These limits are not waived by a passing functional suite.

`python3 tests/src/benchmark_pdl.py` (three runs, final executable):

| Input bytes | Median wall time | Maximum peak RSS |
|---|---|---|
| 100,065 | 0.0137 s | 9,824 KiB |
| 1,000,051 | 0.1052 s | 57,284 KiB |

The earlier run on this host was 0.011/0.1071 s for the same sizes; results have
ordinary process-startup noise. This measures CLI parsing, analysis and rendering,
not workspace indexing or LSP reference-query latency.

Dynamic installation was verified under `/home/banana/.local/bin`; the fully
static installation under `build/install-static/bin` keeps the same command names
in its own prefix. Personal coc.nvim settings were not modified.

## Vendor PDL corpus

The maintained [EDA PDL corpus](../tests/cases/pdl_eda/README.md) adds 79 byte-unique
complete files (114 source paths) to `make pdlcheck` and coverage runs. Hashes,
source attribution, whole-stream retention and exact duplicate elimination are
checked. The suite expects 73 positive files and six IEEE argument-order errors;
negative cases remain unchanged. The corpus README distinguishes documented
Tessent options from observed SLM compatibility forms and records static-analysis
limits. Dynamic module bindings, procedure namespaces and prefixes are accepted without
inventing definition targets. Explicit `iPrefix -` / `iUseProcNameSpace -` restore
known default context.

Corpus extension verification (2026-10-23): `make check-dynamic formatcheck
namingcheck BISON=/usr/bin/bison -j4` passed; five optional ANTLR integration tests
remain skipped (`ICL_REF_DIR` unset). Importer tests passed 3/3, PDL functional
methods 26/26, and dynamic/static corpus runs each passed 73 positive and six
expected diagnostic files. Every fixture was compared byte-for-byte to the
snapshot, and all 114 discovered paths matched the provenance manifest.

`make coverage BISON=/usr/bin/bison -j4` ran the corpus and failed the unchanged
strict gate: 3,517/3,638 lines (96.7%), 5,693/9,649 branches (59.0%). PDL document
source reached 732/802 lines (91.3%). These figures supersede the earlier delivery
snapshot above; the unmet coverage requirement still gates integration.

A native CLI check on this host (Tcl9.0.2, optimized dynamic build) parsed/formatted
`syn_slm/wrapped_pmm_ip_characterization_debug_869fdacdb5f1cd49.pdl`, 868,139 bytes,
three times with output discarded. `time.monotonic` wall times were
0.0220/0.0260/0.0203 s (median0.0220 s); maximum child peak RSS from `getrusage` was
19,140 KiB. Command: `build/pdl-format tests/cases/pdl_eda/syn_slm/wrapped_pmm_ip_characterization_debug_869fdacdb5f1cd49.pdl`.
This is a workload observation, not a speedup or vendor execution measurement.

Independent corpus review confirmed exact snapshot regeneration and found a
false-target case for runtime procedure namespaces. The fix tracks unknown
namespace/prefix values separately from empty defaults. Three real-LSP cases
(definition namespace, call namespace and prefix) failed before the change and
passed afterward; explicit reset navigation is covered as well.

The updated corpus and26-method functional suite also passed against both final
installed variants: `/home/banana/.local/bin` (dynamic) and
`build/install-static/bin` (static). All eight installed executables matched their
build hashes and returned `--version`; static ELF files had no interpreter or
shared-library dependency entries. Installation printed the shared editor setup.
