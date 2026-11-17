# Current Coverage Audit Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Produce complete current authored-code coverage evidence and concrete next test targets.

**Architecture:** Run existing make coverage unchanged in fresh owned directories;
use existing source-located report and native source review to classify gaps.

**Tech Stack:** Existing GCC/gcov/Make/Python; no new dependency.

**Spec:** docs/superpowers/specs/2026-11-17-current-coverage-audit-design.md

## Global Constraints

Parent master94bc043; task/current-coverage-audit. Preserve100%line/90%rawbranch
thresholds, all authored source files, never-executed and exception/template edges.
No product/test changes in measurement phase, no new failure machinery or masked
failures. Preserve old coverage directories and all recovery data. Source matches
Task116 full/SAN32 and Task117 platform51suites776tests pervariant; reuse exactly.

## Review Focus

- A stopped suite is not a complete measurement or a threshold-only failure.
- Old/stale profiles cannot mix with current source.
- Missing headers/grammar source or exception edges cannot disappear from counts.
- Instrumentation-specific failures cannot be reported as normal regression failures.
- Audit completion does not mean the strict requirement passed.

### Task 118: Refresh strict authored-code evidence

**Files:** Documentation-only generator/coverage docs and this plan/spec; existing
Makefile/tests/coverage.py source and thresholds unchanged. Reports under owned
/tmp/icl-coverage-94bc043-20261117, build under worktree build/coverage-20261117.

**Interfaces:** Existing make coverage with COV_DIR/COV_REPORT_DIR overrides;
existing tests/coverage.py --min-line100 --min-branch90 --strict.

- [x] Record task-start/parent, read brief, verify input hashes and fresh output
      directories; preserve any root *.gcov before existing cleanup recipe.
- [x] Build instrumented targets with bounded-j2 then execute all existing coverage
      suites serially. Expected all suites complete; terminal strict threshold
      may fail. Preserve logs, exact exit/walltime/profiles/current source hashes.
- [x] Read complete per-file/total reports and inspect concrete uncovered paths
      via CodeGraph/current source. Record actual semantic test opportunities and
      compiler exception edges separately without changing totals or thresholds.
- [x] Independent evidence review; document actual current measurement and next
      scoped tests, explicit unchanged-code gate reuse and remaining requirements.
      Commit/merge master/delete branch, task-done/final verifier/archive/sync;
      no push. Do not mark broader100%/90% acceptance complete on a failed gate.
