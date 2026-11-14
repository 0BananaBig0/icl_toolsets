# Capture, shift and update source terminal eligibility

Execute inline under existing autonomous authorization. Keep Task56 root files
frozen during verification; work after Task57 in the isolated checkout. Preserve
the earlier verified snapshots and independent source review.

IEEE1687-2014 §6.4.7(j–l) and §6.4.6.8–10 define the child input bindings and
To* Source consumers for ShiftEn, CaptureEn and UpdateEn. Each accepts its own
signal family, ordinary data/constants and aliases (§6.3.12(g–i)). Independent
conceptual review confirms that generated TAP controls use ordinary DataInPort
terminals and need no exemption. No new virtual category is needed; virtual reset
references remain outside these families.

Extend the existing cached classifier with the three matching families. Reject
unrelated control, clock, TCK and scan terminals. Keep current data/clock/TCK/reset
and ToIRSelect rules. This is kind eligibility only: capture/update gating rules,
ShiftEn tie semantics, phase, direction and polarity remain separate requirements.

Connected files: pattern_netlist.cc and existing native/main CLI tests, guide/model
ledger; pattern_flatten.cc only if positive bare-port replay exposes its empty-body
encoding. Reuse existing code, no dependency/runner/new graph pass. Keep existing
alias scale checks.

- [x] Verify consumer grammars/defaults/generated controls with independent review.
- [x] RED unrelated-family/alias and atomic-output cases; positive own-family,
  data/constants, partial bindings, source/flat and generated TAP controls.
- [x] Implement shared classifications/checks; fix only connected serialization.
- [x] Focused native/CLI/scale, naming/style and independent source review.
- [x] Integrate after Task57 passes/commit; full dynamic/static and leak checks,
  document evidence and commit. Broader control conformance remains open.

CLI RED accepts an unrelated TCK source. The first native positive fixture had
a scalar Alias for a two-bit output and failed width validation; explicit Alias
range corrects the fixture. A suspected bare-control serializer issue was checked
against the frozen Task56 binary and all three bare-port roundtrips pass. No
serializer change is needed; the speculative one-line change was reverted before
verification. Native positive family/data/constant/partial cases and injected
virtual-reset rejection checks are now building. Only netlist.cc and existing
native/main test files are in the implementation delta.

Focused native source/flat/partial/virtual-reset checks, main41, TAP9 and client18
pass. Naming/style and independent source review are clean. Existing4096 clock
aliases take0.0161897s;65536-bit/4096 consumers take0.0259002s. No serializer
product delta remains (byte comparison with root confirms it). Three Task58 files
are hash-backed up under /tmp/scan-control-source-task58-files; earlier56/57
snapshots remain separate. Root58 integration is pending its verified dependencies.

Integration ruling: combine Tasks58 and59 after Task57's verified commit. Both
focused/source reviews remain required and have been retained separately. They
share the classifier and Source consumers; one final combined dynamic/static
regression and leak-check snapshot will satisfy both plans. Keep the separately
verified58 checkpoint and59 draft backup. This avoids duplicate full regressions
without bypassing review or final integration verification.

Full combined integration passes in build/control-source-final-full.log/.exit0,
1094.986s wall:41 dynamic/static suites,51 directional/seven legacy goldens,
2523 ICL334/119 rules, editor6 in185.044s; five optional ANTLR tests skipped.
Leak-enabled native scheduler/flatten/netlist and main43/clock41/client18/reset12
checks pass in build/control-source-final-sanitizer-*. Final source/test hashes
match the reviewed Task59 checkpoint. Naming/style and independent source/resource
reviews pass. Strict coverage, formal vendor certification and platform refresh
remain open.
