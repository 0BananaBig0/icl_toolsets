# Remaining work: items 2–15

User instruction: finish items 2–15 from the2026-11-20 inventory. Item 1,
strict 100% authored-line / 90% raw-branch coverage closure, is excluded from
this work. Its last measured96.68%/62.97% failure is retained; no acceptance
threshold has changed. The [completion design](superpowers/specs/2026-11-20-completion-program-design.md)
preserves the original requirements.

This is the current completion tracker. Earlier plans and dated evidence remain
historical records; their unchecked boxes are not automatically a current task
list. Completing one supported case does not complete a broad item below.

| Item | Required result | Status / acceptance |
|---|---|---|
| 2 | Independently phased TAPs, shared serial storage, intermediate taps, overlapping returns and independent lanes | Pending. Common-phase/equivalent-TCK multiple TAPs and disjoint lanes already work. Add state/capture/update/ownership cases for the remaining topologies and prove original/flat/generated replay. |
| 3 | General independently gated and serial-derived scan/reset events | Pending. Retain edge ordering, hidden storage and transient resets across source/flat/generated replay; no guard-only enabling. |
| 4 | Cyclic/self-referential DataRegister updates and callbacks | Task167 supports actual native logical merge histories with source-consistent serialized generation; see its qualified increment below. Task164 supports callback-containing iMerge calls using serial fallback and preserves ordinary batching; all64 gates pass (15native/15SAN,74x1208 each generator,12quality,4041inputs/38products),405 matrix/example and4 allocation processes. Review0C/3I/1M: all Important findings fixed; one new Minor deferred(total16). All28 merged-result checks pass on identical4041inputs/20products;117stashes preserved. Task163 passes63 final qualification gates:15 native and15 leak-enabled sanitizer suites,73 suites/1189 methods in each generator variant,15 C++ quality checks, source packaging and4039 input/38 product hashes. The72 regular downstream processes peak at4.2884s/373760KiB; six512/1024-bit runs peak at1.1308s/121236KiB. All324 new/preserved matrix and guide processes plus4 dynamic allocation probes pass. Independent review found0Critical/5Important/1Minor; one test-first pass fixes all five Important findings. The benchmark checks postcondition counts, widths and allowed names; exact ordered register/value pairs remain a deferred Minor. Functional regressions check identities and payloads. All26 merged-result checks pass on identical4039 inputs and20 copied qualified executables, with117 stashes preserved. Broader inventory and excluded coverage item1 remain open. Task162 supports mixed ordinary/callback alias bits with exact native ordinary accesses and preserved callback arguments/source spans; all61 final gates pass (15native/15SAN,72x1169 each generator,9quality,4036inputs/38products),243 matrix and4 dynamic allocation processes. Review0C/2I/1M is fully resolved; merged and pushed;24 integration checks pass on identical source/product hashes. Task161 supports separately queued ordinary requests with explicit callbacks, preserving known caller goals through native frames/state and actual ordinary histories; all59 final gates pass (15native/15SAN,71x1153 each generator,30quality,4033input/38product hashes); merged and pushed;22 integration checks pass on identical source/product hashes. General symbolic/control/serial automatic callbacks remain open. Pending broader callbacks. Task153 adds explicit callback-only procedure execution, masks/aliases, ancestor namespaces, cancellation/error recovery and source/flat/generated/local-reset replay; all43 post-fix qualification gates pass, including63 suites/1048 tests per generator and14 native/14 sanitizer suites. Task159 supports actual ordinary endpoint observations in explicit write procedures with unknown abstract callback storage. General automatic dependencies, modeled merged/invoked-read histories and physical output remain open. Task145's historical declaration checkpoint added typed callbacks and canonical flat preservation. Its seven focused methods,13 native/13 sanitizer suites,54x900 per generator variant,21 quality gates and packaging pass. Independent review reports0Critical/0Important/0Minor; all18 reviewer-designed probes meet expectations. Task144 adds bounded simultaneous feedback from real preceding storage, known/unknown convergence, controlling history, capture ordering and intermediate ownership/rollback. One independent review found0Critical/3Important/0Minor; one verified fix pass resolves all three findings and preserves autonomous goal intent. All13 native/13 native sanitizer suites,53x893 per generator variant,14 focused replay methods,39 quality gates and source packaging pass. Changing feedback-derived external reset events remain item3. |
| 5 | Stateful absolute time and remaining clock semantics | Pending. Relative waits, clock overrides and restricted clock-only absolute waits already work; Task141 fixes associated-flat override target/source mapping. Cover stateful events, oscillator restoration/frequency, phases and remaining names/source mappings without inventing periods. |
| 6 | Runtime observations/status and interface/chain history | Task167 supports actual native logical merge histories with source-consistent serialized generation; see its qualified increment below. Task162 preserves ordinary histories and cancellation through mixed alias projections, with invoked ReadCallBack history guards retained; all61 final gates pass (15native/15SAN,72x1169 each generator,9quality,4036inputs/38products),243 matrix and4 dynamic allocation processes. Review0C/2I/1M is fully resolved; merged and pushed;24 integration checks pass on identical source/product hashes. Task161 preserves actual ordinary histories in mixed callback caller groups, including canceled reads and completed inner frames; all59 final gates pass (15native/15SAN,71x1153 each generator,30quality,4033input/38product hashes); merged and pushed;22 integration checks pass on identical source/product hashes. Task160 adds actual explicit-only scalar opaque-child/root histories while keeping opaque return bits unknown; all58 post-fix gates pass (15native/15SAN,70x1136 each generator,21quality,4031input/38product hashes); merged and pushed;20 integration checks pass on identical source/product hashes. Task155 adds fixed disjoint modeled root named-chain replay and actual explicit-only chain histories with common controls;16 CLI methods, native per-clock references and93 scale/control processes pass, all47 qualification gates pass after four test-first Important fixes, with two Minors deferred; integration is pending. Pending broader histories/transport. Task152 adds actual native scalar root/child and bound single-TAP raw interface histories plus named opaque parallel-chain unknown histories, with variable widths and separate register/raw planes.  Task148 adds opt-in modeled combinational PIO observations at actual scheduled frames, sparse/inverted history, exact per-bit failure counts and clear semantics. A direct independent review found2Important/1Minor issues, all resolved in one verified fix pass;35 qualification gates,14 native/14 native sanitizer suites,57x930 tests per variant,30 quality gates, packaging and108 scale processes pass. Task149 extends modeled observations to DataRegisters through PIO, with actual read-before-write frames, source-ordered manual planes and legacy-note migration. Its full behavioral qualification passes36 gates/58x946 per variant/14 native+14 native sanitizer suites/15 quality gates/packaging; a help/assertion-only fix for two Minor review findings passes13 focused checks/312 methods, preserving28 native and4004 input hashes. Final4006 inputs/31 products and108 behavioral scale processes are verified. Task150 adds actual standalone/client serial-return observations for ScanRegisters and captured DataRegisters, including chain inversion and unequal disjoint lanes. One fresh direct review found2Important observer-path gaps; one verified fix pass resolves both. Final qualification passes37 gates,14 native/14 sanitizer suites,59x962 methods per generator,18 quality checks, packaging and144 scale processes;4007 inputs/31 products are verified. Task151 adds actual bound-TAP IR/DR observations, captured data and common-phase multiple TAPs, with matched Runtime/output scheduling and exact physical-PDL mode. Its fresh direct review reports0Critical/1Important/2Minor; one verified fix pass resolves subscriber state/fill/cache consistency. Final38 gates,14 native/14 sanitizer suites,60x978 methods per generator,18 quality checks, packaging and144 scale processes pass;4008 inputs/31 products verified. Minor compiled-help/prose findings are deferred. Raw/interface/chain and timed histories remain open; real device access needs a supplied transport. Task141 preserves never-read/sparse unknown history and provably zero initial/no-compare status.|
| 7 | Remaining resource ownership and lifecycle | Pending. Task141 fixes compatible modeled-child port ownership with protected intermediate serial states, scope tests and a bounded invariance fast path. Add remaining clock/scan assignment/frequency, pattern-set lifecycle and TAP ownership. |
| 8 | Remaining raw-scan configurations | Task167 supports actual native logical merge histories with source-consistent serialized generation; see its qualified increment below. Task166 supports complete serialized raw calls in valid iMerge bodies on existing modeled scalar/child/named-chain/single-TAP routes; see its qualified increment below. Task160 supports one declared-width scalar opaque leaf on a static complete root route with modeled parent preservation and unknown actual opaque returns; all58 post-fix gates pass (15native/15SAN,70x1136 each generator,21quality,4031input/38product hashes); merged and pushed;20 integration checks pass on identical source/product hashes. Broader multiple/dynamic/gated/parallel opaque children remain open. Task155 adds fixed disjoint modeled root named-chain replay and actual explicit-only chain histories with common controls;16 CLI methods, native per-clock references and93 scale/control processes pass, all47 qualification gates pass after four test-first Important fixes, with two Minors deferred; integration is pending. Pending broader topology. Task152 adds strict root opaque parallel chains with per-chain default/stable completion and actual modeled raw observations; broader hierarchical/dynamic/shared parallel and multiple-TAP raw, opaque children and broader raw merge configurations remain open.  Add remaining opaque-child, parallel/multi-TAP and raw-merge configurations and qualify remaining vendor-export forms.|
| 9 | Additional repairable caller-only reset transformations | Completed in Task143 for the inventory's state-dependent/identical-argument caller cases. Recorded invocations cover empty/caught calls and once-only arguments with unchanged procedure bytes. Five review findings are fixed; full post-fix qualification passes. Impossible queues/ownership/internal reassertions and uncertifiable file-return/cleanup boundaries reject atomically. |
| 10 | Broader merge/scheduling optimization and scale qualification | Task168 closes the measured 1,024-bit PIO memory case; all current matched measurements meet unchanged bounds. Task169 closes the measured 256-target callback latency case in its final matrix, with deterministic model-evaluation work reduction. Broader optimization remains pending. Task165 reduces shared model-decoding cost; its complete72-process measurements and retained budget misses are recorded below. General referenced/resettable and larger workloads remain pending. Task154 conservatively indexes irrelevant callback storage in primary solves; matched1024-target/four-group source median improves26.2120s to2.6123s with identical output,60 scale/replay processes,20 alternating controls,9 compatibility methods and native state/copy checks. All46 qualification gates pass (14 native/14 sanitizer suites,64x1058 per generator, six quality checks and packaging); the single Important benchmark finding is fixed in a test-first pass and one Minor is deferred. Referenced/resettable storage and larger workloads remain separate acceptance work. Task146 fixes the trailing iWrite -ignore_when_value_already_present payload loss for auto/off/on at the shared Runtime boundary; original trace arguments and existing explicit-load policy remain. Six focused methods, all13 native/all13 native sanitizer suites,55x906 tests per generator variant, six quality gates and source packaging pass; four fresh vendor cases and16 native replay forms agree on changed-value PIO payloads. Cover resource/timing barriers and representative larger networks, preserving valid schedules and reporting measured costs rather than global optimality. Task142 retains explicit512-function generic-primary and parent4096-bit solver-limit reproductions; no limit was raised. Task147 counts actual enabled primary events and retains explicit data-goal intent absent from emitted controller writes, in primary and mixed paths. One related Important replay family is resolved in a verified fix pass; seven focused methods,13 native/13 native sanitizer suites,56x913 tests per variant, six quality gates and packaging pass;108 scale processes retain matched controls. This uses existing native event semantics and does not certify physical timing or wider source acceptance. |
| 11 | Remaining model/default/source/reference audit | Task156 adds reproducible default-model/source audit:1016 actual canonical roundtrips,56 explicit unsupported serializations and691 original default elaboration failures across1334 accepted files/1763 effective modules; twelve valid empty-control serializations and fourteen missing-enum diagnostics repaired. Six post-fix normal/configured-SAN batch measurements pass10s/512MiB; all52 qualification gates pass after five Important review fixes, one Minor deferred; integration pending. Pending corpus/specialization and broader source acceptance. Task140 verifies the eight BSDL/default groups with19 replay methods and eight review fixes. Task142's typed vector-port implementation passes17 focused methods per variant, native caller checks and nine vendor models. Two Important review findings were fixed and full post-fix qualification passes; no Critical/Minor or unresolved blocker remains. All11 former duplicate-port corpus failures reach the existing interface-event boundary (item3), not end-to-end generation success.|
| 12 | Broader vendor and model round-trip qualification | Task156 adds reproducible default-model/source audit:1016 actual canonical roundtrips,56 explicit unsupported serializations and691 original default elaboration failures across1334 accepted files/1763 effective modules; twelve valid empty-control serializations and fourteen missing-enum diagnostics repaired. Six post-fix normal/configured-SAN batch measurements pass10s/512MiB; all52 qualification gates pass after five Important review fixes, one Minor deferred; integration pending. Pending wider qualification. Task140 adds six bounded BSDL cases with48 nonempty original/flat/generated replays and12 matching directional apply-group comparisons; Task142 adds nine bounded port cases with72 nonempty replays and18 exact directional canonical comparisons; these are not new maintained golden certificates. Task143 adds a fresh post-fix invocation/reference vendor pair with equal full exports and eight matching physical replay forms. Extend the remaining model/corpus evidence. Physical import currently covers a single bound TAP with scan-only storage, not general physical/vendor equivalence.|
| 13 | Concrete physical replay diagnostics | Completed in Task139 after independent review and one verified fix pass. Serial bit/expected/observed, named closed routes, unknown loads and original reservation causes are covered; ordinary fallback and failed-output protection are preserved. |
| 14 | Execute five optional reference-ANTLR cases | Completed in Task138. All six existing adapter methods executed without skips using the pinned reference and matching runtime; commands below. |
| 15 | Reconcile current documentation/checklists | Completed in Task138. Current guide/model/README summaries distinguish completed support and remaining work; historical plans link here. |

