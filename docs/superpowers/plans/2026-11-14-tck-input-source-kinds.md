# Child TCK input-binding terminal kinds

Execute inline under existing autonomous authorization. Preserve Task55 root
production files during verification; work in the existing isolated checkout.

IEEE1687-2014 §6.4.7(i) requires concat_tck_signal for child TCKPort input
bindings. §6.3.12 permits TCKPort/ToTCKPort terminals, their aliases, ordinary
data signals and constants. General ClockPort/ToClockPort/ClockMux are outside
that terminal union. Independent conceptual review confirms these rules.

Split TCK terminals from the existing clock category in the shared memoized
classifier. Clock consumers continue accepting both categories. Validate Source
on TCKPort nodes against data/constants/TCK only. Existing explicit ToTCKPort
Source handling is a lowered-model extension (the IEEE declaration grammar has
attributes only); preserve that facility without claiming normative validation
of its Source syntax. Do not change phase/polarity, hierarchy direction or
contextual DataMux rules in this increment.

Connected files: pattern_netlist.cc, existing native netlist and main CLI tests,
guide/model ledger. No new runner, dependency or representation. Keep one
iterative alias classifier and the existing scale checks.

- [x] Read grammar and obtain independent conceptual review.
- [x] RED native/CLI invalid terminal and alias cases; positive data/TCK/constants
  and flat replay, extension compatibility and artifact preservation.
- [x] Extend shared classification; focused native/main/clock tests and review.
- [x] Integrate after Task55 verification/commit, full dynamic/static and selected
  leak checks; document evidence and commit. Broader signal conformance stays open.

CLI RED proves a ClockPort source was incorrectly accepted. Final main39 methods
pass in10.531s; clock41 passes in12.522s. Native positive/negative/alias/flat/constant
cases pass, including preservation of the ToTCKPort Source extension. Existing
4096 clock-alias and65536-bit/4096-consumer cases take0.0178444s/.0286966s.
Formatting/naming and independent source review pass. No Task56 root import yet.

A later client18 check found two existing direct ClockMux-to-TCK binding fixtures.
Keep the source-kind rule; preserve their original gate behavior through an explicit
ToTCKPort Source forwarding node, the already supported lowered-model extension.
This adds tests/src/pattern/test_client_scan_controls.py to the connected scope.
The original client failure log is retained as reset-source-client.exit1 (found
during Task57 checks, caused by Task56's stricter binding check). Corrected18-method
run and independent fixture review pending. The four-file Task56 backup includes
this fixture correction; its original three product/test files remain unchanged.

First full integration fails in the native scheduler's corresponding direct
ClockMux-to-TCK fixture (tck-source-final-full.exit2); sanitizer scheduler likewise
fails (exit1). Preserve both logs. Apply the same explicit ToTCKPort Source
extension forwarding to TestStandaloneIdleClockGates, retaining all0/1/X gate
assertions. This adds tests/src/pattern_scheduler_test.cc to the connected scope
and fifth-file Task56 backup. Root focused native and confirmed sanitizers are
running before confirmed full integration. No product exemption is introduced.

Confirmed full integration passes in build/tck-source-confirmed-full.log/.exit0,
964.274s wall:41 dynamic/static suites,51 directional/seven legacy goldens,2523
ICL334/119 rules and editor6 in182.718s; five optional ANTLR tests skipped.
Leak-enabled native scheduler/flatten/netlist and main39/clock41/client18/reset12
checks pass in build/tck-source-confirmed-sanitizer-*. Original full exit2 and
scheduler sanitizer exit1 are preserved. Source/fixture reviews, naming and style
pass; no kind exemption or solver-budget change was introduced. Strict coverage,
new vendor certification and later platform qualification remain open.
