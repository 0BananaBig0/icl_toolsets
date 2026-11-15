# Active Child Raw Scans Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Retarget static child raw scans inside a known gated root route.

**Architecture:** Reuse current-route interval/parity proof and root composition;
invalidate only dynamic profile/slice caches at audited mutation boundaries.

**Tech Stack:** Existing C++17 scheduler/native state and Python unittest.

**Spec:** docs/superpowers/specs/2026-11-15-active-child-raw-scans-design.md

## Global Constraints

Task100 frozen Root snapshot28 remains unchanged during its gates. No dependencies,
TAP/parallel/internal reset/clock widening or implicit opening of inaccessible
children. Preserve1M active/queued bit,4096-register/graph bounds and16M interval
proof work per dynamic state generation. New tests under tests/ with mirrored paths.

## Review Focus

- Audit every state mutation/copy path before caching state-dependent routes.
- Child interval offsets/parities change when other routes open or close.
- Surrounding controlling fill and expected SO are independent; never infer a load.
- Stable is a physical-root route/control claim; failure must remain atomic.
-1024 queued children must not each rebuild the full current root route.

### Task101: Accessible Child Raw Retargeting

**Files:** src/pattern/pattern_scheduler.cc/.h; maintain
 tests/src/pattern_scheduler_test.cc and tests/src/pattern/test_modeled_root_scan.py;
 reuse gated_sib_raw fixture, add a uniquely named focused fixture if needed;
 update generator/model guides after verified evidence.

**Interfaces:** RawSubchain returns RawSlice by value; RawChain keeps const profile
reference valid through a mutation-free CSU; private InvalidateRawRoute clears
only dynamic caches. Existing public scheduler and native state APIs unchanged.

- [x] Builder review of design, particularly mutation/cache lifetime and throughput.
- [x] Maintained CLI/native RED: open child raw access, static siblings queued,
      root physical reference/parity/X fill, missing/closed child and overlap;
      stale cache after Apply/Assume/Reset/Idle and scheduler copy.
- [x] Reuse interval proof on current profile, by-value slices, bounded generation
      cache lifetime; focused methods and complete native/model CLI become GREEN.
- [x] Measure source/flat/generated1M child and1024 queued intervals/shared selector;
      connected checks, honest vendor probes, all57 offline certificates.
- [x] Independent whole-branch review and immutable style/leak/fullserial/remote gates.
- [x] Commit, fast-forward master and delete completed branch after Task100 integration.

Run focused CLI methods with tests/src/pattern/test_modeled_root_scan.py and native
build/pattern_scheduler_test; expected intended guard failures before implementation,
all passing afterward. Final full make -j1 check must exit0. Do not run local builds
or timed tests concurrently with Root's current sanitizer/full serial gates; use
remote candidate after its frozen Task100 evidence is archived, or wait.

Builder refinement: separate OR proof-required from AND emitted -stable; test
mixed unchanged-route siblings as legal and changing-route stable-child requests
as rejected. Do not invalidate at Normalize/ApplyScans entry. Copy paths start
cold; retain static/immutable caches and derive post-trial proof independently.

Task101 evidence: initial16 connected checks/corpus audit and57 offline
certificates pass; corpus2550 files, zero duplicate-file groups,26 reviewed
module groups. Independent review finds no production defect. Its two test gaps
were strengthened: warm ordinary Apply/copy cache states and independent expected
SO polarity. Fault injection removing Apply invalidation and SO parity produces
the exact native warm-cache failure and4 SO assertions; exact reviewed source
7b0b0a6c is restored, native scheduler and modeled43 pass. No mutation ships.
All26 source-matched scale commands pass:1M active bits via child .29–.30s
(source),1024 queued children with512 shared aliases .37–.38s,97-level child
.10s,128 reopen/write/close repetitions .04s. Peaks107052KiB overall. Three
Task100 physical-reference repetitions .24–.27s wide and .20s queued-network
provide equivalent output hashes; this measures retargeting overhead, not speedup.
Source/flat/generated forms share the same canonical hashes.
Genuine Tessent2026.2 forward run/strict record/strict check0 using unchanged
harness/policy; one schema3 certificate imported after source/artifact/input/
console hash checks. Reverse original source1 reports that child.local is an
internal ScanInterface and iScan is unsupported in this tested context; retained
evidence is not a reverse certificate. All child SO and final child/selector
comparisons remain present. Existing and new forward snapshots total58; final
immutable sanitizer/fullserial/remote/style/offline gates remain pending.

Verified Task101: leak-enabled native7/main69/modeled43/opaque13/clocks51/client25/reset12/trace16/all58offline certificates all0. Full serial make check passes in 1496.6201768030296s. Final remote54 statuses all0 and actual99 source/test/fixture/certificate inputs match. Independent source review found no production defect; two required test-strength refinements are fixed with mutation RED (missing warm-Apply invalidation and SO parity), exact production-byte restoration and GREEN native/model43 suites. Warm-copy isolation, changed offsets, same-width switches and8 inversion/expected-SO combinations are maintained. Existing interval/parity/root-CSU logic reused with constant-sized cached child slices, by-value return and audited dynamic cache lifetime. Any latest stable claim on a dynamic route still requires whole physical-root stability in this bounded increment; OR proof is separate from AND emitted flag. Local-only stable scope and inner child muxes remain next work, with no waiver. All26 scale commands0:1M active bits via child .28-.31s,1024 queued children/512shared aliases .20-.38s,97-level child .04-.10s,128 reopen/write/close .04s;107052KiB peak, hashes equal to equivalent Task100 physical reference runs and across source/flat/generated forms. Source audit2550 files/zero duplicate groups/26reviewed module groups;16 connected/style/naming/offline pass. Genuine Tessent2026.2 FORWARD run/strict record/strict check0 for unique ActiveRawLeaf source; schema3 certificate imported with source/artifact/input/console hashes checked, all child SO and final child/selector comparisons retained. Reverse original source1 explicitly rejects internal ScanInterface in tested context; no reverse certificate claimed and checker policy unchanged. Commit20 files, fast-forward master, delete merged branch and preserve next dynamic-child work. Closed-child preparation, broader raw/ownership/events/reset/clock/lifecycle, strict coverage and final platform/vendor requirements remain open. No push.
