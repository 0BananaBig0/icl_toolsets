# Direct data drivers of reset, TMS and TRST ports

> Execute inline under existing autonomous authorization. Preserve failing-first
> tests, independent review, connected fixture coverage and final verification.

Goal: enforce direct DataMux drivers for ordinary-data Source bits of ResetPort,
ToResetPort, TMSPort, ToTMSPort, TRSTPort and ToTRSTPort. Aliases are transparent;
ordinary ports, registers, logic and groups are not. Preserve typed control-family
forwarding, TRST-to-reset compatibility, literals, virtual resets and binding holes.

Primary IEEE1687 clauses6.4.6.13(d)/.14(g), .15(c)/.16(d), .21(b)/.22(h), printed
pages96/97/99/100/105/106. Cached primary source build/combined-audit/ieee1687.txt.
Independent gap review confirms these structural rules do not depend on actual
special-control provenance. Existing kind unions are broader than port context.

Design: reuse66 canonical terminal resolution in CheckSignalSources. After existing
source-kind validation, for the six families reject ordinary-data terminals whose
canonical node is not DataMux. Constants are separately admitted by the signal
grammar and must not be silently treated as ordinary-data nodes. Validate mixed
vectors per bit; keep source-located diagnostics and output atomicity. No new
metadata/API/dependency, no graph pass or cache. Existing std::map alias lookup
cost remains logarithmic. Module direction qualification stays separate.

Explicit remaining rules: this does not establish mux input shape, associated
reset assertion/polarity, normal-operation selector sensitization or selection of
the reset branch on TRST assertion. Full rules are mandatory later work, not
waived by these tests. Reset forcing data may be arbitrary; CE/UE zero restrictions
must not be incorrectly applied to Reset/TMS/TRST.

Fixture impact is material: existing data-reset/feedback/hidden-event CLI models
wire registers, data ports and logic directly to ResetPorts. Audit connected
fixtures before product changes. Correct lawful frontend networks where they
represent the intended operation; preserve intentional arbitrary-circuit tests in
explicit lowered-netlist native checks. Do not add fake/unreachable reset branches
solely to pass the guard. Standard-first versus observed vendor restrictions stay
explicit. Do not remove functional checks to obtain a pass.

An assert-only reset mux selected by original driver d has useful normal-state
value equivalence: active-high d?1:global_reset; active-low d?global_reset_low:0.
It also forces an active output during global assertion. This alone does NOT prove
TRST branch-selection obligations: the selector must independently reset/justify
to its global-select value. Check each fixture's ResetValue, GPIO control and
initial global state. A source-less child ToResetPort is an implicit global reset
function and correctly inverts for ActivePolarity0, but adding it can alter parent
reset-source inference or introduce ambiguity. Do not apply a blanket conversion.

Qualification: six typed/mux input baselines pass in Tessent2026.2. Original36
loading probes give15 accepted,18 source-specific ICL65 rejections and3 constant
input setup errors ICL37 (top had no ports). Corrected3 constant-input cases with a
valid top data port all load. Exact model+runner hashes are checked under
build/direct-control-driver-{vendor,constant-vendor}-probe; remote experiments
stay in/tmp/icl-direct-control-driver[-constant]-20261114. No PDL/equivalence or
operational conformance claim. This independent remote work runs while66 verifies.

Files: src/pattern/pattern_netlist.cc; native netlist/main CLI source tests; affected
frontend fixtures and explicit lowered native state/scheduler/TAP coverage; current
guide/model/main ledger. All new scripts, if required, belong under root/tests.
Root66 remains frozen during its final checks; next work stays in isolated checkout.

- [x] Verify source/fixture reach, qualify constants and source families, define
  per-fixture reset conversion versus lowered-backend strategy.
- [x] RED all six Source contexts, direct/aliased data producers and mixed slices;
  source/flat positives and atomic pattern/reset/debug preservation.
- [x] Implement canonical per-bit direct-driver guard; fix connected fixtures
  without weakening their intended behavior checks.
- [x] Focused checks, realistic scale/overhead, naming/style and independent review.
- [x] Integrate after66 verified commit; final full/SAN, evidence, docs and commit.

RED direct-control-driver-red-cli.exit1 records all36 direct/aliased data-register,
input and logic cases accepted by66. Native RED1 accepts the same forbidden input.
Draft guard now uses66 canonical terminals and restricts only physical ordinary
data kinds for the six contexts, preserving literals/virtual/typed control cases.
Initial GREEN will expose existing fixture assumptions; do not waive failures.

