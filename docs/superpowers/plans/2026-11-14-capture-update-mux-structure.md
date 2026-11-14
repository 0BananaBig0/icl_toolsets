# Capture/update gating mux source structure

> Execute inline under existing autonomous authorization using the established
> Superpowers workflow. Retain failing-first tests, independent review and checks.

Goal: validate the static zero-plus-corresponding-control input shape of DataMux
bits carrying CaptureEn/UpdateEn signals, including aliases and mux chains.
Primary IEEE1687-2014 clauses6.4.6.4(a)/.5(a) and6.4.6.9(c)/.10(c) specify
structure; corresponding(b)/(d) operational sensitization remains separate.
Cached primary text: build/combined-audit/ieee1687.txt lines5327–5455,5690 onward.

Interpretation: semantic inputs are distinct per scalar bit, not rows or code
alternatives. Allow repeated references/aliases of one control, repeated zeros,
inverted corresponding control terminals and identity-only control muxes. A real
gate can have only one distinct nonzero control path; other inputs must normalize
to literal zero. ScanRegister/input values currently zero are not constants.
Ports retain identity. DataMux chains must satisfy the same family recursively.
Unused actual-control gates still have structural rules; one observed bit must
not impose a family on an unrelated ordinary unused bit.

Scope ruling: this increment applies where CE/UE provenance exists through mux
data inputs and aliases, including downstream muxes used as matching controls.
All-data mux consumer restrictions remain the documented standard/vendor
interpretation question; do not silently impose vendor-only exclusions on generic
data signals. Full output inversion/polarity and operational sensitization are
separate follow-ups. Reset/TMS/TRST mux rules are outside this increment.

Design: reuse Task64 per-bit context offsets. Memoize alias-resolved BitReference
identities with XOR inversion in the existing classifier cache instead of adding
a second alias walk/cache. Compute CE/UE provenance through DataMux data choices
with an iterative memoized bit DAG walk, stopping at ports/storage/logic. Existing
acyclic graph validation runs first. One-byte provenance state scales with real
bits (at most16MiB); memoized edge visits scale with expanded mux source refs (at most4M after65),
with std::map alias lookups and offset lookups retaining logarithmic costs.
Keep at most one canonical nonzero input per checked output, not a set. Validate
upstream muxes once, preserving source-kind and matching-use diagnostic precedence.
No dependency, public API or serialized metadata change.

Callers/artifacts: src/pattern/pattern_netlist.cc; existing native netlist and main
CLI tests; connected flat/state/TAP/scheduler/control tests and affected fixtures;
this plan, guide/model/main ledger. All new scripts stay under root/tests, mirrored
from src/pattern. Reusable loading probe tests/src/pattern/probe_data_mux_gating.tcl
is drafted in the isolated checkout. Root65 source/test inputs stay frozen while
full/SAN runs; preserve separate reviewed checkpoints before integration.

Qualification: initial36 probes all fail a shared invalid literal Alias fixture;
retain them as setup failures, not semantic evidence. Corrected34 loading probes
have12 accepted controls; boundary18 have6 accepted controls. All submitted/fetched
model and runner hashes match. Good/chain/inverted-control/zero-parameter cases
load. Repeated zero rows, alternatives in one control row and three vector rows
with per-bit two-input shapes load. Nonzero/unknown force, extra control/data and
unused invalid special-control gates reject. Duplicate control rows (even aliases)
rejectICL115; single ordinary-data/zero mux output to To* rejectsICL65. Preserve
these explicit standard-first differences rather than claiming vendor equivalence.
Evidence build/data-mux-gating-{structure,qualified,boundary}-vendor-probe; remote
/tmp/icl-data-mux-gating[-qualified|-boundary]-20261114. No PDL/analysis ran.

- [x] Native/main CLI RED for force1/x, extra data/control, aliases and invalid
  outer chain; pattern/reset/debug output preservation.
- [x] Positive source/flat vector/scalar, repeated canonical sources/zeros,
  inversion cancellation, choice alternatives, identity/unused/mixed data fanout;
  retain ordinary-data scope ruling.
- [x] Implement bounded canonical identity/provenance validation; focused checks.
- [x] Matched-output baseline performance for deep aliases, wide control and
  ordinary muxes; naming/style and independent review.
- [x] Integrate only after65 verified commit, full regression/SAN, docs and commit.

Independent design review confirms the provenance trigger and per-bit scope,
including outer gates and unused actual-control bits. Alias cache definitions must
exclude caller inversion; both lookup orders and double inversion are tested.
RED main14 cases accepted on65 (capture-update-mux-red-cli.exit1); native RED1
accepts force-one. Raw initial vendor fixture errors remain separate.

GREEN native netlist passes3.017s, symbolic0.259s and main51 passes12.000s in
capture-update-mux-green-*. Independent source review found no consequential
canonical cache, provenance or scope defect. Final common padded-target guard and
primary performance fixes are synchronized from65; root8-file65 snapshot remains
frozen for full serial regression. Task66 still needs connected checks, matched
performance, naming/style and its final full/SAN integration after65 commits.

Integrated onto verified65 commit3c9fd30 after connected netlist/flat/state/TAP/
address/scheduler checks pass. Main51/clock41/client18/reset12/multipleTAP22 pass
in14.083/13.197/9.780/26.334/30.470s. Naming initially required braces around a
wrapped zero-input condition; confirmed naming/style pass. Measured and final CLI
binaries are byte-identical after this brace-only adjustment. Four final selected
files hash-match/tmp/capture-update-mux-task66-confirmed-files; independent review
found no source defect. Root is frozen for final sanitizer7/full regression.

Three interleaved source/flat runs against65 preserve both output and flat hashes.
Exact commands/platform/compiler/binary/input/source hashes and RSS are in
build/capture-update-mux-scale/comparison.json.262144-bit CE (264 ICL bytes)
changed0.617–0.767s versus0.617–1.219s baseline, peak135032KiB versus126500KiB.
65536-bit UE changed0.215–0.216s versus0.216–0.265s.4096 aliases (83948 bytes)
changed0.115–0.165s versus0.114–0.165s. Ordinary655360 source references changed
0.115–0.165s versus0.165–0.216s, peak68232KiB versus67192KiB. These measure
workload overhead under concurrent late editor checks, not an optimality/speedup
claim. The canonical-reference cache is larger than the earlier kind-only cache;
map lookups retain logarithmic costs. Provenance uses one byte per real graph bit.

Official leak-enabled ASan/UBSan native7 and main51/clock41/client18/reset12 pass
in build/capture-update-mux-final-sanitizer-*. All4 final hashes still match.
Full make-j1check passes in build/capture-update-mux-final-full.log/.exit0,
1774.796s wall: dynamic/static generator suites,51 directional goldens,2523 ICL
334/119 rules, LSP62 and editor6 in255.589s. Five optional ANTLR tests skipped.
All four final source/test hashes match the independently reviewed checkpoint.
Strict coverage, full vendor certification and refreshed platform checks remain open.
