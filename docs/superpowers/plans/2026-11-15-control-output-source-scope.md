# Control output source scope

> Execute inline under existing autonomous authorization. Root74 remains frozen
> through full verification. New work stays in the isolated checkout with TDD,
> independent review and no discretionary confirmation loop.

Goal: reject same-expression-scope To* control outputs used by covered hardware
Sources and conditional DataMux control inputs. IEEE1687 6.3.12(b,f–k,d) lists
local input functions and hierarchical To* output alternatives. Stop canonical
resolution at ports; following their Source would hide the invalid reference.

Scope: Reset, TCK, CaptureEn, ShiftEn, UpdateEn, TMS and TRST typed Sources in
the existing family dispatcher, plus conditional control DataMux data choices.
ToTCK Source remains the existing lowered extension. Ordinary data/clock direction,
ToIRSelect, PDL alias declarations and semantic node.reset associations stay outside
this increment. Keep local TRST input to Reset compatibility under6.4.6.13(a).

Approaches: broader ancestor/depth restrictions need more interpretation, and
following output wiring loses the declared port boundary. Reuse the canonical
terminal cache and compare its original scope with the reference expression scope:
parent for input-port Source bindings, destination scope for To* Source and mux
choices. Restore flat scopes first, as already done. Reject only covered To*
terminals with equal scope. No new metadata, array, graph pass or public API.

Reach: src/pattern/pattern_netlist.cc; existing native netlist/main CLI tests;
affected legal-source fixtures and producer wrappers; two inverted-TCK backend
fixtures may need a child Driver exposing the existing ToTCK Source extension.
Preserve exact backend phase messages and pattern/reset/debug artifact assertions.
Inspect actual failing callers; do not weaken or delete tests to pass the guard.

Review focus: binding scope versus alias-definition scope; original scopes after
flat restore; hardware consumption versus PDL aliases/reset associations; implicit
reset/TMS wiring; generated TAP controls and backend phase coverage. Independent
primary/caller review supports this narrow boundary. Generated TAP controls use
DataInPort, and implicit inference selects local inputs/child outputs or virtuals.

- [x] Qualify direct/aliased own-versus-child output references with matched-input
  vendor loading, distinguishing extensions/context errors from operation.
- [x] Add native/CLI RED cases for own outputs in bindings/To* Sources/mux choices,
  aliases/vectors/shared scope, original/flat positives and altered flat Source;
  retain PDL-only aliases, semantic reset metadata and implied wiring positives.
- [x] Add the smallest source-scope guard after existing type/inversion checks,
  only for hardware Source/choice roles. Repair necessary producer fixtures and
  preserve backend phase/artifact coverage. Run focused/connected GREEN.
- [x] Measure wide/deep/shared/ordinary loading versus retained74; style/naming,
  independent review, immutable integration, full serial/leak checks and verified
  commit after74. Source direction beyond this boundary remains mandatory.

Broader current74 remote native7 and official dynamic41 pass with actual3 source/
test hashes matching the submitted reviewed snapshot; completed archive fetched0.
Those checks completed before the75 candidate update.99 matched-input scope
loading probes run separately. The native matrix, alias-only/reset-association
positives and altered-flat Source test precede all75 product changes. Remote RED
runs against frozen74, and root74 full regression remains isolated.

RED native1/CLI1 accepted own outputs, with actual submitted3 hashes matching
frozen74. First GREEN exposes6 older native positive fixtures and the TCK CLI
positive; move their To* producers into children, preserving kind/polarity/alias/
partial/virtual/flat assertions. Three standalone gated/inverted clock fixtures
and the multi-TAP gate fixture use a ClockForward child exposing the existing
lowered ToTCK Source extension. Original PDL, phase messages and artifact checks
remain unchanged. GREEN2 native/main60/client18/multiple-TAP22 pass. Added mixed
own/child vector negatives in both orders and a partial-binding flat positive;
these now receive the final local connected native run.

99 submitted/fetched vendor models/runners match:46 instance hardware cases and
7 alias-only cases load;46 own-scope hardware references failICL130. This isolates
the scope distinction in direct/aliased and mux contexts, not a PDL certificate.
74 committed as86e3da1 after full serial0 (1345.806595s/editor6 174.686s) and all
required leak checks. Root is clean.75 saved with hashes/full diff under/tmp/
control-output-scope-task75-before-advance-files and a retained stash, then
advanced to86e3da1 on task/control-output-source-scope. Only5 source/tests plus
this plan restored; final74 docs retained. Baseline74 executable saved. Current
independent implementation/test review and local connected checks are running.


Final local connected results: native6/main60/clock41/client18/reset12/multi22/
feedback10 passed initially; scheduler correctly rejected its older own-ToTCK
fixture. The identical ClockForward child repair preserves all0/1/x assertions,
passes independent additional review and scheduler0 in115.012s. No product change.
Mixed-vector/partial additions pass. Serial loading72 all0 with equal output/flat
hashes versus86e3da1; earlier overlapping72 are retained but excluded from timing.
Exact evidence is build/control-output-scope-loading-scale-serial/comparison.json.
Seven8192-bit families0.08–0.13s;262144Reset mux0.51–0.57s;4096aliases0.05–0.12s;
4096consumers0.06–0.09s;CE-only2621440.31–0.34s;ordinary6553600.10–0.15s.
Current6-file source/test remote native7/dynamic41 checks are running. Style then
immutable snapshot import, leak and full serial checks remain before commit.


Final style/naming0 and required leak-enabled ASan/UBSan native7/main60/clock41/
client18/reset12 all0; aggregate build/control-output-scope-final-sanitizer-all.exit0.
All6 frozen source/test hashes match the reviewed snapshot. Full serial make-j1
check started20:20:55UTC, build/control-output-scope-final-full.log. Next76 source
scope plan/tests stay isolated; no changes to this verification snapshot.

Final current-source remote native7/official dynamic41 all0; actual6 source/test
hashes match the submitted reviewed snapshot. Completed evidence fetched0 under
build/control-output-scope-connected-remote/evidence. Task76 remote RED begins
only after this immutable archive exists; vendor69 follows RED completion.


Final Task75 verification: required leak-enabled ASan/UBSan native7/main60/clock41/
client18/reset12 all0. Full serial make-j1check0 in1340.802s, recorded in
build/control-output-scope-final-full.log/.exit/.wall:41 dynamic and41 static
suites,51 directional goldens,2523 ICL files with334/119 grammar/scanner rules,
LSP62 and editor6 in175.279s. Optional ANTLR5 skipped. All6 final source/test
hashes match the independently reviewed snapshot. Current-source remote native7/
dynamic41 all0, with actual6 hashes matching and completed archive fetched0.
These results qualify this source-scope increment; the remaining model, strict
coverage, vendor and full platform/static/install requirements stay open.

Final reviewed source/tests are frozen in/tmp/control-output-scope-task75-reviewed-files.
Commit this verified increment; next ordinary-source scope remains isolated.
