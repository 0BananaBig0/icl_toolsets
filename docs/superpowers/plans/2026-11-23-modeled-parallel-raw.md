# Modeled Parallel Raw Scan Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans inline, task-by-task. Steps use checkbox syntax for tracking.

**Goal:** Execute exact modeled root named-chain raw CSUs and expose actual per-chain readback.
**Architecture:** Static disjoint parallel path metadata plus the existing concrete capture/ShiftComplete/update engine; a vector result preserves all fields under one Apply, and typed chain-aware queues feed it.
**Tech Stack:** Existing C++17, Z3/Tcl and Python standard library. No dependencies or RTL.
**Spec:** docs/superpowers/specs/2026-11-23-modeled-parallel-raw-design.md

## Global Constraints
- Parentmasterd68226f, reused isolated worktree, one active task branch.
- User delegated choices/opted out of questions; record choices as ours.
- One explicit root client with named static disjoint unconditional paths and common direct/virtual controls; no shared storage/SI, dynamic/gated/ambiguous routes, hidden ScanRegisters, child/hierarchical/TAP or event-driven DataRegister models.
- Held/event-free data/capture logic only when controls/routes cannot change during a CSU; all resets direct/virtual and inactive at replay.
- One common CSU per group, max lane length shifts; shorter SI prefix/ignored SO suffix follows existing native convention; actual returned bits come from captured shift state and parity, never expected data.
- Current per-register fill completes unrequested fixed fields with x expectations; only explicitly requested fields publish source histories/claim assigned storage.
- Normalize before replacement/cancel; actual width match on Apply; aggregate effective scan bits<=1048576 before large allocation. Existing bounds/IR/end-state/merge/readback/physical guards remain.
- Reuse ClientControls, concrete Clock/ShiftComplete, preservation/fill and atomic commit/subscriber behavior; no new simulator or per-bit full-netlist scans/padded lane matrix.
- Preserve scalar/child/TAP/opaque behavior, public single-result API contracts and default unavailable readback.
- Tests under tests/; preserve117 stashes, unrelated worktrees/ledgers and excluded coverage1.
- One fresh most-capable whole-branch final review, one test-first Important/Critical fix pass, defer Minors, no rereview.

## Review Focus
- Unequal lane lengths/inversion/capture sources yield real first-return samples and exact final state under one CSU, including unknowns.
- Implicit fields preserve held values without fabricating source histories or claiming ownership; replacement/cancel uses full typed keys.
- Every unsupported field/shape/length/IR/directive preflights before raw effects, assigned IDs or observations; retry preserves state, postcommit subscribers do not roll it back.
- Vector SI updates preserve unrelated bits, complete state/reset semantics and actual common control events; cached topology never caches mutable values.
- Skewed lane sizes remain bounded by aggregate data/work rather than channels*maximum width; benchmark artifacts and qualified executable hashes are fresh/reviewable.

### Task 1: Native static parallel raw replay and vector API
**Files:** Modify src/pattern/pattern_scheduler.{h,cc}, maintain tests/src/pattern_scheduler_test.cc and tests/src/pattern_readback_test.cc as needed.
**Interfaces:** Add vector<Operation> ScanScheduler::ApplyRawScans(const vector<Operation>&, const map<size_t,string>& preserved={}, set<size_t>* assigned=nullptr, vector<ScanStep>* preparations=nullptr, const RawObservation& observe={}); return canonical scans only. Existing single-result APIs reject parallel preflight, scalar fallback delegates unchanged. NormalizeScan uses generic NormalizeScan/ReadScanFields for named parallel fields and preserves defer_length.
- [x] Add native TestModeledParallelRawScans and focused selector --parallel-raw:4/2-bit RootChains with common controls, exact normalized two-command vector, single hardware CSU, actual first0 then9/2 returns, Register final values9/2 then6/1, inverted/ascending/vector/multi-register fields, constant/unknown capture. Add reference per-clock replay with max-width SI prefix/SO suffix alignment; compare complete states/real returns, not expected masks. New API tests RED compile or parent guard; preserve meaningful output.
- [x] Add preflight failure/cancel/length/invalid chain/default/IR/overlap/hidden/gated/reset/end-state/physical boundaries, numeric ownership and retry/subscriber-commit tests. Direct helper tests keep unchanged scalar APIs and source shapes. Unsupported cases preserve old native state/fill/assigned set and zero observations.
- [x] Implement cached immutable root/field paths using existing graph tracing and reference indices; validate all stated shape/control/reset/resource bounds. Reuse static pure-forward capture snapshot and concrete phases/ShiftComplete, or existing bounded per-edge path when required. Parse exact loads/parity, preserve vector bits, complete implicit fields from current fill, validate all fields before state effects, publish owned observations only after commit. Preserve cached-state correctness on copies/Reset/Assume.
- [x] Build dynamic/static generators and native scheduler/readback with existing deps; run --parallel-raw plus scalar/child/TAP raw, opaque parallel, native readback and Task154 state/copy controls. Expected native new behavior passes and prior paths unchanged. Commit and task-done full focused gate.

