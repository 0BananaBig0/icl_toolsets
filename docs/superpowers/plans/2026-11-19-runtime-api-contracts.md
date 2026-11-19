# Runtime API Contract Coverage Implementation Plan
> **For agentic workers:** REQUIRED SUB-SKILL: superpowers:executing-plans.
**Goal:** Cover native runtime parse-failure and context-resolution contracts.
**Architecture:** Extend the existing runtime test runner with real tiny designs
and scripts; unchanged Runtime, PDL frontend and context/parameter implementation.
**Tech Stack:** Existing C++17/Tcl/Make/GCC/gcov/Clang/Python.
**Spec:** docs/superpowers/specs/2026-11-19-runtime-api-contracts-design.md

## Global Constraints
Parentdb5f353; task/runtime-api-contracts. Keep100%line/90%rawbranch gate,
all authored reports/exception edges and other workspaces/stashes. No planned
product changes, dependencies, targets, fabricated IR or failure machinery.
Measured coverage gap RED does not claim a product bug. Any real defect requires
root-cause investigation and an explicit necessary-scope ruling before editing.

## Review Focus
- Structurally malformed later input publishes no partial operations; failed runtime cannot
  resume or finish. Diagnostics identify the actual failing source.
- Delayed EOF errors preserve the nested invocation frame, not just file text.
- Fallback nested lookup succeeds independently of elaboration; parameters
  require real specialization and X values are preserved as literal bit strings.
- Procedure namespace selection/clearing and missing module/instance errors
  affect the intended API paths with independent literal expectations.
- Fresh vs reused qualification, all16 hits and unchanged products remain
  separate from the still-failed full project coverage/release requirement.

### Task 134: Runtime API contracts
**Files:** tests/src/pattern_runtime_test.cc; docs/pattern-generation.md;
docs/pattern-model-coverage.md; this plan/spec.
**Interfaces:** Runtime constructors/Execute/Finish/operations/warnings;
existing MakeDesign/Fails/Require, Design/Elaborate and PDL module/proc context.

- [x] Record parent/native baseline0 and16 zero-hit locations; read brief.
- [x] Add constructor/parse, nested EOF, fallback-instance/parameter and namespace
      cases; wire existing runner. Expected native0 and16 target-line hits.
- [x] Run3 style/strict17 gates, full normal project check, changed native leak
      SAN, fresh focused/full raw coverage and current Rocky native checks.
      Expected executable suites0; report unchanged strict-gate actual result.
- [ ] Prove3993 other input/10 product identities; refresh/check source archive,
      qualification docs and task-done verifier; one fresh whole-branch review.
- [ ] One necessary fix pass only; final verification, merge/delete, ledger
      archive, CodeGraph sync and push. Expected clean synchronized master.

Pre-flight: one native test-only task, no shared interfaces with another task.
