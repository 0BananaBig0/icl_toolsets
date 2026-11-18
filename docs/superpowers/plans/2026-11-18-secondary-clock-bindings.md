# Secondary Clock Binding Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Admit the measured named-negative clock input form in its vendor context.
**Architecture:** Narrow existing CheckSignalSources predicate; unchanged scope,
flat-origin/alias handling and SystemClocks proof.
**Spec:** docs/superpowers/specs/2026-11-18-secondary-clock-bindings-design.md

## Global Constraints

Parent mastere6ca6b3; task/secondary-clock-bindings. Source/test/guide files below
plus this plan/spec only initially. Retain all old source eligibility guards and
raw100/90 thresholds. New maintained fixtures, if directional acceptance is
achieved, belong under tests/ and require recorded provenance. No dependency,
public API/command or clock event model. Preserve previous evidence; push only
verified parent master after merge/delete. Final task verifier will be
python3 /tmp/secondary-clock-verify.py.

## Review Focus

- Exception must require negative ClockPort destination, Source role and named
  secondary ClockPort/ToClockPort driver; no leakage into other source contexts.
- Alias terminal resolution, parent/direct-child scope and restored original
  types/bit identity must still be checked after admitting the source kind.
- Eligibility is not a phase proof; current runtime constraints, old snapshots,
  partial missing bits, cycles/unknown/mismatched routes remain observable.
- Vendor model/source/normalized/export directions and negative-command rejection
  must be distinguished; raw coverage and claimed package identity stay honest.

### Task 129: Support secondary clock input source bindings

**Files:** src/pattern/pattern_netlist.cc; tests/src/pattern_netlist_test.cc;
tests/src/pattern/test_system_clocks.py; docs/pattern-generation.md;
docs/pattern-model-coverage.md; this plan and spec.
**Interfaces:** CheckSignalSources private check lambda and existing AliasResolver;
existing TestClockSignalSources/native test runner and SystemClocks CLI helpers.

- [x] Record base/brief/callers/references; preserve control probes. Add maintained
      positive/negative source/flat/runtime tests first. Expected actual RED at
      current source-kind rejection, rather than vendor negative-command failure.
- [x] Add the narrow eligibility predicate before existing scope validation.
      Expected focused GREEN, unchanged generic source-kind rejections.
- [x] Run changed C++ format/tidy/strict17, full focused dynamic/static/native/SAN
      and existing scale checks. Expected relevant gates0 without new broad policy.
- [ ] Freeze runtime bytes, full normal integration then fresh raw coverage;
      current remote/native51suites per variant, vendor/model/export directions,
      archive/install qualification. Expected normal0; report actual strict gate.
- [ ] Record scope/results; candidate commit; one independent review and necessary
      fix pass; final verification/merge/delete/taskdone/archive/sync/push. Expected
      clean synchronized master, with any broader acceptance gaps retained.

Pre-flight: single task, no shared-task interface. Caller is elaboration Run after
RestoreFlatOrigins and CheckCombinationalConnections; exact caller inspected.
