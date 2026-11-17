# Scheduler C++17 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Remove the measured C++20 binding capture from the C++17 scheduler.

**Architecture:** Ordinary const reference to the same map key; unchanged
overlap callback and runtime control flow. Reuse existing checks and fixtures.

**Tech Stack:** C++17, Clang/GCC, Make, existing Python unittest and ASan/UBSan.

**Spec:** docs/superpowers/specs/2026-11-17-scheduler-cpp17-design.md

## Global Constraints

Parent4104f49, task/scheduler-cpp17. Only source scheduler and four docs change.
No new tests/scripts/framework/flags/dependencies/targets/semantics. Preserve
existing diagnostics, raw coverage evidence and 100%line/90%rawbranch requirement.
No fresh post-edit gcov metric or product speedup claim.

## Review Focus

- Same const map-key reference and lifetime inside the synchronous callback.
- Overlap diagnostics/order and nonoverlapping scans unchanged.
- No unused operation binding or hidden key/Operation copy.
- Actual compiler RED->GREEN and product fingerprints support reuse claims.
- Last pre-edit coverage clearly distinguished from current source attribution.

### Task 121: Use an ordinary key reference

**Files:** src/pattern/pattern_scheduler.cc; docs/pattern-generation.md;
docs/pattern-model-coverage.md; this plan and its spec only.

**Interfaces:** ApplyScans/RawRequests/RawSubchain unchanged; existing raw-client
CLI suites and overlap method; Make dynamic/static generator targets.

- [x] Record base/brief; inspect compiler failure and callers via CodeGraph.
      Expected: existing strict C++17 probe1 with captured-binding diagnostic;
      Makefile CXXSTD=c++17. Preserve old source and two generator artifacts.
- [x] Replace only first loop binding with entry+key const references.
      Expected: same strict compiler command exits0; focused format/tidy0.
- [x] Freeze five files/3994 hashes, delta only scheduler source. Rebuild Root
      generators, compare all10 product hashes and run existing three client
      suites per variant plus leak-enabled focused overlap check.
      Expected: all checks0; if bytes differ broaden generator qualification.
- [x] Compile current scheduler/generator on Rocky with exact source hashes;
      compare all10 products and run affected client checks as required.
      Expected: source/build/checks0; reuse only exact unchanged artifacts.
- [x] Document actual results and pre-edit coverage limitation; candidate
      commit, one independent review/necessary fix pass, verify/merge master,
      delete branch/task-done/archive/sync. Expected: scoped portability gate
      complete; global strict/model/vendor requirements still open. No push.

Pre-flight: no shared implementation interfaces. Small local declaration
change; routine design/execution delegated. Plan/spec self-review complete.

### Scheduler C++17 portability (2026-11-17)

The raw-scan overlap callback now captures an ordinary const reference to the
same map key. Its previous structured-binding capture required a C++20 extension
despite Makefile's C++17 setting. The first interval-checking loop uses only the
key; the unused operation binding is removed. The map outlives the synchronous
callback, and diagnostics, ordering and the later processing loop are unchanged.
No copy, allocation or new control flow; interval work remains O(C log C) time
and O(C) space for C clients. There is no product speedup claim.

The strict Clang C++17 probe changes from exit1 to exit0; changed-section format
and tidy pass. Existing automatic-child, multiple-interface and chain-group
suites pass39 cases per local dynamic/static variant, plus a leak-enabled
overlap test. Rocky GCC15 syntax/build/identity/client checks pass7 gates and39
cases. All3994 input hashes match Root/worktree/remote, with only scheduler
source changed from Task120. All ten local and ten remote product binaries are
byte-for-byte identical to their preserved originals after rebuilding.

Evidence is build/scheduler-cpp17-20261117/: compiler RED/GREEN, local/remote
logs/exits, source/input/product hashes, original generators/source and artifact
hashes. Exact unchanged artifacts support reuse of Task120/119/116/117 runtime
and platform checks. No fresh vendor run or product performance benchmark.
Task120 remains the last complete pre-edit raw coverage measurement at
17331/17961lines (96.5%) and29262/46677branches (62.7%). The declaration edit
shifts source attribution; no post-edit gcov metric is claimed or inferred from
those reports. Strict100%lines/90%rawbranches and broader model/vendor work
remain open. One independent final review found no Critical, Important or Minor findings. The portability fix is qualified for integration. No push.
