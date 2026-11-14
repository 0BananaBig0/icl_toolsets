# Ordinary output source scope implementation plan

> Execute inline with superpowers:executing-plans under the existing autonomous
> authorization. Task75 root source/tests remain frozen for final checks; this
> increment stays in the isolated checkout. Independent review remains required.

**Goal:** Enforce the local-input/child-output distinction for ordinary data,
scan and clock terminals consumed by existing hardware checks.

**Architecture:** Reuse canonical alias terminals and restored expression scope.
Reject eligible DataOut/ScanOut/ToSelect/ToClock/ToTCK outputs declared in that
expression scope. Reuse Task75 output checking after existing eligibility checks,
so bad-kind, rightmost, inversion and direct-DataMux diagnostics retain priority.
No additional array, traversal, public metadata or source-wire following.

**Tech stack:** C++17, existing native/Python tests and Tcl vendor runner.
**Spec:** ../specs/2026-10-25-icl-pattern-generation-design.md; IEEE1687 6.3.12(c–e).

## Design and alternatives

A parser-only check cannot resolve aliases or restored flat scopes. A universal
hierarchy/depth pass would expand this increment and collide with generated TAP
lowering. Reuse the connected source validator, restrict equal expression scope
only, stop at declared output terminals and preserve other hierarchy validation.
Independent design review confirms generated TAP root mux choices refer to child
ScanOuts and return bindings to siblings in parent scope; generated DataOuts live
in TAP children. Verify generated original/flat CLI regressions without exemptions.

Scope includes existing Source and typed data-fallback contexts, mux choices,
Enable/SelectedBy, capture/read/write data/enables, logic and checked OneHot data
inputs. Strict ScanInSource still excludes ordinary data and only takes rightmost
register bits. ToIRSelect selector exception, alias-only declarations, PDL targets,
interface membership, node.reset associations and unchecked lowered ToTCK Source
remain unchanged. DifferentialInvOf is an association, not a parent binding;
6.4.6.20 conversion rules leave an apparent tension with6.3.12(e)'s local list.
Defer its direction restriction while retaining all existing kind/width checks;
probe the local output-pair interpretation separately. No Period/name exemptions.

## Reach and review focus

Modify src/pattern/pattern_netlist.cc, tests/src/pattern_netlist_test.cc and
 tests/src/test_icl_pattern_generation_main.py. Repair only demonstrated invalid
producer fixtures across existing native/CLI suites, preserving their behavior,
backend diagnostics and artifact assertions. New test scripts remain under tests.
Update generator guide/model ledger and this plan with actual evidence.

Reuse the O(bits + alias graph) loading path with O(1) per consumed terminal;
measure seven wide families, child mux,4096 aliases/shared consumers and655360
ordinary references against the frozen75 executable. No unmeasured speedup claim.

- [x] Add direct/alias own-versus-child data/scan/clock consumer matrices, input
  parent scope, vectors/partial, original/flat positives, altered-flat rejection,
  alias-only/PDL and DifferentialInvOf exclusions. Record focused RED before code.
- [x] Qualify source/mux/data-fallback vendor loading and differential ambiguity
  with matched models/runner hashes; distinguish loading from PDL equivalence.
- [x] Add the smallest shared accepted-terminal scope check; preserve old error
  precedence and Task75 cases. Repair failing fixtures without weakening tests.
- [x] Run focused/connected GREEN, representative serial performance, style and
  independent review. Integrate immutable files only after75 verified commit.
- [x] Complete required leak/full serial checks, record exact scope and remaining
  requirements and commit the verified snapshot. No push requested.

Independent design/test review supports accepted-kind placement and restored TAP
wiring. It caught an unenabled OneHot fixture before implementation: replaced by
a dedicated enabled DataOut case, retaining the old enabled-contributor rule.
Added ScanMux/ClockMux ordinary data-choice fallbacks and a strict ScanInSource
wrong-kind diagnostic test. Initial immutable RED archive precedes these fixture
corrections; its first own-output case still demonstrates the missing guard.


