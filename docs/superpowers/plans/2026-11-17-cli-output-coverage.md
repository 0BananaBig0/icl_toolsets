# CLI and Output Coverage Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Cover measured CLI argument/input and staged-output failure behavior.

**Architecture:** Extend two existing test runners; use real files/public API
and observed coverage hits. Production source, product build recipes and gates unchanged.

**Tech Stack:** Existing Python unittest/C++17 Check/Make/gcov/Clang SAN.

**Spec:** docs/superpowers/specs/2026-11-17-cli-output-coverage-design.md

## Global Constraints

Parent masterde758f6, task/cli-output-coverage. Tests-only: no production changes,
new framework/target/failure injection, coverage exclusions or weakened100%/90%.
New test scripts if required must mirror source beneath root/tests; current plan
maintains existing tests in place. Preserve Task118 profiles and raw reports.

## Review Focus

- Invalid arguments return2 and identify the offending option.
- Missing/NUL input failures preserve result/reset/debug artifacts.
- Failed repeated Prepare retains the original stage and other file.
- Failed Publish/destruction preserves directory sentinel and cleans temporary files.
- Targeted GREEN is not a passing global strict gate; retain all raw edges.

### Task 119: Cover argument and output boundaries

**Files:** Modify tests/src/test_icl_pattern_generation_main.py,
tests/unit_tests.cc, the existing native target header prerequisite in Makefile
and generator/coverage docs; create this plan/spec only.

**Interfaces:** Reuse Generator.run_tool(*arguments,ok=False), Check/CheckEq,
toolsets::StagedTextFile::{Prepare,Publish}, existing normal/SAN/COV targets.

- [x] Record parent/task-start/read brief; verify actual baseline target guards
      zero-hit in Task118 reports before adding tests. State coverage RED, not
      missing functionality. Inspect existing callers and helpers.
- [x] Add Generator bad-argument cases and unreadable/NUL ICL/PDL cases, asserting
      return2/problem context/unchanged result/reset/debug files. Add native
      public-stage transition/abandonment/rename-failure checks using owned tmp.
- [x] Run normal unitcheck and full changed Generator suite for dynamic/static;
      expected all cases pass. Run focused fresh instrumented groups and check
      measured guard hits, preserving old profiles. Format/tidy changed C++ test.
- [x] Run fresh leak-enabled SAN for changed native/CLI groups. Freeze tests;
      complete existing coverage recipe in fresh owned directories, expected all
      suites complete, strict report retains actual success/failure. Compare
      source-located target hits and raw global totals without filtering.
- [x] Qualify changed groups on Rocky both generator variants/native unit,
      matching current inputs; explicitly reuse unchanged prior gates. One
      independent final review/necessary correction pass. Document actual
      results and remaining strict gap; commit/merge master/delete branch,
      task-done/final verifier/archive/sync; no push.

Preflight refinement: native unitcheck manually lists header prerequisites and
did not include file_output.h. Real make -n -W header unitcheck RED schedules no
compile; add that header to the existing target only, then GREEN schedules the
unit binary rebuild. Makefile is the third amended input, with product build
recipes/flags unchanged. No new test target or runtime behavior.

Single review fix pass: use otherwise-valid ICL/PDL plus NUL and require the
input-reading diagnostic, so downstream syntax failures cannot satisfy the NUL
test. Empty file lists require their specific missing-files diagnostic. Initial
full-run evidence is retained but is not final qualification of this correction.

### CLI and staged-output coverage (2026-11-17)

Two new cases in the existing generator CLI suite cover argument/input errors
and preservation of generated, reset and debug artifacts. NUL cases use valid
ICL/PDL plus NUL and require the input-reading diagnostic; a parser error cannot
satisfy that check. Existing native tests now exercise staged publication without
Prepare, repeated Prepare, successful publication, abandoned stages and real
rename failure with directory/sentinel preservation. Native checks increase
from 60 to 78. The existing unit target adds its missing file_output.h header
prerequisite; a make what-if check changes from no rebuild to a scheduled compile.
There are no production source, product recipe, flag or external dependency changes.

The corrected final snapshot passes all 71 generator cases in local dynamic,
static and leak-enabled ASan/UBSan runs and both Rocky variants; native normal,
SAN and Rocky checks pass 78/0. Format/tidy pass for the changed C++ sections.
An unchanged legacy whole-file tidy finding remains outside this change.
One independent final review found the NUL fixture weakness and broad empty-list
diagnostics; one correction pass fixes both before final qualification.

The complete fresh instrumented recipe takes 1239.545330s with
GCC/gcov16.2.1, -g -O0 -fprofile-arcs -ftest-coverage, serial suites and at most
two compiler jobs. All 58 Python groups/940 cases pass, including 51 generator
groups/778 cases, plus native checks, 1916 regression and 2180 conformance cases.
All 11 selected previously-zero source guards execute. The final strict gate
still fails: 17,311/17,961 lines (96.4%) and 29,201/46,677 raw branches (62.6%), against unchanged 100%/90% thresholds. Make exits 2
only at that final report. No raw edges are filtered or waived. Missing raw
edges include 12,235 compiler-marked throw edges and
5,241 other edges; these labels do not prove unreachability.

Persistent evidence is build/cli-output-coverage-20261117/: complete log, 54
authored-source reports plus raw library reports/profiles, command/environment,
3994 input hashes, 19 instrumented binary fingerprints, focused and remote
results, review summary, analysis and artifact hashes. Only Makefile and the two
existing test inputs differ from Task118; all product source and ten local and
remote binary fingerprints remain unchanged. The initial pre-review full run
is preserved separately and is not the final qualification snapshot.

Task116 normal full/SAN32 and Task117 platform gates are reused for identical
product bytes, with the changed groups freshly qualified here. No fresh Tessent
run or performance benchmark is claimed for this test-only change. The strict
coverage requirement and broader model/vendor gaps remain open. No push.
