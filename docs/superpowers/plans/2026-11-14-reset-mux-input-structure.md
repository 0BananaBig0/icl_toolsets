# Reset, TMS and TRST DataMux input structure

> Execute inline under the existing autonomous authorization. Keep root67/68
> source and tests frozen through its final verification; use the isolated
> checkout for subsequent code. Require independent review and failing-first
> checks before implementation. No new agents or workflow approval loop.

Goal: finish the input-shape portion of IEEE1687 clauses6.4.6.13(d), .14(g),
.15(c), .16(d), .21(b) and .22(h), printed96/97/99/100/105/106. The direct-driver
rule is Task67. Direction, polarity/forcing, normal operations and TRST assertion
selection remain separate mandatory requirements; do not claim them here.

Primary source: build/combined-audit/ieee1687.txt. Each rule names a DataMux
with data selection and two data inputs: another data_signal and the matching
reset_signal, tms_signal or trst_signal. Selectors already use ordinary-data
validation. Literal forcing values are legitimate data; Reset forcing values
must not inherit CE/UE's literal-zero-only restriction. Existing qualified
Tessent2026.2 evidence accepts alias and nested reset mux gates. New matched
input/output probes distinguish a semantic input from repeated selector rows,
identity gates, duplicate controls, extra distinct data sources and missing
control paths. Loading is not operational proof; preserve any setup failures
and fetched input/runner hashes.

Proposed design, pending primary/source review and those experiments: reuse
CheckSignalSources' inversion-aware terminal cache, FindMuxControlUses' per-bit
context masks and the bounded iterative provenance traversal. Validate each
consumed control mux bit, including all-data mux outputs directly driving these
ports. Follow upstream mux control provenance without treating ordinary ports,
logic, registers or groups as transparent. Require a corresponding control path and no more than two canonical data-input
identities per bit; do not count duplicate rows as additional hardware signals
without evidence. The data_signal conditional-control exception means the other
input may itself be a corresponding control; do not impose an unproven ordinary-
data-only restriction. Identity gates remain to be qualified.
Check control-bearing nested gates independently; the forcing data branch may
itself be an ordinary data mux. Identify how virtual global/async sources and
TRST-to-reset compatibility map to these contexts. Do not silently widen accepted
signal kinds or infer operation from static shape. Existing graph/expanded-bit
bounds must still cap work; no new public API, metadata or dependency.

Reach: src/pattern/pattern_netlist.cc; existing native netlist source/alias/vector
checks; main CLI source/flat/atomic-artifact tests; affected lawful ICL fixtures
and any exact arbitrary circuits needing explicit native lowering. Inspect
current callers/fixtures, retaining reset values and functional assertions.
New test scripts, if necessary, belong under root/tests mirroring source paths.

- [x] Resolve semantic input counting, chain/identity behavior and virtual-source
  treatment using primary rules, matched probes and independent design review.
- [x] Add native and CLI RED cases for missing/multiple control/data sources,
  aliases/inversion/vector positions, all six contexts and artifact preservation.
  Preserve legal original/flat/generated replay, mixed control bits and forcing
  data values. Record acceptance on67/68 before implementation.
- [x] Implement the smallest shared bounded per-bit validation; migrate only
  fixtures whose new rejection would obscure their original behavior assertions.
- [x] Run focused/connected checks, representative wide/deep/ordinary overhead
  and independent review, preserving original solver deadlines and assertions.
- [x] Integrate after67/68 verification/commit, run full serial and leak-enabled
  sanitizer checks, update ledgers and commit the verified snapshot.

Independent read-only design review confirms that context masks propagate through
all mux arms and therefore cannot alone trigger required shape checks. Seed checks
only at the six typed Source consumers through canonical aliases. Walk required
control-bearing upstream mux arms; preserve an all-data mux serving only as the
local forcing value. Unused reset/TMS/TRST provenance alone does not trigger these
six port-source clauses. CE/UE's independently implemented unused gate checks stay
unchanged. Source-less typed ToReset/ToTRST helpers need no new choice exception;
keep existing virtual choice eligibility and direct virtual associations intact.
Reset destinations retain qualified TRST compatibility. Canonical source comparison
must normalize constant inversion and include virtual identity. Duplicate rows are
not extra input wires; primary6.3.12(d) also makes a second matching control a
conditionally valid data_signal. Vendor identity/two-control probes are pending.