Task66 is committed as62d6915 with full0 (1774.796s) and leak-enabled sanitizer7.
Task67 isolated branch task/direct-control-data-drivers is based on that commit;
five initial draft files are preserved with SHA256 under
/tmp/direct-control-driver-task67-initial-files, and the earlier worktree stash
is retained. Production root remains at66. Initial impact sweep completed40
connected CLI suites:15 fail due old direct-data reset fixtures; exact per-suite
exit/wall and failures are in the worktree build/direct-control-driver-impact.

Independent review rejected changing low-active feedback ResetValue0 to1 because
that changes a locked loop into a releasing one. An external reset-mode pin can
select the global branch during iReset, but does not by itself meet normal-scan
branch-selection rules. Generic AST rewriting/test-only CLI adaptation would
complicate flat metadata, inference and graph validation. Prefer repairing shared
frontend topologies with real global/reset muxes and explicit blocked setup, and
preserving the few irreducible circuits through explicit native lowered Source
connections plus CheckCombinationalConnections. Do not claim operational compliance
from static shape alone, and do not blanket-switch suites off the production CLI.

Four affected suites now pass on the production CLI after shared merge fixtures
and setup repairs: merge scheduling/observations/data-registers and pending reset.
Nine data-reset dependency cases pass, retaining source/flat/generated replay and
64/128-register mixed reset chains. The locked self/mixed feedback and mixed
controlling-history/unknown-global circuits are migrating to native scheduler
checks with their exact ResetValues and transition functions. Native execution
is not yet confirmed; do not remove old coverage before replacement passes.

Primary-rule correction after focused independent review:13(f)/14(i) name
trst_signal, not every global reset or iReset event. §5.14.2 distinguishes TLR
from TRST/power-up; §7.9.9 distinguishes ResetPort toggles, async TRST and sync
TMS. A standalone model with no actual or inferred TRST function may retain
selector ResetValue0 in an assert-only low-active mux: both paths assert during
global reset, and release1 selects typed reset for normal operations. Source-less
ToResetPort alone does not establish TRST; source-less ToTRST/embedded TAP cases
still require explicit TRST analysis. The earlier review's blanket global-select
requirement was retracted. Virtual async scheduler bookkeeping is not a normative
TRST function. Reuse original ResetValues/setup in these standalone fixtures,
avoiding needless behavior changes; retain independently reviewed exact lowered
backend coverage for the three migrated circuit checks.

Native --reset-backend passes initial replacement checks. Review caught self-only
CaptureSource and per-Apply rollback gaps; both corrected, pending rerun. Three
Python methods moved to these native checks after the first replacement passed.
Their source/flat/generated replay and CLI artifact-preservation coverage no
longer applies; native checks cover exact transitions and failed-Apply storage
rollback. Other production CLI guards and artifact checks remain enabled.

Second targeted fixture group all10 pass, including main52; third6 group has
resource/client/multiple-client/chain-group/shift-enable all0, with only the
32-register feedback ring timing out at the unchanged2s solver deadline. Quiet
reproduction fails. Mux-only simplify, exact Boolean mux bitmask formulation and
scan-load MinimizeBits experiments all fail source/flat32 checks and are discarded;
production symbolic/scheduler source is restored exactly to62d6915. No speculative
optimization is retained. Native symbolic identity check retains a direct proof
that an inactive global plus assert-one mux preserves its selector including X.

18 quiet ablation runs (three repeats) are in the isolated worktree
build/direct-control-driver-ring-ablation/comparison.json with commands, binary
hashes, bytes, wall/RSS and outputs. Bare32 on66 passes with/without iReset
0.516–0.767s. Physical-reset mux32 fails all repeats on both66 and67. A real
source-less ToReset helper mux32 passes all repeats on both4.831–5.133s and matches
the bare-with-reset output hash. Independent review accepts adding this legitimate
implicit-global path for the pinless ring fixture, preserving tested post-reset
feedback behavior and32/source-flat-generated checks. This is not whole-model
equivalence to the original direct-feedback model. Physical32 performance remains
mandatory next work, not resolved or waived by this fixture adaptation.

The implicit fixture alone does not provide reliable solver margin: full replay
subsequently times out too. Treat that adaptation as semantic scope, not a
performance solution. A captured actual32 input differs from an ablation only by
a newline; their first2,754,636-byte SMT dumps have identical SHA25679b37d... .
Both have4228 assertions,34 objectives and65 reset passes. Checks vary from
1787/1848ms sat to2005ms unknown under the unchanged2000ms setting. Physical
checks1669ms sat/2008ms canceled confirm the same existing margin problem.
Diagnostic source/binaries/dumps are retained in/tmp and worktree
build/direct-control-driver-scan-diagnostic; no diagnostics remain in the product.

