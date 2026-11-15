# Symbolic literal-run implementation plan

> Execute inline with superpowers:executing-plans; Root's Task83/84 source stays frozen until its required checks finish.

**Goal:** Evaluate supported large literal and virtual-reset Source vectors without one Z3 concat per bit.
**Architecture:** Reuse SymbolicValue::Literal for contiguous missing-node references. Share the existing single-bit literal/reset/inversion interpretation between Read and Bits. Finite-node slices/repeats retain their current implementation.
**Tech Stack:** Existing C++17 and Z3; no dependency or public CLI change.
**Spec:** docs/requirements.md A3 and the approved generator design's shared symbolic/concrete model.

Ruling: proceed under the user's repeated instruction to finish all work without further confirmation. This is a measured supported-input cost, within the remaining optimization/scale acceptance work.

Scope: src/pattern/pattern_symbolic.cc; existing tests/src/pattern_symbolic_test.cc; generator guide/model ledger/this plan. Read/Value/Active/Logic/Compute/scheduler callers use the shared path. No graph/clock semantics change. Reuse the existing isolated worktree; preserve Root immutable regression.

Root cause: Bits calls Read then Concat for each missing-node literal or virtual-reset reference. Each reference creates tiny solver values and an increasingly long concat chain. An O(W) character buffer passed once to the existing literal constructor preserves order/masks with O(W) temporary memory. Balancing concat expressions would retain per-bit solver allocation; raising timeouts would hide the demonstrated cost.

Review focus: unknown '?' and x; normal/inverted 0/1; both global reset selectors for all 0/1/x states; physical/literal/physical and repeated/slice boundaries; invalid literal rejection through both APIs. Keep held/shift mode and finite-node range errors intact.

- [x] Add TestLiteralSignalRuns and test-only --literal-scale, sizes512/8192/65536/262144, normal/mixed virtual reset refs. Each proof retains3s; whole benchmark20s.
- [x] RED remote actual3 hashes match. First65k-only pilot passes in2.162/2.031s at65k. Extended run times out124 during262144 after65k2.162/2.042s. Keep both pilots.
- [x] Add small mixed-run/invalid/all-reset-state correctness cases before implementation.
- [x] Share literal-character interpretation and aggregate adjacent missing-node refs. Existing finite repeat/slice logic unchanged.
- [x] Same remote benchmark20s/proof3s GREEN; full native and41 CLI connected checks; independent review and style.
- [x] Advance from verified83/84 commit; required leak/full checks, evidence/docs and commit without push.

Evidence: build/literal-signal-red{,2}-remote/{manifest.json,commands.log,evidence}. No speedup claimed until matched GREEN. No promise at4M maximum until measured.

First matched GREEN build/scale/native all0; actual3 hashes match. Eightcases under unchanged20s/3s limits;65536 .475/.332s and262144 7.321/5.412s. Independent source/test review clean. Formatting applied; three equivalent baseline/changed resource repetitions and final formatted connected remote native7/all41 suites started. Baseline source is frozen Root83/84, with identical final test program for timed --literal-scale.

Local connected build/native7/main68/clock41/client18/reset12/multi22/feedback10 all0. Format/naming0. Ruling: combine the next measured sparse-onehot runtime fix with this task before immutable required leak/full checks; a single final integration run will verify both changes. This avoids duplicate full-suite runs while retaining focused RED/GREEN, connected and independent review gates for each change.

Final combined local connected build/native7/main68/clock41/client18/reset12/multi22/feedback10 all0; format/naming0. Final symbolic suite includes corrected empty-instance fixture and all word/mixed-run/unsorted/source/mode checks. Independent reviews clean. Integrate now onto verified2cb4ec8; freeze source/tests for required sanitizer/full. Exact final remote10-hash run is pending.


Final source-matched remote integration:60 statuses; build/native7/all41CLI/aggregate and all3 changed literal/onehot benchmark repetitions pass. Only3 intentionally failing bounded-memory baseline repetitions return1 (outofmemory). Actual10 final and10 baseline source/test hashes match manifests. Identical final benchmark program uses512MiB VM/20s command/3s proof; baseline fails at262144 in1.12s/457980–458192KiB, changed ten-case profile through1M completes13.46–13.50s/65008–65148KiB on the remote host; largest pair6.080–6.104s. Onehot three runs .09s/42128KiB,65536/512 symbolic .02847–.02861s. Host/compiler/binary hashes and commands are retained; these remote figures are distinct from local measurements. All required local leak-enabled build/native7/main68/clock41/client18/reset12 pass; full serial check is still running with source/test3 unchanged. Evidence: build/symbolic-runtime-final-remote/evidence, build/symbolic-runtime-final-sanitizer-* and build/symbolic-runtime-final-full.*.


Verified Tasks85–87 checkpoint: required leak-enabled build/native7/main68/clock41/client18/reset12 all0. Full serial make check exits0 in1327.585s:41 dynamic and41 static generator suites,51 directional goldens,2523 ICL files with334/334 grammar and119/119 scanner rules,LSP62 and editor6 in177.462s. Five optional ANTLR tests remain skipped. Final remote native7/all41 and changed benchmark repetitions pass, with10 final and10 baseline source/test hashes verified; the3 deliberate memory-baseline failures are preserved. Reviewed source/test3 remain unchanged throughout. Commit this eight-file snapshot without push. ScopeTask88 and capture/updateTask89 remain isolated; broader clock/reset/events, resources, vendor, strict coverage and final platform requirements remain mandatory.
