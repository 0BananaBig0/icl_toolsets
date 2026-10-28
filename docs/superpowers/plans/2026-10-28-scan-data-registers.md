# Scan-driven DataRegister implementation plan

> **For agentic workers:** Use superpowers:executing-plans, TDD, independent review and verification before publication.

**Goal:** Retarget selectable/addressed DataRegister writes and scan-captured reads using the existing serial/TAP backend.

**Architecture:** Extend serial frames with the shared DataRegisterPlan activation/dependency model. Keep Capture before Update semantics: read requirements become comparisons on observable ScanRegister capture bits; updates settle after scan-register updates. Reuse frontends, address resolver, route guards, concrete NetworkState replay and output serializer. Do not enumerate pattern permutations.

**Tech stack:** Existing C++17, Z3, Bison/Flex ICL frontend and separate Tcl/PDL frontend; Python/native regression.

**Spec:** User's icl-pattern-generation requirements, docs/pattern-generation.md, IEEE 1687-2014 §6.4.9 and §7.9. Use preserved Tessent probes to resolve uncertain export behavior. Existing authorization covers native implementation, repo edits and test/commit/push operations.

## Constraints and review focus

- One existing serial boundary / single TAP; mixed primary driving is a remaining separate path.
- Existing 32-frame, 2-second solve and 15-second overall budgets; include DataRegister/route size in structural budgets. Cache immutable structure.
- Protect unrequested storage, reset precedence and reservations; unknown read data must not become expected data.
- Repeated source, flat-ICL and emitted-PDL runs must preserve applies and values.
- Separate explicit source intent from vendor export behavior; retain failed qualifications.

## Task 1: Serial data updates

- [x] Add CLI tests for selectable writes/partial writes, address buses and source/flat/generated replay; observe existing rejection.
- [x] Admit DataRegister goals, expose their controlling scan bits, extend serial temporal state/fill and post-update settling, and verify every data state against concrete replay.
- [x] Include data end-state restoration, simultaneous grouping and preserved scan storage without permitting a data goal to override a reservation.
- [x] Focused native/CLI tests pass; compare a Tessent scan-driven source probe.

## Task 2: Scan-captured reads

- [x] Add failing tests for direct/inverted/mux/OneHot capture routes, shared addresses, read-before-write in one apply, masked and unqualified reads, collisions and disconnected capture paths.
- [x] Reuse DataReadRoutes with ScanRegister CaptureSource roots; select guarded observations in pre-update frames and map comparisons to physical scan capture bits.
- [x] Verify emitted comparisons, capture accessibility, deterministic replay and Tessent directional certificates.

## Task 3: Publication

- [x] Independent review of entire diff, RED→GREEN fixes and complete make check.
- [x] Google style, static/dynamic checks and ASan/UBSan/LeakSanitizer; no sanitizer disabling.
- [x] Update README, capability/evidence docs and remaining requirements; commit and push only after verified checks.

## Execution record

- Shared temporal frames now carry writable DataRegisters and scan capture routes;
  shared ASTs, netlist, activation/address resolution, replay and serializer reused.
- 19 serial CLI cases plus two primary-read regressions added. Initial target
  rejection, capture rejection, fill/end-state bugs and the three review findings
  were reproduced before fixes. Independent reviewer: `scan_data_review`.
- Full regression: `build/scan-data-regression-published.log`, exit 0. Dynamic/static,
  334/334 grammar, 119/119 scanner, both editor corpora, 21 directional and seven
  legacy vendor standards passed. Formatting/naming passed.
- Fresh Tessent 2026.2 selectable TAP case certified in both directions. Original
  no-interface probe failed ICL71-1; a complete TAP/interface fixture replaced it.
- Ruling: preserve the existing solver budgets and report the eight-register
  sanitizer timeout rather than weakening comparison, disabling sanitizers or
  claiming a full instrumented pass. Cost: larger shared-address schedules may
  fail with an explicit resource limit; a scalable heuristic remains needed.
- ASan/UBSan/LSan: native state/scheduler, 18 serial correctness and 20 primary
  cases passed outside ptrace sandbox. The additional eight-register stress test
  passes dynamic/static normal regression but times out under instrumentation.
- No independent-review minors deferred. No installation, RTL simulation or
  strict authored-code coverage remeasurement. Other retargeter limitations are
  retained in docs/pattern-generation.md.
