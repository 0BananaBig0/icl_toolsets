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
| 2 | Independently phased TAPs, shared serial storage, intermediate taps, overlapping returns and independent lanes | Task192 adds fixed interface-free fanout with shared full registers, unequal lanes and wire observations, preserving one physical stream and actual per-register histories. [Evidence](#task192-fixed-scan-fanout). Task191 adds equal disjoint shared-input lanes and hierarchical input forwarding. [Evidence](#task191-shared-serial-inputs). Task188 qualifies mixed IR/DR simultaneous groups across distinct-TMS TAPs on equivalent clocks, with original/flat/generated and actual model replay. Task189 adds qualified local standalone-client groups with shared phase/TCK pins and disjoint lanes. [Evidence](#task189-simultaneous-client-interfaces). General independent state/clock phases, sharing outside the qualified fixed case, intermediate taps and remaining overlapping returns/topologies are pending. [Evidence](#task188-independent-tap-irdr-phases). |
| 3 | General independently gated and serial-derived scan/reset events | Task198 adds qualified feedback-generated independent scan-reset pulses; [scope](#task198-feedback-generated-reset-pulses). Broader events remain pending. Retain edge ordering, hidden storage and transient resets across source/flat/generated replay; no guard-only enabling. |
| 4 | Cyclic/self-referential DataRegister updates and callbacks | Task185 adds bounded primary automatic-callback actual observations; broader cycles/events remain pending. [Qualified increment](#task185-automatic-primary-callback-observations). Task167 supports actual native logical merge histories with source-consistent serialized generation; see its qualified increment below. Task164 supports callback-containing iMerge calls using serial fallback and preserves ordinary batching; all64 gates pass (15native/15SAN,74x1208 each generator,12quality,4041inputs/38products),405 matrix/example and4 allocation processes. Review0C/3I/1M: all Important findings fixed; one new Minor deferred(total16). All28 merged-result checks pass on identical4041inputs/20products;117stashes preserved. Task163 passes63 final qualification gates:15 native and15 leak-enabled sanitizer suites,73 suites/1189 methods in each generator variant,15 C++ quality checks, source packaging and4039 input/38 product hashes. The72 regular downstream processes peak at4.2884s/373760KiB; six512/1024-bit runs peak at1.1308s/121236KiB. All324 new/preserved matrix and guide processes plus4 dynamic allocation probes pass. Independent review found0Critical/5Important/1Minor; one test-first pass fixes all five Important findings. The benchmark checks postcondition counts, widths and allowed names; exact ordered register/value pairs remain a deferred Minor. Functional regressions check identities and payloads. All26 merged-result checks pass on identical4039 inputs and20 copied qualified executables, with117 stashes preserved. Broader inventory and excluded coverage item1 remain open. Task162 supports mixed ordinary/callback alias bits with exact native ordinary accesses and preserved callback arguments/source spans; all61 final gates pass (15native/15SAN,72x1169 each generator,9quality,4036inputs/38products),243 matrix and4 dynamic allocation processes. Review0C/2I/1M is fully resolved; merged and pushed;24 integration checks pass on identical source/product hashes. Task161 supports separately queued ordinary requests with explicit callbacks, preserving known caller goals through native frames/state and actual ordinary histories; all59 final gates pass (15native/15SAN,71x1153 each generator,30quality,4033input/38product hashes); merged and pushed;22 integration checks pass on identical source/product hashes. General symbolic/control/serial automatic callbacks remain open. Pending broader callbacks. Task153 adds explicit callback-only procedure execution, masks/aliases, ancestor namespaces, cancellation/error recovery and source/flat/generated/local-reset replay; all43 post-fix qualification gates pass, including63 suites/1048 tests per generator and14 native/14 sanitizer suites. Task159 supports actual ordinary endpoint observations in explicit write procedures with unknown abstract callback storage. General automatic dependencies, modeled merged/invoked-read histories and physical output remain open. Task145's historical declaration checkpoint added typed callbacks and canonical flat preservation. Its seven focused methods,13 native/13 sanitizer suites,54x900 per generator variant,21 quality gates and packaging pass. Independent review reports0Critical/0Important/0Minor; all18 reviewer-designed probes meet expectations. Task144 adds bounded simultaneous feedback from real preceding storage, known/unknown convergence, controlling history, capture ordering and intermediate ownership/rollback. One independent review found0Critical/3Important/0Minor; one verified fix pass resolves all three findings and preserves autonomous goal intent. All13 native/13 native sanitizer suites,53x893 per generator variant,14 focused replay methods,39 quality gates and source packaging pass. Changing feedback-derived external reset events remain item3. |
| 5 | Stateful absolute time and remaining clock semantics | Task197 accepts independent ordinary clock components in period-bearing programs; merged and pushed after qualification and review repairs. [Evidence](#task197-independent-clock-programs). Task196 adds declared-Period oscillator restoration; merged and pushed after full qualification and the evidence-checker repair. [Evidence](#task196-declared-oscillator-restoration). Task195 qualifies representable partial source/ratio overrides in period-bearing programs; [evidence](#task195-mixed-clock-defaults). Task194 qualifies exact separate period operands on native replay and bounded Tessent2026.2 pairs; [evidence](#task194-clock-period-serialization). Merged and pushed to master. Task193 adds explicit period overrides with retained logical clock programs; [qualification](#task193-explicit-clock-period-overrides) passes after one review repair pass; merged and pushed to master. Broader stateful time remains pending. Relative waits, clock overrides and restricted clock-only absolute waits already work; Task141 fixes associated-flat override target/source mapping. Cover stateful events, oscillator restoration/frequency, phases and remaining names/source mappings without inventing periods. |
| 6 | Runtime observations/status and interface/chain history | Task190 preserves explicit ordinary logical read scope and routed target identity; broader history work remains pending. [Evidence](#task190-ordinary-logical-read-scope). Task185 adds bounded primary automatic-callback histories with actual unknown data and once-only status clearing; broader transport remains pending. [Qualified increment](#task185-automatic-primary-callback-observations). Task184 preserves explicit-only modeled named-chain generated histories and old cared masks; see its qualified increment below. Task183 supports standalone opaque-root serial merges and explicit-only named-chain prefix histories; see its qualified increment below. Task172 adds independent fixed opaque children on one static scalar route; see its qualified increment below. Task167 supports actual native logical merge histories with source-consistent serialized generation; see its qualified increment below. Task162 preserves ordinary histories and cancellation through mixed alias projections, with invoked ReadCallBack history guards retained; all61 final gates pass (15native/15SAN,72x1169 each generator,9quality,4036inputs/38products),243 matrix and4 dynamic allocation processes. Review0C/2I/1M is fully resolved; merged and pushed;24 integration checks pass on identical source/product hashes. Task161 preserves actual ordinary histories in mixed callback caller groups, including canceled reads and completed inner frames; all59 final gates pass (15native/15SAN,71x1153 each generator,30quality,4033input/38product hashes); merged and pushed;22 integration checks pass on identical source/product hashes. Task160 adds actual explicit-only scalar opaque-child/root histories while keeping opaque return bits unknown; all58 post-fix gates pass (15native/15SAN,70x1136 each generator,21quality,4031input/38product hashes); merged and pushed;20 integration checks pass on identical source/product hashes. Task155 adds fixed disjoint modeled root named-chain replay and actual explicit-only chain histories with common controls;16 CLI methods, native per-clock references and93 scale/control processes pass, all47 qualification gates pass after four test-first Important fixes, with two Minors deferred; integration is pending. Pending broader histories/transport. Task152 adds actual native scalar root/child and bound single-TAP raw interface histories plus named opaque parallel-chain unknown histories, with variable widths and separate register/raw planes.  Task148 adds opt-in modeled combinational PIO observations at actual scheduled frames, sparse/inverted history, exact per-bit failure counts and clear semantics. A direct independent review found2Important/1Minor issues, all resolved in one verified fix pass;35 qualification gates,14 native/14 native sanitizer suites,57x930 tests per variant,30 quality gates, packaging and108 scale processes pass. Task149 extends modeled observations to DataRegisters through PIO, with actual read-before-write frames, source-ordered manual planes and legacy-note migration. Its full behavioral qualification passes36 gates/58x946 per variant/14 native+14 native sanitizer suites/15 quality gates/packaging; a help/assertion-only fix for two Minor review findings passes13 focused checks/312 methods, preserving28 native and4004 input hashes. Final4006 inputs/31 products and108 behavioral scale processes are verified. Task150 adds actual standalone/client serial-return observations for ScanRegisters and captured DataRegisters, including chain inversion and unequal disjoint lanes. One fresh direct review found2Important observer-path gaps; one verified fix pass resolves both. Final qualification passes37 gates,14 native/14 sanitizer suites,59x962 methods per generator,18 quality checks, packaging and144 scale processes;4007 inputs/31 products are verified. Task151 adds actual bound-TAP IR/DR observations, captured data and common-phase multiple TAPs, with matched Runtime/output scheduling and exact physical-PDL mode. Its fresh direct review reports0Critical/1Important/2Minor; one verified fix pass resolves subscriber state/fill/cache consistency. Final38 gates,14 native/14 sanitizer suites,60x978 methods per generator,18 quality checks, packaging and144 scale processes pass;4008 inputs/31 products verified. Minor compiled-help/prose findings are deferred. Raw/interface/chain and timed histories remain open; real device access needs a supplied transport. Task141 preserves never-read/sparse unknown history and provably zero initial/no-compare status.|
| 7 | Remaining resource ownership and lifecycle | Pending. Task141 fixes compatible modeled-child port ownership with protected intermediate serial states, scope tests and a bounded invariance fast path. Add remaining clock/scan assignment/frequency, pattern-set lifecycle and TAP ownership. |
| 8 | Remaining raw-scan configurations | Task183 supports standalone opaque-root serial merges and explicit-only named-chain prefix histories; see its qualified increment below. Task172 adds independent fixed opaque children on one static scalar route; see its qualified increment below. Task167 supports actual native logical merge histories with source-consistent serialized generation; see its qualified increment below. Task166 supports complete serialized raw calls in valid iMerge bodies on existing modeled scalar/child/named-chain/single-TAP routes; see its qualified increment below. Task160 supports one declared-width scalar opaque leaf on a static complete root route with modeled parent preservation and unknown actual opaque returns; all58 post-fix gates pass (15native/15SAN,70x1136 each generator,21quality,4031input/38product hashes); merged and pushed;20 integration checks pass on identical source/product hashes. Broader dynamic/gated/parallel opaque children remain open. Task155 adds fixed disjoint modeled root named-chain replay and actual explicit-only chain histories with common controls;16 CLI methods, native per-clock references and93 scale/control processes pass, all47 qualification gates pass after four test-first Important fixes, with two Minors deferred; integration is pending. Pending broader topology. Task152 adds strict root opaque parallel chains with per-chain default/stable completion and actual modeled raw observations; broader hierarchical/dynamic/shared parallel and multiple-TAP raw, opaque children and broader raw merge configurations remain open.  Add remaining opaque-child, parallel/multi-TAP and raw-merge configurations and qualify remaining vendor-export forms.|
| 9 | Additional repairable caller-only reset transformations | Completed in Task143 for the inventory's state-dependent/identical-argument caller cases. Recorded invocations cover empty/caught calls and once-only arguments with unchanged procedure bytes. Five review findings are fixed; full post-fix qualification passes. Impossible queues/ownership/internal reassertions and uncertifiable file-return/cleanup boundaries reject atomically. |
| 10 | Broader merge/scheduling optimization and scale qualification | Task201 qualifies larger automatic-primary inputs through bounded objective packing, including 32 eight-bit lanes and a 1024-bit lane under unchanged limits. [Evidence](#task201-primary-callback-solver-scaling). Task185 measures135bounded callback/observation processes; larger independent-callback planner limits remain pending. Task168 closes the measured 1,024-bit PIO memory case; all current matched measurements meet unchanged bounds. Task169 closes the measured 256-target callback latency case in its final matrix, with deterministic model-evaluation work reduction. Broader optimization remains pending. Task165 reduces shared model-decoding cost; its complete72-process measurements and retained budget misses are recorded below. General referenced/resettable and larger workloads remain pending. Task154 conservatively indexes irrelevant callback storage in primary solves; matched1024-target/four-group source median improves26.2120s to2.6123s with identical output,60 scale/replay processes,20 alternating controls,9 compatibility methods and native state/copy checks. All46 qualification gates pass (14 native/14 sanitizer suites,64x1058 per generator, six quality checks and packaging); the single Important benchmark finding is fixed in a test-first pass and one Minor is deferred. Referenced/resettable storage and larger workloads remain separate acceptance work. Task146 fixes the trailing iWrite -ignore_when_value_already_present payload loss for auto/off/on at the shared Runtime boundary; original trace arguments and existing explicit-load policy remain. Six focused methods, all13 native/all13 native sanitizer suites,55x906 tests per generator variant, six quality gates and source packaging pass; four fresh vendor cases and16 native replay forms agree on changed-value PIO payloads. Cover resource/timing barriers and representative larger networks, preserving valid schedules and reporting measured costs rather than global optimality. Task142 retains explicit512-function generic-primary and parent4096-bit solver-limit reproductions; no limit was raised. Task147 counts actual enabled primary events and retains explicit data-goal intent absent from emitted controller writes, in primary and mixed paths. One related Important replay family is resolved in a verified fix pass; seven focused methods,13 native/13 native sanitizer suites,56x913 tests per variant, six quality gates and packaging pass;108 scale processes retain matched controls. This uses existing native event semantics and does not certify physical timing or wider source acceptance. |
| 11 | Remaining model/default/source/reference audit | Task171 adds eight required canonical wide-decimal defaults; current audit is1032 canonical/44 explicit unsupported/687 elaboration failures. See its qualified increment below. Task156 adds reproducible default-model/source audit:1016 actual canonical roundtrips,56 explicit unsupported serializations and691 original default elaboration failures across1334 accepted files/1763 effective modules; twelve valid empty-control serializations and fourteen missing-enum diagnostics repaired. Six post-fix normal/configured-SAN batch measurements pass10s/512MiB; all52 qualification gates pass after five Important review fixes, one Minor deferred; integration pending. Pending corpus/specialization and broader source acceptance. Task140 verifies the eight BSDL/default groups with19 replay methods and eight review fixes. Task142's typed vector-port implementation passes17 focused methods per variant, native caller checks and nine vendor models. Two Important review findings were fixed and full post-fix qualification passes; no Critical/Minor or unresolved blocker remains. All11 former duplicate-port corpus failures reach the existing interface-event boundary (item3), not end-to-end generation success.|
| 12 | Broader vendor and model round-trip qualification | Task197 adds three configured mux/alias/inverted-source reference/generated pairs; [evidence](#task197-independent-clock-programs). Task196 adds three configured declared-period restoration reference/generated pairs; [evidence](#task196-declared-oscillator-restoration). Task194 adds five configured period reference/generated vendor pairs with identical exports and relative-operation traces; [evidence](#task194-clock-period-serialization). Broader qualification remains pending. Task171 adds eight required canonical wide-decimal defaults; current audit is1032 canonical/44 explicit unsupported/687 elaboration failures. See its qualified increment below. Task156 adds reproducible default-model/source audit:1016 actual canonical roundtrips,56 explicit unsupported serializations and691 original default elaboration failures across1334 accepted files/1763 effective modules; twelve valid empty-control serializations and fourteen missing-enum diagnostics repaired. Six post-fix normal/configured-SAN batch measurements pass10s/512MiB; all52 qualification gates pass after five Important review fixes, one Minor deferred; integration pending. Pending wider qualification. Task140 adds six bounded BSDL cases with48 nonempty original/flat/generated replays and12 matching directional apply-group comparisons; Task142 adds nine bounded port cases with72 nonempty replays and18 exact directional canonical comparisons; these are not new maintained golden certificates. Task143 adds a fresh post-fix invocation/reference vendor pair with equal full exports and eight matching physical replay forms. Extend the remaining model/corpus evidence. Physical import currently covers a single bound TAP with scan-only storage, not general physical/vendor equivalence.|
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

All 450 final matched processes pass their hardware and applicable observation
comparisons: 72 word-decoding cases and 378 merge-history cases. The 90
unobserved merge controls compare against the parent after removing only its
observation notes; other cases retain complete pattern comparisons. All current
cases meet unchanged 10s/512MiB regular and 10s/256MiB wide bounds. The word
matrix covers 64/128/256 connected targets/four groups, 512/1024-bit words,
16-target sanitizer cases and three source/flat/generated repeats. Matched
256-target source medians are 9.3949s parent and
7.9338s current (15.6%
reduction in these runs). Current and sanitizer word cases peak at 7.9704s/304852KiB;
merge cases peak at 0.7677s/438260KiB. Both aggregate RSS peaks are from sanitizer
rows. Wall-time and RSS maxima need not describe the same process; use matched
normal parent/current rows for performance comparisons.
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

## Task170 qualified increment

Task170 supports actual native endpoint histories inside/after invoked read
callbacks. Expected arguments remain comparison data: requesting0 while q is9
observes9 and two mismatches. Logical callback register history remains
unavailable after completion, including generated-prefix reuse; untouched
manual read/miscompare bits remain intact. Source, flat and generated
continuations agree on endpoint writes.

Final independent review found0 Critical,3 Important and1 Minor finding.
One test-first pass fixes stale pending markers across helper frames and
coalesced requests, loss of logical-history invalidation in generated prefixes,
and sanitizer benchmark comparisons against their own potentially incorrect
source. The new tests reproduce six frame variants, two seeded/unseeded prefix
variants, five invalid annotation payloads and four sanitizer corruption cases
before the fixes;15 focused methods and four reporting methods pass afterward.
The review's documentation inconsistency is deferred, bringing the total to25
Minors including24 inherited. Older usage paragraphs still state the previous
read-callback restriction; the current endpoint-history section describes the
supported behavior.

Exact expansion metadata records consumed request indexes per callback node.
Frame-indexed sets remove the corresponding pending requests across queue/apply
frame changes without relaxing native unexpanded-access guards. Work and
storage scale with actual node/request memberships; requests are not copied
wholesale into every callback. Completion emits standard PDL iNote metadata
with payload "modeled callback read history unavailable <mask> <register>".
Runtime validates the canonical read-callback register, binary full-width mask,
and restores only those read/miscompare bits as unavailable. Retain these
modeled notes when reusing generated prefixes. They neither execute hardware
nor repeat callbacks. External tools see ordinary notes.

All83 final qualification gates pass afresh on the post-review implementation:
79 suites/1,268 methods per normal/static generator,15 normal and15 leak-enabled
sanitizer native suites, affected CLI/corpus,15 C++ quality checks, packaging,
allocation and reporting probes. Exact hashes cover4,054 inputs/38 executables.
The earlier composed83-gate evidence and828 measurements remain archived as
pre-review evidence; final results are full reruns. The deterministic real
work counter remains471,064 below600,000. The existing wide
sanitizer pin-memory check remains explicitly unrun; its nine functional
controls pass.

All828 final matched processes satisfy unchanged bounds:378 callback cases,
378 merge controls and72 word controls. Callback PIO rows compare direct
parent accesses with expanded current/SAN read callbacks, including equivalent
empty Apply records, logical declarations and unavailable-history notes.
Raw rows remain direct scan-only controls; ordinary serial read-callback
endpoints are covered separately. Two-scan wide raw controls verify the second
high-bit capture. Observed rows compare complete parent bytes; unobserved rows
compare parent bytes with observation notes removed. Every sanitizer replay
form uses the matching parent output; custom sizes that omit16 generate the
missing parent controls. Four guide processes agree on actual9/6,
status4/clear0 and four Apply records.

The final callback matrix peaks at0.7681s/
438308KiB; merge controls peak at
0.7685s/438164KiB; current word
controls peak at8.0754s/
302700KiB. These aggregates include sanitizer
rows. Regular bounds remain10s/512MiB and wide bounds10s/256MiB.

Automatic prerequisite, physical/timed, unexpanded callback and raw-root
callback-storage guards remain. Broader topology, clock/event, history,
transport and scale inventory stays open; item1 remains excluded. The reviewer
checked retained evidence and focused reproductions, without rerunning the
large suites or matrices; the implementer ran all final post-fix checks.

Two final style corrections brace single-statement if bodies without changing
other tokens. All20 normal/static executables remain byte-identical, preserving
their full regression evidence. Seven sanitizer executables differ after
rebuilding; all15 sanitizer native suites and affected sanitizer CLI checks run
again, along with focused normal/static/SAN tests, all15 quality checks and
source packaging. Exact proof retains4,052 unchanged inputs and31 unchanged
raw executables; final hashes cover all4,054 inputs/38 executables.

All 23 Task170 merged-result checks pass on identical 4,054 qualified inputs
and 20 copied executables. The merge is pushed to master, the owned task branch
is deleted, and all 117 stashes remain. Broader inventory work remains open.

## Task171 qualified increment

Task171 decodes bounded decimal ICL bit patterns independently of checked
integer arithmetic. Plain/based decimal values retain sizing, padding,
concatenation/inversion and significant-bit rejection, through the existing
1,048,576-bit limit. Integer expressions, indices and Constant::AsInteger
retain checked signed64-bit limits. No new dependency, public option or
arithmetic widening is introduced.

All eight hex CLI control methods pass immutable parent21a6aea. Decimal
positives fail through its64-bit literal limit; the compiled native regression
also fails for that reason before the fix. All eight repaired corpus models
independently reproduce the old error with an immutable parent inventory.
They now round-trip canonically and are added to the required registry.
The unchanged2,562 source files produce1,032 canonical models,44 explicit
AccessLink serialization limits and687 original elaboration failures across
1,763 effective modules/1,334 accepted files. The eight repaired records are
the only changes in success/stage classification. Four oversized width
cases remain explicit resource/integer failures.

All88 qualification gates pass:80 suites/1,276 methods per full normal/static
generator,15 normal and15 leak-enabled sanitizer native suites, affected
CLI/corpus, six C++ quality checks, source packaging and existing work/
allocation/reporting probes. Exact hashes cover4,058 inputs/38 executables.
The real work counter remains471,064 below600,000.
The existing wide sanitizer pin-memory case remains explicitly unrun; its
nine smaller functional controls pass.

All846 measured processes pass unchanged bounds:12 native parsing/constant
decoding processes plus378 callback,378 merge,72 word controls and six matched
sanitizer headroom processes. Each native
process checks all ten exact all-ones values at64/65/96/127/128/256/1024/4096/
16384/65536 bits, for120 checked values. Three repeats cover parent-hex/
current-hex/current-decimal/SAN-decimal modes. A current test-only harness linked
against the immutable parent expression object provides the parent hex control;
parent decimal remains unsupported, with no performance claim. An old
executable that ignores benchmark flags and exits0 is rejected in all nine
candidate rows. Process status, exact completion, fixture/binary hashes and
time/RSS are checked.

An initial sanitizer mixed-group scale gate misses its combined10s/512MiB
bound; its failed17-method suite is retained. A standalone reproduction passes
at521872KiB, and matched medians show the first candidate about3.5MiB above
parent. Allocation counts are4 in both. A uint64/from_chars shortcut reduces
allocations to2 but raises sanitizer RSS further (median523704KiB versus
518376KiB parent), so it is discarded. Final Literal preserves the original
scalar path for tokens up to18 characters, whose decimal values fit signed64;
longer tokens use bounded limbs. Scalar allocation counts are4 in both.
A pilot restores median RSS to518344KiB current versus518192KiB parent.

The six final headroom runs match complete outputs, meet the original bounds,
and have median RSS518272KiB current versus
518276KiB parent. The extra relative comparison
allows1MiB for observed RSS variance; every original10s/512MiB bound remains
strict. Full qualification reruns on the final source; earlier candidates are
historical diagnostic evidence.

Native current decimal median is0.016867s
with peak4904KiB; current hex median is
0.005561s/5048KiB
and parent hex median is0.005531s/
4912KiB. Sanitizer decimal median is
0.038125s/
14904KiB. These include fixture IO and
native parsing, rather than isolated conversion-kernel timings. All meet
10s/512MiB. The32-bit limb decoder processes nine digits per multiply/add.
Work is quadratic for very large decimals and storage is linear in significant
bits; routine near-limit inputs need separate measurement and may justify a
different algorithm.

All existing replay controls match actual parent outputs: complete bytes for
observed rows, parent bytes with observation notes removed for unobserved rows.
Raw callback-matrix rows remain direct scan-only controls. Aggregate callback,
merge and current word peaks include sanitizer rows:
0.7679s/438480KiB,
0.8175s/437968KiB and
7.9508s/302876KiB.
Regular bounds stay10s/512MiB and wide bounds10s/256MiB.
Four guide processes match exact128-bit data/miscompare masks and status3/clear0.

Inventory11/12 advances; AccessLink serialization, broader source/model
acceptance and original topology/clock/history/transport/scale work remain open.
Item1 remains excluded and25 inherited Minors remain deferred. Review and
integration are recorded after they finish.

Final independent review found zero Critical, two Important and two Minor
findings. One test-first fix rejects nested literal syntax in decimal data
at the native token API and requires the sanitizer mode in the benchmark.
Two real-harness reporting methods are maintained under expression-check.
All 88 qualification gates and 846 matrix measurements rerun on repaired
source. Two nonblocking test gaps remain deferred: an independent assertion
on a parameter-sized decimal payload, and a maintained limb-growth negative.
There are 27 deferred Minors including the 25 inherited findings.

All 27 Task171 merged-result checks pass on identical 4,058 qualified inputs
and 20 copied executables. The merge is pushed to master, the owned task branch
is deleted, and all 117 stashes remain. Broader inventory work remains open.

## Task172 qualified increment

Multiple fixed declared-width opaque leaves now share one complete static
scalar route with independent default/stable payloads and length knowledge.
Each actual instance/client owns its private interval and queue. Capture
stays unknown; public targets, frames and source/flat exports contain no private
register. Root and local histories retain their explicit-request scope.

Native and CLI tests first fail at the immutable parent800a41f's disconnected
route; the same fixture already passes with one child. The shared expansion
checks route membership, control bindings, side channels and aggregate width
before allocating private storage. Raw projections use ordered root offsets
and copy only child payloads. Scheduling failures roll back all queues;
subscriber errors retain already committed native state and every child queue.

One root -stable flag cannot describe a stable child alongside an unknown
child. Generated iNote -comment with quoted text preserves only the matching
committed child's controlling knowledge. Comment/status tags both replay,
matching repeats are idempotent until the next hardware commit/reset, and
unmatched or malformed note text stays informational under IEEE1687-2014
section7.9.21 (printed180). Such text cannot invent length knowledge: a later
scan still rejects if an active child's length is unknown. No note adds a
hardware operation or actual observation. Legacy user notes remain informational.

The independent final review finds0Critical/3Important/0newMinor. One test-first
fix pass repairs standard tagged/quoted note handling, informational-note
compatibility, duplicate-note replay and whole-root payload copies. All17CLI
methods pass, including source/flat/generated/local-reset replay, repeated
instances, mixed widths, sparse masks, inversion, independent defaults/length,
unknown histories/status, copy/rollback/subscriber semantics, reservations,
privacy, modeled siblings and namespace/split-file/generated-prefix cases.

All86 post-review gates pass:81 suites/1,293 methods per full
normal/static generator,15normal/15leak-enabled SAN native suites, affected SAN
CLI/corpus,21C++quality checks, source packaging and work/allocation/reporting
checks. Final hashes cover4,062inputs/38executables. Earlier composed evidence
is archived; the C++ executions rerun after the three semantic review fixes.
A final braces-only quality correction produces38 byte-identical executables
after rebuilding, with4,061 shared inputs unchanged and only the rendering
source's braces changed. Their completed behavioral results remain valid;
all21quality checks and the archive-native check rerun and pass.

All183 timing/RSS measurements pass:12 immutable-parent one-child source
controls and three source/flat/generated repeats per current/SAN configuration.
Current cases cover1/2/8children at8/64/256/1024bits and32children at8/64/256bits;
SAN covers2/8children at8bits and2children at256/1024bits. Complete input/compare
words, actual data/status notes,8rawscans/9CSUs, widths, hashes and original
bounds are checked. Current one-child forms match complete parent output.
The parent multiple-child RED stays unsupported; no speedup claim is made.
Peak across matrix owners is0.3672s/443820KiB.
Limits stay10s/512MiB for opaque cases; one-child wide controls retain256MiB.
Four guide processes agree on two unknown8-bit returns, parent5/19 and
status0/clear0.

Two additional native allocation probes keep the same8,200-bit root and8,192
opaque bits with8x1024 or32x256children. Before the fix, full-root-sized
allocations grow72→120; after the fix they are56 and
56, with increase0 against
the test's maximum4. Both probes check identical complete root output.
This measures whole-process allocation work, not an isolated decoder or a
historical wall-time improvement.

Qualification also repairs an inherited distribution defect: Task171's
tests/data decimal fixture was omitted. The extracted archive's new native
pattern-expression-check first fails for that missing fixture, then passes
after the dist copy rule includes tests/data. Full distribution/install/
uninstall/native-consumer verification passes.

Retained earlier evidence includes six successful functional SANsource/flat
2x1024 rows at442796–443812KiB that fail an incorrectly assigned extra256MiB
bound. The established opaque recorder requires10s/512MiB; a measured300MiB
boundary regression fixes classification for multiple-child cases and retains
the one-child wide-control bound. All183 runs rerun after that correction and
again on final reviewed C++ source. Prior package-only/recorder reuse proofs
remain historical; no old C++ execution substitutes for final qualification.

All2,562 corpus source identities and classifications remain unchanged:
1,032canonical models,44AccessLink serialization limits,687default elaboration
failures. Inventory6/8 advances without closing broader dynamic/gated/parallel,
multi-TAP/timed/physical/merge/history/transport work. Item1 remains excluded;
27 inherited Minors remain deferred. Integration is recorded after completion.

All 17 Task172 merged-result checks pass on identical 4,062 qualified inputs
and 20 copied executables. The merge is pushed to master, the owned task branch
is deleted, and all 117 stashes remain. Broader inventory work remains open.



## Task173 standard callback history notes

Empty iNote commands and malformed callback-history text now remain
informational. Runtime selects a note's text only for a one-argument legacy
note or a two-argument standard comment/status note. Unknown tags and extra
arguments cannot invalidate seeded history. Invalid masks, widths, targets
and non-callback names are ignored under IEEE1687-2014 section7.9.21,
printed180; no note text raises the former history-annotation error.

New callback-history metadata uses quoted iNote -comment text. Both standard
tags and existing legacy generated prefixes restore exactly the requested
unavailable bits. Matching repeats are idempotent. Ordinary endpoint actual
data, miscompares/status and unaffected callback bits remain intact. Notes
cannot provide actual callback data. Existing local-reset and merge replay
continue through the same Runtime and renderer.

The actual parent reproducer aborts an empty note with a vector range error.
Test-first regressions also expose malformed-note aborts, extra-argument
history invalidation, standard tagged prefixes inventing seeded read data,
and untagged generated metadata. All20 focused methods now pass in normal,
static and leak-enabled sanitizer builds. Their literal partial-read example
has actual q=9, one compared low-bit mismatch, unaffected upper read bits01
and miscompare bits10, with requested lower callback histories unavailable.

The final independent review finds0Critical/1Important/0Minor. Its merge-order
finding is fixed test-first: IEEE7.9.18(b), printed177, requires direct merge
notes before call bodies. When history-note ordering may affect a merge,
Runtime saves resolved calls and evaluated arguments until merge-end, then
runs bodies in encounter order. Complete literal merges with no valid history
metadata retain immediate calls; ordinary/malformed literal notes preserve
that behavior. The same validator serves restoration and this analysis.
In-call getters and history seeds now agree with generated prefixes for legacy and
both standard tags, split sources and modeled/unobserved modes. Existing
once-only arguments and callback partial-commit/error recovery tests pass.
The generator's existing notes-first ordering and scheduling are preserved.
One fix wave is used, with no rereview; all52gates rerun after that change.

All52 qualification gates pass on4,062input/38product hashes:81suites/
1,298methods in each full normal/static generator regression,15normal and
15SAN native suites, affected SAN callback/history/opaque checks, three C++
quality checks, corpus checks, four reporting methods per variant and the
complete source distribution/install/uninstall/archive-native check.

The maintained callback benchmark passes378 timing/RSS processes:16/64/128
calls at4bits and two calls at512/1024bits for PIO/raw routes, with three
source/flat/generated repeats of parent serial, current serial/merged/
unobserved modes. SAN checks16calls/4bits in both route kinds and all forms.
Direct parent controls carry the same standard comment text. Every full
output, actual observation list and two-applies-per-call count is checked
under the existing10s and512MiB/256MiB bounds. Peak across all owners is
0.8170s/438724KiB. No speedup is claimed.

All2,562 corpus identities and classifications remain1,032canonical,
44unsupported AccessLink cases and687default elaboration failures. Broader
inventory2–8/10–12 stays open, item1 stays excluded, and inherited27Minors
remain deferred. No review blocker remains; integration is recorded below.

All10 Task173 merged-result checks pass against identical4,062qualified inputs
and20copied executables. Master is pushed, the owned task branch is deleted,
and117stashes remain. Broader inventory work remains open.


## Task174 opaque child histories across generated prefixes

Explicit fixed opaque-child scans now retain their local read and miscompare
histories when the generated root-scan prefix is continued. Generated quoted
comments identify the root, requested child and exact comparison mask. The
native scheduler validates that identity against its committed raw-CSU
snapshot. A provider supplies unknown actual bits, unknown miscompares for
cared bits and zero miscompares for don't-care bits. Comments supply no actual
data, add no hardware transaction and never recount global failures.

Runtime retains pending intent separately from actual history. Malformed,
unmatched, unanchored and reset-invalidated notes cannot overwrite prior
observations. A completed raw CSU can support a supplied matching interval
identity; ordinary frames cannot. Automatically emitted comments name only
explicitly requested children, so root scans alone create no child histories.
Both standard tags and matching duplicates preserve hardware/status behavior.
Histories persist through later root scans and resets. No provider still means
actual hardware observations are unavailable.

Real parent failures precede implementation: four missing-history/annotation
failures, then two ordinary-frame anchoring failures and a third completed-CSU
scope failure. All25 focused methods pass in normal/static/SAN, including
one/two children, sparse masks, old-history preservation, namespaced split
inputs and local resets. Native tests cover copy/reset, rejected-scan rollback
and subscriber exceptions after commit. The existing20 callback-history
methods and five multiple-child reporting methods also pass.

All65 qualification gates pass on4,062input/38product hashes:81suites/
1,306methods in each normal/static generator regression,30native suites,
21C++quality checks, affected SAN CLI, corpus and source packaging checks.
All2,562 corpus identities retain1,032canonical,44unsupported AccessLink and
687default elaboration-failure classifications.

All696 timing/RSS processes pass unchanged bounds:378callback,216multiple-child
and102single-child rows. The multiple-child grid covers1/2/8/32children at
8/64/256bits and1/2/8children at1,024bits. All57generated-form processes rerun
live getters for every named child and root/status observations, preserving
whole-output equality, exact raw words and CSU counts. Parent native-source
controls carry matching informational comments; this is no claim of old-parent
prefix support or speedup. Peak across all owners is0.8678s/
446284KiB. Two projection-allocation probes retain counts
[56, 56] at an8,200bit physical root; four guide processes agree.

The pre-existing logical-root scope discrepancy remains the next separate
item6 increment: child-only source has no root-interface history, while its
lowered physical-root prefix exposes one. Both the immutable parent and this
candidate reproduce it (root-scope-followup.json). This task restores child
histories; it does not close root-history parity or broader inventory2–8/10–12.
Coverage1 remains excluded and27inherited Minors remain deferred. Final review
and integration are recorded below.

Task174 independent review:0Critical/1Important/2Minor. The Important finding
requires documenting an inherited status-clear limitation before integration:
when a prefix successfully evaluates iGetStatus -clear, generated PDL retains
the returned value but omits the clear. A five-failure root comparison followed
by a clear and a don't-care child scan therefore leaves appended status0 in
source execution but5 after generated-prefix replay. Clearing for the first
time in the appended suffix correctly returns5 then0 on both forms. Parent
and candidate reproduce the prefix discrepancy; it remains a separate item6
behavioral fix. Evidence is retained in the Task174 independent-review/
nonzero-clear-prefix directory. This documentation requirement is resolved;
no product code changed after the65-gate qualification and no rereview ran.

The root-scope follow-up also covers preservation of an earlier root history:
a child-only scan retains old logical-root miscompares in source execution,
while physical-root replay replaces them with zeros. Parent and candidate
both reproduce it (independent-review/older-root). Neither root-history case
is closed by Task174.

Two Minor findings remain deferred: a stale note after an ordinary frame can
change the unobserved getter's rejecting diagnostic, and oversized rejected
annotations make transient parser copies proportional to their input. Actual
modeled data validation and the accepted1Mbit limit hold. Together with the
inherited27, there are29deferred Minors. Real-device transport, broader modes
and scales beyond the recorded matrices remain unqualified.

All10 Task174 merged-result checks pass against identical4,062qualified inputs
and20copied executables. Master is pushed, the owned task branch is deleted,
and117stashes remain. Broader inventory work remains open.


## Task175 logical root histories after child lowering

Child-only scalar raw scans no longer create a logical root-interface history
through generated-prefix replay. Earlier root read data and miscompare masks
also remain unchanged, matching source execution. This covers modeled and
fixed opaque children. Explicit root scans retain their ordinary histories.

Native lowering marks the physical-only root operation. A quoted pre-scan
comment contains the exact canonical command, including input, comparison and
stable option. Runtime binds adjacent matching comment/status notes to the
root scan; malformed, mismatched or stale notes remain informational. Replay
regenerates one canonical comment even for matching duplicates. The provider
still samples and counts every physical comparison, omitting only publication
of the physical root's logical history. No data snapshot, extra CSU or failure
recount is introduced. Existing opaque-child identity comments still restore
their explicitly requested unknown histories.

Immutable-parent regressions reproduce three opaque and two modeled failures
for absent and earlier root histories. All32opaque focused methods and19modeled
raw methods pass, covering suffix status/clear, reset persistence, exact typed
metadata, stale notes and provider-unavailable behavior. Six reporter methods
retain strict hardware counts and matching hand-derived parent comments.

All65qualification gates pass on4,062input/38product hashes:81suites/
1,314methods per full normal/static generator,30native suites,24C++quality
checks, affected SAN CLI, corpus and source packaging. All696matched timing/
RSS processes pass unchanged bounds, including57multiple-child generated
processes with live getters. Peak across owners is1.1189s/
444584KiB. The two8,200bit-root allocation probes retain counts
[56, 56]; four guide processes agree. No speedup is claimed.
All65gates rerun fresh after the independent review fixes; both81-suite
generator variants execute in full with no suite reuse. Earlier passing runs,
assertion failures and parent-fixture correction evidence remain separate.
Corpus identities/classifications remain1,032canonical,44unsupported
AccessLink and687default elaboration failures across2,562identities.

The inherited loss of an evaluated in-prefix iGetStatus -clear remains the
next separate inventory6 fix. This increment closes logical root-history
absence/preservation on these routes, not general callback/local/transport
history parity or broader topology/timing acceptance. Original2–8/10–12remain
open,1excluded and29Minors deferred. Final review/integration follow below.

The independent review finds0Critical/3Important/0Minor. All three Important
findings are fixed in one test-first pass. Truncated or noncanonical scope
annotations cannot hide source histories. Unconsumed reserved comment/status
notes receive one informational prefix in output, preserving their original
tag and visible text while preventing normalization, ownership elision or
queue replacement from activating them on replay. Repeated generation stays
byte-stable. Native returned scope now comes from the winning root request,
matching the committed subscriber. CLI tests reproduce nine failures before
the fixes and pass all32methods afterward; the native overwrite test changes
from one failure to zero. All65gates pass fresh after the code changes, with
both81x1,314generator variants,30native suites,24quality checks and696measured
processes. No rereview runs; the29inherited Minors remain deferred.

All10 Task175 merged-result checks pass against identical4,062qualified inputs
and20copied executables. Master is pushed, the owned task branch is deleted,
and117stashes remain. Broader inventory work remains open.


## Task176 evaluated status clears across generated prefixes

Successful iGetStatus -clear calls now retain their position in generated
prefixes. The original query still returns its pre-clear count; appended
queries see zero and later comparisons accumulate from that boundary. Read
data and miscompare histories stay intact. This closes the inherited five-
failure clear discrepancy recorded by Task174 and retained under Task175.

Runtime records an ordered internal clear event with invocation/callback
context. Generated level0 PDL represents it as a quoted standard comment with
the observed canonical decimal count. Replay validates that count against
available actual status before accepting the clear. Unknown, unavailable,
mismatched or malformed annotations stay informational and render inert, so
they cannot invent a clear or become active after regeneration. Provably zero
clears work without a provider. No hardware operation, extra CSU or failure
recount is added. Called merge streams retain clear barriers; direct level1
merge operations keep their existing rejection. The small shared parser is
inline in its header, preserving Runtime's independence from the model backend.

Three genuine immutable-parent CLI failures precede implementation: PIO,
scalar raw and opaque-child prefixes retain old counts and add later failures
to them. The native provider initially rejects the missing event. All13focused
methods now pass in normal/static/SAN, covering old histories, repeated zero
clears, actual callbacks, merged calls, queued/canceled reads, reset persistence,
both tags, malformed counts, unavailable providers, namespaces, local resets
and bound TAP IR observations. Seven reporter methods cover recorded/live
clear replacement, including nonzero counts, and exact native parent controls.

All68qualification gates pass on4,063input/38product hashes:82suites/
1,327methods per full normal/static generator,30native suites,27C++quality
checks, affected SAN CLI, corpus and complete source packaging. All696matched
timing/RSS processes pass unchanged bounds, including57multiple-child generated
processes with live getters. Their recorded clear comments are replaced by
live clear queries once, preserving full outputs, actual counts, raw words
and CSU counts. Parent fixtures inject only the new clear comments, because
the immediate parent already emits root-scope and child-identity annotations.
Peak across owners is0.8193s/445152KiB. The two
8,200bit-root allocation probes retain counts[56, 56]; four
guide processes agree. No speedup is claimed. All2,562corpus identities retain
1,032canonical,44unsupported AccessLink and687default elaboration failures.

These annotations restore this tool's continuation state; other PDL consumers
see standard informational comments. Real-device transport, modeled child-
local history restoration and broader topology/timing work remain open in
original2–8/10–12. Coverage1stays excluded and29Minors remain deferred. Final review and integration are recorded below.

One independent final review found two Important issues. Both are fixed in
one test-first pass: rejected clear annotations no longer mark a merge as
observed or consume its history deltas; zero clears at the outer merge edges,
including adjacent clear-only calls, preserve same-side batching and captures.
Interior clears retain their ordering barriers. The13-method focused suite
passes in all three builds, with27changed-C++quality checks and all68fresh
gates. No qualification gate or generator suite is reused from an earlier run.
Status preview shares the existing lazy model and retains unpublished deltas
until an ordinary observation. Custom provider integrations without the optional
preview callback accept only already provable zero annotations; ordinary getter
behavior is unchanged. The29inherited Minor findings remain deferred.

All10 Task176 merged-result checks pass against identical4,063qualified inputs
and20copied executables. Master is pushed, the owned task branch is deleted,
and117stashes remain. Broader inventory work remains open.


## Task177 static modeled-child raw histories across prefixes

Generated prefixes now preserve explicitly requested child-local read data and
miscompares on static fully modeled scalar routes. A qualified eight-bit child
returns165,mask165,status4 in source and in original/flat/generated continuations.
Disjoint instances retain their own local output parity. Root omission metadata
still preserves absent or older root history; child projections add no CSU and
do not count the root comparison twice.

Post-CSU standard comments carry root/child identity and expected-mask intent,
never an observed return value. The scheduler reuses cached raw-route intervals
and inversion parity. A neutral preview validates adjacent metadata against a
successful canonical root scan; the provider projects actual returned bits and
retains unpublished deltas until ordinary getters. Rejected notes stay inert
without changing merge observation, old histories or hardware scheduling.
Only one most-recent actual root word is retained, with no per-child snapshot
or repeated full-root normalization. Custom providers need the optional history
validator to restore these child intents; no provider invents observations.

Three initial parent methods produce four genuine missing-child-history
failures. All16focused methods pass in normal/static/SAN, including overwritten
requests, partial care, unknown bits/status, reset/cancel, old root/child history,
typed duplicates, absent provider, merge calls and namespaced local-reset source.
Four new reporter methods reject fake zero-exit generators, missing sanitizer
and corrupted observation lists. The existing seven opaque reporter methods
retain exact controls after removing clear-note injection already native in
Task176. Parent fixtures inject only the new modeled-child identity comments.

All73qualification gates pass on4,066input/38product hashes:83suites/1,343methods
in each generator,30native normal/SAN suites,30changed-C++quality checks,
affected sanitizer CLI, unchanged corpus and source packaging. All912matched
measurement processes pass existing bounds:the prior696plus216modeled-child
runs. The new matrix includes1/2/8/32children at8/64/256bits and1/2/8children
at1024bits;57generated processes issue live child data/miscompare getters.
All raw words, comparisons, counts and complete outputs match native parent
controls. Existing57opaque generated processes remain qualified too.

Static scalar modeled child restoration is complete for these measured cases.
Dynamic/gated/parallel/TAP/timed routes and real transport remain in original
items2–8/10–12. Other PDL consumers see informational comments. Coverage1stays
excluded, and29inherited Minors remain deferred. Final review and integration are recorded below.

Measured peak across912processes:1.0183s/444704KiB; no speedup claim.


One fresh independent final review found0Critical,3Important and1Minor.
All three Important findings are fixed in one test-first pass:the local-reset
verification Runtime receives its own normal/status/history callbacks; scalar
metadata cannot bind a noncanonical -chain default source spelling; an off-route
child annotation follows the existing route diagnostic and remains informational.
The16focused methods pass in all three builds, and all73final gates are fresh:
83x1,343methods per generator,30native/30quality checks and912measurements.
No gate or generator suite is reused from a prior attempt, and no second review
is substituted for the verified fixes.

One new Minor remains deferred:typed C++ function pointers to the earlier
three-argument MakeModeledReadback overload require adaptation because its new
declaration has four parameters. Ordinary three-argument calls still compile.
The29inherited Minors retain their prior dispositions; total30deferred Minors.
Broader topology, timing and real-device work remains in the original inventory.

All13 Task177 merged-result checks pass against identical4,066qualified inputs
and20copied executables. Master is pushed, the owned task branch is deleted,
and117stashes remain. Broader inventory work remains open.

## Task178 dynamic modeled-child raw histories across prefixes

This increment restores already supported dynamic scalar child histories across
generated prefixes. The old route supplies width, offset and inversion even
after a child closes or changes its selection. In the nested example, the
child keeps data/mask0b101001011 and status6 after its current width falls from
nine bits to one. Dynamic intent comments carry expected masks and identities,
with no observed values, extra CSU or repeated comparison accounting.

One immutable pre-CSU route is committed after successful execution and shared
across scheduler copies. Child projection caches remain local. Existing
canonical root provenance, neutral preview and actual-root observations supply
the histories. Static and dynamic note families stay distinct; malformed,
off-route and noncanonical notes retain their informational behavior.

Initial parent failures and current focused results are retained under
build/dynamic-child-prefix-20261125. Final qualification and review follow below.
The30 inherited deferred Minors retain their existing dispositions. Original
items2–8/10–12 remain broader than this increment; coverage1 remains excluded.

All78qualification gates pass on4,069input/38product hashes. Each normal/static
generator passes84suites/1,357methods. All30native normal/SAN suites,
21changed-C++quality checks,14dynamic-focused methods per build, existing
16static-child/13status-clear/32opaque/20callback-focused methods, affected
sanitizer CLI, unchanged corpus and source packaging pass. All gates are fresh.

All1,128matched measurements pass existing10s/512MiB bounds, with single-wide
controls256MiB:378callback,102single-opaque,216multiple-opaque,216static-child
and216dynamic-child processes. The dynamic matrix covers1/2/8/32children at
8/64/256bits and1/2/8children at1024bits, each closing and reopening routes.
It checks actual old words/masks, changed root widths, ordered comparisons,
failure counts and complete parent/current outputs. There are171live generated
history processes across the three child-prefix matrices. Two allocation probes
and four guide processes pass. Measured maximum across all1,128processes is
2.3113s/445204KiB; no speedup or unmeasured-scale claim.

Parent static controls now use their native notes. Only the new dynamic intent
comments are injected in dynamic parent controls; no hardware operation,
observed value or failure count is injected. Independent artifact checks
validate complete matrices, identities, values, shapes and hashes.

Corpus classifications remain1,032canonical/44explicit unsupported/687default
elaboration failures across2,562source identities. This increment restores
supported dynamic scalar histories, not general topology/timing or transport.

One fresh independent final review found0Critical,1Important and1Minor.
One test-first repair fixes the Important native API finding:two identical
successive root scans can execute with different output polarity. Returned
and actual-observed dynamic scan operations now carry a non-owning execution
identity. The scheduler rejects stale results, while copies retain their own
committed route. No old full-route snapshot is retained by a saved operation.
The native regression fails on the pre-repair executable and passes after the
fix. All78gates are fresh after repair, including84x1,357methods per generator,
30native suites,21C++quality checks and all1,128measurements. No rereview.

One new Minor is deferred:dynamic benchmark --help inherits the static runner
description. It does not change the measured workload or checks. The30inherited
Minors keep their dispositions; total31deferred Minors, including the previously
recorded typed-factory-pointer compatibility limitation. Broader topology,
timing, physical transport and excluded coverage1 retain their existing scope.

All16 Task178 merged-result checks pass against identical4,069qualified inputs
and20copied executables. Master is pushed, the owned task branch is deleted,
and117stashes remain. Broader inventory work remains open.

## Task179 dynamic scalar routes with fixed opaque leaves

This increment uses the existing private opaque-storage model on explicitly
opened scalar binary bypass routes. Structural validation walks both mux arms,
checks cycles and complete root-input termination, and follows private data
consumers through mux arms. Execution projects and completes only the private
storage on the actual pre-CSU route. Inactive queue knowledge is preserved;
successful closure uses the old active width, offset and parity.

Opaque read data stays unknown. Cared unknown bits produce unknown mask/status;
uncared mask bits are zero. Root-only scans do not invent child histories.
Existing no-bypass conditional-chain and general opaque-merge restrictions stay
in force. The previously deferred opaque-note diagnostic remains unchanged.

Evidence is under build/dynamic-opaque-routes-20261125. Qualification and review
follow below. Original items2–8/10–12 remain broader than this increment;
coverage1 is excluded and31inherited Minors retain their dispositions.

All83qualification gates pass on4,072input/38product hashes. Both normal/static
generators pass85suites/1,379methods. All30native normal/SAN suites,
15changed-C++quality checks,22dynamic-opaque methods per build, existing
focused history suites, affected sanitizer CLI and source packaging pass.
The corpus retains1,032canonical/44explicit unsupported/687default elaboration
failures across2,562source identities. Final input and product hashes agree.

All1,344matched measurements pass10s/512MiB bounds, with single-wide controls
256MiB:the five prior matrices1,128processes plus216dynamic-opaque processes.
The new workload covers1/2/8/32leaves at8/64/256bits and1/2/8leaves at1024bits,
with three repetitions and sanitizer samples. Every run checks13physical scans,
alternating route widths, complete unknown child words, masks, known root words
and status. There are228live generated-history processes across four child
matrices. Two allocation probes and eight guide processes pass. The measured
maximum across1,344processes is0.8180s/446892KiB.
No speedup or unmeasured-scale claim is made.

The new parent control is a fully modeled unknown-capture twin. Opaque dynamic
source was unsupported in that executable. Physical operations and observations
agree after normalizing only the independently checked child-intent family;
this is not a byte-identical unsupported-parent source claim. No observed data,
hardware operation or failure count is injected. Existing modeled dynamic
controls now use their native notes, with obsolete manual injection removed.

An initial measurement attempt correctly failed all216rows because the shared
validator expected zero child SO expectation words in history annotations.
This workload's child SO expectations contain uncared X; returned miscompare
masks are zero because those unknown child bits are uncared. The runner now
checks the fixture's expected SO word in each annotation; no observed output or
bound was relaxed. The failed rows remain recorded, and final gates were run
fresh after the correction.

Final independent review found0Critical/3Important/1Minor. All three Important
findings are fixed in one test-first repair wave: ordinary parent access leaves
inactive opaque queues untouched; modeled siblings retain actual histories in
mixed networks; eligibility proves a genuine bypass using canonical selectors.
The last proof tracks necessary selector conditions in64-bit batches, rejecting
contradictory choices at shared gates. Its65-selector boundary is tested; more
complex valid correlations may be conservatively rejected. No rereview ran.
The final83fresh gates include22focused methods per build and15C++quality checks.
There are0unresolved Critical/Important findings and32deferred Minors. The new
Minor concerns narrative wording: expected SO care words contain uncared X,
whereas returned miscompare masks are zero. Tests and worked examples distinguish
these correctly; that wording correction and31inherited Minors remain deferred.

All19 Task179 merged-result checks pass against identical4,072qualified inputs
and20copied executables. Master is pushed, the owned task branch is deleted,
and117stashes remain. Broader inventory work remains open.

## Task180 automatic parent-only opaque preparation

The next item8 increment allows a closed fixed opaque child to use the native
planner's real modeled parent-opening CSUs, followed by its requested raw CSU.
Projection/completion occurs on the prepared route, with transaction rollback
and actual unknown returns. Preparation crossing opaque storage remains a later
case. Qualification and independent review are in progress; this paragraph does
not close the broader inventory or change excluded coverage1/32deferred Minors.

Task180 qualification passes all88fresh gates on4,075input/38product hashes.
Normal and static generators each pass86suites/1,391methods. All30native suites,
6changed-C++quality checks,12new focused methods per build, existing focused
suites, affected sanitizer CLI and source packaging pass. The corpus retains
1,032canonical/44explicit unsupported/687default elaboration failures across
2,562source identities, with unchanged input identities and result classes.

All1,560measurements pass10s/512MiB bounds, with256MiB single-wide controls.
The new216run matrix covers1/2/8/32leaves at8/64/256bits and1/2/8leaves at1024bits,
three repetitions and sanitizer samples. It verifies four actual parent-only
openings, eight raw scans,13CSUs, all physical words, unknown child returns,
zero miscompare masks and status clears. Every preparation control is checked
independently. All five child matrices include285live generated-history runs.
Two allocation probes and12guide processes pass. The measured maximum across
1,560runs is0.8188s/469692KiB; no unmeasured-scale or
speedup claim is made.

The frozen parent control for automatic opaque preparation is an actual modeled
unknown-capture twin. Opaque automatic preparation fails in that parent. Exact
physical operations and observations agree after normalizing only validated
child-intent families; no returned data, counts or hardware operation is injected.
The new guide confirms two real openings and three raw CSUs while preserving
the last explicitly requested root history. Independent review follows.

Final independent review found0Critical/1Important/1Minor. The Important public
C++ reservation-validation regression is fixed in one test-first wave: validate
reserved IDs against the public netlist before trial planning, while projecting
opaque requests only after preparation. Native tests pin private and out-of-
range IDs, the established runtime_error boundary, unchanged state/caller
outputs and no delivered observation. All88qualification gates pass freshly
after this repair. No rereview ran;0Critical/Important findings remain unresolved.

The new Minor is a test gap: the inverted-route case uses all-X expected SO and
does not independently detect lost polarity. The reviewer verified cared
comparison mapping directly and found the implementation correct. Defer this
test improvement with32inherited Minors, for33total. Active bound-reset execution,
opaque-containing preparation and broader hardware/clock/transport/vendor work
retain their original requirements; this increment does not close those items.

All22 Task180 merged-result checks pass against identical4,075qualified inputs
and20copied executables. Master is pushed, the owned task branch is deleted,
and117stashes remain. Broader inventory work remains open.

## Task181 opaque-aware preparation

Automatic opening may traverse active fixed opaque leaves using their existing
reset-default or stable-load queue knowledge before each native CSU. Nonstable
unknown lengths still reject. The native planner retains queues in preparation
trials while ordinary solver trials retain their existing native engine. Actual
private preparation input, unknown observations, histories, reservations and
rollback are tested; manual native parent controls avoid invented equivalence.
Qualification/review are in progress. General merges, independent clocks/TAPs,
parallel/timed/transport and broader model/vendor acceptance remain original
work; excluded coverage1 and33deferred Minors remain unchanged.

Task181 qualification passes all93fresh gates on4,078input/38product hashes.
Normal and static generators each pass87suites/1,403methods. All30native suites,
6changed-C++quality checks,12new focused methods per build, existing focused
suites, affected sanitizer CLI and source packaging pass. The corpus retains
1,032canonical/44explicit unsupported/687default elaboration failures across
2,562source identities, with unchanged inputs and result classes.

All1,728measurements pass10s/512MiB bounds, retaining256MiB single-wide controls.
The new168run crossing matrix covers2/8/32leaves at8/64/256bits and2/8leaves at
1024bits, three repetitions and sanitizer samples. Each run checks13raw scans,
17CSUs including four real openings, every stable flag, complete SI/SO words,
public preparation writes, actual unknown histories, masks and status clears.
The same opaque model and frozen parent's explicit ordinary openings give
byte-identical native output; no operation or observation is injected or
normalized away. Native tests separately inspect actual implicit serial input
for reset-default and prior stable loads, copies, rollback and later completion.

All six child matrices include330live generated-history runs. Two allocation
probes and16guide processes pass. The measured maximum across1,728runs is
0.8180s/469868KiB. No speedup or unmeasured-scale claim
is made. A nonstable prior opaque scan still leaves length unknown and rejects
implicit completion; default declarations do not waive that contract.
Independent review follows; broader original inventory remains open.

The fresh independent review recommends merge with0Critical/0Important/1Minor.
It rechecked all4,078input/38product hashes and93gate exits, and independently
verified all168crossing outputs, actual opening writes, stable flags, complete
SI/SO words, histories, masks, status clears and45live continuations. Separate
asymmetric native probes at13/64/256/1024bits pass private-prefix/final-word,
rollback/copy and later-completion checks. A nested newly active opaque leaf
also matches the frozen manual parent; invalidating that leaf's length rejects
later completion without changing the existing output.

One Minor remains deferred: the committed native payloads are palindromic and
cannot detect reversal within each opaque word. The independent asymmetric
probes found no production error. Retain this test gap with33inherited Minors,
34total. There are0unresolved Critical/Important findings; no blocker repair or
rereview is required. Hardware equivalence, unmeasured performance, arbitrary
opaque merges/black boxes, independent clocks/TAPs, parallel/timed/transport,
wider source/vendor acceptance and excluded coverage1 retain prior boundaries.

All25 Task181 merged-result checks pass against identical4,078qualified inputs
and20copied executables. Master is pushed, the owned task branch is deleted,
and117stashes remain. Broader inventory work remains open.

## Serial opaque-child merge calls (Task182)

Native networks with fixed opaque children now accept iMerge call groups. Calls
execute serially through the existing merge traversal and native scan engine.
This includes supported parent-register operations, nested groups, automatic
opening and call-local reservations. Actual opaque captures remain unknown;
expected SO determines care, while status clearing retains its existing order.
An empty merge adds no CSU. No batching or optimality claim is made.

Standalone opaque roots, physical mode and unsupported clock/event operations
retain their existing boundaries. Broader independent TAP, parallel, timed,
transport and arbitrary black-box work remains open, along with excluded item1
and the 34 inherited deferred Minors. Final qualification and review results follow.

Task182 passes all 99 fresh post-repair gates on 4,081 input and 38 product
hashes. Normal and static generators each pass 88 suites and 1,418 methods;
all 30 native suites, twelve changed-C++ quality checks, affected sanitizer CLI,
corpus and source packaging pass. Corpus inputs and result classes remain
1,032 canonical, 44 explicitly unsupported and 687 default elaboration failures
across 2,562 source identities.

All 1,944 measurements meet their 10-second and 512-MiB limits, retaining the
256-MiB single-wide bounds. The new 216-run serial-merge matrix covers 1/2/8/32
leaves at 8/64/256 bits and 1/2/8 leaves at 1,024 bits, with three repetitions,
source/flat/generated forms and sanitizer samples. Every physical word, flag,
intent marker, actual unknown return, mask, old root history and status clear
is checked. The frozen parent's sequential calls and current merge calls use
the same opaque model and produce byte-identical output without normalization.

All seven child matrices include 387 live generated-history runs. Two allocation
probes and 20 guide processes pass. The maximum across all measurements is
1.4792 seconds and 470392 KiB. No batching, speedup or unmeasured-scale claim
is made. The broader original inventory, excluded item1 and 34 inherited
deferred Minors retain their scope.

The independent review found 0 Critical, 1 Important and 0 new Minor issues.
The Important ordering issue is repaired in one test-first pass: direct opaque
stable-state and canonical root-history-omission notes use the existing deferred
merge traversal. The model now restores stable queue knowledge before call
hardware and retains the latest actual root capture after an intervening write.
Malformed or informational notes retain ordinary call-produced argument values;
legacy untagged callback history notes keep their existing ordering guard.

Both metadata tags are covered. The original two ordering cases failed in four
subcases before repair, and informational-note regressions separately failed
before their eligibility checks were narrowed. All fifteen focused CLI methods,
twenty retained callback-history methods and all 99 qualification gates pass on
the final source and products. No second review ran. There are no unresolved
Critical or Important findings and no new deferred Minor; the inherited total
remains 34. Software-model evidence, measured-scale limits and existing per-file
publication behavior retain their prior scope.

All28 Task182 merged-result checks pass against identical4,081qualified inputs
and20copied executables. Master is pushed, the owned task branch is deleted,
and117stashes remain. Broader inventory work remains open.

## Standalone opaque-root serial merges (Task183)

Standalone scalar and disjoint named opaque roots now accept serial iMerge
calls through the existing queue and merge traversal. Implicit default or stable
fills still execute as real scans, but do not invent or overwrite explicit
chain histories when generated PDL is replayed. Existing canonical omission
notes carry that scope; no physical scan or comparison is removed.

Only actual opaque captures are reported, so returned data stays unknown.
A chain retains its prior cared mask through implicit fills and reset while
another chain changes its explicit width. Unknown accumulated status also
stays unknown across reset. Physical/event and
unsupported topology boundaries retain their existing checks. The post-repair qualification
and review result are recorded below; the broader inventory and excluded item1
stay open, with36deferred Minors.

Task183 passes all 108 qualification gates on 4,084 input and 42 product hashes.
Normal and static generators each pass 89 suites and 1,435 methods. All thirty
native suites, two additional static native suites, fifteen C++ quality checks,
normal/sanitizer seventeen-method trace suites, affected sanitizer CLI, corpus
and packaging pass. Corpus inputs and classes retain 1,032 canonical, 44 explicit
unsupported and 687 default elaboration failures across 2,562 source identities.

All 2,226 measurements meet their 10-second/512-MiB bounds, retaining 256-MiB
single-wide controls. The new 282-run matrix covers scalar widths 8/64/256/1024
and named 1/2/8/32-chain widths 8/64/256, plus 1/2/8 chains at 1,024 bits,
three repetitions and sanitizer samples. The frozen parent's real sequential
calls and current merges request all lanes explicitly. Every word, frame, flag,
actual unknown return, mask, status clear and output byte agrees without data
normalization. Separate native/functional cases pin implicit history scope and
old cared masks; the measured all-explicit control does not stand in for them.

All eight child/root matrices include 462 live generated-history runs. Two
allocation probes and 24 guide processes pass. The measured maximum is
1.5422 seconds and 470408 KiB. No batching, speedup or
unmeasured-scale claim is made. Independent review found0Critical/1Important/2newMinor; the Important metadata compatibility regression is repaired in one test-first pass. No rereview is requested. The broader original
inventory, excluded item1 and 36 deferred Minors retain their scope.

The Important review finding was named omission recognition activating on
register-backed roots. Cached existing opaque-model eligibility now limits the
new named behavior, preserving scalar handling and legacy informational notes.
Both call-produced argument and explicit-capture reproductions fail first for
comment/status tags, then pass all 17 root methods across original, flattened
and generated continuations. All 108 final gates ran fresh after the repair.

Two new Minors are deferred: one direct-note fixture uses decimal SI and cannot
distinguish the wrong ordering; provider errors retain the failed Apply location
but omit its nested call stack at later getters. The total is 36 deferred
Minors. No second review or coverage1 execution is claimed.

All34 Task183 merged-result checks pass against identical4,084qualified inputs
and23copied executables. Master is pushed, the owned task branch is deleted,
and117stashes remain. Broader inventory work remains open.

## Modeled named-chain generated history scope (Task184)

Generated PDL now preserves explicit-only histories for already-supported
disjoint register-backed root chains. Implicit lane scans still shift their
complete native words and execute comparisons. They neither create a history
for a never-requested lane nor replace a prior actual capture or cared mask.
The source and its flattened/generated continuations retain the same results.

Canonical generated notes use the distinct prefix `modeled named raw history
omitted `. Existing named `modeled raw history omitted ` notes on modeled roots
stay informational, preserving call-produced Tcl arguments and explicit raw
histories. Scalar and opaque metadata retain their existing scope. New metadata
is consumed only beside an exactly matching canonical scan on a qualified fixed
root route, including its actual chain width. Route normalization is cached.

Existing reset, queue replacement/cancellation, reservations, topology, mode,
payload and publication validation still apply. Post-repair qualification and the independent
review result are recorded below. The broader inventory, excluded item1 and36Minors stay open.

Task184 passes113fresh qualification gates on4,087input and42product hashes.
Normal/static generators each pass90suites/1,449methods;30native normal/SAN,
two additional static native suites,21C++quality checks, normal/SAN17-method
trace suites, affected SAN CLI, corpus and source packaging pass. Corpus retains
1,032canonical/44explicit unsupported/687default elaboration failures and
2,562source identities from the qualified frozen parent audit.

All2,442measurements meet10s/512MiB and256MiB single-wide controls. The new216-run
matrix covers1/2/8/32named chains at8/64/256bits,1/2/8at1024bits, three repeats
and sanitizer samples. Each of four CSUs explicitly requests one lane and
physically fills the others. Actual capture/mask history, absent lanes, nonzero
comparisons and once-only clears are checked after each CSU, not only at the end.
All57new live generated continuations use real getters and absent-history checks.

Frozen-parent source and candidate source/flat/generated hardware and observation
bytes agree after removing only exact validated new omission annotations.
Every annotation is checked against the actual physical implicit scan; SI/SO,
flags, frame positions and observation values are never normalized. Candidate
forms are byte-identical. Reporter RED→GREEN checks reject empty zero-exit
products, forged scope/masks/clear counts and notes moved between frames.

All nine child/root matrices include519live generated runs. Two allocation
probes and28guide processes pass. Measured maximum 1.5284s and
470272KiB. Extra native metadata normalization is cached and measured;
no batching optimum, speedup or unmeasured-scale claim is made. Review found0Critical/1Important/0newMinor; the Important benchmark comparison
finding is repaired in one test-first pass. No rereview is requested; excluded1,36inherited deferred Minors and broader work stay open.

The benchmark now validates each omission annotation against its immediately
following physical scan line before removing only that original annotation's
bytes. Parent/candidate and candidate replay forms are compared as raw bytes;
final-LF and line-ending differences remain visible. Two reproductions fail
first, then all7reporter methods pass. All113final qualification checks ran
fresh after this repair; production C++ and42built products are unchanged.

The reviewer also verified all216pre-repair measured outputs as raw bytes and
all4,860actual annotations adjacent. No production replay failure was found.
The review leaves36inherited deferred Minors, excluded1 and broader inventory
open. The previously advertised read-only artifact checker now validates its
existing gate receipt by equality rather than rewriting it.

All37 Task184 merged-result checks pass against identical4,087qualified inputs
and23copied executables. Master is pushed, the owned task branch is deleted,
and117stashes remain. Broader inventory work remains open.

## Task185 automatic primary callback observations

Original items4/6 gain actual primary observations after automatic write
callbacks in primary-only DataRegister networks. Planning retains callback
intent; actual execution masks callback storage and runs exact native input
frames. DataRegister goal markers never become actual input events or data.
Selected logical reads are checked against actual route guards; preservation
uses actual state. Read-only frames do not generate input events. Failed replay
rolls back before publishing; subscriber failures retain completed state.

The runnable [guide](pattern-generation.md#automatic-primary-callback-observations-task185)
shows known endpoint13, unknown downstream storage and three known endpoint
mismatches cleared exactly once. Original/flat/generated/local-reset tests cover
callback-local old/reset data, sparse/inverted reads, namespaces, once-only
arguments, cancellation/repetition, cyclic retry, serial merges, actual unknown
selectors, reserved storage and sentinel outputs. Two former blanket rejection
fixtures now verify successful bounded support; other unsupported modes retain
their guards. The new maintained benchmark compares actual hardware bytes to
default parent generation, independently validating candidate actual histories.

Broader scan/TAP/clock/physical/timed callback histories, live transport,
excluded1,36inherited deferred Minors and other inventory requirements remain
open. Qualification and review results will be recorded after their gates pass.

Scale scope correction from actual probes: qualify1/2/8lanes at8/32bits,
1/2at64bits and1at256bits. Initial32-lane and two1,024-bit independent
callback controls reach pre-existing solver/time limits; shared-source
alternatives do too. Those retained exploratory failures do not establish
larger support. Larger planner scaling remains item10; no limits are raised.

The new matrix records `ASAN_OPTIONS=detect_leaks=1:quarantine_size_mb=128`.
Normal exploratory measurements peak below157MiB; default sanitizer allocation
retention peaks at593MiB and exceeds512MiB. The128MiBquarantine probe returns
identical bytes at415MiB. This is an explicit sanitizer environment choice,
not a production memory reduction; functional sanitizer suites retain their
default quarantine. No memory guarantee is claimed for unconfigured sanitizers.

Pre-review compatibility correction: collect primary read-route provenance only
when the existing observation callback requests it. The bounded callback
planner explicitly requests provenance and discards planned observation values;
ordinary generation avoids unused route allocations. A native test fails on
unwanted ordinary metadata, then passes. The retained64-callback mixed benchmark
previously reached524744KiB against524288KiB; after the correction its11default
SAN measurements pass at519652KiB maximum, with identical functional results.
The initial attribution to extra planning state was disproved: that explicit
callback fixture has no native WriteDataSource, so the bounded path is inactive.
The final scheduler change invalidates earlier products; all118qualification
gates are now rerun fresh, including both91suite/1464method generator variants.
Earlier attempted same-task reuse is historical evidence, not final reuse.

All118final qualification gates pass fresh after the metadata-allocation fix;
no earlier gate or partial generator result is reused. Normal and static
generators each pass91suites/1464methods. All30native normal/SAN suites,
2static-native suites,21changed-C++quality checks and source packaging pass.
The default-SAN mixed-callback compatibility suite passes17methods.

The artifact validator verifies4090input/42product hashes,2577maintained
timing/RSS rows including555live generated-history runs,32guide processes
and2allocation probes. Allocation counts are56/56with zero root-width growth.
The unchanged source audit retains1032canonical models,44explicit unsupported
models,687default elaboration failures and2562source identities.
Across those2577measured rows, maximum wall time is6.7360s and peak RSS470628KiB.
The new135-run automatic matrix peaks at6.7360s/405740KiB under its documented
sanitizer configuration. Current forms preserve exact hardware bytes against
the immediate parent's default automatic generation; retained named-history
controls use their original frozen pre-feature binary. No physical transport,
unconfigured sanitizer budget or larger planner-scale completion is claimed.
Independent final review is pending; excluded1,36inherited deferred Minors and
broader original inventory requirements remain open.

The one independent final review found0Critical/2Important/1Minor on29abfcc.
Both Important findings are repaired together in one test-first pass. Final
automatic-history validation now processes the remaining provider suffix even
without a getter, rejecting unsupported timed operations and unknown actual
read routes before publishing main/debug/local-reset artifacts. General solver
comparisons retain the selected route, including AccessTogether aliases, for
the same actual-state validation used by ordinary primary reads.

Nine CLI reproductions and a native grouped replay fail first. All19focused
methods and the native replay then pass, covering an already-observed prefix,
ordinary grouped reads with zero callbacks, automatic grouped reads, inverted
aliases and unknown grouped selectors. Full post-repair qualification is in
progress. The API planning-precondition documentation Minor is deferred as37;
the36inherited Minors and broader/excluded work retain their status. No rereview
is requested. The earlier118gate receipt is preserved as pre-repair evidence.

Both Important review findings are resolved in one test-first pass. All118
post-repair qualification gates pass fresh, including91suites/1468methods per
normal/static generator,30native/SAN suites,2static-native suites and21C++
quality checks. The19automatic-primary methods,6reporter methods, unchanged
source audits,32guide processes and source packaging pass. No rereview is used.

The final validator confirms4090qualified inputs,42products and2577maintained
measurement rows, including555live generated-history runs. Maximum measured
wall time is6.9861s and peak RSS470692KiB across those rows. The documented new-matrix
sanitizer configuration and larger-scale exclusions remain unchanged.
Review result:0Critical/2Important/1Minor found;zero unresolved Critical or
Important. The API documentation Minor is deferred;total deferred Minors37.
Original broader items and excluded1 remain open. Merge/integration is next.

All41 Task185 merged-result checks pass against identical4,090qualified inputs
and23copied executables. Master is pushed, the owned task branch is deleted,
and117stashes remain. Broader inventory work remains open.

## Deferred documentation corrections (Task186)

Six inherited documentation findings are addressed: the raw-merge history guard
from Task167, control/RSS labels from Task169, old read-callback usage statements
from Task170, shared benchmark help from Task178, SO/miscompare terminology from
Task179, and the ReplayPrimary planning precondition from Task185. Historical
measurement values and checkpoint review counts retain their original meaning.
The candidate deferred ledger is37minus6=31; final independent review is pending.

Both affected C++ headers preprocess identically before and after these comment
corrections. The shared benchmark AST is identical after excluding its module
docstring. Its three help outputs retain identical options and use an accurate
shared description. Three existing native executable hashes remain unchanged.
Eleven focused checks pass: native primary replay, four CLI suites, three
reporting suites, whitespace and two changed-header formatting checks.
The full behavioral regression is not repeated for these documentation-only
changes. Physical/timed/topology restrictions, resource limits, broader original
requirements and excluded1 remain unchanged.

The one final review found0Critical/2Important/0Minor on5b71a68. Both Important
documentation findings are corrected in one pass: earlier current-use sections
now describe supported read-callback/mixed-alias histories, and raw-merge usage
explicitly retains the physical-output rejection even without a history getter.
The remaining read-callback restriction is explicitly dated to its historical
checkpoint. Physical, timed and logical-storage limits remain stated.

Three targeted existing capability checks pass for merged read callbacks, actual
mixed-alias observations and getter-free physical merge rejection. Together with
the original11checks,14checks pass; C++ tokens, benchmark logic and all three
native executables remain unchanged. No new tests mirror prose, no full
behavioral requalification is claimed, and no rereview is requested.
All six inherited documentation findings are resolved;31other inherited Minors,
broader original requirements and excluded1 remain open. Integration is next.

The documentation batch is merged as16e5aab and pushed. Seven merged checks
pass: three help entry points, native primary replay, read-callback histories,
raw-merge histories and the dynamic-child reporter. Both header token streams,
Python logic and the three native binaries retain their qualified identities.
The owned task branch is deleted;the reused worktree is detached;117stashes
are preserved. Six inherited findings are resolved and31remain deferred.
Broader original requirements and excluded1 remain open.

## Task187 readback factory compatibility and golden repair

The explicit three-argument MakeModeledReadback overload is restored. Its
function-pointer signature is source-compatible again;it forwards to the same
four-argument model owner with no history-output pointer. Removing the fourth
parameter default keeps ordinary three-argument calls unambiguous.

The existing native status-preview test reproduces the compiler failure before
the fix and passes afterward. Typed one/two/three/four-argument pointers and
null outputs return actual data with the expected failure counts. Native
readback, formatting and whitespace pass. Full make check initially exited2
on inherited tessent-golden-check failures, with 198 passing Python suite
executions/3127 methods. All77directional outputs are byte-identical between
the qualified baseline and candidate;both fail the same five old child-history
snapshots because new history notes are missing. Five fresh licensed Tessent
exports and new record/check certificates repair those exact snapshots. The
failed target now passes, including all77directional snapshots;passing unchanged
project checks are retained. A fresh all-green full-command rerun is not claimed.
Compiler/static-linker warnings from unchanged files remain in the check log.
No performance claim, dependency, scheduler behavior or limit is added.
The inherited Task177 API finding is resolved:31minus1=30 deferred Minors.
Broader original inventory and excluded1 remain open;final review/integration
are pending.

The final review found0Critical/1Important/1Minor. Its Important verification
gap is resolved. A fresh full command passed every prerequisite, then reached
the main recipe and failed its source-inventory test:coverage.SOURCES omitted
pattern_readback.cc. The existing seven-test reader suite reproduces that
failure and passes after adding the missing source. Excluded item1 thresholds
remain unchanged. The entire main recipe then passes with completed unchanged
prerequisites retained (15 Python suite executions/139 methods).
CLI/regression/fuzz/LSP/grammar/Vim commands all execute;their retained log records
any optional skips. Five optional ANTLR comparison tests are skipped. The full
rerun exit2 and resumed-main exit0 are reported
separately;a fresh all-green single invocation is not claimed. Five fresh vendor
certificates remain verified. One new Minor is deferred:the preview test could
detect split model ownership by clearing publication status then checking
preview returns zero. No production defect was found in the factory repair.
Current inherited31minus1resolved plus1new=31deferred Minors. No rereview is
requested;integration remains next.

Task187 is merged as b2ba89a and pushed. All six merged checks
pass on identical qualified inputs and four copied executable hashes, including
the readback API regression, coverage inventory and77directional snapshots.
The owned task branch is deleted;the reused worktree is detached;117stashes
are preserved. The Important review gap is resolved;31Minors remain deferred.
Broader original requirements and excluded1 remain open.

## Task188 independent TAP IR/DR phases

Task188 pre-review qualification: full make check passes, including CLI/regression/fuzz/
LSP/grammar/Vim;215Python suite executions report3290tests, with5optional real
ANTLR integrations skipped (ICL_REF_DIR unset). There are37successful check
commands and15C++style checks. All15leak-enabled sanitizer native suites and
focused functional suites pass, including10new-phase and20non-scale existing
multiple-TAP methods. Source packaging and both corpus audits pass.

Three paired7-point measurement rounds retain byte-identical parent/current
common-phase controls. All63points/252CLI replay processes pass. Independent
matrices take12.86-13.73s;their runner/child-process-tree peak is359240KiB. The
slowest measured independent point is2TAPs by256bits, four forms in6.397s.
No speedup over the unsupported parent mixed mode is claimed.

The22-method sanitizer multiple-TAP run fails three16/32TAP scale subcases at
the existing solver limit;fresh parent and candidate builds fail the same three.
Those instrumented scale cases remain unqualified;no limit is raised and no
sanitizer512MiB scale guarantee is claimed. Three Tessent2026.2 probes crash at
elaboration before PDL, including the ordinary control;vendor equivalence is
unqualified. All failures, optional skips and raw evidence remain recorded.

Final help-only wording was followed by fresh normal/static12-method CLI runs,
sanitizer10-method functional replay, help checks and source-package validation.
The final manifest binds4091inputs and44selected executables. The model audit
retains1334files/1763effective modules:1032canonical,44unsupported serialization
forms and687default elaboration failures. No model/default acceptance is added.
The pre-review result is superseded by the post-review qualification below.
31inherited Minors, broader original requirements and excluded1remain open.

### Task188 post-review qualification

Final review found0Critical/2Important/1Minor. One repair pass preserves the
AccessTogether read alias when a reset-held DR emits no write, and prepares an
unselected DR owner's instruction through a real common-phase scan before the
mixed access. Planning snapshots only propose routes;concrete replay verifies
the preparation and original goals before publishing state or observations.
The three/four-TAP cases now require no manual setup. Both contradictory
overview statements were corrected;no new Minor is deferred.

Fresh post-fix full make check passes:215Python suites report3294tests, with the
same5optional ANTLR skips. All33qualification commands pass, including15C++style
checks,15leak-enabled sanitizer native suites,12new-phase functional methods and
20existing non-scale multiple-TAP methods. Both normal/static14-method suites,
source packaging and both corpus audits pass. The final manifests bind4091inputs
and44selected executables.

Three fresh paired7-point scale rounds pass63points/252CLI replays. Parent and
candidate common-phase outputs remain byte-identical. Independent matrices take
13.62-14.06s with362336KiB maximum runner/child-process-tree RSS;the slowest
2TAP-by256bit point takes6.723s for four forms. These replace the pre-review
measurements above;no unsupported-parent speedup is claimed.

The earlier three parent-matched instrumented scale limits remain explicit;
those large sanitizer cases are unqualified. This repair applies to mixed groups
and preserves common-phase scheduling. The three pre-PDL vendor elaboration
crashes still prevent new vendor equivalence. No resource limit is increased.
Integration is pending;31inherited Minors, broader original requirements and
excluded1 remain open.

Task188 is merged as 9dc350b and pushed. All eleven merged checks
pass on identical4091qualified inputs and nine copied executable hashes, covering
native state/readback and both CLI variants plus the corpus audit. The owned
task branch is deleted;the reused worktree is detached;117stashes are preserved.
Both Important review findings and the Minor wording issue are resolved;
31inherited Minors, broader original requirements and excluded1 remain open.

## Task189 simultaneous client interfaces

Task189 supports bounded local simultaneous groups across statically disjoint
standalone clients with shared root phase/TCK pins and uniquely selectable root
SelectPorts. All declared root clients must satisfy the proof. Ordinary requests
and shared-storage switching retain one-client behavior. Generated preparations
retain their simultaneous selected clients. Each combined frame must be
expressible through common writes or a capture alias covering those clients.
Logical read targets,requested bits,cared comparisons and observation frames
survive original/flat/generated continuation;physical capture routes do not
become logical history requests. No physical/vendor-equivalence claim is made.

One final independent review found1Critical/2Important/0Minor findings.
Five maintained reproductions fail first (10failures across both producer modes),
then pass in one repair pass. The Critical routed-comparison false PASS is fixed;
parallel preparation events and other-frame/primary/write-group sparse histories
are fixed. A selection-driven reset-held preparation subcase also passes.
There is no rereview;31inherited Minors remain deferred.

All36fresh post-repair qualification commands pass,including9C++quality checks,
15leak-enabled sanitizer native suites,23new-client functional methods,
12mixed-TAP methods,20existing non-scale multi-TAP methods and seven focused
sanitizer suites. Full make check passes:217Python suites report3344tests with
5optional real ANTLR skips (ICL_REF_DIR unset). Both25-method normal/static
suites,source packaging and model/source audits pass. Final postreview manifests
bind4092inputs/44selected executables;the audit retains1334files/1763modules,
1032canonical/44unsupported serializations/687default elaboration failures.

Three fresh paired7-point rounds pass63points/252CLI replays. Ordinary controls
remain byte-identical to the frozen parent. New simultaneous matrices take5.20-5.91s,
with118492KiB maximum reported runner/children RSS,not a native-only per-point peak.
The slowest four-form point is3.706s. All measured points meet10s/four forms and
512MiB reported RSS. The parent rejects new simultaneous groups;no speedup over
that unsupported parent mode is claimed. Original qualification,prehistory
failures and initial forced-event measurements remain historical evidence.

Fresh Tessent2026.2 accepts the ordinary two-client control,finds no solution for
the same ICL's simultaneous write,and rejects AccessTogether in a separate parse
probe. No new certificate is added. Prior parent-matched16/32TAP sanitizer scale
limits remain unqualified;new scale matrices run normal/static without a
sanitizer memory guarantee. The separate parent ordinary-only sparse-prefix
history defect remains follow-up work. Broader original requirements and
excluded1 remain open. Integration results follow below.

Task189 is merged as 3e5d8c9 and pushed. All eleven merged checks
pass on identical4092qualified inputs and nine copied executable hashes, covering
native state/readback and both CLI variants plus the corpus audit. The owned
task branch is deleted;the reused worktree is detached;117stashes are preserved.
The Critical and both Important review findings are resolved;
31inherited Minors, broader original requirements and excluded1 remain open.

## Task190 ordinary logical read scope

Task190 preserves explicit ordinary logical targets,read bits and cared values
through native,symbolic and primary lowering. A partial X read observes only its
requested bits;known outside history remains intact and never-read bits stay X.
Routed DataRegister reads keep their logical identity and actual selected frame.
Exact raw/physical masks and documented implicit instruction-capture checks keep
their contract. Independent native old-output/controller/final-state assertions
remain;merge tests now expect exact slices and retain ordering/byte replay.

All63post-review qualification commands pass,including9C++quality checks,15leak-enabled
sanitizer native suites,15new functional methods,23client,12mixed-TAP and20older
multi-TAP methods,seven affected sanitizer modules,20merge-observation methods and four changed
DataRegister/scan-data/mixed-I/O consumer modules plus the13-method partial
AccessTogether group contract.
Full make check passes:219Python suites report3383tests with5optional ANTLR skips.
Both16-method normal/static focused suites,14round-trip certification methods,
source packaging and corpus audits pass. Manifests bind4121inputs/44executables.
The model audit remains1032canonical/44unsupported/687default elaboration failures.

Three paired rounds pass57scale points/318CLI processes. Seven full-read client
controls per parent/current round have identical input/output bytes;five sparse
points per round cover primary256/512/1024bits and scan/client256bits. The maximum
sparse producer group is5.725s;the maximum control four-replay group is8.359s.
Reported runner/children peak RSS is150804KiB. Each sparse point checks two
producers/eight continuations;the10s bound applies separately to each producer's
generation/four continuations,with combined wall also reported. No numerical
speedup is claimed. A redundant quadratic read-metadata pass was removed after
one16client control exceeded10s;the failed evidence and frozen binaries remain.

Five refreshed forward certificates compare actual Tessent2026.2 exports of the
previous schema3-certified generated program and the corrected program. Schema4
requires their physical ordered groups to match while binding previous program/
source hashes and retaining both fresh jobs. Current logical traces,exact local
regeneration and idempotency are checked separately. Existing schema3 contracts
are unchanged. All77directional certificates and seven legacy standards pass.
Original-source schedules need not match;queued pending-iState remains rejected
by Tessent. No broader source acceptance or live-hardware equivalence is claimed.

Four full runs failed before final qualification:five read-name goldens,then three
merge spelling assertions,then DataRegister consumer spellings,and an AccessTogether incidental-read
assertion. Their logs remain. Unchanged native/SAN/scale results
were reused only after checking source/executable hashes;changed certificate and
merge contracts and final full check ran freshly. A resume helper's malformed
unittest argument also failed before full execution and was corrected;its log is
retained. None of those failures is counted as a successful qualification.

Existing primary4096/65536solver/scheduler limits,2048solver-boundary behavior and
16/32TAP sanitizer scale limits remain unqualified. The history-only65536bit
cases do not qualify active scheduling. General internal monitors and broader
original2/3/4/5/6/7/8/10/11/12 remain open;excluded1 and31inherited Minors remain.
Three capture-probe consumers retain literal0/9/status checks while querying
requested d history and asserting incidental observer histories remain unknown.
An equally preferred-route case checks logical value000 and unknown q0/q1 histories.
The71remaining-module collection preserves five original failures;fresh focused
normal/SAN checks verify those five repairs,with66unchanged modules passing.
One independent review found0Critical/1Important/0Minor findings. One test-first
repair pass rejects false vendor-input attribution and missing/inconsistent prior
certificate provenance:two maintained methods reproduce10accepted mutations
before the repair,then all14certificate methods pass. Both record and offline
checks bind declared execution to the preserved canonical vendor Tcl script.
All five actual-run recertifications produce byte-identical stored snapshots.
The checker/test changes preserve all C++ and vendor input/output hashes;57
unaffected checks are reused under exact hash proof,with fresh full regression
and packaging. No rereview occurred;31inherited Minors remain. Integration results follow below.

Task190 is merged as 99dc8aa and pushed. All twelve merged checks
pass on identical4121qualified inputs and nine copied executable hashes, covering
native state/readback,both CLI variants,all77directional certificates and corpus
audit. The owned branch is deleted;the reused worktree is detached;117stashes
are preserved. No Critical or Important review finding remains unresolved.
31deferred Minors,broader original requirements and excluded1 remain open.


## Task191 shared serial inputs

Fixed ungated interface-free lanes of equal nonzero length can share one serial
input and its transparent hierarchical forwarding, with disjoint register
storage. Their symbolic load words must produce one physical input stream after
serial inversions. Captures, requested read bits, histories and status remain
separate. Conflicting or omitted-bank writes preserve the previous output;
primary-only frames do not require a scan. Named/client/TAP sharing, storage
overlap, intermediate taps, unequal/empty lanes and switched/gated shared routes
remain pending under the broader items 2/3/6/8.

All 40 final qualification commands pass: the full normal/static check reports
221 Python suite runs and 3,413 tests (five optional ANTLR reference skips), all 15
native sanitizer suites and 12 affected CLI/corpus checks pass, six changed-code
C++ quality checks pass, and source packaging passes. Fifteen new CLI methods
cover four replay forms and prefix histories; a native independent scalar shift
oracle checks the literal common input, separate physical capture streams,
final bank/pin state, copies and ownership rollback. The initial primary-only,
hierarchical-forwarding and legacy-diagnostic failures have verified repairs;
interrupted runs and the sandbox's LeakSanitizer ptrace failure are retained as
historical failures, separately from the final untraced pass.

Three rounds of nine shared-input cases and 18 matched parent/current disjoint
controls cover 45 measured points and 180 CLI processes, plus three Python
runners. The largest shared case is 32 lanes of 256 bits; the slowest four-form
shared replay is 0.3486 s and the runner/children peak is 96,324 KiB. All points meet
10 s/512 MiB; all disjoint control outputs are byte-identical to master. These are
bounded measurements, without an isolated speedup or wider topology claim.
Evidence and hashes for 4,122 source/test inputs and 44 executables are retained
under build/shared-scan-input-20261126/. No fresh vendor or hardware
qualification is claimed. Coverage item 1 remains excluded.

Independent review found zero Critical, one Important and one Minor issue.
One test-first repair scopes shared-bank automatic load permissions to the
current operation's control dependencies. Unrelated muxes and disabled data
consumers no longer authorize omitted-bank writes; actual requested controllers
remain programmable. Four new CLI methods reproduce 14 incorrect publications
on the pre-fix executable across both producer modes, then pass after repair.
Native tests verify failed-write state rollback and compatible explicit writes.
The full post-review qualification above is fresh; no second review was used.
The topology-negative-test precision Minor is deferred (32 total, including
31 inherited findings); no Critical or Important finding remains unresolved.

Additional 4- and 256-bit requested-controller prefix cases pass 20 CLI processes
plus one runner. The slowest producer and four continuations take 0.4146 s;
runner/children peak RSS is 113,624 KiB. These checks retain the 10 s/512 MiB
bounds and prove real controller programming without freeing unrelated bits.

Task191 is merged into master at 31e97ee and pushed. All seven merged-result
checks pass on 4,122 identical source/test inputs and ten copied qualified
executables, including native scheduler state/rollback and both CLI variants.
The owned task branch is deleted, the reused worktree is detached, and all 117
stashes are preserved. The broader inventory and 32 deferred Minors remain open.

## Task192 fixed scan fanout

Fixed ungated interface-free paths may share full-register storage and have
unequal lengths or a wire-only observation alongside storage. Shorter load words
match the longest word's most significant prefix, corrected for inversion.
Concrete replay broadcasts one physical input stream; shared registers clock
once. Actual captured words, partial logical histories, status clearing,
omitted-load preservation and reservation/copy rollback remain intact.

New geometries require invariant inactive resets: constant/global references or
ResetPort forwarding to a root ResetPort. Active/unknown resets and other reset
dependencies reject without replacing output. Existing equal disjoint shared
groups retain their behavior. Intermediate taps, gated/switched sharing and
named/client/TAP sharing remain pending; this increment does not close items
2/3/6/8 or the excluded strict coverage item 1.

All 42 qualification commands pass. The full normal/static check reports 223
Python suite runs and 3,441 tests, with five optional ANTLR reference skips.
All 15 native sanitizer suites and 13 affected CLI/corpus sanitizer checks pass,
along with six changed-code C++ quality checks and source packaging. Fourteen
new CLI methods exercise original/flat/generated replay and prefix histories.
The native independent scalar oracle checks literal input 01011011, eight
simultaneous ticks, final A/B/C values 13/2/10, captured return delays and rollback.

Three rounds of nine fanout cases and 18 matched parent/current disjoint controls
cover 45 measured points, 180 CLI processes and three Python runners. The matrix
covers 2/8/32 observations at 4/64/256 bits; its slowest four-form fanout replay is
0.3252 seconds and runner/children peak RSS is 92,696 KiB. All points meet
10 seconds/512 MiB, and all matched disjoint outputs are byte-identical. No
isolated speedup, larger-network, live-device or new vendor claim is made.

Evidence under build/fixed-scan-fanout-20261126/ binds 4,123 source/test inputs and
44 executables. Historical fixture failures and a measurement/build relink race
are retained separately. The complete 42-command qualification and measurements
are fresh after the review repair. Replacing obsolete topology negatives with
valid boundaries and positive fanout coverage resolves Task191's test-precision
Minor; 31 inherited deferred Minors remain.

Independent review found zero Critical, one Important and two Minor issues.
One test-first repair follows the selected bit through intermediate ResetPort
buses, including ascending/descending nonzero indices. An unrelated dynamic
sibling does not reject an invariant selected bit; selecting the dynamic bit
still rejects. Sixteen passing-case subtests fail on the pre-fix normal/static
executables, then the new regression and complete qualification pass after the
minimal offset fix. No second review was used. The two Minors remain deferred:
scale assertions check only the common bank's literal write, and concrete replay
retains redundant per-observation input work. There are 33 deferred Minors total;
no Critical or Important issue remains unresolved.

Task192 is merged into master at c813110 and pushed. All eight merged-result
checks pass on 4,123 identical source/test inputs and ten copied qualified
executables. The owned task branch is deleted, the reused worktree is detached,
and all 117 stashes are preserved. The broader inventory and 33 deferred Minors
remain open; strict coverage item 1 remains excluded.

## Task193: explicit clock period overrides

Positive `iClockOverride -period` literals retain logical target versions through
source, flat, generated and local-reset replay. Programs without periods retain
previous normalization. Mixed source/ratio overrides in period-bearing programs
require all three fields. Existing modeled-clock-history, physical/raw/opaque,
differential, unsafe-fanout and stateful-time boundaries remain. Actual PIO is
checked independently.

All 36 fresh qualification commands pass after review repairs: 225 Python suite
runs report 3,464 tests, with five optional ANTLR skips. Both generators pass ten
period methods; existing clocks pass 72 methods and shared formatter/LSP tests
pass 61. All 21 leak-enabled ASan/UBSan checks and 18 changed-code C++ checks pass,
as do corpus acceptance and source packaging. Full regression takes 1,649.304
seconds and packaging takes 54.253 seconds. The manifests bind 4,124 runtime/test
inputs and 44 executables.

Three rounds cover 27 period matrix points and 18 matched ordinary controls,
225 CLI processes and three Python runners. The matrix uses 2/8/32 oscillator
bits and 8/128/1024 updates in five forms, checking every retained clock statement
in order. Its maximum is 0.3068 seconds and 61,688 KiB runner/child peak RSS;
all points meet 10 seconds/512 MiB. Ordinary parent/current outputs are byte-identical.
Measurements overlap qualification builds and establish no isolated speedup.

Independent review found zero Critical, three Important and two Minor issues.
One test-first repair pass rejects explicit empty literal/Tcl/backend units and
supports simple level-0 value/unit formals with concrete dispatch validation.
A required encoded vector-source regression passes on the correct implementation,
fails when source canonicalization is removed, then passes after restoration.
The complete suite passes after this repair pass; no second review was used.
No Critical or Important issue remains unresolved. Two Minors remain deferred:
known invalid operands paired with dynamic neighbors can lose declaration-time
editor diagnostics, and the permanent period local-reset fixture does not force
prerequisite insertion (the reviewer verified such insertion independently).
There are 35 deferred Minors total, including 33 inherited findings.

Evidence is under build/clock-period-overrides-20261126/. Original qualification
and measurement records are retained under pre-review-*; the initial invalid
scalar-to-vector measurement fixture is retained separately.
No hardware timing, vendor equivalence or broad-item closure is claimed. Strict
coverage item 1 remains excluded.

Task193 is merged at dcdad4a and pushed to master. All nine merged-result checks
pass on 4,124 identical inputs and ten copied qualified executables. The owned
task branch is deleted, the reused worktree is detached, and all 117 stashes are
preserved. The broader inventory and 35 deferred Minors remain open.

## Task194: clock period serialization

Canonical main PDL emits separate numeric/unit operands. Integer periods retain
their unit; exact decimal/scientific values refine only as needed, for example
1.25us to1250 ns and1.0e-9s to1 ns. Sub-attosecond values and exponents that cannot
be canceled through coefficient zeros and unit refinement retain their exact
native spelling; large cancelable exponents can still emit integer periods.
Current guards and resource limits remain.

Both generators pass the five new serialization methods and ten existing period
methods; legacy clock/readback/reset/frontend/native checks and six changed-code
C++ checks pass. All 39 fresh qualification commands pass: 227 Python suite runs report 3,474
tests, with five optional ANTLR skips; all 22 leak-enabled sanitizer checks and
six changed-code C++ checks pass. Full regression takes 1,668.838 seconds and
source packaging takes 54.962 seconds. The manifests bind 4,125 runtime/test
inputs and 44 executables. Independent review is resolved; integration is recorded below.

Three rounds cover 48 measured points: 27 short-period matrix cases, 18 matched
ordinary parent/current controls and three long-literal cases, using 240 CLI
processes and six Python runners. All points meet 10 seconds/512 MiB. The short
matrix peaks at 0.2973 seconds and 62,188 KiB runner/child RSS. The 100000-zero
cancellation program (100,073 body bytes, 100,105 source-PDL bytes) peaks at
0.1486 seconds and 50,876 KiB. Ordinary outputs are
byte-identical. Refreshed measurements include artifact capture I/O and imply no isolated speedup.

Fresh Tessent Shell 2026.2 verification passes ten sequential jobs:five reference
and generated pairs have identical full exports and literal expected relative
clock-operation traces. Six native input spellings also pass five-form replay
(30 local CLI processes); two identical cases share one vendor pair. All inputs,
products, scripts and exports have hash bindings under
build/clock-period-serialization-20261127/. Exports omit period metadata, so this
is configured-run acceptance/normalization evidence, not standalone export replay,
device behavior or timing signoff. Source-preserving reset-only files retain
native spellings and may need a native generation pass before vendor loading.

Sixteen discovery controls are retained separately:integer/unit equivalents
succeed; attached, decimal/scientific and fractional-attosecond operands in the
measured forms reject. Three direct negative-leg override probes reject the
DifferentialInvOf target, preserving the native guard. These are 2026.2 facts,
not IEEE or v2023.4 prohibitions. The broader inventory, 35 inherited deferred
Minors and excluded strict-coverage item 1 remain open.

Independent review found zero Critical, one Important and two Minor issues.
One evidence-only repair pass captures actual input and five output hashes before
benchmark temporary files are cleaned, retains the bytes, and verifies agreement
across all repeated rounds. The strengthened evidence check fails before the
repair and passes after all 48 measurements are refreshed. Production, tests and
all 44 qualified executables remain byte-identical, so the completed full suite
is retained. No second review was used. The two Minor receipt/guide wording
findings are deferred; there are 37 deferred Minors total and no unresolved
Critical or Important findings. Original measurements remain in pre-review-measure.

Task194 is merged at 9205da4 and pushed to master. All nine merged-result checks
pass on 4,125 identical inputs and ten copied qualified executables. The owned
task branch is deleted, the reused worktree is detached, and all 117 stashes are
preserved. The broader inventory and 37 deferred Minors remain open; strict
coverage item 1 remains excluded.

## Task195: mixed clock defaults

Independent review found zero Critical, two Important and one Minor issue. One
test-first repair pass resolves complete-argument compatibility and adds a
maintained inversion-guard regression. All six complete option permutations
preserve parent argument order and numeric spelling. The inversion test fails
when its guard is deliberately removed and passes on the restored source.
The vendor-wording Minor is deferred, bringing the total to 38.

Valid partial source/ratio overrides in a period-bearing clock program now emit
all three fields using native ICL defaults. Prior period/source definitions do
not supply omitted values. Ordinary programs keep their prior normalization.
Unrepresentable default sources and oscillator-only cases remain guarded; the
change adds no event or elapsed-time engine.

Both generators pass 15 period methods and five serialization methods. Native
clock snapshots and all nine changed-code C++ checks pass. All 39 final
qualification commands pass: 227 Python suite runs report 3,484
tests with five optional ANTLR skips, all 22 leak-enabled sanitizer checks and
nine changed-code C++ checks pass, as do corpus and source packaging. Full
regression takes 1,656.943 seconds and packaging takes 54.306 seconds. The final
manifest binds 4,125 frozen inputs and 44 executables. Six recorded sequential
Tessent Shell
2026.2 jobs pass: three explicit native-default reference/generated pairs have
byte-identical exports and literal expected relative traces. Fifteen fresh native
producer runs cover all five replay forms. Every vendor model, PDL and script
byte matches the pre-review bundle, so the six actual vendor jobs are reused
after current binary, trace and input identities are rechecked. Original partial
source programs are
not vendor-equivalent because their omitted-field rules differ. Exports omit
period metadata; this is configured relative-operation evidence, not device timing.

Three rounds cover 27 mixed matrix points and 18 matched ordinary controls,
225 CLI processes and three Python runners. Each point meets 10 seconds/512 MiB;
the mixed matrix peaks at 0.3572 seconds and 62,300 KiB runner/child RSS. Actual
inputs and all five output files are hash-bound before temporary cleanup, with
identical hashes across rounds. Ordinary parent/current outputs are byte-identical.
Artifact capture I/O is included and measurements overlap qualification; no
isolated speedup is claimed. Evidence: build/mixed-clock-defaults-20261127/.

The broader inventory and 37 inherited deferred Minors remain open. Item1 stays
excluded. Both Important findings are resolved. Task195 is merged and pushed to
master as a31e1e8; all nine merged-result checks pass on identical 4,125 inputs
and ten copied qualified products. The task branch is deleted; 117 stashes and
unrelated worktrees are preserved.

Earlier complete qualifications are retained under pre-help-* and pre-review-*
evidence. A necessary compiled-help correction has a maintained RED→GREEN assertion on
both generators. The repaired final source passes a new complete frozen
qualification; current measurements and vendor producers use the rebuilt binary.

## Task196: declared oscillator restoration

Empty native overrides restore an oscillator's actual declared ICL Period after
source/ratio or explicit-period replacement. Old checked definitions, neighboring
vector bits, procedure/reset replay and default root-relative lowering are
preserved. Missing intrinsic sources, invalid factors, differential targets and
unsupported hardware fanout retain their validation boundaries.

The normal/static generators pass 19 period methods, with 73 ordinary clock
methods and native clock snapshots passing. All 45 workload/control points pass
the unchanged 10 second/512 MiB budgets; the restoration matrix peaks at 0.530 seconds
and 76,272 KiB. One restoration update is a source replacement plus a default
restoration, with one initial period override per point. Input/output artifacts
are captured and bound to the current binaries and maintained tests. Three
explicit-period reference/generated vendor pairs pass six fresh sequential
Tessent 2026.2 jobs with byte-identical exports and literal relative traces.
Fifteen native producer runs cover all five replay forms; current binary, trace,
frozen-input and actual tool-input/export hashes agree.

All 39 final qualification commands pass: 227 Python
suite runs report 3,494 tests with five optional ANTLR skips.
All 22 leak-enabled sanitizer checks and six changed-code C++ checks pass,
along with corpus, editor checks and source packaging. The final identity
manifest binds 4,125 inputs and 44 executables. Full regression takes
1,670.251 seconds; packaging takes 54.778 seconds.
One fresh independent review found 0 Critical / 1 Important / 3 Minor. A single
evidence-only repair pass resolves the Important finding: every control input
and output file is rehashed, all nine parent/current pairs match and all planned
coordinates occur once. Four rejection controls pass and the authentic bundle
still qualifies on unchanged source/product hashes. Three Minors are deferred,
bringing the total to 41. Task196 is merged and pushed to master as e9b66af;
all nine merged-result checks pass on identical 4,125 inputs and ten copied
qualified products. The task branch is deleted; 117 stashes and unrelated
worktrees are preserved.

Parent: master 0eae49b; branch: codex/oscillator-default-restore. The broader
inventory and 38 inherited deferred Minors remain open; item1 stays excluded.
Evidence: build/oscillator-default-restore-20261127/.

## Task197: independent clock programs

Previously rejected partial overrides on independent mux, alias or inverted
clock sources now use the established ordinary root-relative lowering while
other components retain logical period programs. Previously accepted output
bytes stay unchanged. Connections through static routing and future source
overrides keep a component retained, preserving checked versions and guards.
Policy preparation is sparse, per bit and separate from live resource edges.

The normal/static generators pass all 28 period methods. Native snapshots cover
bit identity, old/new root counts, policy copies, future edges and a sparse
65,536-bit oscillator. The compiled help has a maintained parent RED/current
normal-static GREEN assertion.

All 39 qualification commands pass: 227 Python suite runs
report 3,512 tests with five optional ANTLR skips. All 22 leak-enabled
sanitizer checks and 12 changed-C++ quality checks pass, along with corpus,
editor checks and source packaging. The manifest binds 4,125 inputs and
44 executables. Full regression takes 1,808.929 seconds;
packaging takes 66.805 seconds.

All 45 workload/control points pass unchanged 10-second/512-MiB limits, with
0.368-second matrix maximum and 60,456-KiB runner/child RSS. Captured inputs and
five outputs are hash-bound; ordinary parent/current bytes match. Six fresh
Tessent2026.2 jobs pass with three identical explicit-reference/generated exports.
All literal traces match three mode-initialization commands plus eight preplanned
clock operations. Actual tool inputs and jobs are unchanged after adding the
observed setup prefix to the comparison. New driver byte copies are retained.

Independent review repairs qualify; integration is complete. Parent: master8449b13;
branch: codex/clock-program-components. The broader inventory and 41 inherited
deferred Minors remain open; item1 stays excluded.
Evidence: build/clock-program-components-20261127/.

Independent review found zero Critical, three Important and three Minor
issues. One test-first repair pass preserves guarded neighboring partial
defaults, restricts lowering to potentially clock-driven sources while
preserving viable mux choices, and skips invalid non-single-bit operands
without expanding their declarations. Four new CLI methods pass on both
generators, including output sentinels, two 1,048,576-bit/2,048-operation
cases and a 512-operation encoded whole-vector case. Their earlier runs
took about13.8seconds and9.3seconds. All three operand forms pass the
2second bound;18 matched parent/current points peak at0.066seconds
and49,928KiB runner/child RSS.
Native policy-copy assertions also pass. The repaired source passes a
fresh complete qualification; no second review was used. Three Minors
are deferred, bringing the total to44.
Fifteen fresh native vendor producer runs reproduce all five forms.
Every actual vendor model, program and script byte is unchanged, so
the six recorded Tessent jobs remain applicable after current binary,
frozen-input and export/trace identities are rechecked.

Task197 is merged at 3ba4cad and pushed to master. All nine
merged-result checks pass on 4,125 identical inputs and ten copied
qualified executables. The task branch is deleted, the reused
worktree is detached and all 117 stashes are preserved. The broader
inventory and 44 deferred Minors remain open; item1 stays excluded.


## Task198 feedback-generated reset pulses

Task198 extends bounded logical data-feedback settling to uniquely owned,
acyclic ScanRegister reset closures in interface-free models. Concrete and
symbolic propagation retain pulses, simultaneous fanout, cascade snapshots,
unknown joins, controlling fill and transactional replay. Primary-only requests
with independent reset sinks try the primary planner and retain mixed planning
when scan preparation is required. Scan-driven updates keep the mixed planner.
See the [usage and boundaries](pattern-generation.md#feedback-generated-scan-reset-pulses-task198).

Items3/4 remain open for coupled data/reset networks, shared origins, cyclic
scan resets and explicit controller/clock contexts. Existing limits remain;
this increment makes no timed, physical or vendor-equivalence claim. Item1 is
excluded,117 stashes and44 inherited deferred Minors are preserved.

Pre-review qualification passed 35 commands: 227 Python suites reporting 3524 tests,
21 leak-enabled sanitizer checks, 24 changed-C++ quality checks, corpus/editor
checks and source packaging. Exact identities bind 4126 inputs and 44 products.
The 72-point matrix passes all 240 candidate/current replay forms within the
existing limits; six parent controls complete all forms and 18 retain measured
budget misses. Five optional reference tests remain skipped in this full run;
no new vendor or live-device qualification is claimed. This cohort is archived
under `build/feedback-reset-pulses-20261127/pre-review/`.

The independent review found one Critical and four Important issues, with no new
Minors. One test-first repair pass fixes pending-read preservation through reset
waves, unrelated fill retention, representable scan end states, selected-bit
dependencies and public exact replay. Final post-review qualification passes all
35 commands: 227 Python suites reporting 3532 tests, 21 leak-enabled checks,
24 C++ quality checks, the complete matrix, corpus/editor and source packaging.
The final checker binds 4126 inputs and 44 executables; five optional reference
tests are skipped and 44 inherited Minors remain deferred. No second review was
used. Task198 is merged at acb0140 and pushed to master. All nine merged-result
checks pass with 4126 identical inputs and 25 copied qualified executables. The
owned task branch is deleted and its ledger archived; all 117 stashes remain.

A separate pre-existing defect remains: ordinary acyclic reset preparation can
erase a pending scan observation. Parent and current reproductions are retained
in `build/feedback-reset-pulses-20261127/follow-up-discovery.json`; this is the
next task under original items3/6. The broader inventory remains open.


## Task199 pending reset observations

Preparation must preserve each requested logical observation from the start of
its apply, including implicit TAP instruction setup. At capture, the scheduler
compares the requested bits with original storage under current access controls.
This covers direct ScanRegister CaptureSource, primary reads, selected clients,
and data selectors such as DataMux and OneHot groups. Unobserved word bits and
inactive physical observation-route arms may change during safe preparation.
A selected capture precedes its frame's later updates. Expected 0, 1 and X never
supply actual data; impossible preparation fails atomically and preserves prior
outputs, state and callbacks.


The repaired implementation passes 40 qualification gates: 227 Python suites
reporting 3548 tests, 22 leak-enabled native/CLI/corpus checks, 12 C++ quality
checks, grammar/editor checks and source-package build/install verification.
Five optional reference cases are skipped. Exact hashes bind 4126 production
and test inputs and 44 executables. Source packaging is checked again after
recording these results. The initial 37-command cohort remains historical
pre-review evidence, separate from this repaired qualification.

All four Critical and two Important review findings are fixed in one test-first
pass, including the selected-client, direct-capture, partial-bit, inactive-route,
selector and split-TAP-prefix cases. A concrete OneHot repair regression also
passes. No repaired-code re-review was used; the 44 inherited Minors remain
deferred. Task199 is merged at 9eedf42 and pushed to master. All 12
merged-result checks pass on 4126 identical inputs and 25 copied qualified
executables. The owned task branch is retired and all 117 stashes are preserved.

Three-round changed-path measurements pass 12 matched parent/current points
and 36 source/flat/generated processes with up to 128 pending 8-bit registers.
The largest current run is 0.720 seconds/148816 KiB, within 10 seconds/512 MiB.
The separate 72-point feedback matrix passes all 240 candidate/current forms
and all 24 parent controls; its largest current run is 2.024 seconds/187220 KiB.
These are bounded resource measurements, with no speedup claim. Exact commands,
inputs, product hashes and outputs are retained under
`build/pending-reset-observations-20261127/`.

No new vendor, live-device, topology or timing support is claimed, and existing
resource bounds remain unchanged.

A separate pre-existing replay-history defect adds an unrequested all-X scan read
on regeneration of an always-active scan lane. Parent/current reproductions are
retained under `build/pending-reset-observations-20261127/replay-diagnosis/` and
queued immediately after Task199. The broader inventory remains open; item 1
remains excluded.


## Task200 write-only logical history

Write-only preparation can capture serial storage physically without requesting
its logical history. Such incidental captures must leave never-read indices
unknown and preserve earlier read values and miscompare masks. Explicit X reads
still record actual data. The scheduler now uses the existing logical read scope
for write-only applies as well as read-containing applies; physical/raw masks
and documented implicit TAP capture checks retain their existing contracts.

The repaired implementation passes 47 qualification gates: 227 Python suites
reporting 3554 tests, 24 leak-enabled native/CLI/corpus checks, three
C++ quality checks, corpus/editor/grammar checks and source-package build/install.
The leak-enabled write-only CLI check runs both new regressions; full normal/static
ordinary-read suites retain the existing sparse-scale and TAP controls. Five
optional reference cases are skipped. Exact hashes bind 4133 production/test
inputs and 44 executables. Source packaging is checked again after recording
these results. The one review repair pass restores the cared implicit IR check on write-only
mixed TAP groups while retaining read-containing scope. Its four configuration
regressions pass across source/flat/generated replay. One overview-prose Minor
is deferred alongside 44 inherited Minors, for 45 total.

Three-round measurements cover 16/128 eight-bit registers in 12 matched
parent/current points and 48 generation/continuation processes. Current unknown
history/miscompare literals and exact source/flat/generated bytes agree. The
largest current run is 0.224 seconds/62236 KiB, within 10 seconds/512 MiB.
Parent generated continuations reproduce the known history error; no speedup is
claimed over those incorrect results. The separate 72-point feedback matrix
passes all 240 candidate/current forms and all 24 parent controls; its largest
current run is 2.039 seconds/184552 KiB. Exact commands, inputs, hashes and
outputs are retained under `build/write-only-read-scope-20261127/`.

The transitive_releases forward certificate is refreshed with two successful
Tessent2026.2 exports of the corrected generated program and exact prior
schema3-certified reference. Their exports and physical traces are identical;
the existing schema4 policy retains the separate native logical trace and prior
provenance. This certifies that generated-program pair. Broader item6 and the
other completion inventory remain open; item1 stays excluded, 44 inherited
Minors remain deferred, and hardware, topology and timing support are unchanged.

Task200 is merged and pushed to master at `930f8e2`, containing repaired task
`2461a06`. All 16 merged-result checks pass on identical 4133 qualified inputs,
25 copied normal/static executables and 19 sanitizer executables, with all 117
stashes preserved. The final receipt records 48 qualification gates after the
documentation-package refresh. The owned task branch is deleted; its ledger,
16 rulings, helpers and source archive are retained under
`build/write-only-read-scope-20261127/`. The reusable detached worktree and
sibling ledgers are preserved. The one Important finding is fixed; 45 Minors
remain deferred. Continue the queued automatic-primary scale work under item10.


## Task201 primary callback solver scaling

Primary-only automatic callbacks retain their existing actual-data and event
contracts at the larger measured inputs. The scheduler now packs adjacent
unsigned objective bits within the existing 32-bit weight bound, omits
constant chunks and uses one bit for the two ranks of gated fixed differences. Data-deviation, drive-count and pin-deviation priorities retain
their original order. Pin constraints, native replay and resource limits are
unchanged; the serial objective path keeps its existing implementation.

The maintained matrix runs 300 actual generation/replay processes: 39 executed parent
default-generation controls, 117 normal and 117 static modeled forms, and 27
sanitizer forms with the existing 128-MiB quarantine configuration. Normal/static
coverage includes 1/2/8 lanes at 8/32 bits, 1/2 at 64 bits, one at 256 bits,
16/32 at eight bits and one at 512/1024 bits, in three rounds. Fresh actual
histories, miscompare masks/status, once-only callbacks, exact source/flat/generated
bytes and parent hardware are checked. The larger normal/static cases satisfy
10 seconds/512 MiB per process, with 256 MiB for single-wide cases. The 32-lane
normal modeled source median is 3.161 seconds;
the 1024-bit source median is 1.698 seconds and its
largest normal source RSS is 214452 KiB. The protected-parent modeled
32-lane baseline failed all three rounds; no speedup is claimed over those failed
observations. Its successful 1024-bit modeled source baseline exceeded 256 MiB.
Configured sanitizer measurement bounds do not describe the default functional
sanitizer environment, which retains its original leak-enabled configuration.

All 43 core qualification gates pass: 227 Python suites reporting 3558 tests,
25 leak-enabled native/CLI/corpus checks, three changed-C++ quality checks,
corpus/editor/grammar checks and source-package build/install. The new 32-lane
and 1024-bit functional regressions run under default leak-enabled instrumentation
without competing full-project CPU load. Five optional reference cases are
skipped. Exact hashes bind 4133 production/test inputs and 44 executables.
The final documentation-package refresh passed. The independent review and
merged-result checks below complete this task; 45 inherited Minors remain deferred.

Reproduce the maintained matrix from the repository root with built normal,
static and sanitizer generators and a retained parent binary:

```sh
python3 tests/src/pattern/benchmark_automatic_primary_readback.py \
  --binary build/icl-pattern-generation \
  --static build/icl-pattern-generation-static \
  --parent /path/to/parent/icl-pattern-generation \
  --sanitized /path/to/sanitized/icl-pattern-generation \
  --output build/primary-callback-measurements
```

Commands, input/output hashes, actual timing/RSS rows, the failed baseline and
experimental diagnoses are retained under `build/primary-callback-scaling-20261128/`.
Broader item10 workloads and original items4/6 remain open; this changes measured
solver scaling for already eligible models. Coverage item1 stays excluded.

Task201 is merged and pushed to `master` as `221c666`. The sole fresh
independent review found no Critical, Important or new Minor findings. All
44 qualification gates and 12 merged-result checks pass on identical 4133
production/test inputs, 25 copied normal/static products and 19 qualified
sanitizer products. The 300-process matrix and default leak-enabled functional
checks retain their documented environments. All 117 stashes are preserved.
The owned task branch is retired; its source archive, ledger, review, helpers
and receipts remain in `build/primary-callback-scaling-20261128/`. The reusable
detached worktree and sibling ledgers remain available. The 45 inherited Minors
and broader original inventory remain open; coverage item1 stays excluded.


## Task202 deferred guide errata

Seven source-backed documentation findings are resolved; the deferred
count is 45 minus 7 = 38. See [the guide correction record](pattern-generation.md#deferred-guide-errata-task202)
for the affected contracts and 12-check qualification. The independent review and
integration results are recorded below. Historical counts, broader inventory and excluded
coverage item1 retain their status. Evidence: `build/deferred-guide-errata-20261128/`.

Task202 is merged and pushed to `master` as `1dd0eb4`. The sole independent
technical/reader review found 0 Critical, 1 Important and 1 Minor finding.
The incomplete spacing is fixed in one qualified factual RED/GREEN pass;
the plan-checkbox Minor is closed by the planned completion update. No second
review is used. The 12 final gates include 3 fresh repair checks and 9 reused
behavioral checks bound to unchanged inputs/products; all 7 merged-result
checks pass. Seven inherited findings are resolved and 38 remain deferred.
All 117 stashes, 4133 production/test inputs and 44 executable identities are
preserved. The owned task branch is retired; its qualified source archive,
review, ledger and helper copies remain under `build/deferred-guide-errata-20261128/`.
The reusable detached worktree, sibling ledgers, broader inventory and excluded
coverage item1 retain their status.


## Task203 regression oracle maintenance

Three maintained oracles now check shared preview/readback status clearing,
nonpalindromic private opaque payloads through preparation and stable completion,
and every first-group fanout target/value write without imposing write order.
Controlled separate-owner and private-reversal mutants pass the old isolated
native cases and fail the strengthened cases. A deliberately removed downstream
write passes all nine old fanout coordinates and fails all nine strengthened
coordinates. These controls demonstrate test discrimination, not production bugs.

Task191's topology rejection finding was already fixed by Task192 commit
`6f49496`: current cases use valid targets and exact intended diagnostics.
An unrelated-diagnostic control passes the old historical cases and is rejected
by both current cases; actual normal/static shared-input suites also pass.

The 15-check qualification includes four complete native runs (two normal and
two default leak-enabled), four normal/static CLI suite runs reporting
58 tests, six changed-C++ quality checks, controlled-fault validation,
identity/whitespace checks and source-package build/install. Each generator's
existing nine-point fanout grid meets its 10-second bound for four replay forms.
All 4133 tracked source/test paths remain: 4130 inputs and 40 executables,
including every generator, retain their Task202 identities. Only three test
files and the four rebuilt native test executables differ. No production,
hardware eligibility, dependency, resource limit or performance claim changes.

Four inherited findings are addressed; the candidate remainder is 38 minus 4 = 34.
Independent review and integration remain pending. Evidence and exact fault
artifacts are retained under `build/regression-oracles-20261128/`. Broader
inventory and excluded strict-coverage item1 remain unchanged.
