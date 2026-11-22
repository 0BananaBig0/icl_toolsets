# Explicit DataRegister Callback Execution Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking. Inline execution is preserved from the completion program.

**Goal:** Execute explicit callback-only DataRegister Apply groups as real PDL endpoint procedures.
**Architecture:** Keep immutable logical callback request markers, coalesce their per-bit masks at Apply, bind the existing safe Tcl iProc engine to the real ancestor instance, and finish procedure endpoint queues before emitting declared write postconditions. Extend source-action anchors for reset rewriting.
**Tech Stack:** Existing C++17, Tcl9, Z3 and standard library; no dependencies or RTL edits.
**Spec:** docs/superpowers/specs/2026-11-22-callback-execution-design.md

## Global Constraints
- User delegates implementation choices and opted out of further questions; record choices as ours, not invented approvals.
- Callback-free programs keep existing paths and byte-stable supported output.
- No procedure invocation before Apply; reset-canceled queues need no loaded procedure.
- Callback-only caller groups; mixed ordinary/raw targets, -together, logical callbacks inside iMerge, physical callbacks, modeled callback histories and logical callback take/release remain unsupported.
- Real endpoint operations execute; logical write postconditions are explicit assumptions outside ICL connectivity, never read observations.
- Work is O(B log B + emitted procedure commands and indexed lookups); no per-target full-netlist/action scan.
- Preserve interpreter limits, atomic artifact publication,117 stashes, unrelated worktrees/ledgers and excluded coverage item1.
- New maintained tests live under tests/; one fresh final review and one verified blocking-fix pass, no rereview.

## Review Focus
- Snapshot pending operations/procedure definitions before invocation can append/reallocate/redefine them.
- A caught callback body error preserves completed effects but cannot flush pending endpoint writes without Reset.
- Source/flat ancestor bindings and literal placeholder arguments must agree under nested parameterized instances.
- Callback masks and write postconditions must never become invented read observations or failure counts.
- Reset prerequisites must hoist before the complete original caller group, including repeated/split-file invocations.

### Task 1: Callback invocation and all hardware-consumer boundaries
**Files:** Modify src/pattern/pattern_runtime.{h,cc}, pattern_access_queue.h, pattern_readback.cc, pattern_scheduler.cc, icl_pattern_generation_main.cc, Makefile; maintain tests/src/pattern_runtime_test.cc, pattern_readback_test.cc, tests/src/pattern/test_data_register_callback_model.py; create tests/src/pattern/test_callback_execution.py.
**Interfaces:**
- Append bool Operation::callback = false after existing fields, marking only logical callback access records.
- QueuedAccesses::WriteModel() returns pair<vector<BitReference>,string>, symmetric with existing ReadModel(); reuse a private Model(bool read).
- Runtime private CallbackBinding owns ancestor instance and ProcedureKey; BindCallback(size_t node,bool read) caches static ancestor/subnamespace metadata.
- Runtime private InvokeProcedure(const Procedure&,const string& instance,const vector<string>& arguments,const string& invocation,const string& origin,bool apply_tail=false) supplies the shared existing iCall body/parameter/frame handling.
- Runtime private ExecuteCallbacks(const Operation& apply) returns true for an executed callback group; it owns snapshots/masks, preflight, invocation stack and failed-queue state, and leaves no pending body tail.
- Generation skips callback markers after source-anchor processing; ResetTargets ignores them. ModelReadback rejects them. CLI physical/model modes reject executed callbacks before publication.
- [ ] Write native tests named TestCallbackQueueAndBinding and TestCallbackFailureRecovery: canonical actual endpoint operations, relative R, literal D/R strings and numeric/x data, original scalar no-callback operation counts, latest masks/alias inversion, reset cancellation, declaration loaded between files, valid redefinition, ancestor/module parameter injection, no frame leaks, direct/indirect cycle, pure preflight retry, and failed-body Apply/Finish blocking until Reset. Exact known basic write9 must emit endpoint write0b1001, Apply and logical iState0b1001; read callback compares real endpoint data without assigning its expected mask as state.
- [ ] Create CLI basic tests for ordinary PIO/ScanRegister endpoint procedures; source/flat/generated actual writes/reads, repeated targets, aliases/partial masks/ascending indices, nested/module subnamespace binding and special strings, once-only Tcl, split files, tail Apply, nested callback, unchanged unused declarations, and atomic unsupported mixed/-together/merged/model/physical/ownership cases. Update obsolete Task145 applied-rejection assertions to require actual deliberately failing procedure execution with old artifacts preserved.
- [ ] Run new CLI against immutable parent mastercc95460 and native tests against preimplementation source. Expected: callback execution not implemented (or missing new helper fields), not fixture syntax errors. Save meaningful RED.
- [ ] Implement callback queue markers and eager operand validation, reuse Read/WriteModel for latest full-node masks, preflight every callback namespace/procedure/mixed group before effects, then invoke in first effective target occurrence order. Snapshot owning records and look up the latest procedure at each actual call. Isolate pending spans, auto-apply body tails, diagnose active register/mode cycles, emit explicit write-only logical postconditions. Preserve no-op/unknown read requests and do not infer actual data.
- [ ] Implement failed-body queue flag only after invocation starts; completed effects survive, pending writes cannot flush until iReset, Finish refuses an unresolved callback error, preflight errors remain retryable. Restore pending bookkeeping on error. Retain source origin/call stacks and existing iCall limits.
- [ ] Add all consumer skips/guards, including reset targets, model/physical checks and logical callback ownership. Keep ordinary body merge/timing/reservation behavior. Unsupported modes must fail before any artifact publication.
- [ ] Build dynamic/static generators and native Runtime/readback checks with existing Z3/Tcl paths. Run new/legacy callback CLI plus affected observation/readback/queued-state checks. Expected: all pass, supported parent outputs unchanged. Commit.

