# Shared-edge TAP replay implementation plan

> Execute inline with superpowers:executing-plans under the existing authorization.

**Goal:** Make concrete replay of TAPs sharing TCK independent of AccessLink names
when one instrument uses another instrument's explicit control output.

**Architecture:** Retain TapExecution and its single NetworkState. Stage all
controller actions for the observed edge before publishing their control outputs,
then evaluate register controls against the complete set of outputs. Preserve
old serial storage sampling, one joint register clock event, and subsequent IR
synchronization/reset settlement. Do not partition hardware state by TAP.

**Tech Stack:** Existing C++17 native tests; no dependencies.

**Spec:** [Approved pattern-generation design](../specs/2026-10-25-icl-pattern-generation-design.md),
shared state/event requirements and the remaining multi-TAP work.

## Scope and alternatives

The existing Edge loop advances one controller, publishes its controls and
evaluates its registers before advancing the next controller. An explicit
cross-interface control can therefore see a mixture of old and current actions.
Reordering TAP names is not a solution; separate per-TAP state loses actual
connections. Staging actions and then evaluating shared controls fits the existing
model. Confirm the defect with a legal source fixture before changing product code.

This is the concrete replay prerequisite, not multi-TAP scheduling completion.
Keep ambiguous shared-register ownership and the scheduler's multi-TAP rejection
until their respective scheduling/event contracts are implemented. Preserve
single-TAP behavior, unknown values, existing reset ordering and flat metadata.

Files: src/pattern/pattern_tap_execution.cc; existing
tests/src/pattern_state_test.cc; generator guide/model ledger and this plan.
Worktree: .worktrees/tap-edge-snapshot, base473bc9e.

## Implementation and checks

- [x] Add a source/flat native regression with two BSDL interfaces sharing TCK,
  distinct TMS/TDI/TRST/TDO and disjoint scan registers. Connect a child control
  output to the other child's control input. Assert capture/shift/update state,
  old serial sampling and invariance when AccessLink lexical order is reversed.
  Run the test against unchanged product objects and retain the failing result.
- [x] Stage TapActions before mutating shared control pins; publish all controls
  before Controls calls. Retain one NetworkState::Clock call per observed edge.
  Confirm controller inputs cannot see already-published same-edge actions.
- [x] Run native state/TAP/flatten/scheduler tests, affected CLI checks and
  leak-enabled sanitizers; inspect single-TAP callers and unchanged guards.
  Measure representative multi-TAP replay and existing large native cases.
- [x] Obtain independent review, run required full dynamic/static regression and
  style checks, update results/limits, and commit the verified change.

Review focus: lexical-order dependence, stale deasserted controls, unknown
controller inputs, cross-chain old-state sampling and flat round-trip identity.
Work remains O(TAPs + evaluated control references + existing state work) per
edge, with O(TAPs) additional actions. No speed improvement is claimed.

Baseline native state tests pass at473bc9e. Task46 separately qualifies that
unchanged revision on Rocky8; its evidence must not claim this later change.

Independent review confirms the source connection under IEEE1687 §6.4.7(k),
§6.3.12(g) and §5.12(g). Shared TCK semantics follow §6.4.6.17(a) and §5.14.4(c,e).
The native test failed against unchanged product objects (tap-edge-red-native,
exit1), specifically when the producer sorts after the receiver. Six new variants
now pass: forwarded capture/shift/update, each in both lexical orders, with
original/flat equality. Product review has no findings. Existing native cases
retain unknown-controller and old-serial-storage checks; this change does not
claim arbitrary TMS/TDI wiring or global reset ownership for multiple TAPs.

Focused native evidence: build/tap-edge-focused-native.log/.exit(0), 0.12s wall,
23244KiB maximum RSS on local Linux/GCC; 1000 shared idle cycles on two TAPs in
both source/flat forms take0.0339409s. Existing1024-register data and1000-interface
TAP construction cases pass. Formatting and changed-source naming pass. Root
integration and leak-enabled checks pass. Final full evidence is
build/tap-edge-confirmed-full.log/.exit(0): 39 dynamic/static suites,
51 directional/seven legacy goldens, 2519 ICL files covering 334/119 rules,
and six editor tests (177.401s); full wall time 937.498s. Five optional ANTLR
tests were skipped. The earlier detached full run has no completion status;
its log and .incomplete note are preserved and are not counted as a pass.

All build/tap-edge-final-sanitizer-*.exit records are 0: native state/TAP/
scheduler/flatten, main33, TAP replay9 and hidden-scan-event4 CLI methods.
ASan/UBSan and leak detection remained enabled. The sanitized two-TAP idle
measurement is 0.578975s. Existing 65537-bit IR/262151-edge and 10000-opcode
cases pass. Local compiler: GCC16.2.1 20261029 (Red Hat16.2.1-3).
Strict coverage was not rerun; Task45's unmet gate remains open. Rocky8
qualification and vendor certificates do not cover this later change.
