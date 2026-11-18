# Invariant-model absolute-time wait plan
> REQUIRED: superpowers:executing-plans and TDD; one final independent review.
**Goal:** Preserve absolute-time minimum delays on models without clock-driven state.
**Spec:** docs/superpowers/specs/2026-11-18-invariant-time-waits-design.md

## Global Constraints
Parent d7d6d7f; task/invariant-time-waits. No invented -time/-sck combination,
tester period, floating conversion, metadata version, option, dependency or RTL.
No stateful/opaque/event timing support claim or resource ownership expansion.
Keep raw100/90 thresholds and other workspaces/stashes/evidence.
Final verifier: python3 /tmp/invariant-time-verify.py.

## Review Focus
- Shared frontend validates concrete substituted time values; emission preserves
  exact text and ordinary iRunLoop queue/rewrite/bookkeeping behavior.
- Immutable eligibility excludes storage, opaque hardware, clock-driven data
  and forwarded test clocks;
  every call checks held clocks, including zero, copied caches and invalidation.
- Original clock snapshots/overrides and cycle-count/opaque/merge paths retain
  their contracts. No guessed periods, floating loss or per-time graph scans.
- Vendor control/trace/replays, source/platform/archive identities and failed raw
  coverage are reported separately from broader timing/release acceptance.

### Task 132: Absolute time on invariant models
**Files:** src/pattern/pattern_clock.cc/.h;
src/pattern/icl_pattern_generation_main.cc;
tests/src/pattern_scheduler_test.cc; tests/src/pattern/test_system_clocks.py;
README.md; docs/pattern-generation.md; docs/pattern-model-coverage.md; this plan/spec.
**Interfaces:** new internal SystemClocks CheckTimeWait(preserved); existing
BuildSafety/Invalidate, Runtime::Validate, Generate iRunLoop, maintained helpers.

- [x] Record normative/vendor/base/caller controls and add CLI acceptance and
      boundary/scale tests; expected actual unsupported-time RED.
- [x] Implement cached eligibility and per-call held-clock validation, retain
      exact delay in Generate/help; add native guards/copy/snapshot checks.
      Include forwarded-TCK/data-pin hold regression; expected native/dynamic/
      static GREEN and unchanged unrelated rejection boundaries.
- [x] Run12 style/strict17 gates, SAN and measured scale with frozen inputs.
      Expected0 and preserved exact literal/canonical replays.
- [x] Full normal, fresh raw strict coverage, current Rocky, vendor control hash/
      complete trace/export replay, archive/install. Expected normal0, raw actual.
- [ ] Qualification/task-done, one fresh final review/fix pass, final verification,
      merge/delete/archive/sync/push. Expected clean synchronized master.

Pre-flight: no shared task interfaces; one task. CodeGraph structural discovery
covered clock model/period, shared IsTime/frontend validation and serializer;
targeted current reads covered Runtime concrete revalidation, Generate branches,
model kinds and opaque entry. No production edits before CLI RED.
