# Mixed Callback Alias Bits Implementation Plan
> **For agentic workers:** REQUIRED SUB-SKILL: superpowers:executing-plans.
> Execute inline task-by-task; no implementer subagents. Reuse the existing WT.

**Goal:** Retain ordinary bits and explicit callbacks in one alias access,
without changing actual hardware/history/source-action contracts.

**Architecture:** Keep the immutable original logical marker; append compact
ordinary runs in Runtime command normalization. Existing hardware consumers skip
markers and consume ordinary operations; Task161's queue/native proof stays shared.

**Tech:** Existing C++17/Tcl/Z3/Python3/Make; no new dependency.
**Spec:** docs/superpowers/specs/2026-11-23-mixed-callback-aliases-design.md

## Global Constraints
- Parentmaster9fc1e55 -> codex/mixed-callback-aliases; clean isolated existingWT.
  Preserve117stashes/unrelated worktrees/artifacts; archive only this plan ledger.
- User delegated choices/opted out of questions; design choices are ours, never
  label them specific user approval. Builder reports nothing important left.
- Direction-specific partition only. Keep original marker immutable, all original
  IDs/origins/scopes and callback order/once-only args. Pure targets unchanged.
- Ordinary projections cover only selected canonical bits/runs. Normalize
  inversion and duplicate last-wins; use BaseName for segmented/vector selection.
  No full-node X padding that samples or updates unselected bits.
- Validate full operands and every projected write's caller-goal guard before
  appending. Existing binding/arity/cycle preflight precedes procedure effects.
- Existing Apply/reset/consumed-index/caller/native rollback/history/source-action
  paths remain authority. No new public API unless a real regression proves need.
- Retain AccessTogether/together/raw/active-iMerge/automatic-downstream/physical/
  invoked-read modeled-history guards. Logical callback storage stays unknown.
- New maintained tests in tests/src/pattern; existing native Runtime test stays.
- No source edits during builds. Focused checks first; whole checks after review.
- Performance <=10s/512MiB, normal64/256/1024/SAN64,4groups,3repeats/source-flat-
  generated. Fresh actual spare1/2/3/4 getters; validate actual output/work counts.
  New4096/16384 mixed aliases3normal repeats <=256MiB; compact ordinary run.
  Preserve old mixed/callback/wide/corpus budgets,243 total measured/example runs
  if the planned matrix remains appropriate; disclose distinct source command
  counts, no speedup claim. Caller width measured4, wider width unmeasured.
- Expected72suites/1169methods eachgenerator,4035inputs/38products,60gates,
  15normal/15leakSAN native. Quality count is actual (likely6).
- After fresh builds, reuse only byte-unchanged native product + unchanged test
  source/runtime fixture + exact passing-log provenance. Every changed native,
  whole generators, affectedCLI and packaging execute fresh. Document reuse.
- One fresh whole9fc1e55..HEAD review, one Critical/Important TDD pass, defer
  Minors/no rereview. Retain all declined behavior as explicit rulings/costs.
- Merge directparentmaster, verify24root checks/20copiedproducts/identicalinputs,
  push/delete/detach/archive, then continue original inventory; coverage1 excluded.
- Follow current grouped commit-message policy through commit-message-storyteller
  on each finalized selected diff/range; commit/push authorization persists.

## Review Focus
- Original markers and ordinary projections must not execute ordinary accesses
  twice or reinvoke callbacks on nested Apply; duplicate alias bits are last-wins.
- Canonical segmented/ascending/reversed/inverted sparse runs must not read or
  write unselected bits or lose per-bit history.
- A mixed request inside an active callback must not bypass caller-goal guards;
  retain Task161 mapped-enable/state/reset rollback fixes.
- Canceled mixed reads must preserve prior ordinary history without marking a
  ReadCallBack invoked; actual invoked reads keep existing model guards.
- Source spans must own all projected runs and callback expansion without
  modifying iProc definitions or reevaluating arguments.

### Task 1: Compact runtime ordinary projections and callback preflight
**Files:** Modify src/pattern/pattern_runtime.cc,
tests/src/pattern_runtime_test.cc, Makefile.
Create tests/src/pattern/test_mixed_callback_aliases.py.
Maintain old tests only if a named boundary expectation becomes obsolete,
preserving negative mode/conflict coverage and truthful method names.

**Interfaces:** Private Runtime::Impl::CallbackOrdinaryAccesses(Operation,
PdlTarget,string) -> vector<Operation>. Existing public Operation/Runtime APIs,
hardware queues and readback/source-action callers unchanged.

- [x] task-start/read brief and recordedBASE. Add native RED cases:
      TestMixedAliasProjection (marker + compact ordinary run, callback once,
      endpoint retained, prior entries immutable);
      TestMixedAliasRanges (ascending/reversed/inverted/duplicate/segmented bits,
      exact selected spans and no full-node padding);
      TestMixedAliasHistory (ordinary sparse read invalidation/cancellation,
      prior history and committed inner frame);
      TestMixedAliasCallerGoals (mixed alias inside callback cannot bypass known
      caller bits; compatible/direct/derived/mapped-enable controls);
      TestMixedAliasFailureScopes (static invalid/missing binding before effects,
      cycle, caught failure/reset/retry);
      TestMixedAliasSourceSpans (one source command owns marker+all projected
      runs and its callback expansion).
      Add first8CLI methods: test_full_alias_and_actual_ordinary_values,
      test_inverted_nested_and_partial_aliases,
      test_ascending_segmented_and_duplicate_bits,
      test_multiple_callbacks_and_direction_binding,
      test_inner_apply_reset_and_cancellation,
      test_static_preflight_and_publication_atomicity,
      test_caller_goal_conflicts_and_compatible_controls,
      test_read_callback_generation_and_model_guard.
      Run native/new8 against unchanged product; Expected actual guard RED and
      no weakened existing invalid-target/unsupported-mode checks.
