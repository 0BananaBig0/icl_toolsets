# Google Style Implementation Plan

> **For agentic workers:** Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Apply Google C++ naming and layout to handwritten `src/` and consistent GNU Make conventions to the Makefile, without changing tool behavior.

**Architecture:** Retain the shared AST and processing pipeline. Use clang-format's Google preset for C++; inspect names, encapsulation and generator interfaces separately. Flex/Bison notation and Make recipes require language-specific formatting.

**Tech Stack:** C++17, GNU Make, Bison/Flex, clang-format/clang-tidy 22.

**Spec:** User request: Google style is a requirement covering files, classes, variables, functions and other source conventions; `docs/coding-style.md` records the concrete policy.

## Global Constraints

- Preserve executable names, CLI/LSP behavior, ICL syntax, token spellings and all existing fixtures.
- Keep generated parser/scanner files and analysis reports out of Git.
- Keep GNU installation variable names and recipe tabs; Google publishes no Makefile style guide.
- Source filenames already follow lower_snake_case; retain `.cc`, `.h`, `.l`, `.yy` extensions.
- Existing authorization covers repository edits and inline execution without repeated approval.

## Review Focus

- Naming changes must update grammar actions, lexer bridges and test callers.
- Generator-defined hooks (`yylex`, `error`, `YY_*`) are exceptions, not candidates for blind renaming.
- Flex regex and Bison action boundaries must survive layout changes unchanged.
- Make recipe continuation, quoting and source-archive contents must remain valid.
- Style tools must be development-only dependencies and must not enter generated code.

## Task 1: Define and demonstrate the style checks

- [x] Add Google clang-format and naming-only clang-tidy configuration plus documented policy/requirement.
- [x] Run formatting/naming checks on existing code and record violations before refactoring.
- [x] Establish baseline with `make check-dynamic BISON=/usr/bin/bison -j4`.

## Task 2: Apply source and Makefile conventions

- [x] Rename nonconforming handwritten enum values, functions and constants with all callers.
- [x] Give class state private underscore-suffixed members and accessors; retain plain struct field names.
- [x] Format C++ and embedded actions, retaining grammar/regex meaning; normalize Makefile layout and include paths.
- [x] Add `make formatcheck` / `make namingcheck`, and include maintained style configuration in source archives.

## Task 3: Verify and integrate

- [x] Run formatting/naming checks, Makefile tests, full dynamic regression and grammar/scanner coverage.
- [x] Build source archive with generators disabled; check dynamic installation under an ignored temporary prefix.
- [x] Independent review; fix findings, commit, fast-forward and push.

## Execution record

Baseline and style reports are ignored under `build/`. Style failures are the red checks for this behavior-preserving refactor; existing runtime suites must remain green. No new runtime feature is requested.

Ruling: retain `src/` as the include root. Repository-prefixed includes broke
the existing flat installed-header layout; source-root-relative includes meet
Google conventions while preserving compatibility. Added an installation test
that compiles headers without the checkout; it failed before this correction.

## Verification — 2026-10-22

- Baseline and final `make check-dynamic BISON=/usr/bin/bison -j4` passed:
  1876 conformance cases, 796 corpus assertions, 13 goldens, 74 CLI cases,
  62 original LSP cases, 42 semantic LSP cases, 3 protocol tests and 54 unit
  assertions. Final grammar/scanner coverage: 334/334 and 119/119 rules.
- `make formatcheck namingcheck BISON=/usr/bin/bison` passed with LLVM 22;
  deliberately bad formatting/naming samples were rejected.
- All handwritten headers compiled independently. Three Makefile tests passed,
  including the new installed-header regression (observed red, then green).
- Bison grammar XML and all seven Flex matching tables matched the baseline.
- Source archive included the style configurations and built dynamically with
  `BISON=false FLEX=false`. An isolated installation passed formatter parsing,
  formatting/idempotence and the three LSP protocol tests.
- Independent review confirmed preserved control flow and closure of its two
  findings: installed header paths and Makefile continuation indentation.
- Static builds, raw line/branch coverage and live editor integration were not
  rerun for this style refactor. The prior unmet coverage gate remains visible
  in the requirements; rule coverage is not branch coverage.
