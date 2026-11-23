# Mixed Callback Caller Groups Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans
> to implement this plan task-by-task. Steps use checkbox tracking. Inline
> execution is the existing selected method; no implementer subagents.

**Goal:** Execute separate ordinary and explicit callback requests in one caller
Apply, preserving native frames, caller write goals and actual history.

**Architecture:** Reuse QueuedAccesses and the append-only operation stream.
Track consumed logical callback positions within active groups; keep ordinary
pending ranges through procedure execution. Existing native Apply/reset and
ModelReadback remain the hardware/history authority; reject conflicting endpoint
writes instead of dropping known caller goals.

**Tech Stack:** Existing C++17/Z3/Tcl/Python3/Make; no new dependencies.

**Spec:** docs/superpowers/specs/2026-11-23-mixed-callback-groups-design.md

## Global Constraints

- Separate ordinary operations and callback-only aliases supported; a single
  mixed ordinary/callback alias remains rejected. Keep raw/together/merge/
  AccessTogether/physical/automatic downstream and actual invoked-read-callback
  history boundaries. Broader items4/6 and excluded coverage1 remain open.
- Validate all queue operands/bindings/arity/cycles before any procedure effects.
  Keep first-request order, last-by-bit data, original IDs/inversions/scopes,
  reservations, once-only arguments and immutable procedure definitions.
- No mutation of prior Operation entries. No new public operation/Runtime/
  readback interface unless a concrete regression proves a required caller fix.
- Ordinary requests flush/cancel only at actual Apply/reset. Do not add automatic
  preparation CSUs. Do not reexecute selected callbacks on nested Apply.
- Known ordinary caller-write bits remain protected throughout the callback
  group; compatible known endpoint writes allowed, different/unknown writes
  reject before that operation. Body reset clears pending caller constraints.
- Actual observations come from native frames only; preserve prior read history
  when reset cancels a read and do not invalidate completed inner reads again.
- New tests under tests/src/pattern; maintain native runtime tests in place.
  Preserve117stashes, existing worktree and unrelated ignored artifacts.
- Normal64/256/1024 andSAN64 callback targets, four probe groups, three repeats/
  source-flat-generated <=10s/512MiB; fresh generated getters and actual request/
  CSU/operand/endpoint/postcondition counts. Preserve1024callback10s and16Kalias
  256MiB budgets. Queue work is O(queued bits + emitted operations), with map log factors.
  Retained caller masks cost caller width times annotated boundaries; measured
  scale fixes caller width at four bits, as recorded after final review.
- Final15normal/15leakSAN native, both whole generators, selected SAN/corpus/
  producer/protocol/Make/quality/source packaging. Expected71suites/1153methods
  each after17newCLI methods,4033inputs/38products,59gates. Confirm actual counts.
- One fresh final independent review; one Critical/Important TDD fix pass,
  defer Minors/no rereview. Integrate/push/delete completed branch, then continue.

## Review Focus

- A nested Apply must not re-invoke the outer selected callbacks or miss a newly
  recursive request; the index set is not a permanent history.
- A reset or error after a committed inner frame must preserve exactly the
  existing history/poisoning and fixed selected-batch semantics.
- Inverted/partial ordinary aliases, controlling state and x endpoint writes
  must not bypass known caller goal preservation.
- A getter inside a write body followed by outer completion must keep the
  committed observation; canceled reads must keep prior history.
- Source actions spanning ordinary requests, callback expansion, namespaces and
  repeated caller sites must remain repairable without editing iProc bytes.

### Task 1: Runtime mixed queues, histories and caller goal preservation

**Files:** Modify src/pattern/pattern_runtime.cc,
tests/src/pattern_runtime_test.cc and Makefile.
Create tests/src/pattern/test_mixed_callback_groups.py.
Update tests/src/pattern/test_callback_execution.py only to rename its old
conflicting-mixed/together negative accurately; keep its negative cases/count.

**Interfaces:** Existing public Runtime/Operation signatures remain.
Add private Runtime::Impl helpers:
bool PendingCallback(size_t index) const;
void CheckCallbackCallerWrite(const Operation& operation).
Use private std::set<size_t> consumed_callback_operations_ and
std::vector<std::map<std::pair<size_t,int64_t>,char>> callback_caller_writes_.
These are active-group metadata, cleared at a safe outer Apply/reset boundary.
The guard returns immediately when all active caller maps are empty.