RED build0/native1 (scope failure absent)/CLI1 (invalid own DataOut accepted),
actual3 hashes matched unchanged75+new tests. First GREEN actual3 hashes match;
main61/client18/multiple-TAP22 all0, native4 older fixture failures. Clock-source
positive producer and direct-control mux-selector producer move into children;
wrong-kind/direct-driver negatives and all original assertions remain. OneHot
bit-dependency/deep-group fixtures are held pending the specific6.4.15(d)
qualification, not automatically migrated under the generic data rule.
Independent implementation/test review found no consequential issue before this
OneHot interpretation question; subsequent primary review supports preserving
its enabled DataOut collection context if matched probes confirm it.

Initial69 vendor model/runner hashes match:24 legal non-scan child hardware cases
load,24 own cases failICL130;13 scan cases expose unrelated missing ScanInterface
ICL71 (6 own alsoICL130). Four non-scan alias-only models load,scan alias-only is
among the13. Same-module positive differential output pair loads; inverted or
hierarchical associations failICL59. This supports the differential exclusion,
not complete clock conformance.17 qualified ScanInterface/OneHot probes follow.


Followup17 models/runner hashes match:6 legal child scan hardware cases load
(two mux rows warnICL68),6 own scan cases failICL130 (source alsoICL103),scan
alias-only loads. OneHot own direct/alias both failICL130; child both load.
Specific6.4.15(d) restricts kinds/enables but does not override the generic
hierarchy rule. No OneHot exemption; the two old fixtures now use enabled child
producers while preserving per-bit feedback and3000/65resource assertions.
Additional independent fixture review is clean. GREEN2 native0, actual3 hashes
match; product/main hashes unchanged from first GREEN main61/client18/multi22.
Root75 committed1c28b14 after full0 (1340.802s/editor6 175.279s), all required
leak0/current remote native7/dynamic41/hash6match. Saved next draft/full diff
under/tmp/ordinary-output-scope-task76-before-advance-files and retained stash;
advanced onto1c28b14 and restored only3 next source/tests plus this plan.
Final75 docs retained. Current76 remote native7/all41 checks are running.


First local connected netlist/state/tap/address/main61/clock41/client18/reset12/
multi22/feedback10 pass. Symbolic/flatten/scheduler catch three further own-output
forwarding fixtures. Child Producer/Forwarder repairs preserve all9 enable truth
cases/Z3 checks, flat child.raw/forwarded identities/slices, and scheduler's1+4096
reference budget8/8192/zero-cache assertions. Additional independent review is
clean; native3 repair reruns pass (scheduler112.400s). First remote all-CLI wrapper
had a nested quoting syntax error before any CLI recipe ran; its logs are retained
and no41-suite pass is claimed. Corrected shell recipe loop now runs all41 with
individual statuses plus native7 and actual8 source/test hashes on prereuse76.

First valid serial loading84 comparisons all0/output-flat hashes equal against
1c28b14. Wide four8192-bit output families0.05–0.09s; scalar scan0.02–0.04s;
262144 Reset mux0.53–0.60s;4096 aliases0.05–0.12s;4096consumers0.06–0.10s;
CE-only2621440.30–0.33s;ordinary655360 refs0.16–0.18s vs0.12–0.15s baseline.
Exact evidence build/ordinary-output-scope-loading-scale/comparison.json.
Initial wide-ScanOut and mixed Scan/ToSelect pilots hit existing scheduler/event
limits; a scalar scan pilot used an invalid scalar index, then the corrected
scalar source loads0. All failed pilots/logs are retained outside valid timings.
No vector scan-return support is inferred from native validation or these runs.

