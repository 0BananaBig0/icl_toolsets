# IR selector source scope implementation plan

Draft follows the verified79/80 source-context batch. Independent primary review
confirms6.4.11(d) permits ToIRSelectPort selection without a same-module limit;
generic6.3.10.5 hardware boundaries still apply. Retain local ToIRSelect producers
and direct-child output producers, while rejecting restored parent/sibling/deep
outputs and raw aliases owned by another scope. Keep existing one-bit/two-input
ScanMux eligibility and PDL alias behavior. This is wiring validation, not full
controller/interface association or active-chain qualification under6.4.6.23(c).

Move the existing ToIRSelect local exception below raw-alias ownership validation
and condition it on producer.scope == reference_scope. Other ToIRSelect outputs
use the existing output-parent predicate. No new graph pass, allocation or API.

Reach: existing shared netlist check, native netlist and main CLI tests, current
guide/model ledger/this plan; necessary demonstrated fixtures only. No RTL.

- [x] RED local/direct-child/alias positives with original/flat replay; altered-
  flat parent/sibling/grandchild and raw hierarchical alias negatives.
- [x] Isolate unused controller owners from root scan-client physical controls
  for CLI source/flat atomic-artifact verification; record unsupported-context
  pilots separately rather than mislabeling them missing-scope acceptance.
- [x] Watch missing scope checks fail before the minimal predicate/ordering edit.
- [x] GREEN with earlier selector-kind/width diagnostics, independent review,
  representative loading, style and required immutable leak/full checks.

Ruling: share the next verified source-boundary snapshot with Task81 lowered
ToTCK Source validation; preserve separate RED/GREEN and review for both and one
required final regression. Current79/80 source/tests remain frozen in Root.

Before RED, independent test review corrected a vector LogicSignal fixture to
two scalar signals concatenated by Alias, and matched actual ScanMux flat header
syntax (SelectedBy ... { rather than semicolon). No production changed before
RED. Remote build0/native1 with both missing rejections; CLI1 all4 invalid models
accepted0. Actual3 source/test hashes match submitted snapshot. Only afterward
added ToTCK Source to Family::kClock and moved/narrowed localToIR exception below
rawAlias validation. Independent final source/tests review clean. RemoteGREEN
native/fullmain67/clocks41 running; Root79/80 remains frozen through its full run.

Remote GREEN build0/native netlist0/main67 0/clock41 0. Actual4 source/test
hashes match. No producer fixture repair needed beyond the two pre-RED test
setup corrections. Broad remote native7/all41 running; no local CPU tests
while Root79/80 full regression runs. Planned paired loading:262144-bit lowered
ToTCK;4096 aliases;4096 shared ToTCK outputs;10000 shared IR selectors using
qualified child TMS owners; unchanged655360 ordinary refs. Empty PDL only.

Local connected native7/main67/clocks41/client18/reset12/multi22/feedback10 all0;
format/naming0; independent final review clean. Broad remote build/native7/all41
CLI/aggregate all0,50 status files checked and4 source/test hashesmatch (the
clock fixture is unchanged from79/80 but included in the connected snapshot).
Paired loading60 runs all0/equal output hashes versusverified79/80:262144ToTCK
bits .05–.10s vs .04–.08;4096 aliases .06–.08 both;4096 shared ToTCKoutputs
.07–.10 vs .06–.09;10000 shared IR selectors .23–.32 vs .22–.34;655360ordinary
refs .11–.16 vs .13–.17. Loading only, no speedup or controller-runtime claim.
Environment/commands/input/binary/source hashes/wall/RSS are in
build/lowered-source-boundaries-loading-scale/comparison.json.

Tasks79/80 committedfa6b24b afterfull0 1335.271s/editor174.677s andallrequired
leak0/remote7/41 actual4match. Saved11draftfiles/full diff/manifest under
/tmp/lowered-source-boundaries-tasks81-82-before-advance-files andretainedstash.
Advanced ontoverified79/80; restoredonly3 nextsource/tests,2 nextplans andfuture
TCK probe. Final79/80 code/docs retained. Future default-clock probe is excluded
from this snapshot. Required immutable leak/full checks now pending.

Full0 wall1338.568109s/editor6 174.998s; dynamic41/static41,51directional,
2523ICL334/119,LSP62,optionalANTLR5skip. Reviewed3 source/test hashes unchanged;
requiredleak0; all50 remote statuses0 andactual4match. Tasks81/82 complete in
this shared snapshot; implied-clock probe/future code are excluded.
