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

## Task 3: Bit-level ScanRegister reservations

Files: pattern_resources.h/.cc, pattern_scheduler.h/.cc and icl_pattern_generation_main.cc under src/pattern; tests/src/pattern/test_resource_reservations.py, tests/src/pattern_scheduler_test.cc; resource fixtures and tests/src/pattern/probe_resource_slices.tcl.

- [x] Reproduce the unsupported indexed/alias take and the incorrect whole-register lock after an explicit partial write. Tessent 2026.2 probes: high-bit take succeeds, low-bit take blocks access, partial release succeeds, inverted alias succeeds, and taking the full register followed by a high-bit-only write leaves the low bit available.
- [x] Resolve aliases/slices with ResolvePdlTarget and store physical per-bit ownership, preserving inherited/persistent ownership and scoped release. Use per-node masks/counters rather than one allocation per bit. Protect only explicitly assigned taken bits.
- [x] Extend existing symbolic constraints and concrete replay to masked preserved values. Keep unprotected bits free; preserve old-to-requested transitions on protected bits; apply the same masks in greedy planning, idle replay, end states and reset prerequisites.
- [x] Emit partial reset-control writes when other bits of that register are taken but unwritten, so inserted source commands cannot create new reservations. Re-execute rewritten text and retain failed-output protection.
- [x] Test ascending/inverted/concatenated aliases, overlapping ownership, partial release, iState, reset rewriting, greedy paths, TAP/flat/generated replay and larger vectors. Run full regression, leak-enabled sanitizers and independent review; document normative rules separately from vendor observations.

## Task 4: Hidden serial storage events

Architecture decision: retain the existing path scheduler as the fast path;
when explicit interface controls clock storage outside that path, use bounded
symbolic capture/shift/update transitions and verify every committed register
against the existing concrete TAP replay. Do not enumerate possible patterns
or suppress replay mismatches. Use the same held/shift distinction as
NetworkState. Work is bounded by event count times modeled storage, with the
existing solver timeout and an explicit expression-work budget.

Files: src/pattern/pattern_symbolic.h/.cc, src/pattern/pattern_scheduler.cc;
tests/src/pattern_symbolic_test.cc, tests/src/pattern_scheduler_test.cc;
tests/src/pattern/test_hidden_scan_events.py; existing resource probe fixtures.

- [x] Add native symbolic/concrete tests with deliberately different held and shift bits, including mux selectors, aliases, inverted and one-hot serial returns. Extend SymbolicNetwork with a separate optional shift-state view while preserving existing callers.
- [x] Reproduce the original five slice probes on their ungated TAP network. Add capture/no-capture, hidden update, unknown source, protected hidden storage, reset and rollback cases with native expected states and original/flat/generated replay.
- [x] Reuse interface bindings to derive effective events; compute all affected register states from one event snapshot. Carry unknown state without inferring captured data from expected comparisons. Keep full-register state replay equality and per-clock reservation checks.
- [x] Measure representative larger gated and hidden networks; retain the fast path and fail with a resource-limit diagnostic when bounded event planning is not proved. Run focused checks, full regression, sanitizers and independent review; record any remaining model restrictions explicitly.

## Task 5: Explicit reset releases resource reservations

IEEE §7.9.9 resets controlling/expected state; §7.9.19 restricts retargeter
changes to taken resources. Tessent v2023.4 describes global reset and persistent
ownership across pattern sets only when no initial reset occurs. Twelve fresh
Tessent 2026.2 probes establish the remaining ordering detail: explicit async or
sync iReset is allowed after a taken/write, and a subsequent write does not
reacquire protection without a new iTake. Treat clearing ownership as observed
Tessent behavior where IEEE does not explicitly specify the interaction.

Files: src/pattern/pattern_resources.h/.cc, icl_pattern_generation_main.cc;
tests/src/pattern/test_resource_reservations.py; new
 tests/src/pattern/probe_reset_reservations.tcl; user/model docs.

- [x] Add failing tests for written and unwritten ownership across iReset,
  async/sync TAP reset, slice/alias/persistent takes, nested calls, explicit
  retake, pending-operation rejection, original/flat/generated and reset-only
  replay. A low selector written to zero after reset must remain free unless
  explicitly retaken; retaking it must still reject target access atomically.
- [x] Add ResourceReservations::Reset(), clearing owner masks, counters and
  protected values while retaining the active call-stack shape. Invoke only
  after scheduler.Reset succeeds. Keep unapplied-operation rejection and
  physical reset replay unchanged.
- [x] Run focused resource/native tests, full dynamic/static regression,
  leak-enabled sanitizers, formatting/naming and independent review. Record
  empirical behavior without claiming broader lifecycle or vendor coverage;
  commit/push and continue remaining requirements.

## Task 6: Data resource reservations across both scheduling backends

Scope: DataRegister and DataInPort/DataOutPort ownership, including slices,
aliases and instance membership, under the same assignment-driven protection
as ScanRegisters (IEEE §7.9.19–20). Continue rejecting unsupported clock/control
port lifetime semantics explicitly. Reuse existing state and symbolic models;
do not introduce a second ownership system.

Files: pattern_resources.h/.cc and pattern_scheduler.cc; new
 tests/src/pattern/test_data_resource_reservations.py; Makefile and support docs.

- [x] Add failing primary/scan DataRegister and primary-data-port tests:
  blocked implicit changes, allowed explicit replacement, release/retake,
  slice/alias and instance ownership, nested lifetime, end-state conflict,
  reset clearing, reads without invented hardware state, atomic failure and
  original/flat/generated replay. Include a shared-address bank to exercise
  preserved data across prerequisite scans and measure bounded larger inputs.
