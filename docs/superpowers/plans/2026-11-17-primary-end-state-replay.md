# Primary End-State Replay Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Stable replay for unknown-initial primary end-state bits.

**Architecture:** Extend per-frame representability proof; reuse known override
emission and concrete hardware replay.

**Tech Stack:** Existing C++17/Z3/Python; no new dependency.

**Spec:** docs/superpowers/specs/2026-11-17-primary-end-state-replay-design.md

## Global Constraints

Parent master85961e9, task/primary-end-state-replay. No new API/engine, output
repair loop, fake iState, changed override lifetime or raised guard. Only declared
end-state bits gain a knownness requirement on emitted frames. User delegates
routine choices/execution; retain full/SAN/remote/independent review.

## Review Focus

- Unrelated unknown bits retain existing behavior.
- Indexed/inverted aliases use resolved source masks.
- Known nondefault values remain explicit on every emitted apply.
- Read-only IR frames cannot acquire initialization on replay.
- Protected controls and failed schedules preserve state/output atomicity.

### Task 116: Represent unknown-initial primary end-state bits

**Files:** Modify src/pattern/pattern_scheduler.cc,
tests/src/pattern/test_native_tap_ir.py, tests/src/pattern/test_apply_end_state.py,
and generator/coverage docs; reuse existing runners.

**Interfaces:** Add private ConstrainPrimaryEndStates(z3::optimize&, SymbolicNetwork&,
const z3::expr& emitted, const z3::expr& event). Call from ConstrainEndStateFrames and the primary-only
frame loop, using SymbolicValue::Literal(defaults).known mask under emitted
implication. Keep scan proof unchanged. Supported shared-address RED confirms
both paths are affected.

- [x] Record task-start/parent; read brief, reuse exact Task115 full baseline.
      Trace EndGoals/ConstrainEndStateFrames/AppendDataEndOverrides callers.
- [x] Remove marker iState in native IR test and run saved binary: FAIL extra
      marker0 apply. Add default1/indexed/known-override/read-only guards. Inspect
      supported primary-only multi-frame fixture before editing product code.
- [x] Extend existing emitted-frame knownness constraint. Focused new tests
      PASS exact four-view replay; existing end-state/mixed/reset/reservation/
      data cases and1024-bit test pass.
- [x] Format/tidy/native; one independent final review/necessary fix pass.
      Freeze files; fresh full/SAN/remote,77 certificates and relevant98 native
      measurements. Verify actual input/artifact hashes, record limits.
- [x] Document actual evidence; commit exact qualified files, merge master,
      delete branch; task-done/final verifier/archive; no push.

Review fix pass: a known nondefault primary on a comparison-only frame is also
unrepresented. Shared helper requires an actual proved input event or matching
default on emitted frames. Existing override emitter stays unchanged; never add
an unproved physical drive after replay. Addressed read-a/write-b with initial
marker1 reproduces extra marker0 on generated replay before this fix.
