# Pattern generation completion implementation plan

> **For agentic workers:** Use superpowers:executing-plans; implement and verify each task inline. Retain unfinished tasks as acceptance requirements.

**Goal:** Close the remaining pattern-generation requirements without weakening correctness, resource limits or regression checks.

**Architecture:** Extend the shared elaborated netlist, concrete state, symbolic state and existing scheduler. Keep source-preserving reset rewriting separate from normal flattened output. No additional parser or independent retargeter.

**Tech Stack:** C++17, Tcl, Z3, Python unittest, GNU Make.

**Spec:** [Approved design](../specs/2026-10-25-icl-pattern-generation-design.md), including subsequent user instructions permitting bounded greedy scheduling and Tessent behavior probes.

## Constraints and review focus

- Existing autonomous inline implementation authorization applies. No unrelated file changes or generated build products in Git.
- Google style; new test scripts mirror source paths under tests/. Keep ASan/UBSan/LSan enabled.
- Unknown state must not become known by using expected read data. Contending or unknown enables must not produce a valid scan route.
- Shared storage, resets and controls must be replayed together; merely removing an unsupported-case guard is insufficient.
- Each newly supported case must preserve flat-ICL replay and generated-PDL idempotency. Failure must preserve previous outputs.
- Bound traversal and solver work; assess representative larger networks before claiming a performance improvement.
- Immutable procedure bodies can make some caller-only reset repairs impossible. Diagnose such cases rather than changing the requirement silently.

## Task 1: One-hot serial selection

Files: pattern_data.h/.cc, pattern_state.cc, pattern_symbolic.cc, pattern_scheduler.cc, pattern_tap_execution.cc under src/pattern; existing native state/symbolic tests; new tests/src/pattern/test_one_hot_scan_scheduling.py and classified fixtures; Makefile. Update CLI help and current support documentation after verification.

- [x] Add failing native truth-table and CLI tests for single, nested, inverted, conflicting and unknown contributors, source/flat/generated round trips.
- [x] Reuse OneHotReadResolver for scan-output enables and nested groups. Read shift state for serial groups. Constrain a selected route to one known-active contributor and all others known-inactive.
- [x] Reuse the same selection rule for greedy path discovery and concrete scan replay; test dependency scheduling and state rollback.
- [x] Run focused native and dynamic/static CLI tests, complete regression, sanitizers, and independent review. Record evidence and limits.

## Task 2: Multi-apply merge batching

Files: src/pattern/pattern_merge.cc; tests/src/pattern/test_merge_scheduling.py.

- [x] Extend tests to require two disjoint multi-apply calls to merge by apply depth, preserving each call's operation order and fixed points. Unequal lengths, repeated writes, conflicting targets, empty applies, resource/read/wait barriers remain covered.
- [x] Recognize complete write-only streams and maintain disjoint target sets across the entire concurrent batch. Flush batches one apply per live stream per round; remove exhausted streams so work stays linear in operations after target lookup.
- [x] Keep the scheduler's existing replay and fresh-state serial fallback. Do not cross apply boundaries outside an iMerge or reorder opaque effects.
- [x] Run focused merge regression, scale checks, sanitizers, complete regression and independent review; document the supported scope.

## Remaining completion work (not yet completed)

- Mixed primary/scan scheduling and shared/multiple outputs/TAPs: simultaneous event-aware state transitions and cross-backend dependency planning, with common-state replay.
- Independently gated/shadow events, feedback/register callbacks, system-clock/time behavior: establish each model's documented event semantics before implementation.
- Additional reservations and protected reset/lifecycle behavior: extend resource ownership and transitions consistently across backends.
- General caller-only reset rewriting: classify repairable source sites and impossible immutable-procedure cases; retain diagnostics for genuinely impossible transformations.
- Broader merge optimization: preserve reads, last-write semantics, barriers and reservations while grouping compatible work; benchmark representative scale.
- Synchronize current requirement/model coverage ledgers; complete defaults/reference evidence and vendor qualification. Do not call corpus parser coverage generator coverage.

These items require their own concrete test-first steps after inspecting their connected code. This plan does not redefine them as optional or completed.

## Verified execution record

Tasks 1 and 2 passed the final full dynamic/static `make check`, including the
reviewer's late-failure rollback test: `build/pattern-completion-regression-reviewed.log`
(exit 0). Seven new one-hot tests and eleven merge tests pass under
ASan/UBSan/LSan; native symbolic/state tests pass with the same sanitizers.
Independent review found no Important/Critical issues; the retained wide-group
constraint complexity is documented with the measured 32-contributor scale.
Google formatting, changed-source naming and diff whitespace checks pass.
See [implementation evidence](../../pattern-generation.md#one-hot-serial-paths-and-multi-apply-merges-2026-10-29).
No fresh vendor certification, installation or strict coverage run was performed.
The user authorized committing and pushing this verified increment. The remaining
requirements above are still open and execution continues after publication.
