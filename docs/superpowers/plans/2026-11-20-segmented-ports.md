# Disjoint vector-port implementation
> REQUIRED SUB-SKILL: superpowers:executing-plans.
Goal: preserve declared port functions and resolve legal shared vector bases.
Spec: docs/superpowers/specs/2026-11-20-segmented-ports-design.md
Global Constraints: begin after Task141 merge; one branch; no parser rewrite,
dependencies, raised limits, enum-subrange relaxation or broad item11 closure.

## Review Focus
- Every bit keeps its original function kind/properties and selection identity.
- Source and flat restoration enforce the same interval and namespace rules.
- Whole-base direction must not follow declaration order.
- Access checks cannot inherit a first fragment's type/enum.
- All new loops/allocations remain bounded and avoid per-bit full scans.

### Task 1: Typed group indexing and shared source/PDL resolution
Files: src/pattern/pattern_netlist.{h,cc}; src/pattern/pattern_values.cc;
tests/src/pattern_netlist_test.cc; tests/src/pattern_symbolic_test.cc.
- [x] RED minimal legal input/output fragments, order/orientation/parameters,
type/default/attribute retention, aliases and invalid groups.
- [x] Keep actual nodes and canonical full-fragment targets; index intervals
once and share bounded numeric-range selection before existing per-bit checks.
- [x] GREEN native netlist/symbolic; commit/task-done.

### Task 2: Bindings and lossless associated-flat identities
Files: src/pattern/pattern_netlist.cc; src/pattern/pattern_flatten.cc;
tests/src/pattern_netlist_test.cc; tests/src/pattern_flatten_test.cc.
- [x] RED whole/sliced cross-fragment InputPort binding, source/flat round trips,
metadata tampering, attributes/defaults/enum and ordering.
- [x] Distribute validated bindings over real bits; derive validated grouped
views from canonical flat identities, preserving old metadata.
- [x] GREEN native netlist/flatten and old metadata controls; commit/task-done.

### Task 3: Affected clocks/enums/resources and CLI replay/scale
Files: affected pattern_clock/logic/scheduler/netlist/access_link/tap_network callers; existing native tests;
new tests/src/pattern/test_segmented_ports.py; Makefile suite registration.
- [x] RED clock flat-name/index and partial reset-release cases; heterogeneous
enum rejection, mixed function access, histories and reservations.
- [x] Update only affected callers, including CheckModels/AddTopPorts and
AccessLink scan-input discovery; retain scalar TAP pins and forbidden enum
subranges; exercise source/flat/generated/reset replay and atomic failures.
- [x] Run representative many-fragment measurements and recorded11 corpus
cases, reporting next boundaries accurately; GREEN focused/native; commit.

### Task 4: Qualification and integration
Files: current docs/tracker plus scoped spec/plan.
- [x] Run required full native/generator dynamic/static/SAN/style/C++17/package
checks and capture exact input/product identities.
- [x] One independent final branch review; two Important blockers fixed in one pass, with post-fix native/CLI/SAN/full qualification.
- [x] Record every executor ruling/deferred Minor, update exact docs, archive
own ledger; merge/push/delete task branch while preserving unrelated worktrees.
