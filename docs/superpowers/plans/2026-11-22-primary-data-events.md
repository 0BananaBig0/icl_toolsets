# Primary DataRegister event consistency implementation plan

> REQUIRED SUB-SKILL: superpowers:executing-plans.

Goal: make primary-only DataRegister activation match actual emitted pin events.
Architecture: reuse existing enabled/any_drive predicate and concrete replay.
Tech stack: existing C++17/Z3/Tcl and Python unittest.
Spec: docs/superpowers/specs/2026-11-22-primary-data-events-design.md
Base: master2833ac9; standing authorization covers implementation/integration.

## Global Constraints
Preserve limits, actual events, immutable procedure bytes, queue/reset/history,
ownership and atomic publication. No dependencies, RTL or fabricated observations.

## Review Focus
- An event outside a dependency cone is still a real emitted input event.
- No-event, disabled/unknown enable and active reset cannot satisfy a changed goal.
- Side effects and temporary ownership violations remain constrained.
- Read-only/canceled/merged queues retain their actual event boundaries.
- Generated replay must retain physical event and abstract data fill intent.

### Task 1: Shared activation and focused proof
Files: src/pattern/pattern_scheduler.cc; tests/src/pattern_scheduler_test.cc;
tests/src/pattern/test_primary_data_events.py; Makefile.
- [x] Add native concrete-event oracle and CLI valid/invalid replay regressions.
- [x] Observe parent RED on the retained constant-source request.
- [x] Replace dependency-only touched rule with existing actual enabled event;
      retain autonomous DAG goals through the existing mixed replay mechanism.
- [x] GREEN focused/reused callers; record matched scale; commit.

### Task 2: Qualification and current documentation
Files: guide/model/tracker/spec/plan.
- [x] Run required native/dynamic/static/SAN/quality/package checks and freeze.
- [x] Update supported scope and precise evidence; author guide example check.
- [x] Commit, ledger, one fresh independent review; integrate per standing choice.

Final fix pass: share explicit DataRegister goal retention across primary and
mixed paths when no emitted controller write carries its intent. Native checks
apply only real events, then check markers against actual storage. Two additional
methods prove four mixed/four source-ordered primary RED cases, then seven-method
GREEN. Require33 fresh gates,56x913 per variant and108 scale processes. Preserve
pre-review/interrupted receipts; no second review or physical timing claim.
