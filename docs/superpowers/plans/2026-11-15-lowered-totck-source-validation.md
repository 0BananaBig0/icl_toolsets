# Lowered ToTCK Source validation implementation plan

Draft follows the verified Task79/80 snapshot; no current source/test edits.

The project accepts an explicit Source on lowered ToTCKPort models, including
ClockPort/ClockMux/TCK/data sources and inversions. CheckSignalSources currently
omits this role entirely. Extend the existing Source collector with Family::kClock
to reuse terminal-kind, raw-alias and restored hardware-scope checks. Do not use
Family::kTck, which would reject established clock source behavior. No new graph
pass, metadata, public option or dependency. Source-less ToTCK behavior remains
unchanged in this increment.

IEEE1687 6.4.6.18 (printed101–102) defines no Source property. This is validation
of the existing project extension, not IEEE/Tessent qualification. Its owner-TCK,
same-polarity and implicit-equivalence rules require separate work. Preserve
ToTCK extension inversion in TestTypedControlInversion and existing child-clock/
oscillator unsupported-fanout assertions. Independent technical review agrees.

Reach: src/pattern/pattern_netlist.cc, existing native netlist and main CLI tests,
necessary demonstrated fixtures only, guide/model ledger and this plan. New test
scripts belong under tests. No RTL change.

- [x] RED matrices for eligible local/child sources and forbidden kinds, direct/
  alias/inverted/vector paths; original/flat replay; restored child input/deep
  output/raw alias misuse; no-source preservation.
- [x] CLI atomic PDL/reset/debug rejection after isolating physical-control setup.
- [x] Watch missing validation fail tests before one shared collector edit.
- [x] GREEN native/clock safety/CLI and independent review; measure wide/deep/
  shared sources; style and immutable integration under the active batch workflow.
- [x] Required leak/full validation, exact remaining requirements, verified commit.

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
