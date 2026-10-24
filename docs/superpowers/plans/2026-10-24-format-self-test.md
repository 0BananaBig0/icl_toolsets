# Formatter self-test implementation plan

**Goal:** Enforce `F(input) == expected` and `F(F(input)) == F(input)` for the
existing ICL/PDL golden cases, through both CLI and LSP formatting.

**Architecture:** Extend `tests/golden_test.py` and reuse `lsp_test.Client`.
Keep one server per language run. Retain the separate language fixture roots
and existing `.expected` files; no production parser/formatter changes.

**Tech stack:** Existing Python standard-library test runners and Make.

**Spec:** The user's two-pass self-test and saved regression-output request.
Normal checking never changes expected files. Explicit updates may stage
outputs in memory but write them only when every selected case passes CLI
stability and any requested LSP checks. A passing formatter cannot certify its
own semantics; baseline diffs and independent syntax/content checks remain
necessary.

- [x] Add failing runner tests for second-pass differences, errors/timeouts,
  stable-but-wrong output, deferred baseline updates, and LSP parity/stability.
- [x] Implement second-pass checks, optional `--server`, and deferred updates.
  Check actual whole-document LSP ranges, including UTF-16, and diagnostics
  after original open and formatted full-document change.
- [x] Wire CLI/LSP goldens into `goldencheck` and coverage. Document first-pass
  expected files and the first output as the second-pass expectation.
- [x] Run focused runner/golden checks, independent review, complete dynamic
  regression, and static golden checks. Report limitations without claiming
  complete semantic correctness. No commit/push or baseline regeneration.

Scale: 19 maintained golden cases, three CLI calls per case and one persistent
LSP process per language. Keep the broad existing corpus idempotence/property
tests; avoid duplicating the entire generated corpus as tracked golden files.

Validation: 19 runner tests, 13 ICL / 6 PDL goldens through dynamic and static
CLI/LSP pairs, and full `make check-dynamic` passed. Independent review found no
actionable issues. Coverage's functional suites passed; the unchanged strict
gate remains below its thresholds at 97.5% lines / 61.7% branches. No saved
expectations changed. Details are in `docs/formatter-self-tests.md`.
