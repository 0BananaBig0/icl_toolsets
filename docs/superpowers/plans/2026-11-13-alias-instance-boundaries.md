# Alias instance-boundary validation

> Execute inline with superpowers:executing-plans under the existing authorization.

**Goal:** Prevent aliases from turning an internal immediate-child object into
a hardware connection across an instance boundary.

**Architecture:** Extend CheckAliasConsumers after Connect, before flat-origin
restoration. Keep the existing two-level PDL-only rule. Add an Alias-or-PDL-only
classification for immediate-child references that are not output ports, and
propagate it through local alias dependencies using an iterative reverse queue.
Validate all existing consumer categories through the shared pass.

**Tech Stack:** Existing C++17 netlist and Python/native tests; no dependencies.

**Spec:** Existing approved pattern-generation design and requirements; IEEE1687
§6.3.10.5, §6.3.12 and §6.5.2, printed pp.67–71 and135–137.

## Decisions and evidence

Direct hardware references already require child output ports. Generic signal
rules explicitly permit aliases of those legal outputs; retain them. An alias
of a child register/input/internal alias cannot manufacture a legal output port.
Keep these names usable by PDL and other Alias definitions, following the alias
exception in §6.3.10.5 and the allowance in §6.5.2(f). Keep §6.5.2(g)'s stricter
two-level restriction unchanged. This closes the boundary bypass, not every
source-kind rule or all ambiguities in (f)'s wording.

Independent design review agrees. Tessent2026.2 accepts immediate-child output
aliases, but rejects child-register wrapper definitions as well as hardware
consumers. Its stricter wrapper rejection differs from the chosen standards-first
rule. v2023.4 Shell Reference ICL57 (General_Icl57_id8eb00c3a.html) calls
hierarchical aliases PDL-only without defining that term; it does not settle
the wording conflict. Retain the recorded Task42 probes, not new claimed goldens.

Alternatives: banning every nonlocal alias contradicts legal child-output signal
aliases; broadening the existing boolean rejects allowed single-level wrappers.
A separate restriction bit and reverse dependency propagation preserve both.

## Implementation and verification

Files: src/pattern/pattern_netlist.cc, tests/src/pattern_netlist_test.cc,
tests/src/test_icl_pattern_generation_main.py, current support/model/spec docs.

- [x] Add failing native/CLI cases for direct and wrapped child register/input
  aliases, child internal aliases, forward references, mixed aliases whose local
  slice is consumed, mux/logic/register/instance consumers and failure publication.
- [x] Preserve PDL-only wrapper access and source/flat/generated replay, local
  register/input aliases and child-output chains. Keep deep-alias rejection.
- [x] Add reverse edges for Alias dependencies and propagate the new restriction
  once per node. Check non-Alias consumers without following output-port drivers.
  O(nodes + resolved references) time/space; no recursive wrapper traversal.
- [x] Measure long alias chains and wide vectors using existing fixtures/helpers;
  run native/focused CLI, independent review, full dynamic/static regression,
  leak-enabled sanitizers and source style/naming checks. Preserve failed logs.
- [x] Update evidence and commit; all other remaining acceptance work stays open.

Review focus: alias definition order, mixed-alias local slices, cycles (existing
cycle validation still applies), legal output-port boundaries, and flat provenance.
The full test gate for Task44 remains frozen while this work uses a separate tree.

Isolated worktree: .worktrees/alias-instance-boundaries, base91f5f2a. Focused
binaries link the reviewed Task44 root objects and the new isolated netlist object;
root product/tests remain frozen for Task44's full run. Six CLI negative subcases
and one native group reproduce the bypass before implementation. Green native
and33-method main CLI pass (9.115s). Native scale:256 wrappers/6942bytes in0.001s,
4096wrappers/101454bytes in0.022s; existing legal5000wrapper chain in0.018s.
Whole native run:0.46s wall,118388KiB RSS on the local Linux/GCC build. The65536-bit
restricted alias also rejects correctly. No performance-improvement claim.
Independent implementation review has no findings; changed-source naming passes.
Leak-enabled native/CLI checks are running. Full integration follows Task44.

Final focused leak-enabled ASan/UBSan checks pass: native netlist/flatten,
main33 in23.181s, end-state11 in7.561s and merge-observation20 in13.842s. Evidence
build/alias-boundary-sanitizer-*.log/.exit, all exit0. Existing output-port drivers
are intentionally not traversed by classification. Whole-alias restrictions
propagate through local wrappers even when their selected slice is local-only.

Root integration follows verified Task44 commit27affc5. Product/native/main-test
files are byte-identical to their reviewed isolated copies. Full regression is
build/alias-boundary-final-full.log/.exit. A fresh complete coverage build/run is
build/alias-boundary-full-coverage.log/.exit with COV_DIR=build/alias-boundary-coverage
and COV_REPORT_DIR=.worktrees/coverage/2026-11-14-alias-boundary. No prior coverage
artifacts were overwritten; root had no gcov files at launch. The100%line/90%raw
branch thresholds remain unchanged. Source/tests stay frozen for both runs.

Inspection also found stale CLI --help text from before Task44: it implies all
nonzero waits require explicit reset. Correct that wording after the frozen full
and coverage snapshots finish, and verify dynamic/static help output. This is
a documentation-only follow-up; standalone behavior already has native/CLI tests.

Fresh complete coverage finished on2026-11-14. All instrumented tests completed;
the strict gate failed (make exit2) at14623/15208 authored lines (96.2%) and
24524/39581 raw branches (62.0%), below100%/90%. This is a new measurement, not
a passing coverage result. Of15057 untaken raw edges,10690 carry gcov's '(throw)'
marker; this diagnostic classification does not change the gate or exclude them.
The largest uncovered-line counts are scheduler145, flatten54, PDL document50,
runtime46, main44 and netlist42. The fresh reports and full log are preserved.

Final full regression passed on2026-11-14: build/alias-boundary-final-full.log/.exit(0),
39dynamic/static suites,51directional/seven legacy standards,2519ICL334/119rules,
and editor6 in178.257s. The known strict coverage failure remains a global
acceptance gap. Following those frozen runs, corrected only the stale --help
wording about standalone resets. Both dynamic/static rebuilt help outputs and
existing help/version tests pass (alias-boundary-help-smoke.exit0); final style
passes. This follow-up changes no runtime branch. No other requirements waived.