### Task 2: Chain-aware generator/model queues and CLI replay
**Files:** Modify src/pattern/icl_pattern_generation_main.cc, pattern_readback.cc and runtime only if needed; create tests/src/pattern/test_modeled_parallel_raw.py; maintain native runtime/readback tests and Makefile.
**Interfaces:** Consume ApplyRawScans/NormalizeScan from Task1. Both main and model raw maps use tuple<bool,string,string> (IR,target,chain), with ReadScanFields offsets. Emit returned scans then one original Apply; model publishes actual explicit-only ScanReadbackUpdates. Runtime existing interface/chain history planes remain authoritative.
- [x] Write CLI methods for unequal fields, one Apply, all-x reads, care/mismatch/status/clear, independent latest replacement, named default, split files, reset cancellation, unknown/constant captures, SI/SO inversion and ascending indices. Actual returns contradict deliberately wrong -so and do not follow new SI before Capture. Source/flat/generated exact patterns, original/flat getter values and native shared hardware state must agree. On parent/Task1 generator expect scalar queue/API guard (or overwritten-chain failure), retain meaningful RED.
- [x] Add implicit current-fill completion with cleared comparisons/no invented source history, held/unwritten resources under existing policy, ordinary preparation with existing mixed -together rejection, sibling/vector-bit effects, invalid payload/length/overlap/shape/IR/physical/limit atomic artifacts, cancellation before required initial reset and retry-safe native API checks. Keep model iMerge/timed/physical limitations explicit.
- [x] Replace scalar queue keys/hardcoded offsets in both consumers, use vector API and existing reservation/observation publication. Validate every discarded numeric operand and all fields before executing; no expected-to-observation shortcut or fake regular-register history.
- [x] Run new CLI module on both variants, native runtime/readback/scheduler selectors, affected scalar/TAP/opaque/raw-history/queued/resource/state/CLI modules. Expected actual values/status, exact replay and old artifact protection all pass; commit/task-done full focused check.

### Task 3: Measured qualification, review and integration
**Files:** Create tests/src/pattern/benchmark_modeled_parallel_raw.py; update README.md, docs/pattern-generation.md, pattern-model-coverage.md, remaining-work.md and this spec/plan; owned evidence/ledger.
**Interfaces:** Consume Task1/2 exact group/state/histories and qualified binary hash; no broad item6/8 closure.
- [ ] Build a fresh-artifact scale driver with8/32/64 chains at representative32/512/4096-bit widths plus a skewed shape (one long field/many short fields) within1MiB. Three source/flat/generated runs, actual sampled values/state checks, elapsed/RSS, input/output/product hashes and scalar/opaque matched controls; parent parallel remains unsupported, never reported passing. Target representative replay<=45s/process and<=512MiB RSS; no padded field-count*max-width allocation or per-field subprocesses inside execution. Record demonstrated bounds.
- [ ] Run exact-binary Task154 callback<=10s1024-source budget with original Task153 reference and wide-alias256MiB budget, retaining all required controls. Expected existing measured acceptance remains, no unmeasured speedup/global optimum claim.
- [ ] Run all14 native/14 leak-enabled sanitizer suites, complete dynamic/static generators, affected sanitizer CLI, changed-code format/tidy/strict C++17, source packaging and input/product hashes. Expected full green; preserve failed/partial evidence separately.
- [ ] Obtain one fresh independent most-capable whole-branch review with Review Focus and ledger rulings. Re-grade by user effect; fix Important/Critical once test-first with full required checks, defer Minors, rule on each declined-to-judge line; no rereview.
- [ ] Publish runnable nonempty real-return examples/current tracker with exact supported topology, side effects, assumptions and remaining hierarchy/dynamic/shared/TAP/merge/transport scope. Verify examples and original/flat/generated replay; identify modeled data, not live-device evidence.
- [ ] Commit/merge into direct parentmaster, verify source/product/integration hashes/checks, push, detach reused worktree, delete only this completed branch and archive/remove only its own ledger. Expected clean master==origin/master,117 stashes preserved. Continue remaining authorized inventory afterward.
