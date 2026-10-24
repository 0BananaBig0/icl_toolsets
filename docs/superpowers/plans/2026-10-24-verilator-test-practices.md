# Verilator-inspired formatter regression implementation plan

> Use superpowers:executing-plans to implement this plan in the current checkout.

**Goal:** Adopt suitable Verilator regression practices through the existing
formatter golden runner, without importing a compiler-sized test framework.

**Architecture:** Keep source/expected pairs and one runner for both formatters.
Select language and case explicitly; bound each child invocation. Keep ICL/PDL
syntax independent and leave production AST/pipeline boundaries intact.

**Tech stack:** Existing Python standard-library runners, GNU Make and unittest.

**Spec:** User's request to inspect Verilator, adopt suitable tests/organization,
and run complete regression; decisions are recorded below and in the comparison
document. Existing autonomous repo-edit authorization applies.

## Global constraints

- No new dependencies; no Verilator source or HDL fixtures copied into this repo.
- Keep source-attributed fixtures under tests/cases; maintain existing tests in place.
- New runner tests go in tests/tests/test_golden_test.py.
- Downloads, logs and generated files remain ignored under build/.
- Shared per-language AST and pipeline, Google C++ style, separate ICL/PDL rules.
- No automatic acceptance of changed expected output during regression.
- Workload: 13 existing ICL goldens plus a small PDL suite; serial subprocesses
  are adequate. Ten seconds per invocation by default, configurable for slow tools.

## Review focus

- Missing/unmatched cases must fail instead of silently passing zero tests.
- Timeout/crash must fail without rewriting expected files.
- Wrong indentation with otherwise identical tokens must fail exact comparison.
- PDL literal data and command substitutions must survive formatting unchanged.
- Both stdin/file paths must match; existing ICL CLI options remain compatible.

## Tasks

- [x] Add runner self-tests that expose missing PDL selection, case filtering,
  timeout handling, missing expectations and incorrect/stable output.
- [x] Extend tests/golden_test.py with --language, --case and --timeout, preserving
  --binary/--update. Reuse the runner; compare stdin and filename outputs against
  reviewed expected bytes. Report per-case failure details and a unified diff.
- [x] Add small handwritten PDL source/expected pairs under
  tests/cases/ours/pdl/golden; run them through the runner and existing LSP block
  manifest/editor discovery. No PDL execution.
- [x] Integrate golden self-tests and both languages into Makefile check/coverage;
  document selected/rejected Verilator practices and exact upstream revision.
- [x] Independent review, focused checks, full dynamic regression and static
  golden/protocol checks. Investigate failures before claiming success.

## Evidence

Pinned reference: verilator/verilator
63685a23525c7022e17c0b4e5fea6226c42b0306. Read test_regress/driver.py,
test_regress/t/t_lint_width_bad.py and its .out, and docs/internals.rst.
Current gaps: golden_test.py checks only ICL, has no subprocess timeout or case
selection, and a missing/empty discovery set reports success. Existing PDL
tests check idempotence/parity extensively but lack a maintained byte-exact
formatter golden suite. Existing C++ AST ownership/Visitor architecture already
matches the useful separation principle; renaming/moving it adds no value here.