- [x] Implement CallbackOrdinaryAccesses with original per-bit identity/last
      occurrence and inversion-normalized values. Aggregate selected indices per
      node, coalesce consecutive runs in declaration direction and build canonical
      BaseName ranges/scalars. No X widening. Prepare and caller-check every run
      before appending original marker then projections in one source span.
      Preserve flags/origin/stack and old pure-target stream shape.
      In ExecuteCallbacks skip ordinary bits when building callback positions;
      reuse full-target queue validation/coalescing and existing ordinary pending/
      consumed-index/known-goal logic. Expected6newnative/8CLI GREEN.
- [x] Build normal/static generator and Runtime native with existing Z3/Tcl pins.
      Run all Runtime cases/new8CLI bothvariants plus40callback,17mixed-groups,
      23endpoint-history,10primary-callback,15queued and30observation methods.
      Run changed-range format/tidy/C++17. Expected all pass; no public API change.
- [x] Bind exact executed source/product/log hashes in task1 receipt; stage final
      diff, use grouped storyteller message, commit, task-done with
      python3 /tmp/mixed-callback-aliases-task1-check.py. Expected core proven
      before expanded replay/benchmark work.

### Task 2: Full replay, measured scale, review and integration
**Files:** Extend tests/src/pattern/test_mixed_callback_aliases.py.
Create tests/src/pattern/benchmark_mixed_callback_aliases.py.
Modify README.md, docs/pattern-generation.md, docs/pattern-model-coverage.md,
docs/remaining-work.md. Only concrete shared caller failures justify changes
in existing main/readback/merge/source-action code and relevant native tests.

**Interfaces:** Consume Task1 projections without new public APIs. Reuse native
ModelReadback, original SourceAction begin/end and reset rewriting. Benchmark
imports existing pure fixture/hash utilities where appropriate; keep actual
mixed-alias dimensions and output checks explicit.

- [ ] Add final8 methods before implementation:
      test_actual_getters_inside_write_procedures,
      test_canceled_mixed_reads_preserve_prior_history,
      test_prefix_namespace_split_files_once_only_arguments,
      test_local_reset_rewrite_preserves_procedure_bytes,
      test_repeated_sites_returned_calls_and_source_spans,
      test_sparse_masks_status_and_unknown_callback_storage,
      test_scale_receipt_dimensions_and_generated_getters,
      test_wide_alias_compact_projection_budget.
      Expected16 total methods; source/flat/generated/reset-source and split
      behavior agree, untouched ordinary history bits remain intact, invoked
      ReadCallBack model guard stays, canceled uninvoked reads keep prior data.
      Fix concrete shared caller regressions test-first, recording interface
      refinements in design/ledger before edits.
- [ ] Implement mixed alias benchmark normal64/256/1024/SAN64,4groups,3repeats/
      source-flat-generated and separately queued parent/current controls.
      Keep actual4*(N+2)Applies/4*(N+1)PIOframes/4emptyouter/4*Nendpoint/
      postconditions/8spare writes; check all actual output accesses are intended
      PIO and no iScan before reporting zero scan work. Record declared callback
      storage/ordinary width, actual source command/operand counts, fresh getters,
      tools/environment/commands/time/RSS/input-output-binary hashes.
      New grid81 + mixedwide4096/16384 x3=6. Preserve old mixed81, primarycallback60,
      purewide6, corpus6 andguide3 ->243 processes. Each grid process<=10s/512MiB,
      wide<=256MiB; no unmeasured speedup claim.
- [ ] Write full guide model/procs/commands and actual3/9 example. Author-check
      source/flat/generated3invocations; no independent reader claim. Reconcile
      current support/remaining bounds in README/model/tracker without closing
      broad4/6 or excluded1.
- [ ] Commit final reviewed implementation selection via grouped storyteller.
      Request one fresh independent final review whole9fc1e55..HEAD with template
      and exact ReviewFocus. Regrade/rule all declined behavior; one Critical/
      Important RED->GREEN pass, defer Minors/no rereview. Fresh builds/byte-proof
      native reuse only as specified. Run full60gates:15normal/15SAN native,
      both72x1169 whole generators, selected SAN/newalias/oldmixed/opaque/history/
      raw/TAP/legacy/model/corpus/producer/protocol/Make, actualC++quality and
      source packaging. Bind4035inputs/38products and matching final measurements.
- [ ] Merge directparentmaster and verify24root integration checks, identical
      input/product hashes and20copiednormalproducts. Push, detach preservedWT,
      delete completed branch, archive only this plan ledger and117stashes.
      task-done: python3 /tmp/mixed-callback-aliases-completion-check.py.
      Expected clean pushedmaster/onlymaster; continue original inventory.