- [x] Extend reservation target/member classification to DataRegister and data
  ports. Share desired protected-value validation and end-state conflict checks
  between primary and serial scheduling. Pass protection through the primary
  solver, constrain every frame, and verify it during concrete replay. Access
  ScanStep loads only for ScanRegisters; data registers update through existing
  model events. Preserve explicit old-to-requested transition rules.
- [x] Run the focused suite, complete dynamic/static regression, leak-enabled
  sanitizers and independent review. Document supported data resources and
  remaining control/clock/lifecycle limitations; commit/push before continuing.

## Task 7: Mixed parallel-I/O and serial time frames

Tessent Shell Reference v2023.4 `iApply` specifies a sequence of Scan and
ParallelIO time frames, all writes satisfied at the end, and each read observed
once before subsequent changes. Reuse the existing state, route resolvers and
bounded solver. A serial transaction and a parallel input event must not be
modeled as interchangeable or simultaneously applied without evidence.

- [x] Probe primary-data/scan-enable and scan-data/primary-enable models with
  Tessent; retain scripts and self-constructed fixtures under tests/. Add failing
  CLI/native tests for both dependency directions, prior-value reads, protected
  storage, aliases, rollback, and original/flat/generated replay.
- [x] Extend shared scheduling with explicit parallel/serial frame kinds and
  shared state transitions; bound primary vector count/bit width and total frame
  work. Preserve scan-only/primary-only fast paths and existing goldens. Replay
  each frame through existing concrete models; do not concatenate independently
  planned schedules that ignore shared storage.
- [x] Validate TAP and non-TAP mixed events, read ordering, end-state and
  AccessTogether obligations; add representative scale checks, independent
  review, full dynamic/static regression and leak-enabled sanitizers. Record
  vendor evidence separately from local coverage and update support docs.

## Remaining completion work (not yet completed)

- General forwarded/computed mixed-port writes, primary-port AccessTogether, and shared/multiple outputs/TAPs: simultaneous event-aware state transitions and cross-backend dependency planning, with common-state replay.
- General independently gated/serial-derived events, feedback/register callbacks, system-clock/time behavior: establish each model's documented event semantics before implementation.
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


Task 3 passed the final full dynamic/static `make check` (exit 0):
`build/resource-slices-regression-reviewed.log`. All 31 reservation cases and
native scheduler tests also pass Clang ASan/UBSan/LeakSanitizer with leak checks
enabled. Independent review found and verified the fix for unknown unwritten
reset bits; no findings remain. Google formatting, changed-source clang-tidy and
diff whitespace checks pass. Five fresh Tessent probes establish bit semantics,
but are not certified round-trip goldens; their ungated hidden TAP storage
exposes an additional outstanding event-model limitation. See
[bit-level evidence](../../pattern-generation.md#bit-level-reservations-2026-10-29).
The remaining completion work above is still required, not marked complete.


Task 4 passed full dynamic/static regression (`build/hidden-regression-final.log`),
29 directional and seven legacy offline vendor standards, and leak-enabled
ASan/UBSan native and CLI tests. Eight newly recorded Tessent 2026.2 certificates
cover four original shadow-storage reservation probes in both directions.
Independent review found no correctness issues. Wide shadow paths use existing
dependency-guided batches above 32 scan bits; smaller paths retain joint planning.
A 32/128-bit original/flat/generated test and 1024-bit work-limit rejection pass
in 2.89 s / 131484 KiB. Original joint planning timed out on those wide cases.
Final additional native coverage exercises protected hidden storage rollback and
reset for both 4-bit joint and 32-bit greedy paths. Current event restrictions
and the remaining requirements are retained in the generator/model guides.


Task 5 passed `make check` dynamic/static regression (exit 0,
`build/reset-ownership-regression.log`) and all 34 resource tests under Clang
ASan/UBSan/LeakSanitizer (`detect_leaks=1`, 43.288 s). Google formatting,
changed-source naming and whitespace checks pass. Independent review found no
blocking issue and additionally checked local/persistent post-reset retakes.
Vendor observations are scoped to ordinary takes; other lifetime variants have
local coverage and are not individually vendor-certified. No broader lifecycle
or other remaining requirements are marked complete.


Task 6 passed full dynamic/static `make check` (exit 0,
`build/data-resources-regression-reviewed.log`), all nine new data-resource
cases, all 34 original reservation cases and native scheduler tests under
Clang ASan/UBSan/LeakSanitizer. Independent review found a stale protected-value
handoff between address batches; full/partial 32-register replacement tests
failed first, then passed after the fix. Formatting, naming and whitespace
checks pass. Source/flat/generated replay and failed-output protection are
covered; no additional vendor certification is claimed. See
[data-resource evidence](../../pattern-generation.md#data-resource-reservations-2026-10-29).


Task 7 passed the final full dynamic/static `make check` (exit 0,
`build/mixed-io-regression-final.log`), including 31 directional and seven legacy
offline vendor standards. Native scheduler tests, 15 mixed-I/O, nine data-resource,
34 reservation and four hidden-event cases passed ASan/UBSan/LeakSanitizer with
`detect_leaks=1` (`build/mixed-io-sanitizer-final.log`). Focused native, mixed and
25 scan-data tests also passed after the final primary-input helper extraction.
Google formatting, changed-source clang-tidy and whitespace checks pass. Two
fresh Tessent forward certificates qualify the mixed fixtures; reverse apply
comparisons remain failures and no reverse standards were recorded. Independent
review findings were reproduced and fixed, with actual-state and TAP rollback
coverage. No broader model completeness or shortest-pattern guarantee is claimed.