Implementation economy: existing per-bit provenance can include all five control
families once. After matching-use and CE/UE validation finish, reuse the context
mask storage as the memoized three-family shape worklist state rather than adding
another16MiB dense array. A bounded uint32 worklist can process monotone required
families; each bit/family is checked once per newly arrived mask. Count at most
three distinct canonical sources to reject the third immediately. This is a design
candidate, not a measured complexity/performance claim; review current code and
write RED checks before editing the implementation.

Task67/68 sanitizer all0; its full serial regression now runs with frozen root
sources. Task69 RED CLI method rejects none of36 missing-control/third-input
models (all accepted on67/68), exit1; native RED reports the same missing guard,
with unrelated cases passing. These logs are in the isolated build/reset-mux-
structure-red-*. No root source/test change is made for69.

Draft implementation reuses the finished context-mask storage, expands the existing
iterative provenance to all five families, and seeds only direct typed Source
consumers. Canonical max-two counting includes constant inversion normalization;
corresponding control-bearing branches are validated, forcing-only data muxes and
unused gates remain untouched. Independent draft review finds no product blocker;
it caught a scalar output declaration in the vector negative, corrected before
GREEN. Added identity/same-alias/two-control/inverted-constant positive roundtrips
lock the retained compatibility interpretations. These interpretations and vendor
differences must be explicit rather than described as settled conformance.

Remote status reports72 completed loading cases: identity-single loads; data-only
and constant-only do not; repeated special rows/two controls are rejected; three
canonical inputs load. Hash-qualified fetched results remain pending. The proposed
max-two rule follows the explicit primary two-input language despite that vendor
permissiveness. Canonical duplicate and two-matching-control acceptance preserves
existing behavior under semantic input identity and the conditional data_signal
exception; it is not a broader operational qualification. No new CPU checks run
while root's solver-sensitive full regression is active.

Fetched72 models and runners now match every submitted SHA256, with per-case exit
and exact loaded markers retained in build/reset-mux-structure-vendor-probe/
comparison.json (collection.exit0). Thirty load: qualified, identity-single,
three-distinct, control-chain and data-forcing-chain, each in all six contexts.
Twelve missing/constant-only control cases fail ICL65; thirty duplicate/complement/
second-special-selection cases fail ICL115. No PDL or operation was exercised.
The remote archive finished while the original SSH batch process remained open;
it was independently fetched, not treated as a fresh vendor rerun. Local/remote
clock skew is about124s, so no vendor timing claims are made.

An isolated focused build on Rocky8.10/GCC15/Tcl8.6/pinnedZ3 passes native netlist
and all53 main CLI methods, including the new negatives/positives and existing
artifact/replay checks. All3680 submitted source/test file hashes match the fetched
manifest, and the source archive SHA check passes. The three changed69 files still
match that candidate snapshot. Evidence: build/reset-mux-structure-remote/evidence.
This is early focused validation while local67/68 full serial runs, not a final
platform/install/static/full-regression qualification or a waiver of local checks.
Remote native7 plus the official dynamic CLI suite is now running sequentially;
its source/test snapshot remains frozen. Generated parser/scanner inputs were
supplied through the existing .prebuilt mechanism; no source patch was needed.
The completed loading-probe SSH transport was closed with exit130 after independent
archive/hash collection; the72 individual vendor results remain authoritative.

Local next-step native coverage additionally retains512/4096 control-mux chains
with a typed consumer at every stage, exercising shared upstream validation and
flat identity rather than only alias depth. This test-only addition is after the
remote source/test snapshot; product code remains identical. Local compilation,
GREEN and scale measurements wait for root67/68 full completion. Do not claim
that the remote run exercised this subsequently added scale loop.

