# Physical Flat Clock Target Plan
> REQUIRED: superpowers:executing-plans and TDD, one final independent review.
**Goal:** Accept physical clock names in native flat ICL clock checks/waits.
**Spec:** docs/superpowers/specs/2026-11-18-flat-clock-targets-design.md

## Global Constraints
Parent97b3ecc; task/flat-clock-targets. Files below only initially.
No new public API/option/schema, ordinary-target or override spelling expansion,
source/phase/event policy, dependency or RTL. Original names win collisions;
non-flat original models retain their contract. Keep raw100/90 thresholds.
Preserve prior evidence/other worktrees. New maintained tests stay under tests/.
Final whole-task verifier: python3 /tmp/flat-clock-target-verify.py.

## Review Focus
- Metadata-gated, strictly canonical hex clock lookup only after original
  declaration lookup fails; scalar/vector suffix/type validation remains shared.
- Only Check/Wait use the helper; overrides and data/resource commands retain
  behavior, generated PDL is canonical, and cached metadata follows immutable
  netlist/copy semantics with original names winning collisions.
- No repeated AST/node scan per target; native snapshots and original-spelled
  overrides preserve version, rollback and held-clock behavior.
- Vendor replay on flat input vs original, raw vs mapped naming, exact reuse
  identity, unchanged source-scope gaps and failed raw coverage stay distinct.

### Task 131: Resolve physical clock targets in native flat models
**Files:** src/pattern/pattern_clock.cc; src/pattern/pattern_clock.h;
tests/src/pattern_scheduler_test.cc; tests/src/pattern/test_system_clocks.py;
docs/pattern-generation.md; docs/pattern-model-coverage.md; this plan/spec.
**Interfaces:** private SystemClocks Target/Check/Wait and immutable state;
ResolvePdlTarget, current native --clock-snapshots and CLI helpers.

- [x] Record base/callers/Task130 replay controls; add native and maintained CLI
      boundary/replay/scale tests. Expected actual unknown-target RED.
- [x] Add the private strict decode/metadata spelling helper on Check/Wait only.
      Expected focused GREEN, unchanged original/override/data failures.
- [x] Run nine changed-C++ style/strict17 gates, native/dynamic/static/leak SAN and
      matched scale. Expected0 with frozen runtime bytes.
- [x] Full normal then fresh raw coverage, current Rocky, vendor exact-identity
      raw/mapped replay, source archive/install. Expected normal0; raw gate actual.
- [x] Qualification commits, independent whole-branch review/necessary fix pass,
      final verify/merge/delete/archive/sync/push. Expected clean parent master.

Pre-flight: no shared task interfaces; one task. Structural CodeGraph discovery
inspected ResolvePdlTarget, SystemClocks Target callers and Runtime command
qualification; hardware clock errors occur after Runtime records qualified ops.
