# Developer guide

Start with [project requirements](requirements.md). User installation and editor
instructions are in the [main README](../README.md).

## Shared architecture

The two ICL executables link the same scanner, parser, AST definitions, document
analysis and formatting implementation. `ParsedDocument` owns the source,
parser driver, AST and semantic index. The formatting chain is:

```text
source -> shared Flex/Bison parser -> shared AST -> format tree -> rendered text
                                          |
                                          +-> scoped declarations/references
```

`FormatDocument` builds a document for a CLI job; `FormatParsedDocument` formats
an existing document. The LSP retains the parsed document until its text changes,
so formatting can reuse it. Keep diagnostics, style normalization and change
detection in the shared layers. CLI file handling/options and LSP protocol,
workspace management and responses form the necessary outer boundaries.

| Source | Responsibility |
|---|---|
| `src/icl_lexer.l`, `src/icl_parser.yy` | ICL tokens, grammar productions and local validation actions. |
| `src/icl_ast.h`, `src/icl_driver.{h,cc}` | Shared AST ownership, source spans, parser state and validation helpers. |
| `src/icl_semantics.{h,cc}` | Parsed documents, scoped declarations/references and workspace resolution. |
| `src/format_tree.{h,cc}` | Layout decisions and rendering. |
| `src/format_job.{h,cc}` | Shared formatting requests, outcomes and diagnostics. |
| `src/format_cli.h`, `src/*_format_main.cc` | Shared CLI implementation and language adapters. |
| `src/lsp_server.h`, `src/*_lsp_main.cc`, `src/json.{h,cc}` | Shared LSP services/transport and separate language adapters. |
| `src/pdl_document.{h,cc}` | Shared PDL source tree, non-evaluating Tcl parsing, analysis and formatting; see [PDL tools](pdl-tools.md). |

The ICL AST uses three node shapes: comment, statement and header/body block. Nodes
are non-copyable and owned by an `AstUnit` arena; child pointers are non-owning.
Formatting walks the AST through `AstVisitor`. Source spans do not depend on
generated Bison types. Semantic indexing and formatting share those spans;
protocol positions are converted to UTF-16 for LSP clients.

Separate parsing, layout and rendering responsibilities. For example, a legacy
`PortGroup` list can stay on one line, while a line comment forces a multiline
layout. A trailing comment on a closing brace must stay attached to that brace.
These are layout decisions, not additional grammar productions.

The scanner has file-scope state and is not reentrant. Parse serially and reset
scanner state between documents; do not introduce concurrent parsing without
changing that ownership model. The semantic index is not a hardware elaborator.
ICL and PDL must keep separate language definitions.

## Pattern-generation architecture

The generator consumes the existing language frontends. It does not add a
second ICL or PDL parser, and editor tools do not execute the runtime.

```text
ICL ParsedDocument/AST -> ordered Design -> elaborated Netlist -> TAP expansion
PDL Document          -> bounded Runtime -> ordered Operation records
                                          |
                                          v
                           ScanScheduler / symbolic constraints
                                          |
                                          v
                           NetworkState / TapExecution replay
                                          |
                                          v
                           staged PDL and optional flat ICL
```

| Source | Responsibility |
|---|---|
| `pattern_design.*`, `pattern_netlist.*`, `pattern_expression.*`, `pattern_logic.*` | Ordered module registry, specialized hierarchy, external connections and behavioral models. |
| `pattern_bsdl.*`, `pattern_access_link.*`, `pattern_tap.*`, `pattern_tap_network.*` | BSDL metadata, AccessLink resolution, TAP controller and child wiring. |
| `pattern_runtime.*` | Bounded Tcl/PDL execution and source-attributed operations; no invented hardware readback. |
| `pattern_state.*`, `pattern_tap_execution.*` | Unknown-initial hardware state and concrete event execution. |
| `pattern_symbolic.*`, `pattern_values.*`, `pattern_scheduler.*` | Z3 constraints, PDL values/targets and complete-scan scheduling. |
| `pattern_flatten.*`, `icl_pattern_generation_main.cc`, `file_output.h` | Canonical structural output, CLI orchestration and shared staged file replacement. |

