# Mixed Clock Defaults Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Preserve native omitted-field semantics while retaining mixed period clock programs.

**Architecture:** Expand validated native Definitions to explicit source and factors at the existing normalization boundary. Keep the default path unchanged when no normalized operation is requested.

**Tech Stack:** Existing C++17, Z3, Tcl and Python unittest; no new dependencies.

**Spec:** docs/superpowers/specs/2026-11-27-mixed-clock-defaults-design.md

## Global Constraints

Preserve native IEEE omitted-field defaults, checked versions and atomic failures.
Keep dynamic/unrepresentable source, oscillator-only and existing hardware guards.
Keep C++17, existing resource limits, source-preserving procedures and item1 exclusion.
Use existing five-form replay and owned scratch evidence; preserve 117 stashes.

## Review Focus

- Omitted fields use ICL defaults after prior period and source/ratio overrides, never the last override.
- Default sources through muxes, aliases, inverted edges and missing sources cannot be silently frozen or misnamed.
- Associated-flat vector sources and procedure dispatch retain exact source-bit identities and checked versions.
- Rejected normalization cannot publish an output or commit a new definition, including later recoverable operations.
- Vendor references explicitly implement native defaults; paired exports cannot establish equivalence of vendor omission semantics or physical timing.

### Task 1: normalize partial mixed overrides

**Files:** Modify src/pattern/pattern_clock.cc, src/pattern/icl_pattern_generation_main.cc,
tests/src/pattern_scheduler_test.cc, tests/src/pattern/test_clock_period_overrides.py
and tests/src/test_icl_pattern_generation_main.py;
update docs/pattern-generation.md and docs/remaining-work.md.

**Interfaces:** Consumes SystemClocks::Override(const Operation&, z3::context&, Operation*)
and its existing Definition/Name/Target validation. Produces the same API, with
partial normalized non-period arguments
{target,-source,source,-freqMultiplier,m,-freqDivider,d}; already complete forms
retain their established argument order and spelling.

- [x] Add tests for all eight option subsets on ICL source ck with factors 3/2,
  using values other/7/5 when specified. Assert fully explicit output, restoration
  after a period, and five-form equality. Add procedure/vector/version tests and
  unrepresentable source plus publication guards. Expected: new positive cases fail.
- [x] Add native normalization and failed-sentinel assertions; build the native
  target against the old implementation and run --clock-snapshots. Expected: FAIL
  on the emitted source/ratio operand shape before production edits.
- [x] Normalize validated non-period Definitions only when requested; reject
  unrepresentable default sources before commit. Remove the generator's blanket
  partial-form guard. Expected: explicit complete programs remain identical.
- [x] Build normal/static generators and native snapshots, then run focused
  period/serialization/legacy clocks/frontend/readback/reset checks. Expected: PASS.
- [x] Measure the nine specified mixed workloads and ordinary controls three times
  with actual artifact capture and repeat agreement; run bounded configured vendor
  reference/generated pairs with explicit native-default references. Expected: all
  budgets, hash bindings and literal trace comparisons pass.
- [x] Add an assertion to Generator.test_help_version for native-default expansion;
  observe RED on the old help, correct only the compiled help literal, rebuild and
  observe GREEN. Expected: help matches implemented behavior.
- [x] Run full make check, affected sanitizers, C++ quality, corpus and packaging.
  Record actual counts and skips. Expected: all commands exit0 on frozen inputs.
- [x] Update documentation with actual evidence; read the finalized selected diff,
  commit using grouped messages and run task-done with
  python3 /tmp/mixed-clock-defaults-check.py. Expected: QUALIFIED with matching
  source, product, measurement and vendor identities.

## Branch completion

- [x] One fresh independent whole-branch review, all findings/declined areas graded,
  one Critical/Important test-first repair pass if required; defer Minors.
- [ ] Merge into master, verify merged inputs and focused tests, push and delete
  only codex/mixed-clock-defaults; archive only this plan's own workspace.
