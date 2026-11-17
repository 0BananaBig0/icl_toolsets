# Oscillator Cached Waits Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Accept measured cached-wait patterns while retaining fresh-check failures.

**Architecture:** Narrow existing Override guards; reuse missing-source and
versioned checked-route handling. Extend existing tests and preserve protection.

**Tech Stack:** C++17, existing Python unittest/native C++/Make/gcov/Clang SAN,
Tessent2026.2 reused driver and Rocky GCC15 qualification.

**Spec:** docs/superpowers/specs/2026-11-17-oscillator-cached-waits-design.md

## Global Constraints

Parent e2d6bdc, task/oscillator-cached-waits. Seven files, no new API/header/state
framework/dependency/product target. Existing test script placement retained.
No optionless Period restoration, intrinsic frequency, self-reference or event
semantics. Preserve failure atomicity, model/hardware and100%line/90%rawbranch gate.

## Review Focus

- Old checked route/ratio survives missing current source and failed fresh check.
- Invalid factor/source updates preserve current and checked state atomically.
- Explicit recovery and untouched vector bits preserve original metadata.
- Generated/reset-only replay does not lose a needed clock-model effect.
- Reservations/cycles/unsafe fanout remain protected; no general frequency claim.

### Task 123: Keep earlier checked oscillator waits usable

**Files:** src/pattern/pattern_clock.cc; tests/src/pattern/test_system_clocks.py;
tests/src/pattern_scheduler_test.cc; docs/pattern-generation.md;
docs/pattern-model-coverage.md; this plan and its spec only.

**Interfaces:** SystemClocks::{Override,Check,Wait}/DefinitionAt unchanged;
existing SystemClocks.replay/rejected and native oscillator fixture;
new test-only --clock-snapshots selector reuses two existing unit functions.

- [x] Record base/brief, source/callers and Task122 vendor positives/nativeRED.
      Add two CLI positives, updated fresh-check negatives and native snapshot
      cases/selector first. Expected: old tool/native fail explicit-source guard.
- [x] Narrow only two Override guards: Period requires an option, non-Period
      missing source still rejects. Expected: focused CLI/nativeGREEN, recovery,
      rollback/metadata and five replay forms preserved; no new graph state.
- [x] Format/tidy changed C++ sections and strict C++17. Run full changed clock
      suite dynamic/static, native default and focused leak-enabled native/clock/
      main checks. Expected: all0; existing4096/1024 and65536 cases within limits.
- [x] Freeze seven selected files/3994 hashes and preserve old coverage artifacts.
      Run complete normal integration, then fresh-profile full raw coverage,
      serial local heavy work. Expected: normal0; coverage reports real strict
      result without exclusions/threshold changes or filtered throw edges.
- [x] Qualify current inputs/new dynamic/transferred-static generators/native
      checks on Rocky; verify two cached exports against vendor and normalized
      forms through unchanged driver. Expected: exact input/artifact hashes,
      tests0, matching scoped traces; no unclaimed formal certificate.
- [ ] One independent final review and necessary fix pass; document actual gates
      and remaining gaps, candidate/final verified bytes, commit/merge master,
      delete branch/task-done/archive/sync. Expected: scoped feature complete,
      broad strict/model/vendor requirements still open. No push.

Pre-flight: shared checked-route version contract is produced/consumed inside
SystemClocks, so tests explicitly pin preservation and failed Check rollback.
No changed implementation interface between tasks. Small existing-state repair;
design/plan self-review complete, routine choices delegated by standing user.

### Oscillator cached waits (2026-11-17)

Factor-only overrides of a Period ToClockPort now preserve waits using an
earlier iClock check, including a check that previously selected an external
source and ratio. This closes the two native gaps measured in the omission
audit. The latest omitted ICL source may be missing; a fresh iClock still fails
on that inactive route, matching the measured Tessent2026.2 patterns. Optionless
Period overrides stay rejected, and unconnected non-Period targets still need
a source. No intrinsic frequency, self-reference, absolute-time or event model.

The change reuses existing versioned definitions and checked routes, with no
new API/header/cache/state representation. Native source/flat cases prove saved
ratios survive a failed fresh check, invalid source/factor updates publish no
partial change, an explicit source recovers, other vector bits stay independent,
and original period metadata/hardware remain unchanged. Two new CLI methods
cover multiplier/divider variants through five replay forms; existing fresh
failure checks preserve all previous output artifacts. Native --clock-snapshots
selects existing clock tests for focused sanitizers; default execution retains
the full tests. Actual CLI/native RED failures precede the two-guard repair.

Dynamic/static clock suites each pass53 methods; leak-enabled ASan/UBSan passes
the native snapshot selector, all53 clock and71 main CLI methods. Changed C++
format/tidy and strict C++17 syntax pass. The4096-stage/1024-wait and65536-bit
oscillator workloads remain within the existing45s limit; measurements are in
the clock logs. History remains one record per override, with the existing ordered-map route
lookups and binary search within each bit's history. No product speedup claim.

Complete normal integration passes in 1871.732309s. The fresh
complete raw coverage recipe takes 1389.920953s, with all58
Python groups/942 cases (51 generator groups/780 cases), native checks,1916
regression and2180 conformance cases passing. Strict coverage still fails:
17,333/17,963 lines (96.5%) and 29,260/46,667 raw branches (62.7%), against unchanged100%/90% thresholds. Make exits2 only at the
final report. All54 authored reports and raw compiler exception/cleanup edges
are retained; untaken edges include 12,230 compiler-marked
throw edges and 5,177 other edges. No filtering or waiver.
Optional ANTLR checks in normal integration remain skipped when unavailable.

Rocky qualifies the current3994 inputs, updated native tests and both generators:
51 suites/780 cases pass per dynamic/transferred-static variant. Dynamic builds
from current source; the transferred static generator matches the Root hash and
has no ELF interpreter. Only the two generator products change; other eight
product bytes remain unchanged on each platform. This is generator qualification,
not a fresh five-tool install/source-archive refresh.

Both new normalized programs pass fresh Tessent2026.2 retargeting with the
unchanged driver, and vendor exports replay successfully through the native
tool. Their matched wait traces are child4/child4 and child4/root12/root12.
These scoped probes add no formal directional certificate or general oscillator
equivalence claim. Evidence is build/oscillator-cached-20261117/: normal/SAN and
raw coverage logs/profiles,54 reports,3994 input hashes,19 instrumented binary
fingerprints and product fingerprints, remote and vendor data, comparison and artifact hashes. Only clock
source and two existing tests differ from the qualified audit runtime inputs.
Independent final review and integration are pending. Strict and broader model/
vendor requirements remain open. No push.
