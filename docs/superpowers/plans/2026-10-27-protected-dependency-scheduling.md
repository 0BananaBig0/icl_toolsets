# Protected dependency scheduling implementation plan

> Execute inline with Superpowers TDD, independent review and full regression.

**Goal:** Permit dependency-guided batches on large write-only networks with
protected ScanRegister values, including explicit changes to taken controls.
**Architecture:** Reuse TryGreedy and Solve. Validate reservation values before
selecting the heuristic, constrain prerequisite choices to the held or explicit
requested value, and pass protection through every trial scan. Carry the actual
held state across batches so an explicit transition cannot be undone.
**Tech stack:** Existing C++17, Z3, shared ICL/PDL frontends and Python unittest.
**Spec:** ../specs/2026-10-25-icl-pattern-generation-design.md; extends the user's
approved dependency-guided reset/select scheduling requirement.

## Decisions and constraints

- Keep the existing >=32-register write-only threshold, 4096 control-bit limit,
  bounded fallback, trial-state isolation and per-clock replay. No exhaustive
  schedule enumeration, new solver backend or extra dependency.
- Protected control values may stay at their held value or advance to the
  original apply's explicitly requested full value. Automatic local-reset writes
  cannot acquire this permission. Retain original partial/X fill across batches.
- A failed heuristic leaves live state unchanged and falls back to the bounded
  solver. No general optimality or expanded model support is claimed.
- Scope: scheduler implementation, existing dependency/resource tests and their
  source-classified fixtures/documentation. No CLI changes.

## Task 1: Preserve reservations across prerequisite batches

- [x] Add tests to tests/src/pattern/test_dependency_scheduling.py: 40-level
  tree with a protected non-control register; taken gate explicitly opened;
  fixed-point original/flat/generated replay; protected blocked gate fails
  without publishing; partial/X protected writes retain original fill.
- [x] Run a new deep protected case against the existing binary and observe
  the 32-scan/resource-limit failure.
- [x] Extend TryGreedy inputs with held/desired values and explicit-write mode.
  Move reservation validation before its call, constrain control assignments,
  and pass/update held values through the existing one/two-step Solve calls.
- [x] Run dynamic/static generation and scheduler checks, then independent
  review focusing on explicit permissions, monotonic transitions, reset effects,
  early-visible fast path, fallback state preservation and TAP replay.
- [x] Run full make check, Google formatting and leak-enabled ASan/UBSan.
  Record timing/input sizes for the 40-level case; do not claim optimality.
  Update README, generator guide and coverage map with remaining limitations.

## Evidence

Prior accumulated changes committed and pushed as 63d6629.

- RED: the protected 40-level fixture failed at the old 32-scan limit,
  `build/protected-dependency-red.log` (1.314 s, failure rather than throughput).
- GREEN: dynamic/static generation targets and native scheduler tests passed,
  `build/protected-focused-regression.log`: 13 dependency cases, 17 resource
  cases, 26 existing generator cases and 9 merge cases for each executable.
- Independent review found no actionable defects in the implementation or tests.
- The new 40-level protected case uses a 5398-byte ICL input and emits 41 scans.
  Command: `/usr/bin/time -f 'wall_seconds=%e peak_rss_kib=%M' python3
  tests/src/pattern/test_dependency_scheduling.py build/icl-pattern-generation
  DependencyScheduling.test_deep_tree_with_protected_storage`.
  WSL2, g++ 16.2.1 `-O2`, Z3 4.13.3: CLI 0.313 s; test including generated/flat
  replay 3.60 s wall and 78,356 KiB peak RSS. This overlapped other checks and is
  not a speedup claim. Log: `build/protected-dependency-measurement.log`.
- ASan/UBSan with LeakSanitizer enabled passed all 13 dependency cases,
  17 resource cases and native scheduler checks. Logs:
  `build/protected-dependency-sanitize.log`, `build/protected-resource-sanitize.log`,
  `build/protected-scheduler-sanitize.log`. External Z3 is uninstrumented.
- Full `make check BISON=/usr/bin/bison -j4` passed, exit 0, with
  `ICL_REF_DIR=/tmp/icl-reference-honza-20261021` and the existing reference venv
  on PATH. Log: `build/protected-dependency-regression.log`. Includes seven
  offline vendor goldens, 334/334 grammar alternatives, 119/119 scanner rules,
  and Vim/Neovim checks on 38,644 ICL and 812 PDL inputs each. Google formatting
  and diff whitespace checks passed. README and developer docs were updated.
- Fresh Tessent execution and strict authored-C++ coverage were not rerun.
  The broader generator model/scheduling requirements remain incomplete.