## Reference verification

Reference: Honza255/icl_parser at 333f8bc7a54964138ac14a34fde9e942c496436d;
runtime antlr4-python3-runtime 4.7.2. Both are isolated under owned /tmp paths.
The reference is an independent syntax oracle, not normative authority.

    ICL_REF_DIR=/tmp/icl-reference-completion-20261120 /tmp/icl-reference-runtime-20261120/bin/python tests/tests/test_antlr_diff.py

Result: six tests ran, zero skips, exit 0, including all five formerly skipped
reference cases. Temporary paths are reproduction details, not installed
application dependencies or a guarantee that an external checkout remains
available. Task 138 evidence is retained under build/completion-records-20261120/.

## Existing qualified baseline

Task 137 at400d2ba is complete: 28 reset cases per binary variant, normal and
leak-enabled sanitizer suites, Rocky 51 suites/820 cases per variant, source
archive and dynamic/static installation checks. Four archived vendor exports
replay natively with --physical_pdl and preserve 13 explicit loads/seven applies.
All 13 reset-specific implementation plans are complete. This does not close
the broader items above or strict coverage item 1.

See [model acceptance](pattern-model-coverage.md#current-evidence-and-remaining-work)
and [current verification](pattern-generation.md#current-verification).

## Task152 qualified increment

Actual scalar root/child and bound single-TAP raw return histories, plus strict
root opaque named-chain execution/unknown histories, pass40 qualification
gates,62x1008 tests per generator,14 native/14 sanitizer suites,30 quality
checks, packaging,4011 input/31 executable hashes and57 scale processes.
Both Important review findings are fixed in one test-first pass; two current
Minor findings and the prior Task151 numeric-prose Minor remain deferred.
Broader item6/8 topologies, callbacks, timed histories and device transport
remain pending. Physical PDL retains its raw iScan rejection; use normal raw
PDL for these histories. Coverage item1 remains excluded.


## Task153 qualified increment

Explicit callback-only DataRegister procedure execution and reset-source replay
pass43 qualification gates:14 native/14 sanitizer suites,63x1048 tests per
generator,24 quality checks, source packaging and4014 input/31 product hashes.
One fresh independent review found0 Critical,7 Important and1 Minor findings.
A single test-first pass fixes all7 Important findings; the benchmark's original
fixture-dimension labels remain deferred and are explained beside the results.
Fifty-one matched scale/replay processes and six wide-alias memory checks pass.
At1024 targets/four Apply groups, median source runtime is25.9599s; larger routine
workloads need measured scheduler work under item10. Automatic downstream/mixed/
merged callbacks, callback histories and physical output remain item4 work.
Coverage item1 remains excluded.

Task153 is merged into master; seven integration checks,4014 input hashes,
16 copied qualified executables and117 preserved stashes are verified.


## Task154 qualified increment

Conservative primary solver state indexing improves the measured1024-target/
four-group callback workload from26.2120s to2.6123s with identical patterns,
106920KiB peak RSS and60 corrected fresh-directory scale/replay processes.
All46 qualification gates pass, including14 native/14 sanitizer native suites,
64x1058 tests per generator,13 sanitizer CLI modules, six quality checks,
packaging and4016 input/31 product hashes. The single Important benchmark
repeatability finding is fixed in one RED→GREEN pass; one ownership-test
diagnostic Minor remains deferred. The broader item10, remaining callback
functionality and other pending inventory items are still open.

Task154 is merged into master; seven integration checks,4016 input hashes,
16 copied qualified executables and117 preserved stashes are verified.

## Task155 qualified increment

Fixed modeled root parallel raw groups now retain both chain requests,
execute one native CSU, return actual captured words and complete omitted
fields with their current fill. Source/flat/generated exact replay, native
rollback/ownership/copy tests and93 scale/control processes pass. All47 post-fix qualification gates and179 performance/control processes pass;
review's four Important issues are fixed and two Minors deferred. Items6/8 remain
pending for the broader topologies, raw merge and transport work.

Task155 is merged into master; seven integration checks,4018 input hashes,
16 copied qualified executables and117 preserved stashes are verified.

## Task156 qualified increment

The default-model corpus audit is reproducible and wired into corpuscheck.
All1016 required canonical identities pass, including twelve previously
invalid empty ToSelectPort serializations. Fourteen missing-enum inputs now
report names/source context. Declared unsupported and original elaboration
errors stay visible, without invented dependencies or defaults. All52 qualification gates and92 measured processes pass after five test-first
Important fixes; one Minor remains deferred. Broader11/12 acceptance remains open.

Task156 is merged into master; nine integration checks,4024 input hashes,
19 copied qualified native products and117 preserved stashes are verified.

## Task157 legacy launch declaration increment

The existing ScanOutPort LaunchEdge Falling compatibility form now survives
canonical flattening with its original source, Enable, attributes and scope.
This preserves declarations through the current model; it adds no physical
launch timing semantics. The maintained default audit moves one identity to
canonical acceptance (1017 total); four other legacy SIB models reach their
existing PortGroup boundary (55 explicit unsupported total). The 691 original
elaboration errors remain visible. Broader 11/12 remains pending.
All 53 qualification checks and six corpus measurements pass after one
Important test-first review fix; two Minors are deferred. Master integration
is pending.

Task157 is merged into master; eleven integration checks, 4025 input hashes,
19 copied qualified native products and 117 preserved stashes are verified.

## Task158 legacy PortGroup increment

Native membership now resolves declared local ports across commented/inline
lists, keyword names, ranges, repeated instances and segmented functions.
Valid annotations survive canonical flattening without adding data-flow or
PDL target behavior. Current corpus:1024canonical/44unsupportedAccessLinks/
695original elaboration errors. Seven actual models advance to canonical
acceptance; five undefined-member fixtures are explicitly invalid models.
All54 qualification checks pass after three test-first Important fixes, with
14native/14SAN,68x1094 methods per generator, nineCLI methods and21C++quality.
Fresh108scale/6corpus/3guide-example processes pass with exact hashes and the
measured10s/512MiB bounds. Independent review has noCritical/Minor finding.
Master integration is pending.
Broader11/12 remains open; coverage1 remains excluded.

Task158 is merged into master; thirteen integration checks, 4026 input hashes,
19 copied qualified native products and 117 preserved stashes are verified.

## Task159 write-callback endpoint observation increment

The qualified increment enables --model_readback for actual ordinary endpoint
observations during explicit write callbacks. Abstract callback storage and
requested/generated postconditions never become actual observations, including
mixed/inverted alias assumptions and reset defaults. Invoked read callbacks,
physical callbacks, automatic/mixed/together/merged dispatch and broader timing
remain open. This increment does not close whole items4 or6, or excluded item1.
All55 post-fix qualification gates pass (14 native/14 sanitizer,69x1117 each
generator,18 C++ quality,4028 input/36 product hashes). All five Important review
findings are fixed in one test-first pass; no Minor findings.90 actual-observation,
60 existing-callback, six alias and six corpus measurements pass their bounds.
Integration evidence is recorded after the merge.

Task159 is merged into master;17 integration checks pass on4028 identical input
hashes/19 copied qualified native products, with117 stashes preserved. Receipt:
build/callback-endpoint-readback-20261123/merged/receipt.json.


Task162 is merged into master;24 integration checks pass on4036 identical input
hashes and20 copied qualified native products, with117 stashes preserved.
Receipt:build/mixed-callback-aliases-20261123/merged/receipt.json. Broader original
items remain open as listed above; coverage item1 stays excluded.


## Task163 qualified increment

Task163 passes63 final qualification gates:15 native and15 leak-enabled sanitizer suites,73 suites/1189 methods in each generator variant,15 C++ quality checks, source packaging and4039 input/38 product hashes. The72 regular downstream processes peak at4.2884s/373760KiB; six512/1024-bit runs peak at1.1308s/121236KiB. All324 new/preserved matrix and guide processes plus4 dynamic allocation probes pass. Independent review found0Critical/5Important/1Minor; one test-first pass fixes all five Important findings. The benchmark checks postcondition counts, widths and allowed names; exact ordered register/value pairs remain a deferred Minor. Functional regressions check identities and payloads. All26 merged-result checks pass on identical4039 inputs and20 copied qualified executables, with117 stashes preserved. Broader inventory and excluded coverage item1 remain open.


## Task164 qualified increment

Task164 passes 64 final qualification gates: 15 normal and 15 leak-enabled
sanitizer native suites, 74 suites and 1,208 methods in each generator
variant, 12 C++ quality checks, source packaging, and exact hashes for 4,041
inputs and 38 executables. The 72 regular merged-callback runs peak at
4.5367s/394712KiB; six 512/1024-bit runs peak at 1.1075s/121104KiB. All 405
new and preserved matrix/example processes and four dynamic allocation probes
pass.

Independent review found zero Critical, three Important and one Minor finding.
One test-first pass fixes pending-queue transfer between calls, inherited
callback-context barriers, and filtered prerequisite anchors. The benchmark
checks ordered register/value postconditions and endpoint frames separately;
checking their relative Apply-group placement remains a deferred Minor.
Functional regressions check actual frame boundaries. The total deferred Minor
count is 16. All 28 merged-result checks pass on identical 4,041 inputs and 20 copied
qualified executables, with 117 stashes preserved. Broader inventory and
excluded coverage item1 stay open.


## Task165 qualified increment

Task165 passes 68 final gates: 15 normal and 15 leak-enabled sanitizer native
suites, 75 suites/1,211 methods in each generator variant, three C++ quality
checks, corpus audit, source packaging and exact hashes for 4,044 inputs/38
executables. All 72 matched benchmark/replay outputs agree, and four dynamic
allocation probes pass. At 256 connected targets/four value groups, three-run
source medians are 15.9208s for parent 92e6295 and 10.0586s for Task165, a 36.8%
runtime reduction with complete byte-identical outputs. Across current normal
and sanitizer cases, peak time is 11.2675s and peak RSS is 335856KiB. Final current bound misses: 4, against
unchanged 10s/512MiB regular or 10s/256MiB wide limits. Coverage includes
64/128/256 normal targets, 16 sanitizer targets, 512/1024-bit words and
source/flat/generated replay. Generated callback
notes are literal; only source/flat forms execute the callbacks.

Original independent review found 0 Critical, 0 Important and 2 Minor findings.
An actual timing miss made early-exit reporting an Important acceptance
gap; one test-first fix completes the matrix and still returns failure for
missed bounds. Its standalone test passes on normal/static/sanitizer builds.
Timed-out benchmark child cleanup remains deferred. Total deferred Minors:
17, including 16 inherited. Maintained width/mask/serial checks pass on
normal/static/leak-enabled sanitizer products. Reviewer-reported 416 direct
decoder combinations also passed, but that temporary harness was removed;
it is not an archived reproducible qualification artifact. Earlier retained
runs missed the bounds at 256 normal targets (10.8165s) and 64 sanitizer
targets (586672KiB). Runtime varies; larger workloads and the 64-target sanitizer
case are not generally certified. All 13 merged-result checks pass on identical
4,044 inputs and 20 copied qualified executables, with 117 stashes preserved. Broader inventory10 and the other
pending items remain open; item1 remains excluded.

## Task166 qualified increment

Task166 passes 70 qualification gates. Full normal/static generator runs each
cover 76 suites/1,223 methods; affected leak-enabled sanitizer CLI, corpus,
three C++ quality checks, source packaging and four allocation probes pass.
The 15 normal/15 sanitizer native suite results are retained from Task165,
with exact proof for 4,041 shared inputs, unchanged native build rules,
30 executable hashes and original passing raw logs. These are reused
executions. Final hashes cover 4,046 inputs and 38 executables.

All 84 matched raw-merge/serial/source/flat/generated measurements and four
guide-example processes pass. Cases cover 16/64/256 calls, 512/1024-bit words
and 16-call sanitizer runs. Measured maxima are 2.5990s and
403624KiB, within the existing regular 10s/512MiB and wide 10s/256MiB
bounds. Every input/compare word and Apply boundary matches its parent serial
control. No parallel raw speedup or larger topology/latency guarantee is claimed.

Independent review found 0 Critical, 1 Important and 2 Minor findings.
One test-first pass replaces an obsolete empty-merge rejection with exact
serial replay, retaining three unsupported-topology negatives. Deferred
Minors concern separate source/flat reset artifacts and high-bit data in
committed wide benchmarks; independent alternating-byte probes match both
wide widths. Total deferred Minors: 19, including 17 inherited.
One guide run hit LeakSanitizer's sandbox ptrace restriction; the same check
passes outside the sandbox with leak detection enabled. Both logs are retained.
All 13 merged-result checks pass on identical
4,046 inputs and 20 copied qualified executables, with 117 stashes preserved. Broader inventory8 and
other pending items remain open; item1 stays excluded.

## Task167 qualified increment

Task167 passes 74 qualification gates: 15 fresh normal and 15 leak-enabled sanitizer native suites, 77 suites/1,243 methods in each generator variant, affected sanitizer CLI, corpus, 12 changed-range C++ quality checks, source packaging and four allocation probes. Exact hashes cover 4,049 inputs and 38 executables.

All 378 matched measurements have correct observations/replay, and four guide processes pass. The matrix includes 16/64/128-call PIO/raw histories, parent/current serial and current observed/unobserved merges, three source/flat/generated repeats, 512/1024-bit high-bit words and 16-call sanitizer observations. Maxima are 2.6263s and 438028KiB, with 342 cases within the existing regular 10s/512MiB and wide 10s/256MiB bounds. All 36 1,024-bit PIO parent/current serial and observed/unobserved source/flat/generated cases exceed the 256MiB wide memory bound (301332–337624KiB); no time bound is missed. The benchmark retains all misses and exits nonzero. 1,024-bit PIO memory closure remains open under inventory 10, alongside Task165’s 256-call latency misses. Source/flat queries and generated literal notes are reported separately; actual captures, full observation lists, exact serial controls and Apply counts are checked. No parallel observed-merge speedup or larger-scale guarantee is claimed.

A native-only RED probe demonstrates that removing the old guard would expose source value 0 versus generated value 9. Provider-aware serial normalization fixes that trajectory; no-provider/cached/manual/never-read/zero-status/pre-merge controls retain existing batching. Distinct source and flat reset artifacts are checked. Physical/opaque/invoked-read/automatic/timed history limits remain. Broader inventory 4/6/8 and the other pending items stay open; item 1 remains excluded.

Independent review found 0 Critical, 3 Important and 3 Minor findings. One test-first pass fixes both standalone benchmark false-green paths and adds a real callback reset-release/order regression. Missing flat models now fail without skipping generated replay; unobserved frames must match the parent serial output after only observation notes are removed. The reset test fails on a temporary executable missing only the second render latch argument and passes on the qualified executable. The new three-method reporting suite passes on normal/static/sanitizer variants, and both full77-suite/1,243-method generator variants plus affected sanitizer history and packaging checks are refreshed. All 4,046 shared inputs and 38 executables are identical across this Python-only fix pass; the 30 native passes are fresh executions from this task, before the test/reporting fixes.

Three new Minors remain deferred: wide raw cases capture reset0 rather than a high-bit word (independent two-scan512/1024-bit probes pass); failed-row query/note counters derive from fixture expectations; and an older raw-merge guide paragraph still lists the previous modeled-history guard. Total deferred Minors:22, including19 inherited. The 36 wide-PIO memory misses remain performance acceptance work under inventory10; no resource bound is raised.

All 15 merged-result checks pass on identical 4,049 inputs and 20 copied qualified executables. The merge is pushed to master, the owned task branch is deleted, and all 117 stashes remain. Broader inventory items and the retained performance acceptance failures remain open.

## Task168 qualified increment

Task168 passes 76 fresh post-review qualification gates: 15 normal and 15
leak-enabled sanitizer native suites, 78 suites/1,253 methods in each full
generator variant, affected sanitizer CLI, corpus, three C++ quality checks,
source packaging and four allocation probes. All ten focused methods pass
normal/static; sanitizer passes nine functional methods and explicitly skips
the large memory gate. Final hashes cover 4,050 inputs and 38 executables.
The earlier Python-only evidence reuse and pre-review matrix remain archived;
the final full qualification uses the narrowed implementation.

All 378 final matched processes preserve actual observations and exact
serial/source/flat/generated replay; four guide processes pass. All current
cases meet the existing 10s/512MiB regular and 10s/256MiB wide bounds. The 27
current 1,024-bit PIO serial/observed/unobserved cases use
76148–111072KiB and at most 0.1155s,
versus 301396–336164KiB for nine parent controls.
All 369 cases other than those parent misses meet their bounds. The benchmark
retains nine parent memory failures and exits 1. Overall maxima, including
sanitizer cases, are 2.2729s/438160KiB.
This closes the measured wide-PIO case under inventory 10. The earlier
72-process callback follow-up still has three 256-target time misses; that
latency target and broader optimization remain open.

Independent review found zero Critical, two Important and one Minor finding.
Removing a constant objective preserves preference ordering but can change
Z3's choice among equally optimal models. One parent-backed test detects a
different implicit read-enable history that makes a Tcl condition fail;
another detects different physical read targets/comparison masks. Both fail
on the reviewed implementation and pass on the parent and final build. One
test-first fix retains the entire original objective list unless every primary
candidate has a full binary goal and every requested read bit has exactly one
candidate route, as well as requiring a one-frame horizon. These guards reuse
the existing route/pin walks. Symbolic words, constraints, drive count, data
deviations, reset behavior and later-frame preferences remain intact.

The first literal-word substitution failed memory and was reverted. The
partial Massif trace remains diagnostic evidence. An earlier temporary
horizon-guard mutation changed intermediate pin 11 to 9 and failed the
maintained control; the final stricter implementation also passes that control.
The reset-less fixture rejection was a fixture error, not an unsupported
schedule. Partial-X fill, inversion, ascending slices, unforced enables,
reservations, end-state restoration and failed-output preservation pass.

The new deferred Minor is the wide test's ASAN_OPTIONS-based skip: exporting
that variable on an ordinary executable can also skip the gate. Raw final
normal/static logs confirm the memory gate ran. Total deferred Minors: 23,
including 22 inherited. Wide-ASAN memory closure and broader topologies are
not certified by these measurements.

All 14 merged-result checks pass on identical 4,050 qualified inputs and 20
copied executables. The merge is pushed to master, the owned task branch is
deleted, and all 117 stashes remain. Broader inventory work stays open.

## Task169 qualified increment

Task169 passes 80 final qualification gates: 15 fresh normal and 15 leak-enabled
sanitizer native suites, 78 suites/1,253 methods per full generator variant,
affected CLI/corpus, three C++ quality checks, source packaging, reporting and
four allocation probes. Exact final hashes cover 4,052 inputs/38 executables.

A real linker-wrapped Z3 API counter is RED on parent with 997,408 model
evaluations and GREEN at 471,064, below its 600,000-call
work bound. Both runs verify all 1,028 physical frames and 1,024 callback
postconditions. Existing word/mask/serial and pin-choice controls pass on
normal/static and leak-enabled sanitizer builds. The large sanitizer pin-memory
gate remains explicitly unrun; smaller functional cases pass.

The local cache stores complete decoded word/mask pairs only for one successful
primary model. It changes three decode call sites and retains every concrete
state/fill check, objective and constraint. Retained frame expressions keep
the AST-ID keys alive; cache entries never cross a model. Memory is proportional
to unique decoded pairs and their word lengths, with logarithmic map lookup.
No public interface or new dependency is added.

All 450 final matched processes agree with complete parent outputs and actual
observations: 72 word-decoding cases and 378 merge-history cases. All current
cases meet unchanged 10s/512MiB regular and 10s/256MiB wide bounds. The word
matrix covers 64/128/256 connected targets/four groups, 512/1024-bit words,
16-target sanitizer cases and three source/flat/generated repeats. Matched
256-target source medians are 9.3949s parent and
7.9338s current (15.6%
reduction in these runs). Current word cases peak at
7.9704s/304852KiB;
merge cases peak at 0.7677s/438260KiB.
Four guide-example processes pass. The parent already contains Task168's
wide-PIO memory fix; all 378 merge measurements meet their bounds.

Timing varies: the earlier 72-process follow-up retained three 256-target
misses, while this task's fresh 12-process native baseline passed near
9.3–9.4s before the cache. Both records are retained. The native decoder probe
measures 524,292 repeated pairs among 793,612 calls; a partial Callgrind run
hits the existing solver timeout under instrumentation and is diagnostic,
not a native performance result. The deterministic real-call work reduction
does not depend on that timing variation. This closes the measured 256-target
case in the final matrix; broader inventory10 work and larger workloads remain
open. Item1 remains excluded; 23 inherited deferred Minors stay deferred.

Independent review found zero Critical, two Important and one Minor finding.
One test-first pass replaces assertion-based process/counter checks with
explicit errors (including zero/missing counts) and pins the complete work-case
output to the qualified parent SHA-256. Optimized-Python failures, moved
postconditions and an added empty Apply all fail before the fix and pass the
new rejection tests afterward. The three-method reporting suite, real linked
API counter and source package are refreshed. The 79 broader gates ran before
these Python-only fixes; exact proof preserves 4,050 shared inputs and all 38
executables, including both full generator variants and 30 fresh native passes.

The new deferred Minor concerns the description of unobserved merge controls
and labels for aggregate sanitizer RSS peaks. Detailed matrix rows retain the
actual owners and comparisons. Total deferred Minors: 24, including23 inherited.

All 17 Task169 merged-result checks pass on identical 4,052 qualified inputs
and 20 copied executables. The merge is pushed to master, the owned task branch
is deleted, and all 117 stashes remain. Broader inventory work remains open.
