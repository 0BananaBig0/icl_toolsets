# Native Clock Contract Coverage Implementation Plan
> **For agentic workers:** REQUIRED SUB-SKILL: superpowers:executing-plans.
**Goal:** Cover native clock argument, cycle and unrelated-consumer contracts.
**Architecture:** Existing scheduler test runner and real elaborated fixtures;
unchanged SystemClocks APIs and qualification recipes.
**Tech Stack:** C++17, existing Make/GCC/gcov/Clang/Python.
**Spec:** docs/superpowers/specs/2026-11-19-native-clock-contracts-design.md

## Global Constraints
Parent869f31d, task/native-clock-contracts. No production changes, mocked IR,
new dependencies/targets, altered100%line/90%rawbranch gate or recovery deletion.
Reuse qualification only by current source/product identity, report every unrun
suite explicitly. Existing-behavior coverage RED is not a product-failure claim.

## Review Focus
- Malformed native arguments must fail for the intended reason, without
  publishing earlier parsed fields or changing saved/current clock definitions.
- Cycle detection must visit inactive mux choices; fixture remains valid ICL.
- Unrelated logic/client pins must neither taint root clocks nor get changed.
- Source/flat forms preserve identities; literal expected durations independent
  of tested implementation.
- Coverage hits, broad reuse and platform/archive evidence remain distinguished
  from failed strict acceptance and independently reproduced qualification.

### Task 133: Native clock contract tests
**Files:** tests/src/pattern_scheduler_test.cc; docs/pattern-generation.md;
docs/pattern-model-coverage.md; this plan/spec.
**Interfaces:** SystemClocks Override/Check/Wait/CheckTimeWait;
ScanScheduler Assume/Value; SerializeIcl; existing native runner both modes.

- [x] Record parent, baseline native0 and8 zero-hit authored lines; read brief.
- [x] Add TestClockApiContracts and TestClockUnrelatedConsumers with literal
      diagnostics/durations/state assertions; wire both native modes.
      Expected native0 and measured8-line coverage GREEN.
- [ ] Format/tidy/strict17, full normal project command, native SAN, fresh
      focused and complete raw coverage, current Rocky native checks.
      Expected executable suites0; report actual strict gate result.
- [ ] Prove unchanged3993 runtime files/10 products, refresh/archive identity;
      qualification docs, task-done verifier and fresh whole-branch review.
- [ ] Fix only necessary findings once; final verification, merge/delete,
      archive ledger, sync and push. Expected clean synchronized master.

Pre-flight: one test-only task, no shared interfaces with another task.
