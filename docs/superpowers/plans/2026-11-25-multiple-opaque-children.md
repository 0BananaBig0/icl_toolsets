# Multiple opaque children implementation plan

> REQUIRED: superpowers:executing-plans, native inline execution.

**Goal:** Execute multiple private opaque child intervals on one static route.
**Architecture:** One expanded native network; per-client default/stable queues.
**Tech stack:** Existing C++17, Tcl, Z3 and Python test harness.
**Spec:** docs/superpowers/specs/2026-11-25-multiple-opaque-children-design.md
**Parent:** master800a41f8873f37224fe48af4feb29cad55f53a79.
**Owned branch:** codex/multiple-opaque-children.

## Global constraints
Keep one-megabit scan/payload caps and existing traversal/solver/event bounds,
10s/512MiB per measured process and existing256MiB wide-control bounds.
Opaque actual bits remain unknown. Source IDs/exports and transactional native
behavior stay intact. Preserve117stashes/27deferredMinors; item1 excluded.
No dependencies, RTL, public options, timing or hardware-transport invention.

## Review focus
- Repeated instances of one leaf module keep independent default/stable/length state.
- Root and local inversion/offset projections do not swap child payloads or histories.
- Failure in any child or subscriber commits no partial native/queue state.
- Every private interval stays unknown and absent from public targets, frames and exports.
- Missing, disconnected, overlapping or side-channel opaque routes never gain fabricated behavior.
- Measured child/request scaling avoids repeated whole-network scans and meets unchanged bounds.

### Task 1: Qualify multiple static opaque child intervals

**Files:** Modify src/pattern/pattern_opaque.h/.cc,
src/pattern/pattern_scheduler.cc, tests/src/pattern/pattern_opaque_test.cc.
Create tests/src/pattern/test_multiple_opaque_children.py and
tests/src/pattern/benchmark_multiple_opaque_children.py. Modify Makefile and
current guide/model/tracker docs. Add another affected existing test only
when required by a changed internal descriptor.

**Interfaces:** Keep ExpandOpaqueChild(const Netlist&) returning a shared
immutable OpaqueChildModel. Introduce OpaqueChildInterval{client,storage,
default_load}; OpaqueChildModel retains netlist/source_nodes/root and owns
intervals. Scheduler queue state is keyed by client node ID. OpaqueRawState
returns candidate per-client queues and fills a vector of completions;
ApplyScans applies all completions. Existing public scheduler signatures stay.

- [ ] Step1: Add native and CLI regressions for repeated two-child default/
  stable completion, simultaneous disjoint requests, actual unknown histories,
  resets/cancellation, parent state and private exports. Run with immutable
  parent; expected disconnected-route failure. Retain single-child controls.
- [ ] Step2: Generalize expansion with shared route/privacy validation and
  per-instance descriptors, then scheduler queue candidates and ordered raw
  interval projections. Expected focused native and source/flat/generated
  cases pass with unchanged public contracts.
- [ ] Step3: Add negative/rollback/copy/subscriber, sparse/inverted, modeled
  sibling, namespace/split/local-reset and invalid route/length cases.
  Expected no partial state/publication or private leakage, exact actual data.
- [ ] Step4: Add the measured2/8/32-child,8/64/256/1024-bit replay matrix and
  supported immutable-parent single-child controls. Check complete outputs,
  dimensions, actual histories/status, time/RSS and binary/input hashes.
  Expected unchanged10s/512MiB and existing wide-control limits.
- [ ] Step5: Run full normal/static generator,30freshnative, affected SAN CLI/
  corpus, C++ quality and packaging. Expected no regression; record exact
  source/product hashes and actual corpus classifications.
- [ ] Step6: Commit and run one fresh independent final review. Re-grade,
  fix blockers once test-first and retain deferred Minors/rulings.
  Expected no unresolved Critical/Important finding; no rereview.
- [ ] Step7: Merge/push master, verify identical qualified inputs/products on
  merged focused checks, delete owned branch, close docs and archive only this
  ledger. Expected clean pushed master and117stashes preserved.

Focused: python3 tests/src/pattern/test_multiple_opaque_children.py build/icl-pattern-generation
Completion: python3 /tmp/multiple-opaque-children-completion.py

Self-review: one shared backend extension; no independent task interfaces.
Every Review Focus maps to Step1/3/4. Small design needs no builder review.
