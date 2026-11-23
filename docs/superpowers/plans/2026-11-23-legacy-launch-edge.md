# Legacy LaunchEdge Flat Preservation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Canonically retain the already accepted ScanOutPort LaunchEdge Falling property.

**Architecture:** Add a guarded property-expression serialization branch to Declaration.
Reuse existing frontend token storage, metadata origins and canonical elaboration.

**Tech Stack:** C++17, existing ICL frontend/Make/native suites, Python3 unittest.

**Spec:** docs/superpowers/specs/2026-11-23-legacy-launch-edge-design.md

## Global Constraints

- No new accepted grammar/value, scheduler/timing semantics, dependency or product API.
- Keep existing limits, invalid-input atomicity,117 stashes and excluded coverage1.
- New maintained test script under tests/src/pattern/.
- Normal/configured leakSAN batch <=10s/512MiB, three fresh runs each.
- One fresh final independent review; one Critical/Important fix pass; defer Minors.

## Review Focus

- Nested repeated instances must retain distinct original property scopes.
- Vector/inverted/Enable sources must retain connections without coercing directions.
- Malformed programmatic properties must not silently become Falling.
- Legacy corpus models with a second unsupported construct must remain unsupported.
- Invalid value/placement must preserve previous output artifacts.

### Task 1: Preserve and qualify accepted legacy LaunchEdge

**Files:** Modify src/pattern/pattern_flatten.cc,
tests/src/pattern_flatten_test.cc, Makefile, tests/cases/model_source_acceptance.json,
docs/pattern-generation.md and docs/remaining-work.md.
Create tests/src/pattern/test_legacy_launch_edge.py.

**Interfaces:** Consumes NetlistNode.properties Expression and existing parsed token
storage scoped by the property's Expression.scope; produces unchanged std::string SerializeIcl(const Netlist&).

- [x] Add native root/vector/inverted/Enable and repeated-child canonical property
      assertions plus malformed property span/value/kind rejection; add CLI source/
      flat/generated nonempty writes/reads and invalid value/placement artifact guards.
- [x] Build/run focused native and CLI tests against unchanged serializer.
      Expected: failure specifically at flat property not implemented: LaunchEdge.
- [x] Add only the existing accepted token's guarded serialization in Declaration.
      Run flatten/netlist/state and CLI dynamic/static. Expected: all pass.
- [x] Run actual maintained corpus, promote only the proven canonical identity
      and change the four SIB models to their PortGroup boundary; register new CLI suite in both
      generator variants. Expected:1017canonical/55unsupported/691original errors.
- [x] Run full14native/14leakSAN, both full generator variants, selected normal/SAN
      parser/model corpus+8producer/13protocol/3Make, C++ quality and source packaging.
      Expected: every gate exit0, no skipped required suite; record exact source/tool hashes.
- [x] Measure three normal/three configuredSAN fresh audit runs <=10s/512MiB.
      Expected:6 valid complete reports, exact environment/command/hashes and RSS.
- [x] Document behavior/evidence/physical-timing limit. Commit; one fresh independent
      final review; test-first fix Critical/Important once, defer Minors, no rereview.
- [x] Merge into master, verify matching inputs/products and focused integrated checks,
      push, detach preserved worktree, delete merged branch, archive own ledger.
      Completion command: python3 /tmp/legacy-launch-edge-completion-check.py.
      Expected: qualified/integrated exact evidence, clean pushed master, only master,
     117 preserved stashes; continue inventory afterward.
