# Reset source terminal eligibility

Execute inline under existing autonomous authorization. Keep Task55 root files
frozen; prepare this increment after Task56 in the isolated worktree. Preserve
independent source review and each increment's verified backup.

IEEE1687-2014 §6.4.7(m) and §6.4.6.14 use concat_reset_signal for child ResetPort
bindings and ToResetPort Source. Accept reset-family terminals, ordinary data,
constants and aliases. Preserve existing TRSTPort-driven reset acceptance:
§6.3.12(b) omits local TRSTPort, whereas §6.4.6.13(a) expressly discusses it.
Do not invent a categorical rejection while that discrepancy remains unqualified.
ToTRSTPort is expressly listed as an eligible child output.

Extend the shared classifier with reset/TRST/global-reset categories. Permit
inferred kGlobalReset Source connections; retain kAsyncGlobalReset separately
for later TRST-source validation. Keep register node.reset association semantics
unchanged. No generated/name exemptions. Other consumer families must retain
their current eligibility. This is terminal-kind eligibility only: DataMux reset
forcing/selection rules, polarity, direction and serial reset behavior remain
separate acceptance work.

Connected files: pattern_netlist.cc and existing native/main CLI tests, guide/model
ledger. No new runner, dependency or graph walk; retain cached alias-bit lookup.

- [x] Inspect normative consumers, inference and virtual references; independent review.
- [x] RED invalid unrelated signal kinds, aliases and output atomicity; positive
  reset/TRST/data/constant unions, source/flat and inferred-global cases.
- [x] Implement classification and consumer checks; focused checks and review.
- [x] Integrate after Task56 passes/commit; full dynamic/static and selected leak
  checks, document exact evidence and commit. No broad reset-conformance claim.

Confirmed CLI RED2 accepts an invalid TCK source before implementation. First RED
fixture had two top modules; specifying Network corrects that setup. Main40 passes
in10.020s and native source/flat reset/TRST/data/global cases pass. First inferred
fixture formed an existing ResetPort/ToResetPort cycle; putting ToResetPort at
the root makes the positive global-reset case legal, with no product change.
Naming and independent source review pass. Client18/reset-preview12 checks running.
Tessent2026.2 loads and analyzes direct TRSTPort-to-ResetPort and ToResetPort
connections in the isolated probe and exits0. Its exercised PDL is only a clock
wait; this supports current loading compatibility, not functional reset equivalence.
Evidence retrieval pending. Three Task57 files are hash-backed up under
/tmp/reset-source-task57-files; Task56's separate verified backup is preserved.

First integration fails on the native edge-snapshot test's direct TCK-to-ResetPort
ICL binding; first sanitizer scheduler also fails. Preserve reset-source-final-*
failures. That engine-only test now elaborates a legal constant-reset circuit, then
explicitly constructs its synthetic lowered TCK-derived reset edge before scheduler
creation. Both rising/falling snapshot/priority assertions remain unchanged.
This adds tests/src/pattern_scheduler_test.cc to Task57's connected scope/fourth
backup file. It does not introduce a frontend exception or claim IEEE eligibility
for direct TCK reset sources. Focused native and confirmed leak checks are running.

Confirmed full integration passes in build/reset-source-confirmed-full.log/.exit0,
955.735s wall:41 dynamic/static suites,51 directional/seven legacy goldens,2523
ICL334/119 rules and editor6 in182.967s; five optional ANTLR tests skipped.
Leak-enabled native scheduler/flatten/netlist and main40/clock41/client18/reset12
checks pass in build/reset-source-confirmed-sanitizer-*. First full exit2 and
scheduler sanitizer exit1 remain recorded. Corrected source/fixture reviews,
naming/style and focused checks pass. No frontend exemption is introduced.
Strict coverage, formal vendor certification and platform refresh remain open.