### Task 2: Source anchors and local-reset callback replay
**Files:** Modify src/pattern/pattern_runtime.cc and icl_pattern_generation_main.cc if needed; extend tests/src/pattern_runtime_test.cc and tests/src/pattern/test_callback_execution.py.
**Interfaces:** Reuse Operation::callback and ExecuteCallbacks from Task1. Extend completed SourceAction.end by original marker index using sorted begin lookup; preserve SourceActions(bool whole_commands), argument capture and source ranges. ResetTargets already filters markers.
- [ ] Write failing tests for callback-request anchors queued inside a returned iCall/Tcl compound, actual endpoint local-reset dependencies, repeated same call site requiring different releases, queue across files, reset-canceled callbacks, callbacks with explicit/implicit body Apply and callbacks recovered after caught failure. Source reset rewrite must preserve original procedure bytes and once-only argument evaluation; source/flat/generated/reset-source outputs agree.
- [ ] Run focused cases. Expected: missing/unextended anchors or reset prerequisites expose incorrect behavior; retain actual RED.
- [ ] Extend each owning completed main action through actual callback execution and outer Apply; active call/compound scopes close naturally. Update end positions on partial callback errors for canceled queue grouping. Use stable operation indices and owned snapshots, never borrowed vector references across invocation.
- [ ] Ensure Generate processes marker source anchors before skipping hardware access; reset groups cover the whole invocation through outer Apply, and actual endpoint prerequisites/releases retain ownership. Do not flush a preceding caller queue early or modify callback procedure bodies.
- [ ] Run callback module plus existing reset invocation/argument/caller/compound and observation/queued-state modules with dynamic/static generators and native Runtime checks. Expected: all pass with byte-preserved definitions and unchanged legacy paths. Commit.

### Task 3: Measured qualification, review and integration
**Files:** README.md, docs/pattern-generation.md, pattern-model-coverage.md, remaining-work.md, this spec/plan; create tests/src/pattern/benchmark_callback_execution.py.
**Interfaces:** Publishes runnable explicit callback examples and measured limits; records current evidence.
- [ ] Run real endpoint examples and matched parent/current callback-free/unused-declaration controls; measure64/256/1024 callback targets/repeated applies with elapsed time/RSS, hashes and representative replay forms. Expected: supported cases pass within existing limits; report demonstrated limits without raising bounds.
- [ ] Run all14 native/14 leak-enabled sanitizer suites, complete dynamic/static generator suites, callback/state/readback/CLI sanitizer checks, affected format/tidy/strict C++17 and source packaging. Expected: all gates pass; preserve partial failure evidence without calling it full qualification.
- [ ] Obtain one fresh independent whole-branch review on most capable available model, with this Review Focus and ledger rulings. Re-grade by user effect; fix Important/Critical findings in one test-first pass and green suite; defer Minors, no rereview. Expected: no unresolved Important/Critical findings and exhaustive dispositions.
- [ ] Update examples/current tracker with exact supported behavior and remaining automatic/mixed/merge/readback/physical callback scope. Keep broad item4 pending until its full requirements are met. Expected: technical facts match source and executable examples.
- [ ] Merge into master, verify matching source/product hashes and focused integration checks, commit/push records, detach reused worktree, delete only this merged task branch and archive/remove only its own ledger. Expected: clean master equals origin/master,117 stashes preserved.
