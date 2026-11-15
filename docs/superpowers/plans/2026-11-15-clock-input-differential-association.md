# Clock input differential association implementation plan

Draft follows Task79 under existing autonomous execution. Independent primary-
source review identifies a narrow, explicit rule in IEEE1687 6.4.6.19(b): a
negative ClockPort input is associated with a positive ClockPort in its own
module. Do not equate the association with its parent Source binding. Do not
require exactly two ClockPort declarations across an entire module: multiple
pairs and unrelated clocks may coexist.

Reuse canonical terminals in CheckSignalSources and the existing scope helper.
For DifferentialInvOf on ClockPort only, require a local positive ClockPort;
preserve aliases and concat/slice/inversion syntax permitted by the grammar.
Retain earlier cycle/width/unsupported-kind diagnostics and raw alias ownership.
No new graph pass, clock traversal, metadata or dependency. Preserve actual
input bindings (including partial bindings, constants and inversion) independently
from the declared pair association.

ToClockPort is a separate rule:6.4.6.20 permits conversion anywhere and gives
concat_clock_signal width/mutual-exclusion rules without the input-port pairing
restriction. Keep its current behavior pending separate qualification. Generic
6.3.12(e) permits aliases; vendor alias rejection alone is not authority to ban
them. The27 matched vendor probes establish only vendor loading behavior:
direct same-kind Clock/ToClock pairs load, other25 failICL59, including aliases.
Two constant rows have no Alias syntax and are marked alias_applied=false.

Reach: src/pattern/pattern_netlist.cc; existing native netlist tests and main CLI
atomic-artifact tests; necessary positive-clock fixture correction; guide/model
ledger/this plan. New scripts remain under tests. No RTL or input-binding change.

- [x] Review explicit rule and RED original/flat own/child/mixed vector matrices.
- [x] Preserve alias parity, slices/reversed ranges, unrelated clock inputs,
  multiple pairs, and partial/bound-negative Source replay.
- [x] Watch non-Clock/missing-scope association acceptance fail tests before code.
- [x] Add narrow input association check; repair demonstrated incorrect input
  association positives while retaining all ordinary Source/mux/binding tests.
- [x] Connected GREEN, independent review, representative loading measurements
  and style.
- [x] Complete the immutable combined79/80 leak/full snapshot after verified78,
  record exact evidence and commit.

Independent primary review agrees with this split; output same-kind/local-only
restrictions and inversion bans are not justified by the cited input clause.

Ruling: final integration/commit shares Task79 immutable snapshot and required
leak/full checks, with separate RED/GREEN tests. This does not defer the input
association requirement or merge output-association ambiguity into it.

Native/CLI draft review clean: local positive aliases, inversion syntax, opposite
vector directions and independent pairs; otherwise-eligible data/TCK/ToClock/mux
negatives; literal/mixed lanes; restored parent/sibling input references. Four CLI
invalid models retain a complete independent scan client and preserve3 artifacts.
Necessary existing fixture repair after RED: keep ordinary Source/mux/binding
positive coverage; only incorrect ClockPort association positives become explicit
negatives. Replace oscillator unsafe differential-input fixture with a local
positive/negative pair in a child whose positive input is driven by the oscillator,
retaining the unsupported-fanout assertion. Do not weaken that clock safety check.

Combined RED1 build0/native1 records both missing guards, but3 CLI subcases
hit later event-integration diagnostics. Preserved pilot and corrected unused
owner declarations into child modules with explicit TMS/TRST bindings. Corrected
RED2 build0/native1 (both missing guards), CLI1 with all7 invalid models accepted0.
Actual3 source/test hashes match submitted snapshot. Production unchanged until
RED2 was inspected. Added shared count/type/scope checks only afterward.
Independent production/tests/necessary fixture review clean; remote native/full
main65/clock41 GREEN running. Root78 source remains frozen for its full run.

Remote GREEN build0/native netlist0/main65 0/clocks41 0, actual4 source/test
hashes match. Legal local input pairs, all owner/altered-flat cases and atomic
artifact rejection pass. Existing clock safety and bound-negative metadata tests
retain their assertions. Broad remote native7/all41 is running. Local CPU tests
wait until Root78 full verification completes; no source/test mutation in Root.

Task78 committeddf746a0 afterfull0 1345.595s/editor175.874s, allrequiredleak0
and finalremote7/41 actual5match. Saved all12draftfiles/full diff/manifest under
/tmp/source-context-associations-task79-80-before-advance-files and retainedstash.
Advanced onto verified78 and restoredonly4 next source/test files and future
plans; final78 code/docs retained. Local connected/performance/style runs serial.

Local connected: native7/main65/clocks41/client18/reset12/multi22/feedback10 all0.
Format/naming0. Independent production/fixture review clean. Loading comparisons
60 source/flat runs all0/equal PDL/flat hashes versus verified78.5000 owners
.25–.31s both;65536-bit owner controls .03–.07 vs .02–.08;65536-bit clock pairs
.03–.04 both;4096 aliases+4096 pairs .11–.18 vs .10–.18;655360 ordinary refs
.14–.17 vs .11–.17. Empty-PDL loading only; no unmeasured speedup or generic TAP
runtime claim. Environment/commands/input/binary/source hashes/wall/RSS are in
build/source-context-associations-loading-scale/comparison.json. Original wide
owner model loaded successfully; no speculative child-model substitution used.

Immutable integration after verified78 begins now; required leak/full/broad
remote completion remains pending. No push requested.

Broad remote build/native7/all41 CLI/aggregate all0, actual4 source/test hashes
match. Retrieved archive50 status files all0 under build/source-context-
associations-connected-remote/evidence. Required leak-enabled native7/main65/
clocks41/client18/reset12 all0, aggregatefinal-sanitizer-all.exit0. Full serial
regression now running. Root4 source/test hashes remain the reviewed snapshot.

Combined79/80 full0 wall1335.270639s/editor6 174.677s; dynamic41/static41,
51directional,2523ICL334/119,LSP62,optionalANTLR5skip. Reviewed4 source/test
hashes unchanged, leak aggregate0, all50 remote status files0 andactual4match.
Tasks79/80 complete together; no Source-less ToTCK or output-association claim.
