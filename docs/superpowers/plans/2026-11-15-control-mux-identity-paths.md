# Possible capture/update/TMS identity paths

> Execute inline under existing autonomous authorization. Root combined72/73
> remains frozen through its full regression; only the isolated checkout contains
> this next draft. Preserve failing-first checks and independent final review.

Goal: require a possible non-inverted matching typed path for consumed CaptureEn/
ToCaptureEn, UpdateEn/ToUpdateEn and TMS/ToTMS DataMux bits with matching typed
ancestry. This is a derived static necessary condition for the transparent normal
path, not an explicit Reset-like polarity clause or complete gating conformance.

Primary: IEEE1687 6.4.6.4(a,b)/.5(a,b), printed84–85; .9(c,d)/.10(c,d),91–92;
.15(c,d)/.16(d,e),99–100. They require sensitizing the corresponding enable/TMS
path.6.3.12(d) and data_signal grammar permit inversion at mux inputs, so preserve
net alias/edge/output cancellation. Do not impose a raw-arm inversion ban.

Approaches: per-consumer graph traversal repeats shared work; a raw inversion ban
rejects cancellation permitted by data expressions. Extend the existing bounded
uint16_t origin masks with three polarity pairs512/1024,2048/4096,8192/16384;
move unknown to32768. Inversion swaps all pairs, and consumers require their
non-inverted matching bit. Keep separate typed families and existing Reset/TRST
behavior. No wider allocation, new traversal, public API, metadata or dependency.

Scope: src/pattern/pattern_netlist.cc; existing native netlist/main CLI tests;
current generator support ledgers. CE/UE-only networks must reach this check
before the old !intercepted_control_mux return, without activating Reset/TMS/TRST
shape rules for CE/UE. Reuse the existing consumer scan, seed shape work only for
its old families, and retain prior structural diagnostic precedence.

Boundary: ports, storage, logic and groups are opaque terminals; ordinary forcing
branches do not supply typed identity. Preserve the unresolved all-data-enable
consumer scope and unused mux behavior. A passing cancellation case may invert
a forcing-zero branch to one: identity alone does not prove suppression, associated
ownership, reachability, selector justification or operational conformance.

Review focus: CE/UE-only early return; cross-family flags/sentinel; nested and
alias/output cancellation; parent versus child-output producers; shared consumers
and vector positions. Independent primary/design review supports this narrow
necessary condition and these qualifications.

- [x] Add native and CLI RED odd-net paths in three families/input+To* contexts,
  parent/child producer forms, alias/vector/shared cases; preserve cancellation,
  all-data enable scope and source/flat positives plus atomic three-artifact checks.
- [x] Extend existing uint16_t masks and shared consumer pass, retaining shape
  triggers and diagnostics. Run focused/connected GREEN; repair necessary callers
  without weakening assertions or deadlines.
- [x] Qualify vendor loading differences with matched hashes; measure wide/shared/
  deep/ordinary source/flat loading versus the retained combined72/73 binary.
- [x] Style/naming and independent review; immutable integration, full serial and
  leak checks, current support ledger update and verified commit after72/73.

Prior evidence: vendor70 accepts inverted CE/UE/TMS mux outputs. Record that as a
divergence from this derived normal-path interpretation; do not manufacture vendor
PDL equivalence. New loading probes run separately from frozen Root verification.

Remote RED build0/native1/CLI1 is reproduced against the frozen combined72/73
source, with actual3 source/test hashes matching submitted inputs. First GREEN
build0/main59 pass; one older canonical inverse-arm fixture needs balanced output
parity. All original canonical rows, alias orders and source/flat checks remain;
only inverse cases receive complemented output. This establishes identity only,
with suppression explicitly separate. GREEN2 native/main59 pass, matched hashes.
Independent current code/test review finds no consequential issue, including the
CE/UE-only early-return repair, unqueued shape state, masks/sentinel and scoped
all-data/unused behavior.72 matched vendor models all load, including odd paths;
this is a recorded divergence from the derived transparent-normal-path condition,
not vendor PDL or suppression proof. Remote interleaved scale comparison now runs
before broader remote checks. Root combined72/73 remains frozen in full serial
regression after all required leak checks passed.

Combined72/73 committed asbd99b48 after full serial0 (1341.862s/editor6 175.000s),
all required leak checks and final3 source/test hash identity. Root is clean. The
Task74 draft was saved with full diff/hashes under/tmp/control-mux-identity-task74-
before-advance-files and a retained stash, then advanced onto bd99b48 on
 task/control-mux-identity-paths. Only3 source/tests plus this plan restored; final
72/73 docs are intact. Its baseline CLI is retained for local scale comparison.

All72 remote interleaved source/flat loading comparisons pass with identical
pattern/flat hashes and source matching GREEN2.65536 bits in each of3 families:
0.418–0.468s, max82928KiB versus83096KiB. CE-only262144 bits0.569–0.620s,
max103732KiB both;4096 shared capture gates0.268–0.319s;4096 aliases0.118s;
ordinary655360 references0.168s; jointReset/TRST1.072–1.221s. Whole empty-PDL
CLI/debug measurements, not isolated cost or speedup; exact commands, environment,
compiler/source/binary/input hashes/time/RSS are in the retained remote scale
archive. Broader remote native7/dynamic41 and combined local connected checks
are running. Local scale/style and required full/leak integration remain.

Current local connected native7/main59/clock41/client18/reset12/multiple-TAP22/
feedback10 all pass on bd99b48-based draft. Local72 interleaved loading comparison
now runs before final style and immutable integration. Root remains clean.

Final local72 interleaved source/flat comparisons pass output/flat hashes versus
bd99b48:3x65536 control bits0.265–0.266s,max82564KiB versus82668KiB; CE-only262144
bits0.315–0.366s,max103380KiB versus103420KiB;4096 shared capture gates0.215–0.266s;
4096 aliases0.115–0.165s;ordinary655360 refs0.115–0.166s;jointReset/TRST0.567–0.618s.
Exact commands, platform/compiler, source/binary/input hashes, repetitions/time/RSS
are in build/control-mux-identity-loading-scale/comparison.json. These end-to-end
empty-PDL/debug measurements do not claim an isolated cost or speedup. Current
local connected native7/main59/clock41/client18/reset12/multiple-TAP22/feedback10,
style/naming and independent code/test review pass. Freeze the reviewed increment
for required leak/full integration; broader remote current-source checks remain
supplementary and must be labeled by their actual snapshot.

Final required leak-enabled ASan/UBSan native7/main59/clock41/client18/reset12 passes,
aggregate build/control-mux-identity-final-sanitizer-all.exit0. All6 root files match
the immutable reviewed snapshot before full serial make-j1check begins in
build/control-mux-identity-final-full.log. Production/tests stay frozen; further
draft work remains isolated.

Final full serial exits0,1345.806595s,41 dynamic/static generator suites,
51 directional goldens,2523 ICL334/119 rules,LSP62,editor6 in174.686s. Optional
ANTLR5 skipped. All3 final production/test hashes match immutable review. All
required leak/connected/performance/style/review checks pass. Current-source
remote native7/dynamic41 and completed archive pass with matched hashes; original
transport status is retained independently. Commit this verified74 increment,
retaining all broader requirements and derived-identity qualifications.
