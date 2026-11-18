# Typed Flat Clock Forwarding Plan
> REQUIRED: superpowers:executing-plans; TDD and one final independent review.
**Goal:** Serialize hierarchical clock inputs using output-clock declarations,
preserving native originals and accepting older flat files.
**Spec:** docs/superpowers/specs/2026-11-18-flat-differential-clocks-design.md

## Global Constraints
Parent6d7ed76; task/flat-differential-clocks. Only files below initially.
No Source-scope widening, metadata version/public API/dependency/RTL changes.
Keep100% line/90% raw-branch thresholds; record failed gates honestly.
Current Source3994 input names remain frozen during qualification; no maintained
test scripts outside tests/. Preserve Task128/129 evidence and recovery work.
Final whole-task verifier: python3 /tmp/flat-clock-verify.py.

## Review Focus
- New representation must preserve root ports and every other family, with Source
  for positive outputs and DifferentialInvOf only for negatives.
- Restore new and legacy encodings without allowing an original ClockPort to
  acquire output-only frequency/Period semantics or bypass old scope/type guards.
- Check aliases, vectors, holes, nested connections, metadata original physical
  negative bindings and old-to-new canonical migration across runtime replays.
- Distinguish actual vendor source/flat stages, encoded port names, mapped trace
  comparison, raw export replay, reused identity evidence and strict coverage.

### Task 130: Correct flat differential clock declaration kinds
**Files:** src/pattern/pattern_flatten.cc; tests/src/pattern_flatten_test.cc;
tests/src/pattern/test_system_clocks.py; docs/pattern-generation.md;
docs/pattern-model-coverage.md; this plan and its spec.
**Interfaces:** private Declaration, RestoreFlatOrigins and existing metadata;
existing flatten native runner and SystemClocks replay/rejected helpers.

- [x] Preserve parent/prototypes/callers and add declaration/original/vector/
      nested/unconnected/legacy tests plus CLI replay first.
      Expected actual RED at old DataOutPort/ClockPort encoding.
- [x] Share the private encoding selection and accept legacy loader types.
      Expected new declaration tests GREEN and forged factor/Period test RED.
- [x] Reject output-only properties when restoring original clock inputs.
      Expected all focused tests GREEN; genuine output factors/oscillators intact.
- [x] Run six C++ style/strict gates, native/dynamic/static/leak SAN and scale.
      Expected0 with correct current byte/product identity.
- [x] Full normal then fresh raw coverage, Rocky/current suites, vendor directions
      and archive/install qualification. Expected normal0; report actual raw gate.
- [x] Record qualification/candidate, whole-branch review, necessary fix pass,
      final verify/merge/delete/archive/sync/push. Expected clean synchronized
      parent master; explicitly retain broader acceptance gaps.

Pre-flight: no shared task interfaces; one task. Exact Declaration/SerializeIcl
and RestoreFlatOrigins caller in Elaborator::Run inspected via CodeGraph then
targeted current-source fallback where the index elided the loader body.
