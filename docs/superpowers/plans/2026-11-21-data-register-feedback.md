# DataRegister feedback settling
> REQUIRED SUB-SKILL: superpowers:executing-plans.
Spec: docs/superpowers/specs/2026-11-21-data-register-feedback-design.md
Base: master ed64d30; user-delegated implementation choices remain authorized.
Global Constraints: preserve old DAG/reset behavior, budgets and rollback;
no new solver, dependency, RTL or metadata. Callbacks follow separately.

## Review Focus
- Old state anchors feedback; no arbitrary simultaneous fixed-point choice.
- Concrete and both symbolic paths share groups, limits and event/fill history.
- Intermediate ownership, failed-event rollback and capture ordering hold.
- No full netlist copy/scan per delta; measure matching controls.

### Task 1: Shared component plan and concrete settling
Files: src/pattern/pattern_data.{h,cc}; pattern_state.{h,cc};
pattern_scheduler.cc reset component extraction;
new src/utility/strong_components.h; existing native state/scheduler tests.
- [x] RED normative self hold, mutual/convergent updates and nonconvergence.
- [x] Reuse iterative components, preserve DAG/reset order and dependency closure.
- [x] Bounded simultaneous group rounds with exact events and rollback; GREEN.

### Task 2: Symbolic state updates and both scheduling paths
Files: src/pattern/pattern_symbolic.{h,cc}; pattern_scheduler.cc;
existing native symbolic/scheduler tests.
- [x] RED hot/cold cache equivalence, failed assignment rollback and feedback goals.
- [x] Transactional state-view updates and bounded transition unrolling in both
primary/mixed paths, preserving fill/activation and final stability; GREEN.

### Task 3: Intermediate ownership, CLI replay and measured scale
Files: affected state/TAP/scheduler event interfaces; native tests;
new tests/src/pattern/test_data_register_feedback.py; Makefile registration.
- [x] RED intermediate held changes, failure/retry and capture-before-update.
- [x] Scoped tentative-state checks with no stored callbacks; source/flat/
generated/reset replay and atomic negatives; GREEN.
- [x] Matched feedback/control scale with hashes, sizes, wall time and RSS.

### Task 4: Qualification and integration
Files: current guide/model/tracker plus this spec/plan.
- [ ] Full native/dynamic/static/SAN/style/C++17/package and frozen identities.
- [ ] One independent branch review and one necessary blocking fix pass.
- [ ] Record decisions/costs and exact vendor limitation; archive ledger,
merge/push/delete, then continue remaining item4 callback work.
