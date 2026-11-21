# Invocation-sensitive caller reset repair
> REQUIRED SUB-SKILL: superpowers:executing-plans.
Spec: docs/superpowers/specs/2026-11-21-occurrence-reset-rewrite-design.md
Base: master5defefe. Existing user-delegated design/implementation choices apply.
Global Constraints: preserve procedure bytes, cheaper rewrites, ownership and
atomic publication; no new dependency, raised budget or excluded coverage work.

## Review Focus
- Count empty/caught-error invocations without reevaluating argument words.
- Preserve source file boundaries, prefixes, final PDL level and cleanup.
- No user namespace clobber, repeated source scans or duplicated output records.
- Full exact-operation/strict replay validation precedes publication.

### Task 1: Recorded invocation fallback
Files: src/pattern/icl_pattern_generation_main.cc;
src/pattern/pattern_runtime.{h,cc};
tests/src/pattern/test_argument_reset_sources.py; tests/src/pattern_runtime_test.cc.
- [x] RED existing identical-argument write/call case and final-level control.
- [x] Capture actual preambles; intern records/sequences and use guarded private
state in existing argument wrappers; retain cheaper fallback order.
- [x] GREEN focused/native; commit/task-done.

### Task 2: Boundaries, replay and scale
Files: existing argument/compound reset tests and scoped implementation fixes.
- [ ] Cover empty/caught calls, side effects, prefixes/vectors, multi-file EOF,
namespace interference/cleanup, unexpected arguments/count and immutable bodies.
- [ ] Source/flat/generated/reset and idempotence; retain impossible negatives.
- [ ] Measure repeated caller scale and bounded output growth; commit/task-done.

### Task 3: Qualification and integration
Files: current guide/tracker plus this spec/plan.
- [ ] Full native/dynamic/static/SAN/style/C++17/package checks and identities.
- [ ] One independent whole-branch review and one necessary blocking fix pass.
- [ ] Record decisions/costs, archive own ledger; merge/push/delete task branch.
