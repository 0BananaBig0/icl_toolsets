# Toolset audit fixes implementation plan

> **For agentic workers:** Use the Superpowers implementation, TDD, review and
> verification workflow. Track completed checks below; keep changes uncommitted
> until requested by the user.

**Goal:** Fix the eight reproduced issues in the review of commit `c9206af`.

**Architecture:** Retain shared CLI/LSP infrastructure and each language's own
shared document/AST pipeline. Fix I/O in the common CLI, scope and resource
handling in the corresponding language document, and highlighting in the
standalone runtime. No new runtime dependencies or generated tracked files.

**Tech stack:** C++17, POSIX file operations, Tcl's non-evaluating parser APIs,
Flex/Bison for ICL, Python unittest, Vimscript.

**Spec:** The user's approved eight-finding audit in this conversation: write
failure data loss, PDL nested substitution crash, quadratic ICL path storage,
ICL enum comparison navigation, PDL variable identity, unchecked input reads,
ICL numeric highlighting gaps, PDL constant highlighting in substitutions.

## Global constraints

- Keep ICL and PDL grammars, tokens and document definitions separate.
- CLI and LSP for a language retain one shared parser/AST/analysis pipeline.
- Changes stay in this repository; editor checks use isolated configurations.
- New scripts belong under `tests/`, mirroring their tested source path.
- Preserve Google C++ style. Do not commit generated output or vendor code.
- No execution of vendor PDL, no new interpreter evaluation in product code.
- Work in the current checkout under the user's existing autonomous-edit scope.

## Review focus

- Failed writes leave original bytes intact, including symlink destinations;
  successful replacements preserve file permission bits (task 1).
- Empty input succeeds, but a directory/read failure fails; no temporary files
  remain after write errors (task 1).
- Nesting protection distinguishes real substitutions from escaped text,
  braced data and comments without rejecting ordinary vendor sources (task 2).
- Parentheses/subscripts preserve enum context, and repeated assignments share
  one variable identity without merging different scopes (tasks 2 and 3).
- Highlighting newlines/comments preserves their syntax groups and does not
  bleed into strings, other languages, or following statements (task 4).

## Tasks and verification

### 1. Safe common formatter I/O

Files: `src/format_cli.h`, new `tests/src/test_format_cli.py`, `Makefile`,
user/developer documentation where behavior needs clarification.

- [x] Add and observe failing regressions for write-limit failures and directory
  inputs in both formatters; cover empty input, permissions, symlinks and cleanup.
- [x] Write through a unique sibling temporary file, check write/close and
  replacement, and leave originals intact on failure. Preserve intended symlink
  targets and modes. Report input and stdout failures.
- [x] Run the focused CLI suite and existing CLI/formatter checks.

### 2. PDL parser robustness and variable identity

Files: `src/pdl_document.{h,cc}`, `tests/src/test_pdl_tools.py` and focused tests
under `tests/src/` as needed.

- [x] Reproduce deep-substitution termination and split variable references with
  bounded child processes and core dumps disabled.
- [x] Protect recursive Tcl parsing before it can exhaust the native stack;
  retain opaque data and the existing diagnostic range contract.
- [x] Reuse one declaration per variable scope and treat later assignments as
  references. Keep distinct procedure/namespace variables separate.
- [x] Run functional, syntax-property, block and vendor-corpus PDL checks.

### 3. ICL semantic context and path storage

Files: `src/icl_semantics.{h,cc}`, relevant shared AST/parser metadata only if
needed, `tests/src/test_icl_lsp.py`, bounded resource regressions in `tests/src/`.

- [x] Add RED regressions for `mode == (ON)` and `mode[$IDX] == ON`, including
  a same-named unrelated port and parenthesized/subscript variations.
- [x] Preserve complete comparison operand context instead of the most recent
  unrelated name use. Keep the parsed representation shared by both tools.
