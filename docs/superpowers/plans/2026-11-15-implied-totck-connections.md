# Source-less ToTCK ownership and forwarding implementation plan

Execute inline after verified81/82; current Root snapshot remains immutable.

IEEE1687 6.4.6.18(a,b) requires an owning TCKPort and same-polarity forwarding.
6.4.6.17(a) declares multiple local TCK definitions equivalent. Infer real Source
references from one local scalar/indexed-scalar representative; broadcast it to
all output bits. Multiple scalar declarations use a representative without
inventing an ordinal relationship. A scalar remains usable alongside vector
inputs. If every local TCK declaration is vector-valued, report unsupported
implicit mapping rather than claim the declarations invalid or guess a bit.
Existing explicit lowered Source properties retain behavior, including ownerless
ClockPort and inversion examples. Do not change root TAP scalar restrictions.

Vendor34 actual model/runner pairs match build/totck-association-vendor-probe/
comparison.json. All34 load, including missing owners. Unique scalar/indexed
TCK supplies each output bit in both fan-in query APIs; vector-only, mixed and
multiple-input cases expose no inferred fan-in. Scalar broadcast is vendor-
qualified; multiple-scalar/mixed representative inference is standards-derived.
Missing-owner rejection follows the stricter explicit IEEE clause. No vendor
pattern/waveform equivalence is claimed. Query command Context/Mode checked in
v2023.4 Reference Manual; actual execution is Tessent2026.2.

Reuse fresh ValidateInterfaces per-scope port map after RestoreFlatOrigins.
Find one scalar per TCK scope once and move its ID to the vector front; do not
rescan all local inputs for every output. In the existing node loop infer only
missing ToTCK Source buffers. Existing cycle/source checks then validate them.
Aliases stop at ports, so their terminal cache remains valid. New and legacy
source-less flat dumps use restored ownership; no provenance exemption/version.

Share one4M newly inferred control-reference allowance with Reset/TRST/TMS.
Original inference preflights both early and delayed TCK buffers before any new
allocation, debits early buffers, and fresh-scope TCK preflight debits delayed
buffers. Restored already-explicit controls do not consume new-inference quota.
No new whole-graph pass, dependency or public flag. Existing16M graph bound is
later and cannot replace this allocation preflight.

Reach: shared netlist elaboration; existing native netlist/state/symbolic tests
and main CLI atomic/flat tests; tracked vendor probe under tests/src/pattern;
guide/model ledger/this plan. Necessary demonstrated fixture repairs only.

- [x] RED owner/ambiguity guards and concrete/symbolic forwarding before code.
- [x] Scalar/indexed/vector-output broadcasts, multiple equivalent and mixed
  owners, child paths, original/flat and source-less legacy replay.
- [x] Inferred-edge cycle, scalar selection scale, combined shared-allocation
  boundary, explicit-source and existing inversion/gating diagnostics preserved.
- [x] Minimal shared-index/budget implementation; focused/connected GREEN,
  independent review and representative loading/runtime measurements.
- [x] Integrate after81/82 verification; required immutable leak/full checks,
  exact evidence and remaining requirements; commit without pushing.

Independent design review supports the representative/ambiguity split and fresh
indexing. It requires once-per-scope selection and genuinely shared accounting,
not separate quotas that merely reuse a numeric constant.

RED build0/native1 (missingowner/sharedquota), state1 (missing Source map::at),
symbolic1 (unknown rather than known forwarding), CLI1 bothinvalidaccepted0.
Actual5 source/test hashes match. Added hierarchical source-less legacy case after
independent test review before RED. Production added only afterward.

First GREEN: build0/state0/symbolic0/clocks41 0; native1 andmain1 expose unrelated
wrong-kind ToTCK fixtures missing owners. Added local TCK declarations without
changing intended data-kind diagnostics. GREEN2 build0/native0/state0/symbolic0/
main68 0/clocks41 0. Shared65×65536 preflight .000203s and TCK-only .000157s.
First failed fixtures retained. Independent source/accounting review clean.

