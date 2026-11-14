# Bound expanded mux data and selector dependency work

> Execute inline using superpowers:executing-plans under existing autonomous
> authorization. Preserve failing-first tests, independent review and verification.

Goal: bound combined selector/source/code expansion to4M bits per final netlist,
and avoid output-width times selector-width graph work. No public API or new
metadata/dependencies. All DataMux/ScanMux/ClockMux callers and generated TAP
routes participate. Existing object1M/width1M/graph16M limits remain unchanged.

Design: preflight remaining mux bits before resolving selectors, padded sources
or code constants. Numeric codes only. Recheck the actual combined final graph at
CheckCombinationalConnections, including SerialAssembler additions. Reuse current
checked subtraction and source-located error handling. One private auxiliary
selector vertex per wide mux shares dependencies without collapsing real output
bits; preserve register cuts and exclude auxiliaries from enabled-data propagation.
Independent design review already identified padding and final TAP accounting as
necessary, and confirmed bit-specific reachability.

Expected scale: existing65536-bit/10-choice and262144-bit control cases stay
supported.4M expanded entries bounds storage proportional to actual references;
selector edges become O(outputs+selectors), data edges stay O(choice references).
Dense real-bit state stays under16M; auxiliary state scales with mux count. No
cache or recursion. Baseline build/mux-resource-baseline/comparison.json records
safe768MiB-limited runs:65x65536 sources and5x1M code masks wrongly accepted;
string code accepted without debug but fails debug serialization.65536outputs x
1024selector bits takes1.621s with debug. Never run268M refs unconstrained.

Scope: src/pattern/pattern_netlist.cc; existing native netlist/TAP tests and main
CLI test suite; guide/model/main ledger. Review CheckCombinationalConnections's
three call sites and SerialAssembler final check. Flat replay must have the same
budget as original source, including actual generated TAP routes.

- [x] Native/CLI RED: aggregate selectors/sources/codes, alternatives, instances,
  all mux kinds, padded sources and string codes. Preserve output artifacts.
- [x] Positive boundaries, numeric codes, per-bit DAG/cycles, register cuts,
  enabled ScanMux data versus enabled selector; actual TAP combined boundary and
  accepted flat replay.
- [x] Implement shared expansion bounds and selector graph vertices; focused checks.
- [x] Reproducible repeated paired selector benchmark, matching output hashes;
  retain ordinary/control scale behavior. Naming/style and independent review.
- [x] Integrate verified checkpoint; full dynamic/static regression and affected
  sanitizers; record exact evidence, documentation and commit.

This addresses resource/performance requirements only. Conditional mux structure,
forcing/polarity/operating rules and other recorded runtime, coverage, vendor and
platform requirements remain open.

RED: mux-resource-red-cli.exit1 records all9 rejected expectations failing;
netlist.exit1 accepts excessive sources and string codes; tap.exit1 accepts the
combined budget+1. Existing graph semantics tests pass before the optimization.
GREEN: main50 passes12.439s, native netlist/TAP and connected flatten/state/
scheduler/address pass. Naming/style pass and independent source/caller review
reports no consequential findings. Four reviewed files are SHA-backed up under
/tmp/mux-resource-work-task65-files before root integration.

Three interleaved source/flat runs per variant against9e602dd preserve output and
flat hashes. build/mux-resource-scale/comparison.json records exact commands,
platform/compiler, binary/source/input hashes and RSS.65536output/1024selector
(169 ICL bytes): changed0.1146–0.1652s versus1.8713–1.9725s baseline;4096selector
same output: changed0.1144–0.1146s versus8.9460–8.9966s. Changed peak52728KiB.
Ordinary655360 source references changed0.2148–0.2656s versus0.2149–0.2151s;
262144-bit control changed0.5160–0.6166s versus0.5661–0.6166s, peak126512KiB.
These are measured workloads, not a worst-case speed or memory guarantee.
Root integration begins from verified64 commit9e602dd; full and affected sanitizer
checks remain pending. Other original completion requirements remain open.

Initial full build/mux-resource-final-full.exit2 (845.420s) stops on optimized
multiple-TAP16/32 solver timeouts; most dynamic/static checks continue, but this
is not a full pass. Quiet focused rerun mux-resource-confirmed-multiple-tap.exit0
passes22 in31.038s, including both affected scale tests. Initial leak-enabled
scheduler.exit1 hits the existing2s primary-input solver deadline in a no-mux
1024-bit case. Remaining native5/main50/clock41/client18/reset12 pass under
mux-resource-final-sanitizer-*. Preserve all failed logs. Confirm scheduler at
quiet load, then complete full regression serially without relaxing deadlines.

Final scope additionally fixes the shared Resolve target-width budget, not just
mux precharging: an unsized operand could otherwise pad beyond max_bits and
underflow a OneHotScanGroup remaining counter. Direct native width4/max3 RED
accepts it; width4/max4 remains accepted; preflight before parsing gives GREEN0
and naming0. The final checkpoint includes this guard.

Verification exposed a separate primary-solver performance defect. The linked
[primary objective/vector plan](2026-11-14-primary-objective-width.md) supplies
necessary slice/chunk fixes, with diagnostic ablation and legal-scale evidence;
no deadline changes. Optimized primary3 runs, symbolic concrete/held/shift, main50,
merge, mixed-I/O and data-reservation focused checks pass. First naming required
braces around a wrapped loop; confirmed naming0. All8 selected source/test files
are SHA-backed up under/tmp/mux-resource-primary-task65-files before the final
sanitizer7/full serial runs.66 source/main/native/probe draft stays isolated.

Final integration: make-j1check passes in build/mux-resource-primary-confirmed-full
.log/.exit0,1641.867s wall. Dynamic/static generator checks,51 directional goldens,
2523 ICL334/119 rules, LSP62 and editor6/216.032s pass; optional ANTLR5 skipped.
Leak-enabled native7 and main50/clock41/client18/reset12 pass under
build/mux-resource-primary-final-sanitizer-*. All8 SHA-256 values match the reviewed
checkpoint. Naming/style and independent source/test review pass. Initial failed
runs remain preserved. Strict coverage/vendor certification/platform refresh and
previously recorded runtime/conformance requirements remain open.
