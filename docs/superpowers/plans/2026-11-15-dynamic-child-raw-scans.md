# Dynamic Child Raw Scans Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Retarget an accessible binary child route and preserve local stable scope.

**Architecture:** Root-selected mux map, cached local segments, pure post-trial
segment proof and conservative projection of local flags to one physical root CSU.

**Tech Stack:** Existing C++17 scheduler/native control engine, Python unittest.

**Spec:** docs/superpowers/specs/2026-11-15-dynamic-child-raw-scans-design.md

## Global Constraints

Root Task101 frozen20 files unchanged during its gates. No dependencies, hidden
preparatory scans, TAP/IR/parallel/internal reset/clock/selector widening. Preserve
1M active/queued bits,4096 registers,65536 graph vertices and16M segment traversal
work. Only current accessible binary routes. New tests beneath tests/ mirrored paths.

## Review Focus

- Local -stable cannot be silently widened to an unrelated physical-root claim.
- Local geometry must be checked when root length happens to remain unchanged.
- Pre-update gates determine opening/closing effects; post proof uses another copy.
- Trial segment/phase proof must not poison live caches or work counters.
-1024 target setups must reuse one root derivation and cached local segment metadata.

### Task102: Dynamic Local Segments and Stable Projection

**Files:** src/pattern/pattern_scheduler.cc/.h; maintain tests/src/pattern_scheduler_test.cc
and tests/src/pattern/test_modeled_root_scan.py; new unique dynamic_child_raw fixture
under tests/cases/ours/pattern_generation/; generator/model guides.

**Interfaces:** RawProfile.muxes maps nodeID->choice. RawSegment stores RawSlice,
first/count indices into its root profile and total local parity (no duplicated
register vectors). Stable comparison reads IDs and rootSuffix XOR slice.output_inverted
from the two immutable profile spans. RawSegmentFor(size_t client,
const RawProfile&,size_t* work) returns a fresh segment without state/cache mutation.
RawSubchain returns RawSlice; ReplayScan takes latest stable client IDs; native
controls accept the phase's active set for the separate post proof. Public APIs
and Native ShiftComplete unchanged.

- [x] Independent builder review, resolve segment lifetime/budget and flag details.
- [x] Maintained RED nested child1->9->1 scans and local stable root-width change;
      compensated-width stable failure, root claim failure, native rollback/parity.
- [x] Minimal mux-map/segment factoring and scoped stable proof/flag projection;
      focused/native/model CLI GREEN, existing static/root semantics retained.
- [x] Connected58offline/corpus/style;1M/1024/deep/repetition scale and honest vendor probes.
- [x] Independent final review and immutable leak/fullserial/remote/certificate gates.
- [x] Commit and fast-forward master, delete completed branch, preserve future work.

Establish intended guards with the frozen Task101 binary before editing product.
Do not run local builds/tests while Root's timed sanitizer/full serial checks run;
use remote after its Task101 evidence is archived. Full make -j1 check exits0 before
integration; unavailable suites are not reported as passed.

Task102 evidence: source-matched remote native/state/opaque13/modeled47 pass after
formatting. Local16 connected checks, corpus2551 files/zero duplicate groups/26
reviewed module groups and58 offline certificates pass before new vendor data.
Independent final review found no production defect and one Important regression
gap. Equal-count/equal-total-parity ID-swap and same-ID suffix-parity changes with
compensating inversions now have negative and legal-repeat tests. Disabling only
the comparison loop produces2 failures; exact reviewed source31b31ef6 is restored
and all48 CLI methods pass. A first mutation harness formatting anchor failed
before any source write; corrected brace anchor supplied the intended mutation.
No mutation or checker exception ships; no re-review loop.
All26 scale commands pass. Source repetitions:1M active bits .32s,1024 queued
dynamic children/2049 registers/512 shared aliases .85–.86s,97-level incremental
opening .48–.49s,128 nested open/write/close cycles .06s. Peak165192KiB includes
the flattened2049-register form. Equivalent Task100 physical references .27s
wide/.46–.47s many; hashes match between references and child/source/flat/generated
forms. Measurements describe overhead, not a speedup claim.
Genuine Tessent2026.2 FORWARD run/strict record/strict check0 for BOTH nested
dynamic_child_raw and local-stability projection fixtures. The second case keeps
a child stable while the physical root grows8->16, preserving explicit SO masks
and final child/sibling observations. Two schema3 forward snapshots imported with
all hashes checked; total directional offline snapshots60. Both reverse original
source runs1 explicitly reject internal ScanInterfaces in this tested context;
no reverse certificate is claimed. Policy/harness remain unchanged. Final style/
60offline/leak/fullserial/remote gates and integration remain pending.

Verified Task102: leak-enabled native7/main69/modeled48/opaque13/clocks51/client25/reset12/trace16/all60offline certificates all0. Full serial make check passes in 1400.9289951249957s. Final remote54 statuses all0 and actual119 source/test/fixture/certificate inputs match. Independent final source review found no production defect; its Important equal-count ID/suffix-parity test gap is fixed. Disabling only the per-register comparison produces2 failures; exact31b31ef6 production bytes restored and all48 CLI methods pass. Local stable proof compares ordered register IDs and parity, with immutable pre/post segment spans and bounded shared proof work; full native eligibility retains pre-update versus post-proof active sets. AND emitted stable is removed when local geometry stays stable but the physical root changes. All26 scale commands0:1M active bits .32s source/.33s flat/.31s generated;1024 dynamic children/2049 registers/512shared aliases .85-.86s source/.90s flat/.45s generated;97levels .48-.49s source,128reopens .06s. Peak165192KiB. Hashes equal across forms and equivalent Task100 physical references (.27s wide/.46-.47s many); no speedup claimed. Source audit2552 files/zero duplicate groups/26reviewed module groups;16 connected/style/naming/offline pass. Genuine Tessent2026.2 FORWARD run/strict record/strict check0 for both dynamic_child_raw and dynamic_child_local_stability, with two schema3 certificates and all source/artifact/input/console hashes checked. Explicit SO and final reads retained, including local-stable root8->16 projection. Both reverse originals1 reject internal ScanInterfaces in tested context; no reverse certificate claimed and checker unchanged. Commit29 files, fast-forward master, delete merged branch and preserve Task103 reservations work. Closed-child preparation, broader raw/ownership/events/reset/clock/lifecycle, strict coverage and final platform/vendor requirements remain open. No push.
