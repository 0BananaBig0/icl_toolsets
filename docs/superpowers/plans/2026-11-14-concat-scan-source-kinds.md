# Concat scan source terminal eligibility implementation plan

> **For agentic workers:** Use superpowers:executing-plans under the existing
> autonomous inline authorization. Preserve independent review and final checks.

**Goal:** Reject unrelated control and clock terminals in concat-scan connections.

**Architecture:** Extend CheckSignalSources with a scan family using its existing
per-alias-bit cache. Keep ScanRegister in the ordinary-data set: concat_scan_signal
admits ordinary data as well as scan terminals. Stop at legitimate mux/port
terminals; retain existing direct ScanOutPort rightmost and alias-boundary checks.

**Tech Stack:** C++17, existing native and Python unittest suites; no dependency.

**Spec:** [Approved generator requirements](../specs/2026-10-25-icl-pattern-generation-design.md),
illegal connection validation, and the main pattern-completion ledger Task60.
IEEE1687-2014 §6.3.12(c–d), pp.70–71; §6.4.6.2, p.83;
§6.4.7(f), p.111; §6.4.11, p.124. Primary text inspected at
build/combined-audit/ieee1687.txt lines4452–4584,5258–5286,6969–6970,7728–7755.

## Global constraints

Keep root Tasks58/59 source frozen during their integration checks. Implement in
the existing isolated checkout, retaining hash-backed checkpoints. New test
scripts belong under root tests; this task only extends existing scripts.
Do not narrow bare ScanInSource or alias-hidden ScanOutPort rightmost behavior
before separate qualification. Preserve omitted scan-register index selection,
expressly permitted by §6.4.8(u), p.115.
Hierarchy direction, contextual DataMux rules and general scheduler support remain
separate acceptance work. Preserve explicit lowered fixtures where needed.

## Review focus

Per-bit mixed aliases; partial binding holes; ScanRegister middle bits as data;
valid scan terminals remaining invalid in ordinary-data selectors; flat-origin
restoration and atomic output preservation. Cover these in the existing tests.
OneHotScanGroup Port already has the stronger enabled-scan-fan-in check; retain
that rule and its existing tests rather than introduce a weaker replacement.

## Task60

Files: src/pattern/pattern_netlist.cc, tests/src/pattern_netlist_test.cc,
tests/src/test_icl_pattern_generation_main.py, current guide/model/plan ledgers.
Consumes Elaborate/SerializeIcl; produces source-located eligibility diagnostics
without new interfaces or metadata. Existing O(nodes + checked bits + alias cache)
work remains bounded by the current width/graph limits. Reuse native scale cases
(4096 aliases and65536-bit fanout); record measured time without a speedup claim.

- [x] Inspect current classifier/callers, primary grammar and independent design review.
- [x] Add CLI invalid-family cases for ScanOutPort Source, child ScanInPort and
  ScanMux choices; require preserved pattern/reset/debug artifacts. Run RED.
- [x] Add native source/flat positives for scan/data/constants, partial/mixed aliases,
  child outputs, muxes/groups, middle ScanRegister bits in concat data contexts;
  inject virtual reset negatives and check selector isolation.
- [x] Add kScan terminals ScanInPort/ScanOutPort/ScanMux/OneHotScanGroup and family
  checks for Source/choices; retain data/ToIRSelect selector validation.
- [x] Run native/main/flat/TAP/client and scale checks; review source and naming/style.
- [x] Integrate after58/59 commit; run full dynamic/static and affected sanitizer
  checks, update measured evidence and commit. Preserve prior failed evidence.

Focused evidence: concat-scan-source-red-cli.exit1 accepts all24 unrelated-family
cases; native RED exit1 accepts a restored virtual reset. Main44 passes10.398s;
native/flat, TAP9 and client18 pass. Corrected TAP command is recorded separately
(the first invocation used a nonexistent script path and exited2). Independent
source review is clean; its mixed scan/TCK alias recommendation now passes for
allowed and rejected slices, including flat replay. Naming/style/whitespace pass.
Final native0.86s wall,121020KiB RSS;4096 clock aliases0.0194806s;65536-bit/4096
consumer fanout0.0223692s. Three final files are hash-backed up under
/tmp/concat-scan-source-task60-files. Root integration awaits58/59 final results.

Integration ruling: combine60 with the separately qualified bare-scan/alias-tap
Task61 because they share the classifier and source tests. Preserve this verified
checkpoint, separate reviews and RED/GREEN evidence; one combined full dynamic/
static and leak-check snapshot must pass before the increment is complete.

Full combined integration passes in build/scan-sources-final-full.log/.exit0,
1121.542s wall:41 dynamic/static suites,51 directional/seven legacy goldens,
2523 ICL334/119 rules, editor6 in198.149s; five optional ANTLR tests skipped.
Leak-enabled native scheduler/flatten/netlist/state/TAP and main46/clock41/client18/
reset12 checks pass in build/scan-sources-final-sanitizer-*. Final three source/test
hashes match the reviewed Task61 checkpoint. Naming/style and independent source
reviews pass. Strict coverage, formal vendor certification and platform refresh
remain open.
