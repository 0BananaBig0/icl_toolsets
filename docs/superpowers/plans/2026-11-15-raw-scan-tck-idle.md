# Raw Scan TCK Idle Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Permit existing native TCK idle replay between qualified modeled raw scans.

**Architecture:** Admit iRunLoop through the modeled-raw whitelist and reuse the
existing Generate handler, runtime queue validation and native Idle implementation.

**Tech Stack:** C++17 existing frontend/scheduler and Python unittest.

**Spec:** docs/superpowers/specs/2026-11-15-raw-scan-tck-idle-design.md

## Global Constraints

No new dependencies, budgets, clock semantics or topology support. Frozen Root
Task97 inputs remain unchanged. Test scripts stay beneath tests/.

## Review Focus

- Pending raw setup must still fail before a wait, including discarded setup.
- Partial-X controlling fill must survive native idle shift-shadow invalidation.
- Large counts require identical complete-state fixed points, not unchanged update bits alone.
- Opaque paths and unsupported system-clock/time targets retain explicit rejection.
- Emitted waits retain exact counts across source/flat/generated/local-reset replay.

### Task98: Existing TCK Idle in Modeled Raw Programs

**Files:** src/pattern/icl_pattern_generation_main.cc whitelist only;
maintain tests/src/pattern/test_modeled_root_scan.py and tests/src/pattern_scheduler_test.cc;
create tests/cases/ours/pattern_generation/raw_scan_tck_idle/{model.icl,source.pdl,cases.json,README.md}.
Update docs/pattern-generation.md and docs/pattern-model-coverage.md after evidence.

**Interfaces:** Existing NormalizeScan/ApplyScans and ScanScheduler::Idle unchanged.

- [x] Independent design/builder review.
- [x] Meaningful maintained RED for raw-plus-idle programs; native state oracle.
- [x] Minimal whitelist edit; GREEN focused/connected checks pass.
- [x] Vendor probe, large-count and representative scale measurements.
- [x] Independent final review, style/naming/offline and immutable required leak/fullserial/remote checks.
- [x] Commit, fast-forward master, delete merged branch and preserve subsequent work.

Independent review approves whitelist-only reuse: native Idle retires updated_ shift shadows while preserving controlling fill and compares complete fixed-point state; pending raw setup is rejected even before zero cycles. No new clock/event model.

Local Task97 CLI RED:3 methods,10 failures at the intended modeled-raw iRunLoop whitelist barrier. New native oracle extends child per-bit state checks with7 literal idle cycles. Its first insertion precondition failed without modifying native source, then was corrected before the whitelist edit. No Idle/runtime semantics changed.

GREEN actual17hashes match: build/native state/native scheduler/opaque12 all0. Modeled27 initially failed one expected-diagnostic assertion (TCK type fails ClockPort validation before prior-iClock check); corrected to ClockPort. Focused GREEN2 modeled27=0, actual17hashes match; script asserts every non-CLI input unchanged from native GREEN, so native evidence is reused. No production follow-up change.

Vendor first probe actual9/10 preparedhashes match, original/normalized both exit1 at iRunLoop0: Tessent2026.2 requires positive TCK count. Existing native/CLI zero-wait behavior is retained; the positive vendor fixture now uses7/3cycles, while embedded CLI zero tests and negative vendor evidence remain intact. No production change.

Positive vendor probe actual9submitted/10prepared hashes match: original/normalized0, exports byte-identical; zero-count vendor rejection remains separate. Twenty maxUINT64 TCK-wait scale commands0 with equal source/flat/generated hashes:1M root/child .27-.33s peak108172KiB;1024children .19-.37s peak102420KiB;97levels .06-.13s peak64020KiB. Independent final review found no blocker or missing required coverage. Final local/remote/fullserial gates remain pending.

Local15 connected checks0, formatting/naming0 and53 fresh offline goldens0. Early corpus audit0 using the existing Root inventory binary against WT files:2547files,0duplicate-file groups,26reviewedmodulegroups. Initial audit exit2 only reflected the absent WT inventory executable; the unchanged Root parser inventory was reused explicitly. Freeze11 reviewed files; refresh copied build-input mtimes so Make cannot reuse an older object for newly copied bytes.

Verified Task98: required leak-enabled build/native7/main69/modeled27/opaque12/clock51/client25/reset12 all0. Full serial make check passes in 1369.6011084119673s; editor6 in182.739s. Final remote52 statuses all0 and actual45 source/test/fixture hashes match. Local15 connected checks, early corpus audit, formatting/naming and53 fresh offline directional goldens pass. Independent final review found no blocking defect or missing coverage. The whitelist-only product change reuses native idle/fixed-point behavior; native literal edges cover eight polarity combinations in source/flat, CLI checks exact counts through UINT64MAX, implicit controls, partial-X fill and atomic invalid cases. Twenty maximal-wait scale commands pass with equal replay hashes: root/child1M .27-.33s/~108172KiB,1024children .19-.37s/~102420KiB,97levels .06-.13s/~64020KiB. Positive-wait vendor original/normalized0 with byte-identical exports, actual9submitted/10preparedhashes match. Tessent zero-count rejection and ignored-stable/default-chain limits remain recorded; no strict certificate claim for this stable fixture. Copied build inputs received fresh mtimes before required builds. Commit11 files, fast-forward master, delete merged branch and preserve the default-chain draft and genuine certificates for its later verified integration. Remaining general models/events/ownership/lifecycle, strict coverage and final vendor/platform requirements remain open. No push.
