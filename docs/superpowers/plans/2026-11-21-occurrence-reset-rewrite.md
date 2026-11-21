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
- [x] Cover empty/caught calls, side effects, prefixes/vectors, multi-file EOF,
namespace interference/cleanup, unexpected arguments/count and immutable bodies.
- [x] Source/flat/generated/reset and idempotence; retain impossible negatives.
- [x] Measure repeated caller scale and bounded output growth; commit/task-done.

### Task 3: Qualification and integration
Files: current guide/tracker plus this spec/plan.
- [x] Full native/dynamic/static/SAN/style/C++17/package checks and identities;13 native suites reused by exact unchanged input/product identity.
- [x] One independent whole-branch review; one Critical/four Important findings fixed in one pass, with fresh affected qualification.
- [x] Record decisions/costs, archive own ledger; merge/push/delete task branch.

Task2 pre-review measurements (single local runs; no speedup claim): equivalent plain
parent/current outputs match at64/256/512 calls. Parent0.229/0.779/1.446s versus
current0.224/0.769/1.470s. Invocation repair0.626/2.375/4.437s and71040/72176/
74528KiB peak RSS; rewritten source1443/1829/2341 bytes. Existing runtime budgets
remain unchanged. Exact commands, environment, source/product hashes and all
nine measurements: build/occurrence-reset-20261121/scale/results.json.

Final post-fix measurements: matched parent/current/repair outputs at64/256/512
calls; repair0.628/2.313/4.571s,61120/69948/71276KiB,2212/2982/4006 rewritten bytes.
Exact immutable-table checks add one bounded expected copy per site, retaining
linear sequence growth. Fresh vendor pair and all eight physical forms match.
Fifteen affected qualification/identity steps and12 code-quality gates pass;
normal native13 reuse has3992 matching inputs and13 unchanged products.
