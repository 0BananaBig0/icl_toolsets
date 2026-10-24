# LSP review fixes implementation plan

> Execute with the Superpowers TDD and inline execution workflow, followed by
> one independent integrated review. Keep progress and evidence here.

**Goal:** Fix all six findings reviewed at fe5c97b, and run regressions.

**Architecture:** Keep the separate ICL/PDL AST pipelines and shared LSP/CLI
adapters. Extract checked stream reading for reuse. Tag imported ICL documents
as nonformattable by PDL. Reuse the PDL variable index; retain lexical completion
contexts and explicit namespace-variable bindings.

**Tech stack:** C++17, Tcl parse APIs, Python protocol tests, Vim/Neovim tests.

**Scope/authorization:** User approved all six fixes and required regression
execution and investigation of failures. Continue in the current authorized
checkout, as in the preceding implementation. No commits, pushes, installs,
new runtime dependencies, or generated tracked artifacts.

## Tasks

### 1. Document and filesystem safety
- [x] Add RED protocol regressions for ICL-buffer formatting via pdl-lsp and
  watched paths changed into directories/FIFOs in both language servers.
- [x] Add an explicit document format-eligibility guard.
- [x] Extract checked stream reading from format_cli.h into src/text_input.h;
  reject nonregular workspace paths and erase unreadable stale documents.
- [x] Run the new safety tests and existing CLI I/O suite.

### 2. PDL variable identity and lookup cost
- [x] Add RED definition/reference tests for array variables and literal
  namespace variable bindings, including repeated writes and distinct scopes.
- [x] Normalize array variable names, model namespace bindings, and use the
  existing variable index during resolution instead of scanning declarations.
- [x] Add a bounded lookup workload and measure the same 2k/4k/8k workloads
  used during review (three request timings, optimized dynamic binaries).

### 3. Completion contexts
- [x] Add RED tests for namespace-local procedures and top-level iCall under
  a module binding, with scope restoration and separate PDL/Tcl namespaces.
- [x] Retain source-range contexts in the shared PDL document and use them for
  completion; include only visible variables and statically callable procedures.
- [x] Run the complete focused PDL tests and syntax/corpus checks.

### 4. Integration
- [x] Independent review of all changes; resolve substantive findings.
- [x] Run make check-dynamic, formatcheck and namingcheck; investigate any
  failure against intended behavior before changing implementation or tests.
- [x] Build static variants sequentially and run focused new regressions.
- [x] Update developer documentation, record limits and final diff hygiene.

## Evidence and decisions

- Review baseline: fe5c97b. Six reproductions were established before approval.
- Existing strict coverage gate fails (97.5% lines / 61.4% branches); do not
  misrepresent a functional regression pass as a coverage-gate pass.
- Tests belong under tests/src; no vendor PDL execution.
- Retain ordinary immutable source/context snapshots; avoid a second parser,
  runtime evaluation, whole-workspace reference caches or new dependencies.
- RED evidence: directory watcher requests aborted both servers; FIFO requests
  timed out; imported ICL formatting returned an empty replacement; variable
  and context regressions failed; the 32k-variable reference request took
  3.398 seconds against a two-second budget.
- Existing incomplete-procedure completion caught a context regression during
  implementation. A Tcl token probe established that an unfinished braced word
  has a zero-length raw token. Independent review then caught empty completed
  bodies being confused with incomplete words. The added proc/iProc scope test
  failed in both subcases before the raw-token check; no assertions were weakened.
- Final dynamic verification: `make check-dynamic BISON=/usr/bin/bison -j4`
  passed, including 46 PDL executable/LSP tests, 438 syntax properties, 192 PDL
  block cases, 79 unique vendor streams, 1,416 ICL regression cases, 2,055 ICL
  conformance cases, 29,868 block variants and 5,568 keyword-context cases.
  All 334 grammar and 119 scanner rules were reached. Vim and Neovim each
  checked 38,487 ICL and 713 PDL files (525,625 lines), with no failures.
  Five optional ANTLR checks skipped because ICL_REF_DIR was not configured.
- `make formatcheck namingcheck BISON=/usr/bin/bison` passed; clang-tidy
  suppressed 613 non-user-code warnings and reported no project diagnostics.
- `make static BISON=/usr/bin/bison -j4` passed. Static runs passed 2 filesystem
  safety methods (including file replacement), 12 CLI I/O tests, 46 PDL tests,
  6 ICL protocol tests and 44 ICL LSP tests. Static Tcl/glibc NSS linker warnings
  remain as documented in docs/pdl-tools.md; target-host portability is untested.
- Final optimized 32k-variable request: 11.1 ms dynamic, 9.75 ms static.
  Final dynamic benchmark medians: 1.209 / 1.778 / 3.147 ms at 2k/4k/8k.
  Earlier before/after measurements and scope are recorded in docs/pdl-tools.md.
- Local evidence is under ignored build/review-*.log and
  build/review-reference-benchmark.json; no generated outputs are staged.
- `make coverage BISON=/usr/bin/bison -j4` completed all functional tests and
  failed only its unchanged strict gate: 4,125/4,234 lines (97.4%) and
  6,959/11,331 branches (61.4%). Baseline was already below gate at 97.5%/61.4%.
  New text_input.h has 6/6 lines and 18/24 branches covered; unreadable-regular-
  file branches in the shared LSP reader remain uncovered. This task does not
  claim full coverage or lower the acceptance thresholds.
- `git diff --check` and the generated-file hygiene regression pass. All changes
  remain uncommitted; no push, installation or user-configuration edits occurred.