- [x] Add native RED TestMixedCallbackQueue (ordinary spare3 plus callback9,
      both source orders; one invocation, both hardware writes retained).
      Add TestMixedCallbackHistory (prior ordinary LastReadValue3; a pending
      read canceled by a body reset keeps3; completed inner reads are not
      invalidated again). Add TestMixedCallbackCallerWrites (same p9 allowed,
      caller p3 vs endpoint p9/x rejects with caller bit; inverted/partial alias
      identity; controlling iState cannot bypass; reset clears constraint).
      Add TestMixedCallbackFailureScopes (static invalid ordinary operand before
      calls counter; nested cycles; committed inner frame/error/reset retry;
      callback-only old operation entries remain unchanged).
      Run focused native RED; expected existing separate-Applies rejection.
- [x] Add first8CLI methods: test_orders_and_actual_values,
      test_ordinary_reads_and_callback_tail, test_inner_apply_and_selected_batch,
      test_body_reset_cancels_reads_and_writes,
      test_compatible_and_rejected_write_collisions,
      test_static_preflight_before_effects,
      test_reset_error_retry_and_independent_ownership, test_retained_guards.
      Use existing real CLI replay helpers, one argv-consuming helper module.
      Expected actual notes3/9, correct caller/native endpoint payloads, one
      invocation per selected request, prior canceled history3, atomic old
      output/debug/reset artifacts on failure; unsupported mixtures remain
      rejected. Register suite in existing Make generator macro.
- [x] Run newCLI RED with current dynamic binary and archive failure logs.
      Implement PendingCallback filtering by original operation position,
      uniform QueuedAccesses validation/coalescing, callback-only argument
      extraction and scoped consumed-index/caller-mask stack. Retain live
      pending range through invocation; inner Apply/reset use existing dispatch.
      Invalidate only still-pending hardware reads at actual Apply boundaries;
      ignore abstract logical callback markers. Keep failure/cycle/trace logic.
      Invoke CheckCallbackCallerWrite on normalized endpoint iWrite/controlling
      iState before history/pending/stream mutation; compare canonical resolved
      bit identities, not alias spelling. Reset clears active masks in place.
      Expected native and8CLI GREEN; no old callback behavior regression.
- [x] Build normal/static generator and runtime native with established Z3/Tcl
      paths. Run native, new8CLI eachvariant plus existing40callback,
      23endpoint-observation and primary-callback/queued/history compatibility.
      Run changed C++ format/tidy/C++17. Expected all pass and immutable earlier
      operation entries remain byte/field unchanged.
- [x] Commit and task-done using python3 /tmp/mixed-callback-groups-task1-check.py
      binding exact executed source/product/log hashes. Expected runtime queue/
      history/goal contract proved before expanding CLI qualification.

### Task 2: Complete replay, measured scale, review and integration

**Files:** Extend tests/src/pattern/test_mixed_callback_groups.py.
Create tests/src/pattern/benchmark_mixed_callback_groups.py.
Modify README.md, docs/pattern-generation.md, docs/pattern-model-coverage.md,
docs/remaining-work.md. Required caller fixes, if proven by failing tests, stay
in src/pattern/{pattern_runtime.cc,pattern_readback.cc,
icl_pattern_generation_main.cc} and their existing native tests.

**Interfaces:** Consume Task1 queue/goal/history behavior. The concrete native
indirect-write regression requires default-empty Operation.callback_preserved
caller masks, shared ResourceReservations.CallbackConstraints/CallbackValues and
a transactional guarded NativeScheduler.Assume overload; the refinement below
records the proof and caller scope. Reuse modeled-readback, source action and
reset rewriting; benchmark is standalone Python standard library.

- [x] Add final8CLI methods: test_split_prefix_namespace_arguments,
      test_local_reset_source_and_immutable_procedures,
      test_repeated_source_sites_and_returned_calls,
      test_getters_inside_body_and_after_outer_apply,
      test_read_callback_generation_and_model_guard,
      test_sparse_alias_masks_status_clear,
      test_selected_batch_body_reset_and_unselected_tail,
      test_scale_receipt_counts_and_fresh_generated_getters.
      Expected17CLI methods after one review state-bypass regression;
      source/flat/generated/reset-source and split replay
      agree, actual observed data/status remain independent of requested values,
      selected batch semantics match existing callback-only reference. Local
      reset may reject only existing impossible flush/ownership/reassertion
      cases atomically. Fix concrete shared caller failures test-first.