Necessary Task68 work now shares this implementation checkpoint: when a one-bit
selector is provably known and exactly two single-code rows0/1 cover it, reuse
Select instead of building redundant unknown-selector consensus. Either row order
is supported. Require both expanded source lengths to match declared mux width;
otherwise keep the old generic rejection. Reuse identical arm knowledge masks to
avoid repeated traversal in deep selector chains. Unknown selectors, overlapping/
uncovered/alternative codes and other widths retain generic behavior; shared
Enable and serial/held handling remain unchanged. No public API or metadata change.

Independent review found the malformed-both-branches width regression; native
known-mux-width-red.exit1 reproduces it, confirmed native0 after guards. Variable
selector identity plus243 ternary/inverted/reversed vector cases and existing
held/shift/oracle checks pass. Before shared-mask reuse, all27 repeated implicit/
physical/reversed32 source/flat/generated runs pass with successful baseline
output/flat hashes matched (build/direct-control-driver-known-mux-scale).
Final-source repeats remain pending under ...known-mux-confirmed-scale.

Deep512/4096 data and selector chains have independent closed-form proofs under
a2s solver setting. Old-symbolic data4096 completes; old selector4096 does not
complete within60s; there is no completed measurement or peakRSS. Candidate selector
4096 construction2.94475s fell to0.0226–0.0272s with identical-mask reuse; data4096
construction0.125–0.137s/check0.485–0.965s. Full native symbolic passes, including
the four scale checks. Source hashes and commands are retained in the deep scale
comparison; before-reuse source reconstructs to the recorded32-ring checkpoint.

Six optimized natives passed. Scheduler full reached four late old reset fixtures;
these now use valid forcing muxes and explicit global reset before the original
assumptions, including true source/flat prerequisite replay. Focused --reset-backend
passes with all four. Final full scheduler remains required. Review also moved the
new identity proof into main so coverage-budget exception handling cannot mask it.
Unused Python reject helper was removed after its three callers migrated to native.

Final reviewed snapshot contains29 selected source/test/fixture files, all hash
matched on import into the root checkout. The optimized connected regression
passes all7 native binaries, including full scheduler120.184s, main52 and clock41/
client18/reset12/multiple-TAP22/feedback10 suites. Evidence is copied to root
build/direct-control-driver-confirmed-connected, aggregate all.exit0. Three final
repetitions of implicit/physical/reversed32 source/flat/generated cases all pass
(27 changed runs), with successful baseline output and flat hashes matched.
Final4096 selector construction0.026–0.031s/check0.003–0.004s; data construction
0.132–0.139s/check0.473–0.942s, peak138856KiB. Empty-PDL wide/deep/ordinary loading
checks separately measure direct-driver validation overhead. Diagnostics confirm
NeedsScanEvents is unused and retain65 passes/34 objectives while reducing
constraints4228/4227 to2148/2147. Product scheduler remains byte-identical to66.
Independent final review and format/naming checks pass. Root leak-enabled native7
plus selected CLI checks are running; full serial regression remains required.
Task68 is integrated into this checkpoint because lawful67 fixtures expose the
existing reset-mux timeout. No deadline, assertion or coverage threshold changed.

Root leak-enabled ASan/UBSan passes all7 native tests and main52/clock41/client18/
reset12 CLI methods, aggregate build/direct-control-driver-final-sanitizer-all.exit0.
Scheduler retains primary4096 check under the unchanged2s solver setting (total
Apply5.178s under instrumentation is not check-only time). Main52 takes33.418s;
argument-reset12 takes46.373s including64-iteration/five-form26.700s scale case.
All29 root source/test hashes still match the reviewed immutable snapshot before
starting make-j1check. Full serial regression runs in
build/direct-control-driver-final-full.log; it remains required before commit.

Final integration succeeds: build/direct-control-driver-final-full.log/.exit0,
1808.877704s wall with make-j1check. All41 dynamic and41 static generator suites,
51 directional goldens,2523 ICL334/119 grammar/scanner rules, LSP62 and editor6
in220.236s pass. Optional ANTLR5 skipped. All29 root source/test SHA256 values
match the reviewed snapshot. Required native7/main52/clock41/client18/reset12
leak-enabled ASan/UBSan, focused proofs, repeated scale/loading checks, naming/
style and independent source/fixture/documentation review pass. Reviewer wording
corrections distinguish baseline SAT/unknown and an attempted60s timeout from an
unattempted run. Product scheduler objects also match the measured worktree.
Task69 draft stays isolated and is excluded from this commit. Structural/
operational, runtime/TAP/lifecycle, strict coverage, full vendor certification and
refreshed platform goals remain mandatory; this does not close the whole project.
