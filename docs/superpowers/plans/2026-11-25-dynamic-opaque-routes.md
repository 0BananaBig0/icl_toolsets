# Dynamic opaque scalar routes implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking.

**Goal:** Execute fixed opaque leaves behind modeled scalar bypass muxes.
**Architecture:** Reuse private opaque storage and native event/route execution;
validate all structural mux arms and complete only pre-CSU active opaque queues.
**Tech Stack:** C++17, Tcl, Python unittest, existing Z3 and sanitizer products.
**Spec:** docs/superpowers/specs/2026-11-25-dynamic-opaque-routes-design.md

## Global Constraints

- Original items2–15 except1; no new dependency, RTL or fabricated observations.
- Parentmaster4f1977d; reused .worktrees/bsdl-defaults; preserve117stashes.
- Fixed opaque leaves, binary scalar modeled bypasses, explicitly opened routes.
- Aggregate1048576bits;10s/512MiB bounds, single-wide controls256MiB.
- One inline implementer, one fresh final review, one TDD blocker-fix wave.
- Preserve31deferred Minors and the prior unsupported-mode contracts.

## Review Focus

Inactive leaves must not advance queues or invent histories during root opening.
Closure uses pre-CSU geometry even when the next route is shorter.
Mux data forwarding must not let private payload reach selectors or controls.
Shared/merged graph arms must terminate correctly without hiding real cycles.
Failed execution and copied schedulers must preserve independent queue state.

### Task 1: Dynamic opaque scalar execution

**Files:** Modify src/pattern/pattern_opaque.cc, pattern_scheduler.cc and their
headers only as required; maintain existing native opaque/readback tests and
Makefile. Create tests/src/pattern/test_dynamic_opaque_routes.py,
tests/src/pattern/benchmark_dynamic_opaque_routes.py and the mirrored reporter
tests/tests/src/pattern/test_benchmark_dynamic_opaque_routes.py. Reuse existing
benchmark helpers where they fit; maintain three guide/inventory documents.
**Interfaces:** Keep ExpandOpaqueChild(const Netlist&) and opaque queue/history
public signatures. Extend internal structural validation and existing
OpaqueRawState pre-CSU interval/completion selection. Runtime readback and
serialization continue consuming existing opaque metadata and actual callbacks.

- [x] Step1: Add parent RED cases. One-bit root SI1 opens an eight-bit opaque
  leaf; leaf scan returns8unknown bits and uncared-zero mask/status0; root9SI0
  closes it while preserving its last history. Add two independently selected
  leaves. Expected parent root-opening rejection, not syntax/fixture errors.
- [x] Step2: Implement bounded iterative mux-route validation and private-output
  consumer checks; filter projected/completed queues by actual pre-CSU storage.
  Expected focused GREEN with exact physical words and no extra CSU.
- [x] Step3: Pin unknown care/status, closure/reopening, absent/old histories,
  aliases/inversion, inactive queues, cycles/dangling arms, payload leaks,
  cancellation/copy/rollback, default/flat/generated/local-reset continuation
  and ordinary static compatibility. Add reporter/workload checks test-first.
  Expected correct actual observations and truthful matched-control records.
- [x] Step4: Run complete normal/static generators,30native normal/SAN suites,
  affected sanitizer CLI, corpus, changed-C++quality and packaging. Validate
  all prior/new matrices and exact qualified input/product hashes. Expected all
  pass with no changed corpus class unless a valid new case is explicitly ruled.
- [x] Step5: Commit finalized selected diff and obtain one fresh whole-branch
  review. Re-grade, fix blockers once test-first, qualify final state and record
  all rulings/Minors. Expected0unresolved Critical/Important without rereview.
- [ ] Step6: Merge/push master, verify merged hashes/checks, detach reused tree,
  delete only owned branch and archive own ledger. Expected clean pushed master,
  preserved117stashes and accurate broad remaining inventory.

Completion command: python3 /tmp/dynamic-opaque-routes-completion.py
Self-review:one shared expansion/route/queue flow; no independent task interfaces.