- [x] Implement reproducible benchmark normal64/256/1024 andSAN64,3repeats/
      source-flat-generated. Prepare spare with an ordinary Apply before each
      mixed group, then queue ordinary spare write/read plusNcallback writes,
      Apply and fresh spare_out getter (actual1/2/3/4). Report four probes and
      actual4*(N+2)emitted Applies/4*(N+1)PIOframes/4emptyouterApplies/
      0scanCSUs, explicit operand bits,4*Nendpointwrites and4*Npostconditions.
      Ordinary/unused controls emit8nonemptyPIOApplies/4endpointwrites.
      Generated probes replace exactly4literal notes with fresh level1 getters.
      Current observed27+SAN9 plus current ordinary/unused/observer-off27 and
      parent ordinary/unused18 gives81processes. Controls disclose different
      callback/storage/request dimensions; no speedup claim.
      Run fresh grid and preserved callback/alias/corpus budgets; expected bounds.
- [x] Write one exact guide example with full model/procs/commands and actual
      source/flat/generated3/9 notes, plus conflict/reset/history limitations.
      Author-check3invocations; no independent reader claim. Reconcile current
      README/model/tracker support without closing broad items4/6.
- [x] Commit; request one fresh final independent review of whole0c79c35..HEAD.
      Resolve/regrade all declined behavior; one Critical/Important TDD pass,
      defer Minors/no rereview. Run15normal+15leakSAN native, both whole
      generators, selected SAN histories/callbacks/new suite/opaque child,
      corpus/producer/protocol/Make/changed C++ quality/source packaging.
      After fresh full builds, reuse only exact-byte unchanged native products
      with unchanged test-source/fixture hashes and prior passing-log provenance;
      execute every changed native product. Whole generators/affectedCLI/
      packaging run current code. Bind exact maintained inputs/products/
      measurements; expected59gates and
      actual71x1153 eachvariant/4033inputs38products if no justified additions.
- [x] Merge into master; verify identical source/product hashes and focused
      root integration (existing20checks plus newCLI bothvariants=22checks,
      20copied normal products). Push, detach preservedWT/delete completed
      branch, archive only this ledger, preserve117stashes. task-done command
      python3 /tmp/mixed-callback-groups-completion-check.py. Expected clean
      pushed master/onlymaster, then continue original remaining inventory.


## Native caller preservation refinement

A concrete Task2 probe (build discovery in /tmp/mixed-indirect-caller-jilzuem2)
shows why a runtime target check alone is insufficient: after an inner Apply,
iWrite q9 can drive q's source p from the caller's3 to9. Add the minimal
required interface handoff: Operation::callback_preserved is a default-empty
map<size_t,string> of desired ordinary caller masks by original node ID,
never observed values. Snapshot it on callback Apply/controlling iState/idle
boundaries. Source operation entries remain immutable; no public PDL command.

Add ResourceReservations::CallbackValues(const Operation&,
const QueuedAccesses&) const -> optional<map<size_t,string>>. Merge these
caller constraints with existing reservations; known direct pending writes
matching the desired mask need no prior-state protection on their first frame.
A conflicting direct queued value rejects. Remaining known bits use existing
native preserved-resource proofs, including derived ports/clocked effects.
No extra writes or CSUs are introduced. Both Generate and ModelReadback use
the same helper; annotate caller context on failures without hiding the cause.

Add ScanScheduler::Assume(target,bits,preserved) as an overload. Empty masks
retain the old fast path; guarded assumptions use a native trial, existing
resource-value validation/concrete preservation and commit only on success.
Reject private opaque IDs as at the other public native boundaries. Callback
reset clears masks; actual invoked-read history/physical/raw boundaries remain.
Protect waits through existing preserved-map Idle/Wait APIs. Preserve masks
through merge processing or retain annotated Applies as barriers if required
by the actual regression. This extends the initially expected runtime-only
file surface to pattern_runtime.h, pattern_resources.h/.cc,
pattern_scheduler.h/.cc, main/readback and the affected native tests; the
existing private-state/solver machinery remains the sole event authority.

Native test TestMixedCallbackFrameGoals verifies exact Apply/state snapshots,
no prior-operation mutation and reset cancellation. Existing opaque native
privacy test verifies guarded-Assume rollback/private-ID rejection. Same-method
CLI regressions cover both derived-write directions, compatible derived writes,
indirect controlling state and a merged ordinary endpoint body. Run fresh checks
for every rebuilt/changed native product; reuse eligibility is byte-based, never
assumed from filenames.