The measured ordinary-reference overhead motivates canonical terminal reuse in
this same source loop, avoiding its duplicate alias-map lookup. No array/cache/
pass/API added. Alias inversion/constant/virtual/hole semantics and old diagnostic
precedence are unchanged; independent optimization review reports no findings.
Pre-reuse source saved/tmp/ordinary-output-scope-before-canonical-reuse.cc and
binary build/ordinary-output-scope-pre-canonical-reuse. Connected checks rerun
before interleaved75/pre-reuse/new126 loading measurements. No speedup claimed
until the equivalent measurements complete. Prior84 timings remain preserved.


Final canonical-reuse connected native7/main61/clock41/client18/reset12/multi22/
feedback10 all0 (scheduler114.355s). All126 interleaved source/flat comparisons
among1c28b14, retained pre-reuse76 and final76 preserve output/flat hashes. Four
8192-bit families0.05–0.10s; scalar scan0.02–0.03s;262144 Reset mux0.52–0.55s;
4096 aliases0.06–0.12s;4096 consumers0.06–0.09s;CE-only2621440.31–0.33s.
Ordinary655360 references0.12–0.16s versus pre-reuse0.15–0.19s and baseline
0.12–0.15s; before/final peak66716KiB, baseline66832KiB (no savings claim).
Environment/compiler, exact commands/input/binary/source hashes, phase timing and
RSS are retained in build/ordinary-output-scope-loading-scale-terminal-reuse/
comparison.json. These are whole CLI source/flat runs, not isolated guard time.
The earlier84 valid runs and failed fixture pilots remain separate and retained.
Independent canonical-reuse review is clean. Required leak/full integration and
current-source remote qualification remain pending.


Final shared-output fixture qualification: prereuse remote native7 and40/41 CLI
suites pass with actual8 hashes matching; shared-output gating/vector fixture
used an own ScanOut Source. Reviewed child Forwarder repair retains Rootinner as
an enabled observation, gated/inverted a[3:2], b and all original/flat/generated
replay assertions. Normal9tests0 in1.418s; leak-enabled shared9tests0 in3.145s.
All required leak ASan/UBSan native7/main61/clock41/client18/reset12/shared9 pass;
aggregate build/ordinary-output-scope-final-sanitizer-complete.exit0. All7 changed
source/test files match independent review snapshots. Final current-source9-file
remote native7/all41 runs separately. Full serial integration is starting.


First full integration stopped2 after76.961s at a further old own-output sparse
OneHot benchmark fixture. Logs/exit/wall are retained as build/ordinary-output-
scope-first-full-fixture-failure.*. Reviewed Producer child repair retains the
1048576-bit group,10000 scalar contributors,5000-instance hierarchy/flat replay,
wide parent bus mapping and unchanged10s script deadline. Focused build/benchmark/
format all0; the complete optimized benchmark takes0.839s (sparse elaboration
30.093ms). Production and prior7 changed source/test hashes are unchanged; no
sanitizer rerun is needed for this independent benchmark-only fixture. Full serial
integration restarts after this fix; no full-suite success is claimed yet.


Final Task76 verification: full serial make-j1check0 in1318.863s, recorded in
build/ordinary-output-scope-final-full.log/.exit/.wall, after the separately
retained first benchmark-fixture failure.41 dynamic and41 static generator
suites,51 directional goldens,2523 ICL files with334/119 grammar/scanner rules,
LSP62 and editor6 in174.320s pass. Optional ANTLR5 skipped. Required leak-enabled
ASan/UBSan native7/main61/clock41/client18/reset12/shared9 pass; aggregate
build/ordinary-output-scope-final-sanitizer-complete.exit0. All8 changed source/
test hashes match their independent review snapshots. Current generation code
and native/CLI fixtures pass remote native7/all41, with actual9 submitted hashes
matching and completed archive fetched0; the extra benchmark fixture is locally
qualified0 under its unchanged10s deadline. No full platform/install or vendor
PDL equivalence claim is made. Broader restored-scope, clock association, gating/
selector, runtime/TAP/lifecycle, strict coverage and final vendor/platform/static/
install requirements remain mandatory.

Commit this verified76 snapshot. Task77 altered-flat and TAP RED tests stay isolated.
