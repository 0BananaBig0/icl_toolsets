# Source categories implementation plan

> Execute inline with superpowers:executing-plans; independent final review and full regression.

**Goal:** Implement the user's requested `src/icl`, `src/pdl`, `src/pattern`, and `src/utility` layout.
**Architecture:** Move existing files without changing runtime behavior or executable names. Keep qualified includes relative to `src`; mirror category paths in authored build objects and installed headers. Generated Bison/Flex output remains under the existing generated directory.
**Tech stack:** C++17, GNU Make, Flex/Bison, Python test harnesses.
**Spec:** The user's four-directory request, with existing language separation and shared-AST requirements unchanged.

## Constraints and decisions

- ICL includes AST, parser/scanner, semantics, format tree/job, and CLI/LSP entry points. PDL includes its document frontend and entry points. Pattern includes the generator entry point and all pattern models/runtime/scheduling files. Utility includes common CLI/LSP templates, JSON, UTF-8, positions and file I/O.
- Preserve existing test locations; update their includes and source references. Do not rename executables or regenerate golden outputs.
- Reuse the clean current checkout and prior full regression baseline; the user has authorized autonomous repository edits. No new worktree or generated committed artifacts are needed for this path-only task.
- Complexity and runtime behavior remain unchanged. Build/test/distribution consumers must discover every nested source/header.

## Task 1: Move sources and repair consumers

- [x] Update existing Makefile integration tests to require categorized installed headers and nested dependency rebuilds; observe failure before migration.
- [x] Move all authored sources, update qualified includes and guards, Makefile object/source paths, directory creation, header installation/uninstallation, archive layout, style/coverage/dependency discovery and source-reading tests.
- [x] Update current developer documentation and audit for stale active paths. Keep historical review plans as historical records.
- [x] Run focused Makefile/coverage/source-distribution checks, clean build and full regression. Do not weaken any oracle or golden.
- [x] Independent final review; fix issues and verify affected checks. Leave a reviewable working tree for the user.

## Review focus

1. Fresh builds and packaged builds cannot depend on stale flat objects or old checkout headers.
2. Every installed header resolves using only the installation's include root; uninstall preserves unrelated files.
3. Normal, coverage, grammar-coverage, static and sanitizer variants retain dependency tracking and source discovery.
4. Test scripts and Vim keyword comparisons still read the actual language-specific sources.
5. Goldens and grammar behavior remain unchanged; include changes must not accidentally merge language interfaces.

## Verification ledger

- 57 sources moved: 13 ICL, 4 PDL, 32 pattern, 8 utility. Compared to the baseline, their contents differ only in includes, include ordering and header guards.
- The installed-layout test failed before migration, then passed. Nine Makefile integration tests pass, including the new upgrade/uninstall ownership case.
- Fresh source-archive build/install/uninstall passed with Bison/Flex and static Tcl unavailable; ordinary dynamic build and Google formatcheck passed.
- Coverage reader tests and a fresh instrumented build/run across all four categories passed. `gcov -b -c -n` successfully read nested objects; this smoke run is not a new full coverage measurement.
- Independent review found an optional Graphviz dependency in the new graph test and obsolete installed headers on upgrade. Both fixed with focused checks; reviewer confirmed no remaining findings.
- Ruling: preserve existing test locations, qualify includes, and update only active documentation. Historical plans retain their old paths.
- Full regression passed twice; the final run after installer/style fixes exited 0. Command: `ICL_REF_DIR=/tmp/icl-reference-honza-20261021 PATH="$PWD/build/combined-audit/reference-venv/bin:$PATH" make check BISON=/usr/bin/bison -j4`. Evidence: `build/source-layout-final-regression.log`. Both editors processed 38,634 ICL and 806 PDL files; all 334 grammar/119 scanner rules were exercised. Seven Tessent-certified goldens passed without fixture changes. Final Google formatcheck and whitespace checks passed.
- No parser/runtime logic changed. Sanitizers and the full strict C++ coverage gate were not rerun for this relocation; the fresh instrumented smoke run only verifies relocated coverage build/report plumbing. Changes are left uncommitted.