`PdlDataValue` resolves read/write enum operands from `NetlistNode::enumeration`
on the original target declaration before alias-bit mapping. It uses the shared
elaborated constants and PDL bit-sizing helper; no second enum parser or global
member-name lookup is involved. Numeric-only commands continue using `PdlNumber`.

`NetworkState::Assume` handles supported `iState` snapshots. The scheduler checks
every alias target through shared validation before committing any changes;
the TAP executor delegates to the same state method. Port assumptions use no
clock/reset processing. The scheduler reads each affected object's previous
vector once, then edits its selected bits, avoiding repeated whole-vector copies.

Registers model ports, state, behavior and connections to other components;
they are not expanded into internal flip-flop/gate netlists. The scheduler uses
runtime interface membership and must replay candidate scans before committing
state. `iApply -together` adds a shared update-cycle constraint; replay must
confirm actual updates, not merely matching final values (a held reset may
already provide those values). Prerequisite scans and read observations retain
their own ordering. TCK idle replay uses the same native event engine and
commits state only after successful completion. Fixed-point detection compares
all network storage, driven inputs and reset state at complete cycles in known
Run-Test/Idle; the output count is never shortened.
Native state records known reset assertions through each propagation pass, so
idle replay clears fill even when a reset pulse ends within the same cycle.
See [current support](pattern-generation.md) and the
[model ledger](pattern-model-coverage.md) before extending an unsupported path.

## Build and test

Run from the repository root. Override `BISON` when the default executable is
older than 3.8. Python 3.10+ is needed for the full test suite; individual integration
checks may need additional tools described by their scripts.

| Command | Checks or output |
|---|---|
| `make dynamic` | All five dynamic executables in `build/`; the generator also needs Z3. |
| `make static` | All five static variants; requires static runtime, Tcl and Z3 libraries. |
| `make pdlcheck` | PDL functional/LSP tests, complete vendor corpus and source-attributed block cases. |
| `make check-dynamic` | Full dynamic functional suite, live corpus diagnostics, syntax properties, corpus audit and grammar/scanner rule coverage. |
| `make keywordcheck` | Independent keyword/identifier-context matrix through CLI and live LSP. |
| `make syntaxcheck` | Independent IEEE whitespace/comment and nested-index transformations; generated inputs remain ignored. |
| `make check` | Full suite with the default dynamic/static build prerequisites. |
| `make unitcheck` | AST/layout/JSON unit assertions without generating the parser. |
| `make goldencheck` | ICL/PDL byte-exact golden outputs, second-pass stability, CLI/LSP parity and runner safeguards. |
| `make corpuscheck` | Importer/provenance checks and all maintained ICL duplicate checks. |
| `make pattern-scheduler-check pattern-generation-check` | Focused scheduler and generator CLI/replay checks. |
| `make pattern-scheduler-static-check pattern-generation-static-check` | The same contracts with static executables. |
| `make gramcov` | Distinct Bison production and Flex rule hits. |
| `make coverage` | Strict handwritten line/branch coverage gate. |
| `make nvimcheck` | Neovim built-in LSP integration. |
| `make coccheck` | Real Vim -> coc.nvim -> icl-lsp integration. |

The coverage gate currently requires 100% lines and 90% raw branches and is not
met by the last recorded run. Full rule coverage does not prove that all actions
or error branches have executed. See [verification](icl-verification.md) for the
measurement scope and remaining gaps. Generated Bison/Flex implementation code
is excluded; handwritten `.l`/`.yy` logic and C++ headers remain in scope.

Run full regression and coverage sequentially: their fixture-generation paths
overlap. The latest generator-inclusive measurements are recorded in
[pattern-generation.md](pattern-generation.md).

Coverage reports are stored under `.worktrees/coverage/` (ignored by Git).
Instrumented binaries and counters remain in `build/cov/`. `make coverage`
moves gcov's output into the report folder before applying the strict gate;
`make clean` removes both locations. To reread existing reports without rerunning
the tests, use `cd .worktrees/coverage && python3 ../../tests/coverage.py`.
Only that coverage subdirectory is removed by `make clean`; sibling checkouts
and archived reports are retained.

For focused checks:

```sh
python3 tests/src/test_icl_conformance.py --binary build/icl-format --block enum
python3 tests/golden_test.py --binary build/icl-format --server build/icl-lsp
python3 tests/golden_test.py --language pdl --case 'nested*' --server build/pdl-lsp
python3 tests/src/test_icl_lsp.py --binary build/icl-lsp
python3 tests/src/test_icl_protocol.py --binary build/icl-lsp
```

Use [testing practices](icl-testing-practices.md) and the existing runners when
adding cases. New test scripts belong under `tests/`, mirroring the tested source
path. Cases belong under `tests/cases/`, grouped by provenance and grammar block.
Maintain positive, negative, nested, boundary and malformed-input coverage.
Do not weaken an expectation merely because a reference parser accepts it.

See [the Verilator comparison](verilator-testing-practices.md) for the adopted
golden/scenario practices and architecture decisions. Expected files are reviewed
test specifications. `--update` is an explicit maintenance operation and is never
used by regression; inspect its diff before accepting a formatting change.
See [formatter self-tests](formatter-self-tests.md) for the exact first/second
pass contract, explicit baseline updates and their limitations.

The duplicate audit compares exact file bytes and namespace-aware module
identities. Whole-file formatting variants and complete examples can require
repeated module definitions; each retained group needs a specific reason and
exact occurrence hashes. Follow [the deduplication record](../tests/cases/CORPUS_DEDUPLICATION.md)
when updating canonical paths or source manifests.

## Editor and protocol development

The real editor check is separate because Vim/Neovim and coc.nvim are not build
dependencies. `tests/setup_coc.sh` can prepare a test environment; inspect its
downloads and installation steps before running it. For an existing installation:

```sh
make coccheck COC_VIM=/path/to/vim COC_PLUGIN=/path/to/coc.nvim
```

Keep editor checks isolated from personal configuration. The tested Vim/coc
setup and the untested optional CLI adapter are distinguished in the verification
report. Vim's `.icl` association must be overridden with `setlocal filetype=icl`;
`setfiletype` alone may leave the file classified as Clean. coc.nvim forwards the
buffer's indentation setting to LSP formatting.

`icl-lsp` advertises full-document synchronization. Workspace discovery excludes
hidden and build directories and does not follow symlink directories. Open
documents override disk content; watched file events update unopened documents.
Completed statements in an incomplete module can remain useful for completion,
but stale symbols from a previous document version must not survive an update.

Protocol limits are currently 8192-byte headers, 64 MiB payloads and JSON nesting
of 256. Invalid framing terminates the stream; malformed JSON in a valid frame
must not consume the next frame. Unsupported requests return `-32601`; unknown
notifications are ignored. Keep diagnostics off protocol stdout.

## Source distribution and generated output

### Local workspace layout

Use `.worktrees/` as the single ignored location for local Git worktrees and
retained reports. Local contents may include:

| Path | Role |
|---|---|
| `.worktrees/icl-conformance/` | Registered detached Git checkout; inspect with `git worktree list`. |
| `.worktrees/coverage/` | Generated gcov reports; recreated by `make coverage`. |
| `.worktrees/audit-fixes/`, `.worktrees/pre-integration/` | Retained local audit/integration records; ordinary directories, not registered checkouts. |

These local contents are not required to build or install the tools and are
not included in a clone. Git worktrees are identified by Git's registration,
not merely by being inside `.worktrees/`. Use `git worktree move` when relocating
a registered checkout; use ordinary file moves for report directories.

The former singular report directory was consolidated into `.worktrees/`.
Coverage and audit files were moved without overwriting existing entries, and
their contents and file modes were verified. The registered checkout stayed at
its existing path. Do not use the former directory name for new output.

The 2026-10-25 consolidation verified 87 moved entries, a clean registered
checkout, and the cleanup regression. Full `make check-dynamic` passed, as did
66 local documentation links and the README's coc.nvim JSON comparison.
`make coverage` wrote reports to the new location; its functional checks passed,
while the existing strict gate remained at 97.5% lines / 61.7% branches.
Local evidence is ignored under `build/worktree-consolidation.json`,
`build/worktrees-regression.log` and `build/worktrees-coverage.log`.

