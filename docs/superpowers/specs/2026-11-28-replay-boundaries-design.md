# Replay boundary regressions (Task204)

After Task203, 35 findings remain deferred. This test-only batch addresses four:
Task203's missing whole-event check, Task180's all-X inverted comparison,
Task193's local-reset fixture without a real prerequisite and Task197's missing
mixed lowered/retained encoded-flat clock case. Existing production contracts
and all 44 executable identities must remain unchanged.

Strengthen only the corresponding maintained Python tests:

- Require two standalone apply lines in the fanout scale output before checking
  its first-group write multiset. Preserve the existing nine-coordinate grid and
  10-second bound for four replay forms. A control removing the entire first
  write/apply event must pass the old assertion and fail the strengthened one.
- Exercise an aliased, inverted opaque-child route with a mixed known/X expected
  word. Check the physical comparison's inverted known bits and unchanged X mask.
  Actual child data remains unknown; unknown cared status must stay unknown and
  cannot be cleared into a known value. Retain source/flat/generated history.
- Reuse the maintained data-reset fixture and require a real release prerequisite
  before a procedure whose clock check precedes the first period override.
  Inspect the rewritten local-reset file, preserve the procedure bytes and
  ordering, and replay the source, flat, generated and rewritten programs.
- Use two distinct vector bits: one with a period override and complete retained
  source update, the other with an independent mux-source partial override that
  lowers to its root. Supply explicitly encoded associated-flat target/source
  names and assert the complete canonical clock statement sequence across the
  existing five-form helper.

Use protected current generators to verify each fixture before maintained edits.
The missing historical Task193 temporary probes are not needed: current tracked
fixtures, retained review descriptions and actual source decide the expectations.
Keep temporary fault controls and their exact inputs/outputs/driver bytes in
the evidence bundle. They demonstrate oracle coverage, not production defects.

This changes no producer, API, dependency, hardware eligibility, reset/clock
semantics, time/solver limit or vendor certificate. Reuse existing helpers and
do not add per-lane subprocesses or new general parsing infrastructure. Fixtures
are small fixed networks; the existing fanout grid remains the largest new
assertion workload. Expected-word and event-count work is linear in output size.

Require full affected normal/static modules, selected new functional cases under
default leak-enabled instrumentation, source packaging and exact source/product
identity checks. All 4133 source/test paths remain, with exactly three Python
test files differing and 4130 other inputs plus all 44 products unchanged.
Do not repeat unrelated full regression without a relevant failure or dependency.
No new performance, vendor or live-device claim is made.

Reuse the completion program, Task185's standing design-question opt-out and
inline authorization for existing-contract test maintenance. Use one independent
whole-branch review and at most one Critical/Important repair wave. Resolve the
four inherited findings only after their actual checks pass; the candidate
remainder is 35 minus 4 = 31. Preserve 117 stashes, sibling ledgers and excluded
coverage item1. Merge, verify, push and retire this owned branch before continuing.
