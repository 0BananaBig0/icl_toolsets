# Clock Period Serialization Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Emit exact separate period operands accepted by the qualified vendor subset.

**Architecture:** Reuse shared validation; privately convert validated decimal
period text to integer/unit words only when rendering requests them. Preserve
native-only values and current clock state/routing semantics.

**Tech Stack:** Existing C++17/Tcl/Z3 and Python standard library; no dependencies.

**Spec:** docs/superpowers/specs/2026-11-27-clock-period-serialization-design.md

## Global Constraints

- Preserve all accepted native input and invalid-input/publication guards.
- No floating point, new time engine, dependencies or resource-limit increase.
- Keep ordinary no-period output byte-identical and existing execution boundaries.
- Native fallbacks retain exact metadata; do not claim vendor support for them.
- New test scripts live under tests/src/pattern/.

## Review Focus

- Separate Tcl operands, including all seven units and default ns, cannot accidentally become one braced word.
- Exact rescaling preserves decimal/scientific quantities, leading/trailing zeros and the original unit whenever an integer fits.
- Huge exponents and long coefficient cancellation cannot overflow shifts, allocate exponent-sized buffers or change native-only values.
- Procedure/reset replay, vector names and fully explicit mixed overrides preserve ordered target/version statements.
- Vendor acceptance/export parity is limited to actual versioned configured runs; unsupported differential and sub-attosecond cases stay clearly bounded.

### Task 1: Exact period words in canonical replay

**Files:** src/pattern/pattern_clock.cc; tests/src/pattern_scheduler_test.cc;
tests/src/pattern/test_clock_period_overrides.py;
create tests/src/pattern/test_clock_period_serialization.py; Makefile;
docs/pattern-generation.md; docs/remaining-work.md.

**Interfaces:** Consume pdl::NormalizeClockPeriod(value, optionalunit) unchanged.
Add private pair<string,string> PeriodWords(const string& validated_compact).
When normalized output is requested, SystemClocks::Override emits arguments
{canonical_target,-period,number,unit}; no public header or state-model change.

- [x] Add ClockPeriodSerialization tests: integer/default operands; exact Decimal
  refinement table (1.25us→1250 ns,1.0e-9s→1 ns,1.25fs→1250 as,1000.0ns→1000 ns);
  lossless native fallbacks; ordered procedure/mixed-source replay;100000-zero
  coefficient cancellation. Reuse five-form helpers. Update existing period
  literal assertions and native normalized argument expectation to four operands.
- [x] Run new focused tests and native clock snapshots on the parent products.
  Expected: FAIL on compact/decimal emission; preserve existing invalid-input tests.
  Fresh vendor compact/decimal/scientific RED and integer controls already ran.
- [x] Implement private exact digit/unit conversion and separate emitted words.
  Preserve integer spelling/unit, refine decimal/exponent units only when needed,
  and return lossless native operands below supported unit precision. Expected:
  no state/routing changes; O(literal bytes) work/space.
- [x] Build normal/static generators and native snapshot target before testing.
  Run new and existing period suites, old clocks, frontend, readback and reset
  checks. Expected: all pass; exact quantity equality and five-form replay.
- [x] Run fresh sequential vendor reference/generated pairs with matching input
  hashes and compare actual exported operations. Expected: configured-run parity
  for20 ns,default20,1250 ns,1 ns and7 as equivalents; native-only cases remain bounded.
- [x] Run full make check, native and affected CLI ASAN/UBSAN, changed-code C++
  format/tidy/C++17 checks, corpus and source packaging. Expected: exit0; disclose skips.
- [x] Measure nine short-literal matrix cases, the100000-zero five-form case and
  matched ordinary parent/current controls in repeated runs under10s/512MiB.
  Expected: exact bytes/quantities, recorded time/RSS/hashes, no speedup claim.
- [x] Record evidence and read complete finalized selected diff; commit using
  grouped messages. Run task-done with python3 /tmp/clock-period-serialization-check.py.
  Expected: QUALIFIED with final frozen input/product and vendor identities.

## Branch completion

- [x] One fresh whole-branch review; grade every finding/declined area. One
  Critical/Important test-first repair pass and green full suite; defer Minors.
- [x] Merge into recorded parent master; check merged identity and focused tests,
  push, retire only the owned branch and archive only this plan's ledger.