Task67/68 is committed as e156f88 after full serial0 (1808.877704s), editor6
220.236s and all required leak checks. The isolated checkout is advanced to that
commit on task/reset-mux-input-structure. Its earlier combined draft is preserved
in a retained stash plus /tmp/reset-mux-structure-task69-before-advance-files,
including manifest and complete diff. Only the3 Task69 files were restored.
Production source and native tests were formatted before local connected build;
local native7/main53/clock41/client18/reset12/multiple-TAP22/feedback10 checks are
now running. Local root still contains only verified67/68 code. No next-rule
source has been imported into it.

Local connected all.exit0: native7 including scheduler120.827s, main53=13.064s,
clock41/client18/reset12/multiple-TAP22/feedback10 all pass. The retained4096-stage
shared reset network has4096 typed consumers and392916 input bytes; native source/
flat verification takes0.360738s (512-stage0.0345933s). Final independent code/test
review finds no remaining consequential issue. Initial naming check catches one
multiline-if missing braces (format0/naming1), retained in initial-naming logs;
only braces are added before rebuilding. Require byte-identical object/CLI/native
comparison or appropriate focused checks before carrying connected evidence forward.
Benchmarking did not start after the failed style gate.

Final formatting/naming pass after the brace-only fix. Rebuilt CLI, native netlist
binary and netlist object are byte-identical to the connected-test versions;
comparison hashes are retained in build/reset-mux-structure-braces-comparison.json.
All60 interleaved loading runs pass, with baseline/changed output and flat hashes
matched. Cases cover262144 reset bits,4096 aliases,655360 ordinary references and
512/4096 shared control-mux/port stages. Changed wide source/flat0.666–0.868s versus
baseline0.566–0.817s, peak137084KiB versus134836KiB; shared4096 source/flat
0.215–0.366s versus0.215–0.316s, peak86268KiB versus86256KiB. Exact source/binary
hashes, bytes, command, compiler/platform, repetitions, wall/RSS and per-phase
records are in build/reset-mux-structure-loading-scale/comparison.json. Empty PDL
measures loading, not solver behavior. No worst-case/optimality claim is made.

Remote connected result archive contains aggregate0, native7 exits0 and official
41 dynamic CLI suites exit0. The SSH transport reached its1200s timeout after the
remote work and archive had completed; archive, individual exits and manifest
were fetched independently, not relabeled as a transport pass. Evidence is in
build/reset-mux-structure-remote/connected-evidence. This early snapshot lacks the
subsequently added512/4096 native scale loop and final formatting/braces; no final
platform/static/install qualification is inferred. Local final native scale and
connected checks use the current snapshot, whose3 files are preserved under
/tmp/reset-mux-structure-task69-reviewed-files and imported unchanged into root.
Root leak-enabled native7 plus main53/clock41/client18/reset12 checks now run.
Source/test files remain frozen until final sanitizer/full regression completion.

Root leak-enabled native7 and main53/clock41/client18/reset12 all pass, aggregate
reset-mux-structure-final-sanitizer-all.exit0. Main53 takes35.851s, clocks41
31.505s, client18 37.938s and argument-reset12 45.708s (64-iteration/five-form
25.896s). All3 current root hashes match the reviewed snapshot before full serial
execution. Final make-j1check is running in build/reset-mux-structure-final-full.log.
Independent documentation review finds the evidence and limitations accurate.
CLI loading measurements include parsing, construction and debug-output I/O;
they do not isolate time spent inside the new validation guard.

Final full serial make-j1check succeeds: build/reset-mux-structure-final-full.log/
.exit0,1618.418233s wall. All41 dynamic and41 static generator suites,51 directional
goldens,2523 ICL334/119 rules, LSP62 and editor6 in211.728s pass. Optional ANTLR5
skipped. All3 final root source/test hashes match the reviewed snapshot; native7
and main53/clock41/client18/reset12 leak checks, naming/style, focused/performance
and independent code/test/documentation reviews pass. Task70 remains isolated and
is excluded from this commit. Remaining directional/forcing/polarity/operational,
runtime/TAP/lifecycle, strict coverage, vendor and platform requirements stay open.