- [x] Replace per-component full prefix copies with shared path storage; test
  navigation on each component and unchanged instance/member completion.
- [x] Measure 500/1000/2000/4000-component inputs, then run ICL semantic checks.

### 4. Vim highlighting

Files: `vim-icl-pdl-modern/syntax/{icl,pdl}.vim`,
`tests/vim-icl-pdl-modern/test_syntax.vim`.

- [x] Observe RED for newline/comment-separated ICL literals, time suffixes,
  and `STD_1687_2014` inside a quoted command substitution in both editors.
- [x] Extend the existing syntax rules, preserving comment/string boundaries.
- [x] Run the focused plugin tests; full corpus scan runs during integration.

### 5. Integration and independent review

- [x] Review all changes independently, resolve substantive findings, and update
  developer documentation with behavior/resource limits and measured evidence.
- [x] Run `make check-dynamic formatcheck namingcheck BISON=/usr/bin/bison -j4`.
- [x] Build static variants sequentially after dynamic checks if the installed
  Tcl archive supports them; run focused new regressions on those binaries.
- [x] Check the diff and tracked/untracked paths; report limitations accurately.

## Progress

- Baseline: commit `c9206af`, clean checkout. Prior full dynamic suite passed;
  strict C++ coverage remains below its existing gate and is not a claim of this
  work. Eight failures have concrete reproductions from the read-only audit.

- Focused checks: shared CLI 12 methods, PDL 39 functional methods and ICL 44
  LSP semantic methods passed. The new I/O tests first failed on the baseline;
  the additional directory-stdin case also reproduced before its fix.
- Vim/Neovim focused checks: 4 methods / 7 editor runs passed, including
  a multiline block-comment delimiter edge discovered during implementation.
- Independent integrated review found no additional defects. Required Google
  formatting, naming and braces checks passed after correcting new-code style.
- ICL measured 4,000-component peak RSS fell from 270,456 to about 5,632 KiB;
  the bounded 128 MiB regression passed. Details are in docs/development.md.

- Final dynamic verification passed with `make check-dynamic
  BISON=/usr/bin/bison -j4`; `make formatcheck namingcheck
  BISON=/usr/bin/bison` passed separately on the same source.
  This includes 1,416 regression cases, 2,055 conformance cases, 29,868 block
  variants, 5,568 keyword cases, 438 PDL syntax-property cases, 192 PDL block
  cases, and the vendor PDL corpus.
- Grammar instrumentation reached 334/334 parser productions and 119/119 lexer
  rules. Rule coverage is not a claim of complete semantic or branch coverage.
- Both Vim and Neovim scanned 38,487 ICL and 713 PDL files (525,625 lines per
  editor, 39 explicit real-file probes), with no errors. All six plugin test
  methods passed, including isolated vim-plug installation/loading checks.
- `make static BISON=/usr/bin/bison -j4` passed. The static binaries passed
  12 shared CLI methods, the bounded ICL resource regression, 44 ICL LSP methods,
  and 39 PDL functional methods. ELF inspection confirmed static linking.
  Optional Tcl/glibc NSS linker warnings remain; target-host portability was
  not tested.
- `make coverage BISON=/usr/bin/bison -j4` ran the instrumented suites
  successfully but failed its existing strict gate: 4,079/4,182 handwritten
  source lines (97.5%) and 6,861/11,175 branches (61.4%), below 100%/90%.
  Parser source lines reached 570/570; lexer source lines reached 211/213.
  File-size-limited children exclude exit-time gcov writes as documented.
- Five optional ANTLR integration checks were skipped because `ICL_REF_DIR`
  was unset. No fresh real coc.nvim session or installation was run for these
  fixes; protocol and isolated Vim/Neovim tests provide the editor evidence.
- Final diff check passed. Only source, tests and documentation changed;
  generated binaries, fixtures, benchmark logs and coverage reports remain
  ignored. Changes are deliberately uncommitted.
