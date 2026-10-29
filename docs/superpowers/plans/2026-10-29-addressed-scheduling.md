# Shared-address scheduling implementation plan

> **For agentic workers:** Use superpowers:executing-plans, TDD, one independent whole-change review and verification before publication.

**Goal:** Make the existing eight-register scan-driven bank pass with sanitizers enabled, and schedule a 64-register bank without enumerating address-write permutations.

**Architecture:** For write-only ordinary applies with at least four addressed data goals on the same physical address vector, partition normalized goals by address code and process codes deterministically. Reuse the existing temporal solver/concrete replay for each group on a speculative scheduler. Check all final goals before publishing its state; incompatible groups or dependency ordering fall back to the original bounded solver. This is a scheduling heuristic, not a second retargeter or parser.

**Tech stack:** Existing C++17, Z3, shared ICL/PDL frontend and Python/native regression.

**Spec:** User's dependency/greedy scheduling requirement and docs/pattern-generation.md (scan-driven DataRegister verification). Existing autonomous implementation and commit/push authorization applies on master.

## Constraints and review focus

- Retain two-second solver and 15-second planning attempt bounds; keep ASan/UBSan/LSan enabled.
- Do not split AccessTogether, -together, reads or end-state obligations. These use the existing solver.
- Freeze original controlling fills for partial/X goals before sub-batches; source order within an apply is not hardware order.
- Preserve reserved scan state through every speculative scan and leave live state unchanged on failure.
- Compare physical address vectors, including direction; do not group unrelated buses by width or by source object name.
- Verify all final goals after sequential groups: local resets or cascades can invalidate earlier values.
- No optimality claim outside the tested homogeneous shared-bus topology. Bound traversal/work and record measured size/time/memory.

## Task 1: Bounded address groups

- [x] Add 64-register shared-bus and reordered-input/partial-write regressions to tests/src/pattern/test_scan_data_register_scheduling.py; observe existing limit. Existing eight-register sanitizer failure is the baseline.
- [x] Add TryAddressedWrites alongside TryGreedy in src/pattern/pattern_scheduler.cc, consuming normalized goals and preserved state; call only for the scoped eligible applies. Reuse Solve with greedy recursion disabled and preserve speculative state rollback/final-goal validation.
- [x] Test incompatible -together, protected bus rollback, multi-bus fallback and source/flat/generated-PDL replay; all focused cases pass.

## Task 2: Verification and publication

- [x] Independent review, reproduce and fix important findings before publication.
- [x] Full make check, Google style, native and complete DataRegister sanitizer suites (including the eight-register case); investigate failures.
- [x] Measure equivalent eight-register runs and larger bank runs; recheck existing 21 directional/seven legacy vendor standards offline.
- [x] Update README/docs with supported scope and measurements, commit and push.

## Execution record

- Baseline a93fffe already committed/pushed; no unpublished changes at start.
- New 64-register case first failed with a scan solver limit. Address batching
  passed it and the existing eight-register leak-enabled sanitizer stress case.
- Six CLI additions (25 total); native rollback, same-address lanes and feasible
  ordering fallback retained. All source/flat/generated-PDL replay checks pass.
- Independent review: `address_batches_review`. Already-satisfied goals initially
  emitted eight scans instead of one; RED→GREEN for wholly/partly satisfied banks.
- Ruling: retain explicit solver-resource errors rather than retrying without
  bounds. Cost: a resource-limited speculative batch can report a limit without
  attempting the joint path. No reproduced correctness regression; ordinary
  bounded infeasibility does fall back. This is a documented minor limitation.
- Full regression exited 0; dynamic/static CLI25, grammar334/scanner119,
  Vim/Neovim38656 ICL/824 PDL, offline21 directional+7 legacy standards.
- ASan/UBSan/LSan: all25 CLI cases and native scheduler passed, no exclusions.
  Google formatting and changed-source naming checks passed.
- Normal eight-register CLI 0.10–0.12s (previous1.95s); 64-register CLI1.52s.
  Complete logs/measurement caveats in docs/pattern-generation.md.
- No fresh Tessent/RTL simulation/install/strict authored-coverage run.