Ruling: include a measured symbolic repeated-source cost in this feature. A
512/8192/65536-bit normal/inverted profile with3s proof and20s command bounds
timed out124 during65536;512 cases .014/.013s and8192 cases2.815/2.658s. Actual3
hashes match; no budget widened. Extend existing run grouping with bitvector
repeat for identical finite-node/index/inversion refs, retaining range checks,
known masks, inversion and serial state. This prevents one concat per repeated
bit. Identical bounded GREEN benchmark and independent review pending.
Generic fetch driver initially expected native/CLI logs; corrected scale-specific
reader after preserving driver1, transport0 and extracted actual baseline data.

Repeat GREEN same20s/3s bounds build0/scale0, actual3 hashesmatch:512 .00195/
.00167s;8192 .02488/.03262s;65536 .77981/1.13677s, allunsat. Baselinewhole
20s timed out during65536 after8192 .2.815/2.658s. Independent repeat-source
review clean; requestedmixed repeat/descending/inversion boundary comparison
added toexisting held/shift test against concatenatedindividual Reads.

Tasks81/82 committed68d62ae afterfull0 1338.568s/editor174.998s, allrequired
leak0 andremote7/41 actual4match. Nextdraft willadvance tothis verifiedbase.

Final local connected: symbolic/netlist/flatten/state/address/scheduler andmain68/
clocks41/client18/reset12/multi22/feedback10 all0; initialTAP1 exposed invalidhost
negativecontext. AddedlocalownerTCK only; focusedTAP0 retains intendedinterface
error. Newparent/child/sibling ownernegatives focusednetlist0. Format/naming0.
Independent finalincremental review clean. Earlierremote build/native6/all41CLI0,
TAP1/aggregate1 only; actual8 checked. Final9-file broad run includesTAP repair.

Loading60 runs all0, equal PDL outputs; canonical flat replay within eachvariant.
Expectedflatdifferences occur only fornewdefault connections.65536-bit default
.09–.21s vs .02–.03 unconnectedbaseline;4096 vectorcandidates+scalar+4096outputs
.16–.20 vs .15–.18;3000scopeddefaults .18–.26 vs .16–.21;explicit65536vector
.03–.08 vs .03–.07;ordinary655360refs .12–.13 vs .11–.13. These aredifferent
models onlywhere expected normalization addsSource fields, not speedup evidence.
Inputs/commands/environment/binary/source hashes/wall/RSS and expectedchanges are
in build/implied-totck-loading-scale/comparison.json.

Saved12draftfiles/full diff/manifest under/tmp/implied-totck-tasks83-84-before-
advance-files andretainedstash; advancedonto68d62ae, restoringonlyfuturecode/
tests/probe/plan. Final81/82 docs retained. Requiredimmutable leak/full begins;
no source/test changes inRoot until verification ends.

Final corrected remote run:50 statuses, build/native7/all41CLI/aggregate all0; actual9 source/test hashes match. Required immutable leak-enabled build/native7/main68/clock41/client18/reset12 all0. Full serial check remains running; source/test8 hashes still match reviewed snapshot11. Do not mark completion until full exit evidence is collected.


Verified checkpoint: required leak-enabled build/native7/main68/clock41/client18/reset12 all pass. Full serial make check exits0 in1346.653s:41 dynamic and41 static generator suites,51 directional goldens,2523 ICL files with334/334 grammar and119/119 scanner rules,LSP62 and editor6 in174.913s; five optional ANTLR tests remain skipped. Final corrected remote build/native7/all41CLI/aggregate all pass, with9 source/test hashes matched. Reviewed source/test8 remain unchanged. Loading60 comparisons and the bounded repeated-source benchmark are recorded above. Evidence: build/implied-totck-final-{full,sanitizer-*}, build/implied-totck-final-remote/evidence, build/implied-totck-loading-scale and build/implied-totck-repeat-{red,green}-remote. Remaining clock/reset/forcing/scheduler/vendor/coverage requirements are not waived.

Task83/84 completion gate met; commit this reviewed snapshot without push. Next literal-run task remains isolated and is not included.
