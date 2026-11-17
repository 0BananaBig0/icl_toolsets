# Public Operand API Coverage Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Verify measured public numeric/target API rejection and valid limits.

**Architecture:** Extend two existing native test runners using real public
APIs and existing fixtures/helpers; production behavior and build gates unchanged.

**Tech Stack:** C++17, existing Make/gcov and Clang ASan/UBSan, Rocky GCC15.

**Spec:** docs/superpowers/specs/2026-11-17-public-operand-coverage-design.md

## Global Constraints

Parent master5fcd6a2, task/public-operand-coverage. No production, target,
dependency or threshold changes;100% lines/90% raw branches stay required.
Maintain existing tests in place; no new maintained scripts. Preserve Task119
raw evidence and frozen old native binaries before reusing instrumented objects.

## Review Focus

- Intended API diagnostic, not parser rejection or arbitrary exception.
- Valid max-size and parameter/alias ordering controls still accepted.
- Malformed public IR distinguished from source-language model support.
- Bounded allocations/loops: <=1MiB payload, no huge target-reference vector.
- Final raw strict verdict retained, including exception/cleanup edges.

### Task 120: Cover public operand boundaries

**Files:** Modify tests/src/pattern_expression_test.cc,
tests/src/pattern_symbolic_test.cc and docs/pattern-generation.md,
docs/pattern-model-coverage.md; create this plan and its spec only.

**Interfaces:** Reuse Rejects/Require, TestPdlOperands fixture,
EvaluateInteger/EvaluateConstant(SourceToken range), PdlNumber(context,text,width),
ResolvePdlTarget(netlist,text), PdlDataValue(context,netlist,target,text).

- [x] Record task-start/base/brief and baseline zero-hit guards in Task119
      reports. Expected: coverage RED for expression48/111/149/193/261/267/
      297/298/306/311 and values25/39/101/137/144/153/161/169; behavior exists.
- [x] Add direct expression-token/range cases and PDL numeric/target/malformed
      public-IR cases specified above, retaining valid controls. Run normal
      two native runners. Expected: intended runtime errors and both exit0;
      no product fix or fabricated functional RED.
- [x] Format/tidy changed sections; run leak-enabled SAN two runners and
      focused fresh-profile instrumented runners. Expected: exit0 and actual
      target guard hits; preserve full Task119 profiles/binaries first.
- [x] Freeze six selected files and3994 input hashes (delta only two tests).
      Run changed native groups on Rocky with exact hashes; run unchanged
      complete coverage recipe with cleared profiles/owned report directory.
      Expected: suites complete; report actual strict success/failure unchanged.
- [ ] One independent final review and necessary fix pass. Verify raw reports,
      fingerprints and diagnostics; document exact results and remaining gaps.
      Expected: scoped task gates verified without claiming global acceptance.
      Commit/merge master/delete branch/task-done/archive/sync; no push.

Preflight: the two runners share public operand implementation but neither
produces an interface the other consumes; no production changes. Existing
symbolic fixture and expression Rejects helper avoid new framework/targets.
Plan/spec self-review: all required cases/checks mapped above; small test-only
scope uses standing routine delegation and inline execution.

### Public operand API coverage (2026-11-17)

The existing native expression and symbolic runners now exercise public numeric
and target validation directly. Malformed token spans/literals/operators and
target ranges/indices require the intended runtime-error diagnostic. Valid
parameter arithmetic, target declaration/alias ordering and exact supported
numeric width/text boundaries remain accepted. The largest text is 1MiB+3
characters for a just-over-limit rejection; valid payloads are at most one
megabit. Cases run inside the two existing native processes with bounded linear
work and a few MiB transient data; no product speedup is claimed.

Small caller-created public IR copies exercise oversized targets, aliases with
missing/cyclic sources and an empty enum target. These cases establish API
validation, not acceptance of those models through the ICL parser. Product
source, recipes, targets, flags and external dependencies are unchanged.

Both native runners pass normal, leak-enabled ASan/UBSan, instrumented and Rocky
checks. Changed-section format/tidy and strict C++17 test-source syntax checks
pass. A new structured-binding capture was replaced with a C++17 pair reference;
an unchanged production scheduler capture warning stays outside this test-only
scope. The first focused command retained old binaries because COV_DIR was
omitted; its guard assertions correctly failed. The corrected Make target
selection rebuilds both test runners and hits all 18 designated guards. Initial
logs/reports and Task119 profiles/raw reports/old native binaries are preserved.

The complete unchanged coverage recipe clears profiles and uses GCC/gcov16.2.1,
-g -O0 -fprofile-arcs -ftest-coverage, serial suites and at most two compiler
jobs. It reuses 17 identical instrumented binaries and rebuilds the two changed
native runners. Wall time is 1344.132797s. All 58 Python groups/940
cases pass, including 51 generator groups/778 cases, plus native checks, 1916
regression and 2180 conformance cases. The strict gate remains failed:
17,331/17,961 lines (96.5%) and 29,262/46,677 raw branches (62.7%), against unchanged 100%/90% thresholds. Make exits 2 only at the
final report. All raw exception/cleanup edges stay included: missing edges
include 12,232 compiler-marked throw edges and
5,183 other edges. No unreachability proof or waiver.

Evidence is build/public-operand-coverage-20261117/: full log, 54 authored-source
reports plus raw library reports/profiles, command/environment, 3994 input
hashes, 19 instrumented binary fingerprints, focused/remote data, analysis and
artifact hashes. Only the two native test inputs differ from Task119. All ten
local and ten remote product binaries remain unchanged. Task119 CLI71/native78,
Task116 full/SAN32 and Task117 platform gates are reused only for unchanged
bytes, with the changed native groups freshly qualified here. No fresh Tessent
run or product benchmark is claimed. Independent final review and integration
are pending; strict coverage and broader model/vendor work remain open. No push.
