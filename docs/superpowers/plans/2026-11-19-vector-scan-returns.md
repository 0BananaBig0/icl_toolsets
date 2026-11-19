# Independent vector scan returns implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax.

**Goal:** Schedule independent vector one-hot scan returns without redundant chain declarations.
**Architecture:** Infer each root endpoint in dependency order, then reuse CheckIndependentChains and the existing parallel frame/replay engine. Preserve the single-path alias case and require no TAP/effective interface for inference.
**Tech Stack:** Existing C++17/Z3/Tcl and Python unittest; no dependencies.
**Spec:** docs/superpowers/specs/2026-11-19-vector-scan-returns-design.md

## Global Constraints

No new public API, ICL syntax, metadata, dependencies or RTL. New maintained tests stay under tests/. Preserve unsupported control/overlap boundaries and raw 100% line/90% branch coverage gate. O(V+E) endpoint pass, existing O(R log R) root-map work and structural limits.

## Review Focus

- Root aliases and gated observations must retain their existing behavior: native regression and full suite.
- A branch with a different endpoint or no endpoint must be rejected even when inactive: negative CLI variants.
- Shared intermediate storage or input bits must not become independent lanes: native/CLI rejection.
- Explicit interfaces and TAP event ownership must not be bypassed: negative CLI plus existing chains/TAP suites.
- Multiple lane padding, inversions and state copies must agree with concrete native state: native assertions, explicit-chain control and replay.

### Task 135: Independent scan return inference

**Files:** Modify src/pattern/pattern_scheduler.cc; tests/src/pattern_scheduler_test.cc; tests/src/pattern/test_one_hot_scan_scheduling.py; tests/src/pattern/test_shared_scan_outputs.py; README.md; docs/pattern-generation.md and docs/pattern-model-coverage.md. Create this plan and its spec.
**Interfaces:** Consumes ScanScheduler::Impl graph_, order_, roots_ and CheckIndependentChains(map<Key,Key>); produces unchanged ScanStep/ScanLane APIs and parallel_chains_ behavior.

- [x] Step 1: Add test_vector_independent_returns_and_replay with two lanes of widths 4 and 2, values 9 and 2, both branches, offset/ascending ports and inversions, canonical original/flat/generated/reset replay. Run against the base generator; Expected: nonzero with multiple scan outputs require shared-control transaction scheduling.
- [x] Step 2: Infer unique endpoints across all graph edges, gated by empty effective interfaces and TAPs, validate ownership, enable existing parallel mode. Run the new CLI test; Expected: pass.
- [x] Step 3: Add native TestImplicitIndependentScanChains asserting serial lanes 0001/1001, padded expected 01xx/1001, unused input pins, copied state and failed reserved writes rollback. Add CLI rejection variants for shared input/storage, ambiguous/disconnected endpoints, explicit controls, unknown/contending one-hot enables; all preexisting outputs preserved. Add 8 lanes x 8 branches control and replay. Expected: native and all one-hot methods pass; record timings without a speedup claim.
- [x] Step 4: Run changed-line clang-format, clang-tidy and strict C++17 checks; native/leak sanitizer, full normal check, fresh raw coverage; Rocky full generator dynamic/static suites and fresh local/remote source archive/install checks. Expected: all executable suites zero, ANTLR environment skips disclosed, strict coverage exit2 retained if below 100/90. Record every deviation/ruling and measured size/timing.
- [x] Step 5: Update English/Chinese current support/qualification sections, commit candidate, run task-done using the evidence verifier (zero only when all scoped checks are recorded and raw coverage failure is explicitly retained), request one fresh whole-branch review, regrade findings, one necessary fix pass, defer minors. Expected: no unresolved Critical/Important findings.
- [ ] Step 6: Refresh final archive and evidence hashes, merge into parent master, synchronize changed generator products, verify byte identities and clean origin tracking, push and delete this merged branch, archive only this plan ledger. Expected: clean master/origin and no task branch. Full project acceptance remains open while strict coverage fails.

Self-review: all spec clauses map to this single task; interfaces and test placement match existing files. Execution is inline under the user standing Proceed authorization with one fresh final reviewer.
