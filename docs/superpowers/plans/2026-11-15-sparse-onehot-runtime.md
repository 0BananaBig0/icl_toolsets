# Sparse one-hot runtime implementation plan

Execute inline using superpowers:executing-plans, on verified2cb4ec8 plus reviewed Task85 literal batching.

Goal: preserve OneHotData/Scan values, masks, X padding and enable ambiguity without declared-width work for every short contributor. No new dependency, public flag, graph model or event semantics.

Spec: docs/requirements.md A3; existing IEEE6.4.15 source order immaterial and unknown padded lanes; shared concrete/symbolic generator model.

Root cause: both group evaluators call OneHotSourceBit across every declared lane for each contributor. Elaboration already stores only actual source bits and sorts descending width for graph traversal; retain that representation and sort invariant.

Scope: src/pattern/pattern_symbolic.cc, src/pattern/pattern_state.cc, existing tests/src/pattern_symbolic_test.cc (already exercises both models), guide/model ledger/this plan.

- [x] RED --onehot-scale uses valid unique enabled child output producers, widths/counts512/8,8192/64,65536/512; concrete unique selection and multiple-active unknown; symbolic dynamic data, known single enable; proof3s/command20s.
- [x] RED build0/scale124 at65536/512;512 symbolic .00508s,8192 .41314s; concrete .000057/.00201/.12848s; whole20.01s/95572KiB. Source/test manifest retained in WT/build/sparse-onehot-red.
- [x] Small mixed-width1/3/7 source tests: all27 enable states, four data0/1/X patterns, source inversion, opposite group ranges/order, explicit expected concrete and symbolic values.
- [x] Concrete: evaluate all guards, retain the selected actual payload, and append X padding only for the final unambiguous selection. Validate actual active references without materializing full-width padded payloads.
- [x] Symbolic: traverse existing descending-width descriptors in reverse; accumulate actual-width payloads, widen the accumulated result with unknown prefix only when needed; append final padding once. Preserve all guards/ambiguous/unknown enable semantics. Retain the existing empty-descriptor selection-condition rejection before payload evaluation.
- [x] Identical20s/3s benchmark GREEN; focused small cases, all existing native/CLI callers, style and independent review.
- [x] Combine Task85/86 for one immutable required sanitizer/full integration, source-matched remote regression, honest scale evidence and commit.

Review focus: mixed widths, no/all/unknown enables, inverted source and X masks, empty payload contributors, held/shift OneHotScan state, duplicate/overlapping contributors, and source-order invariance. No change to OneHotReadResolver enable interpretation; all-data Enable semantics remains separate.

Ruling: actual OneHotReadResolver rejects empty descriptors before payload evaluation (no selection condition). Removed unreachable empty-payload fallback; add explicit concrete/symbolic rejection tests. Reverse one test group descriptors manually so runtime correctness with unsorted descriptors is covered instead of relying only on elaboration resorting.

Matched local GREEN build0/scale0:512/8 .00139s,8192/64 .00161s,65536/512 .00851s symbolic; concrete .000055/.000126/.000867s. Whole .05s/42112KiB vs RED20.01s/95572KiB timeout. Independent source review clean. Test-constructor and inline-instance grammar pilots failed, preserved separately; corrected fixture now passes final native symbolic suite, including explicit empty selection-condition errors and manually unsorted sources. Other connected native/CLI checks from second pilot all0; final combined connected run includes corrected symbolic tests.

Final combined local connected build/native7/main68/clock41/client18/reset12/multi22/feedback10 all0; format/naming0. Final symbolic suite includes corrected empty-instance fixture and all word/mixed-run/unsorted/source/mode checks. Independent reviews clean. Integrate now onto verified2cb4ec8; freeze source/tests for required sanitizer/full. Exact final remote10-hash run is pending.


Final source-matched remote integration:60 statuses; build/native7/all41CLI/aggregate and all3 changed literal/onehot benchmark repetitions pass. Only3 intentionally failing bounded-memory baseline repetitions return1 (outofmemory). Actual10 final and10 baseline source/test hashes match manifests. Identical final benchmark program uses512MiB VM/20s command/3s proof; baseline fails at262144 in1.12s/457980–458192KiB, changed ten-case profile through1M completes13.46–13.50s/65008–65148KiB on the remote host; largest pair6.080–6.104s. Onehot three runs .09s/42128KiB,65536/512 symbolic .02847–.02861s. Host/compiler/binary hashes and commands are retained; these remote figures are distinct from local measurements. All required local leak-enabled build/native7/main68/clock41/client18/reset12 pass; full serial check is still running with source/test3 unchanged. Evidence: build/symbolic-runtime-final-remote/evidence, build/symbolic-runtime-final-sanitizer-* and build/symbolic-runtime-final-full.*.


Verified Tasks85–87 checkpoint: required leak-enabled build/native7/main68/clock41/client18/reset12 all0. Full serial make check exits0 in1327.585s:41 dynamic and41 static generator suites,51 directional goldens,2523 ICL files with334/334 grammar and119/119 scanner rules,LSP62 and editor6 in177.462s. Five optional ANTLR tests remain skipped. Final remote native7/all41 and changed benchmark repetitions pass, with10 final and10 baseline source/test hashes verified; the3 deliberate memory-baseline failures are preserved. Reviewed source/test3 remain unchanged throughout. Commit this eight-file snapshot without push. ScopeTask88 and capture/updateTask89 remain isolated; broader clock/reset/events, resources, vendor, strict coverage and final platform requirements remain mandatory.