### Source archives and installation

```sh
make dist
```

This writes `build/dist/icl-format-1.0.0.tar.gz`. The current Makefile package
identifier remains `icl-format`; the Git repository is `icl_toolsets`.
The archive contains generated parser/scanner sources and a `.prebuilt` marker,
so a recipient can build with the C++ compiler and Make without Bison/Flex.
The archive also includes `docs/`, the test sources and fixture provenance.

Generated parser/scanner files, binaries, archives, coverage reports, caches,
graphs and generated stress fixtures must stay out of Git. Imported vendor ICL
and reviewed test expectations/provenance are maintained test inputs. Keep
temporary extraction and analysis output under ignored `build/`.

`make install-dynamic` and `make install-static` install all four tools using the
same command names and write an installation manifest. They print one `icl`
and one `pdl` language-server entry, each supplying diagnostics, navigation and
shared-engine formatting, plus standalone CLI usage. The optional external CLI
adapter remains documented in `editor/coc-cli.json`. Installation verification must exercise the installed
paths, not only the build-directory binaries.

## Code navigation and quality

Use an available CodeGraph index to discover callers, dependencies and existing
helpers. Treat current source as authoritative when an index is stale or absent;
fall back to targeted searches. `make codegraph` produces `.codegraph/` outputs:
Python helper graphs, compiler-generated C++ header dependencies and rendered
dependency graphs. Those files are generated aids and are not source code.

Follow the required [Google coding style](coding-style.md), including names,
headers, class interfaces and layout. Run `make formatcheck` and
`make namingcheck` alongside the relevant functional checks. Prefer
existing shared code and the standard library. Remove redundant or dead code
without removing error handling or required tests. Avoid speculative abstractions.
For performance changes, measure representative merged ICL and larger/deeper
inputs with command, toolchain, input size, wall time and peak memory recorded.
Do not claim a speedup from code inspection alone.

## Formatter I/O and parser regression checks

The common formatter CLI reads files and stdin through checked stream reads.
File output uses a unique sibling temporary file, checked writes, metadata
updates, sync and close, then rename. A failure before replacement leaves the
destination unchanged. Existing symlinks retain their link and resolve to the
target; dangling output symlinks are rejected. Replacement preserves ownership
and ordinary permission bits, but does not preserve extended attributes or ACLs.
Other hard links retain the previous inode. Directory sync and concurrent
external changes are outside this replacement guarantee.

Run the focused checks for both languages with:

```sh
python3 tests/src/test_format_cli.py --icl build/icl-format --pdl build/pdl-format
python3 tests/src/test_icl_semantics.py --binary build/icl-format --benchmark
```

The CLI suite injects write failures with a child-only file-size limit. These
children set `GCOV_PREFIX=/dev/null` because the same limit would truncate gcov
counter files. Their exit-time coverage counters are deliberately excluded;
their functional assertions still run. Normal I/O cases contribute counters
when run against instrumented binaries.

ICL name uses retain token-index spans into the shared parsed document instead
of copying each dotted path prefix. Comparison operand source spans come from
the grammar, preserving enum context through parentheses, subscripts and unary
operators. PDL declarations reuse a variable identity within the same owner,
namespace and name; subsequent assignments are references to that identity.

The 2026-10-24 path benchmark used GCC 16.2.1, `-O2`, Linux/WSL2 and the command
above. Inputs contained 500/1000/2000/4000 repeated dotted components (about
1/2/4/8 KiB). Median results from three new runs were:

| Components | Baseline RSS (KiB) | New RSS (KiB) | Baseline wall (ms) | New wall (ms) |
| --- | ---: | ---: | ---: | ---: |
| 500 | 9,276 | 4,676 | 10.28 | 4.88 |
| 1,000 | 23,316 | 4,820 | 18.83 | 4.34 |
| 2,000 | 74,132 | 4,792 | 75.20 | 4.83 |
| 4,000 | 270,456 | 5,632 | 274.59 | 5.89 |

Wall time includes process startup and is noisy at this scale. The regression
also runs the 4,000-component case under a 128 MiB address-space limit. This
measures parsing/formatting of this input, not LSP throughput for all workloads.
