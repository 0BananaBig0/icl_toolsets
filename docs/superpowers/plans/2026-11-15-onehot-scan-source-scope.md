# OneHotScanGroup source scope implementation plan

> Execute inline with superpowers:executing-plans under existing autonomous
> authorization. Root77 stays frozen through full verification. Independent
> review remains required; no delegated implementation.

**Goal:** Validate OneHotScanGroup Port sources using the same scan-kind and
hardware-scope rules already used by other scan consumers.

**Architecture:** Extend the existing OneHot port collector to scan groups and
pass Family::kScan. Keep the existing enabled-fan-in/width/cycle checks, which
run earlier, and reuse the canonical source/scope validator. No array, graph
pass, metadata, dependency or public API is added.

**Tech stack:** C++17, existing native and Python tests, Tessent Tcl loading.
**Spec:** ../specs/2026-10-25-icl-pattern-generation-design.md; IEEE1687
6.3.12(c),6.4.14(b), printed127–128.

## Evidence and design

Current CheckSignalSources collects explicit OneHotDataGroup Port sources only;
OneHotScanGroup references bypass that consumer check. Existing own-ScanOut
native positives demonstrate current acceptance.6.4.14(b) requires an enabled
ScanOut/OneHotScanGroup in controlling fan-in; it does not replace the generic
scope rule. Independent design inspection confirms this concrete omission.
Matched unused/consumed own-vs-child, direct/alias/inverted vendor probes run
under build/special-source-context-vendor-probe; all actual hashes and outcomes are recorded below.

Use Family::kScan, not kScanInput: retain concat_scan data alternatives and the
existing ScanMux/OneHot cascade semantics. Do not impose ScanInSource rightmost
rules on all group contributors or narrow valid enabled mux fan-in. Aliases used
only by PDL remain unchanged. Earlier enabled-source/width/cycle diagnostics keep
precedence. This increment does not change OneHot selection, event timing or
unresolved ToIRSelect/differential association rules.

## Reach and performance

Modify src/pattern/pattern_netlist.cc; tests/src/pattern_netlist_test.cc;
 tests/src/pattern_symbolic_test.cc and any demonstrated producer fixture callers;
 tests/src/test_icl_pattern_generation_main.py for atomic rejection. Update the
current generator guide/model ledger/this plan. New test scripts belong under
project tests with source-relative paths; preserve existing tests in place.

The extra work is O(group contributor references + existing alias resolution),
with no scan of padded output width. Retain sparse10000-scalar/1048576-bit group
and3000-group tests and the unchanged10s benchmark deadline. Measure existing
wide/deep/shared loading and OneHot constructor/value truth-table checks. Repair
producer scope only, retaining held-versus-shift state, all enable combinations,
unknowns, inversion, flat replay and budget assertions.

## Review focus and steps

- [x] Obtain matched vendor source-scope evidence, retaining interface-context
  diagnostics separately. Confirm legal child controls still load.
- [x] Add direct/alias/inverted own-vs-child RED matrices; original/flat and
  alias-only positives; altered-flat deep-output/raw-alias negatives; mixed
  vector contributors and preserved earlier fan-in diagnostics.
- [x] Add main CLI artifact-preservation failure before implementation.
- [x] Extend the shared collector minimally; repair only demonstrated invalid
  producer fixtures without weakening truth tables/phase/replay assertions.
- [x] Run focused/connected GREEN, performance/style and independent review;
  immutable integration only after77's verified commit.
- [x] Run required leak/full serial checks, record actual remaining requirements
  and commit the verified snapshot. No push requested.

Matched58 vendor model/runner hashes include16 OneHotScan cases:8childload,
8ownfailICL130 (consumed4alsoICL103;unused4onlyICL130). Direct/alias/inverted
controls agree. Source/PDL proof is still separate from vendor loading. Native
and CLI RED run against unchanged77 before code.77committed7f57a21 afterfull0
1338.899s/editor6 174.509s, allrequiredleak0, broadremote7/41 andfinal5hash
source-matchedflatten/TAP/main62. Saved nextdraft/full diff under/tmp/onehot-scan-
source-scope-task78-before-advance-files and retainedstash; advancedonto7f57a21,
restoredonly2nexttests/thisplan. Final77code/docs retained.

Task78 RED build0/native1 (missing own-output scope rejection)/CLI1 (invalid
source accepted), actual3 source hashes match submitted manifest under
build/onehot-scan-source-scope-red-remote/evidence. Only then extended the shared
Port collector with Family::kScan. Independent production/test review clean.
First connected native netlist/flatten/state/TAP/address0. Symbolic1 exposes
own-output producer fixture; repair uses child enabled/forwarding outputs while
retaining root register identities, all27 enable/unknown combinations, inversion
and independent held/shift storage assertions. Remaining connected checks pending.

Ruling: repairing the symbolic fixture exposes a legitimate child ScanInPort
forwarding gap in OneHotReadResolver::Resolve. The existing elaboration fan-in
propagates through bound ScanInPort, while runtime selection traversal omitted it.
Repair2 adds ScanInPort only to the existing transparent Source clause, reusing
visited nodes and append/shared-budget charging; absent bindings still fail.
Do not broaden to unrelated DataInPort paths. Before this fix repaired symbolic1
at ScanForward incoming; after fix symbolic0 and netlist0. Both failed pilots
remain recorded. Independent review agrees; no truth-table assertion changed.

Connected2: native7/main63/clocks41/client18/reset12/multi22/feedback10 all0.
First connected and failed repair logs retained separately. The repaired original
symbolic fixture and new source/flat scope matrix pass. Source-matched loading
comparisons72 all0 with flat/PDL hashes identical versus7f57a21. Cases:65536 scan
references .03–.04s vs .02–.03;3000 deep groups .05–.07s both;4096 shared groups
.06–.08s both;4096 aliases .05–.12 vs .06–.13;4096 ordinary consumers .06–.09
vs .06–.10;655360 ordinary refs .11–.12 vs .11–.13. This is loading-only evidence
with empty PDL, not a speedup or wide scan scheduling claim. Environment, input
hashes/sizes, binary/source hashes, commands, wall and RSS are in
build/onehot-scan-source-scope-loading-scale/comparison.json. Initial formatting
violation retained; final format/naming0 after whitespace-only condition wrap.
Independent production/tests/necessary ScanIn traversal review clean.

Final required leak-enabled ASAN/UBSAN: build0; native7/main63/clock41/client18/
reset12 all0, aggregate build/onehot-scan-source-scope-final-sanitizer-all.exit0.
Root source/tests remain the reviewed5-file snapshot. Full serial make-j1check
started after sanitizer completion; source/test mutation is paused until it ends.
Remote first connected native6/all41CLI0 but symbolic1 confirms the same missing
ScanIn traversal, actual4 hashes matched. Final5-file remote snapshot with the
traversal repair is running; retain first run as pre-repair evidence only.

Final remote build/native7/all41 generator CLI suites all0; actual5 source/test
hashes exactly match submitted final snapshot. Complete archive retrieved0 under
build/onehot-scan-source-scope-final-remote/evidence. No baseline overwrite occurs
in this runner. Local full regression remains active; source/tests stay frozen.

Full0 wall1345.595189s/editor6 175.874s; dynamic41/static41,51directional,
2523ICL334/119,LSP62,optionalANTLR5skip. Reviewed5 source/test hashes still match;
all50 final remote status files0 andactual5match. Scope/traversal fix complete;
source-context owner/input-pair tasks remain separate and are not claimed done.
