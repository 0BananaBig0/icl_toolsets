# Runtime, clock-name and compatible child ownership implementation
> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans.
**Goal:** Repair three concrete audited boundaries in items5–7.
**Spec:** docs/superpowers/specs/2026-11-20-runtime-clock-ownership-design.md
**Global Constraints:** Initial parent master35c927f; separate BSDL increment must
integrate before final combined qualification. No full item5/6/7 completion,
hardware/time guesses, new parser/dependency or excluded coverage1.
## Review Focus
- Encoded clock names must preserve literal collisions and checked snapshots.
- Never-read x must not hide unresolved actual reads after apply, reset or files.
- Initial status must never manufacture comparison outcomes or clear unknowns.
- Failed apply validation preserves histories/status; invalid CLI operands still
  reject with their original cause and preserve published artifacts.
- Child port locks stay enforced on every edge and afterward; root exemptions
  and register assignment refresh must remain unchanged.
### Task 1: Reuse associated-flat override target/source mapping
**Files:** src/pattern/pattern_clock.cc; tests/src/pattern/test_system_clocks.py;
tests/src/pattern_scheduler_test.cc as necessary.
**Interfaces:** unchanged SystemClocks API.
- [x] Add source/flat/generated/reset replay, source+target mappings, snapshots,
  collision and invalid-target tests; Expected RED on old mapping.
- [x] Reuse ClockTargetName at both override lookups; Expected GREEN with all
  existing iClock/Wait and override validation.
- [x] Commit and task-done after focused verification.
**Verification:** python3 tests/src/pattern/test_system_clocks.py build/icl-pattern-generation
### Task 2: Distinguish never-read history and unresolved observations
**Files:** src/pattern/pattern_runtime.cc; tests/src/pattern_runtime_test.cc;
tests/src/pattern/test_observation_state.py.
**Interfaces:** existing runtime getters only; no adapter/device guesses.
- [x] Add never-read/partial/supplied/invalidated aliases and initial status/
  clear/read/no-compare/session tests; Expected RED on current rejection.
- [x] Keep sparse x defaults separately from unavailable applied reads; preserve
  explicit histories and reject unknown status. Expected GREEN.
- [x] Commit and task-done after focused and native runtime verification.
**Verification:** python3 tests/src/pattern/test_observation_state.py build/icl-pattern-generation
### Task 3: Enforce compatible child port reservations during raw replay
**Files:** src/pattern/pattern_scheduler.cc; existing modeled-child/native tests;
current docs/tracker.
**Interfaces:** unchanged raw replay and reservation APIs.
- [x] Add compatible child selection and conflicting selection/clock/output,
  ordinary follow-up, scoped ownership and atomic failure tests. Expected RED
  on compatible child case.
- [x] Let child ports use existing per-edge/final checks; retain root exemption
  and storage rules. Expected GREEN.
- [x] Run combined native/full generator/dynamic/static/SAN/style/C++17/package,
  update exact tracker evidence without broad closure, commit/task-done, fresh
  independent review, one necessary fix pass, merge/push/delete branch.
**Verification:** python3 tests/src/pattern/test_automatic_child_raw.py build/icl-pattern-generation
