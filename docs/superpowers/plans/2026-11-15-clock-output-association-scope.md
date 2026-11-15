# Differential output association scope implementation plan

Execute inline; Root Tasks85–87 remain immutable during sanitizer/full checks.

Goal: validate ToClockPort DifferentialInvOf scope after flat origin restoration, preserving existing same-owner positive ToClockPort conversion associations.

Authority: IEEE1687 6.3.12(e)/concat_clock_signal and6.4.6.20 allow clock/data conversion associations across hierarchy. Ordinary signal rules require local terminals or direct-child output terminals and raw aliases owned by the expression scope. 6.4.6.19(b)'s positive ClockPort-only requirement belongs to ClockPort input pairs, not ToClockPort conversion. Tessent2026.2's existing27 differential probes accept direct same-kind local pairs and reject tested cross-kind/aliases; keep these dialect differences explicit. v2023.4 IJTAG How to Influence ICL Extraction Through Commands describes representative/associated input clock extraction; it does not supply a contrary output-scope rule.

Ruling: retain current locally named positive ToClockPort pair behavior and alias/slice forms. Existing native TestOrdinaryOutputSourceScope explicitly pins this conversion behavior; qualified direct same-kind vendor pair also loads. Narrow this existing exemption to that owner/type context instead of bypassing every scope check on every ToClockPort differential reference. No new universal positive-ToClock type restriction; generic clock/data alternatives stay as before.

Reach: shared src/pattern/pattern_netlist.cc CheckSignalSources; existing tests/src/pattern_netlist_test.cc; existing tests/src/test_icl_pattern_generation_main.py atomic failures; guide/model ledger/this plan. No change to SystemClocks differential execution restrictions or Source formats.

- [x] RED forged-flat child-input/parent/sibling/deep-output/nonlocal-alias references; CLI atomic PDL/reset/debug preservation.
- [x] Positive same-owner ToClock pairs including vector aliases/slices/inversion, normal direct-child positive output and generic input conversions preserve canonical replay.
- [x] Restrict the existing DifferentialInvOf skip to the same-owner positive-ToClock terminal case after raw-alias ownership checks; ClockPort input pair validation stays unchanged.
- [x] Focused/connected checks, independent review, representative scope loading measurements; no redundant broad scan or new cache.
- [x] Advance only after verified Tasks85–87 commit, then required immutable leak/full and source-matched remote gate.

Review focus: restored versus source scopes; same-owner pair alias path; nonlocal raw aliases resolving otherwise eligible terminals; partial/vector associations; preservation of original ordinary clock/data kind checks. Existing terminal memoizer keeps work linear in references plus resolved alias links.

Independent design/test review supports the narrow scope exemption. Corrected original deep spelling to its existing parser diagnostic (icl_parser.yy signal_reference depth>1); AddIcl is now within Fails. Restored-flat deep output exercises the new source-scope guard. RED build0/native1 expected missing reference-scope rejection, CLI1 all3 childinput/deepoutput/nonlocalalias cases invalidaccepted0. Actual3 source/test hashes match; no new production before RED.

Focused remote build/native/main69/clocks41 all0; actual3 hashes match. Independent final source/test review clean. Loading60 all0 with equal PDL/flat hashes and actual changed netlist hash matched; wide/local/alias/scoped/child/ordinary figures are recorded in the guide. Broad remote native7/all41 with final10source/test hashes is running. Root Task85–87 source/test3 remains frozen; its required leak checks all0 and full serial check is in editor tests.

Advanced onto verified464b62f; previous14draft files/full diff/manifest and stash retained under/tmp/clock-output-control-tasks88-89-before-advance-files. Final87 docs retained; onlyscope section restored. Combined snapshot source/tests freeze for required final leak/full verification after localconnected/focused/style/offline gates.

Final combined88/89 remote integration passes50 statuses: build/native7/all41CLI/aggregate all0, with actual12source/test hashes matching the submitted immutable snapshot. Required local leak-enabled build/native7/main69/clock41/client24/reset12 all0. Full serial regression remains running; no production/test/case/certificate changes during these final gates. Evidence: build/clock-output-control-final-remote/evidence and build/clock-output-control-final-{sanitizer-*,full.*}.

Verified Tasks88/89 checkpoint: full serial make check exits0 in1381.145s,
41 dynamic and41 static generator suites,53 directional goldens,2525 ICL files
with334/334 grammar and119/119 scanner rules,LSP62 and editor6 in174.760s.
Five optional ANTLR tests remain skipped. Required leak-enabled build/native7/
main69/clock41/client24/reset12 all0. Final remote50 statuses all0 and actual12
source/test hashes match. All26 production/test/case/certificate files remain
unchanged from independent review through final gates. Commit this31-file
checkpoint without push; Task90 remains isolated. Broader model, vendor, strict
coverage and final platform requirements remain mandatory.
