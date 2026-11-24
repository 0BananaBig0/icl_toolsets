# Downstream Write Callback Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: superpowers:executing-plans.
> Execute inline in the existing isolated worktree; no implementer subagents.
> Reuse the user's delegated choices and opted-out questions.

**Goal:** Invoke WriteCallBack prerequisites for transparent downstream writes,
then execute their ordinary requirements with existing native scheduling.
**Architecture:** Trace selected write-data paths and coalesce effective masks.
Stage dependent ordinary requests until callback postconditions exist; retain
immutable logical markers and reuse existing dispatcher/queue/source consumers.
**Tech Stack:** Existing C++17/Tcl/Z3/Python3/Make; no new dependency.
**Spec:** docs/superpowers/specs/2026-11-23-downstream-write-callbacks-design.md

## Global Constraints

- Direct parentmasterb9e59e2 -> codex/downstream-write-callbacks, retained clean
  isolated .worktrees/bsdl-defaults. Preserve117stashes/unrelated worktrees.
- Designd6cb9ed plus measured baseline refinement; fresh builder reports
  nothingimportantleft. Choicesours, not specificuserdesignapproval.
- IEEE1687-2014 6.4.9 printed117: callbacks can supply downstream data. Plain
  expected reads do not create controlling writes. No invented observations.
- Transparent selected Source/Alias/WriteDataSource paths only; preserve indices,
  inversions, segments and instance identity. Hand ordinary feedback/scan/primary
  boundaries back to native scheduling; do not reject valid old feedback graphs.
- Effective same-target last-wins before inference; aggregate compatible callback
  demands. Cross-target and explicit/inferred known-bit conflicts preflight.
- Dependent requests wait until prerequisites finish. Unrelated caller goals,
  immutable source stream, consumed markers and source spans retain Task162.
- Body reset cancels staged requests; failed queues require reset/retry. Binding,
  arity and cycles preflight before effects. No independent preview scheduler.
- Keep raw/together/AccessTogether/active-iMerge restrictions for actual dispatch
  and retain ownership restrictions. Physical
  and modeled automatic callback histories reject before artifact publication.
- Pure and mixed explicit callback behavior stays qualified. Public APIs stay
  unchanged unless an actual regression proves need and design is refined first.
- Normal16/64/128,SAN16,4groups,3repeats/source-flat-generated<=10s/512MiB.
  Parent16/64/128 controls0.2314/1.2178/4.0630s;256control15.8781s is explicitly
  outside the selected scale before implementation, upgrade under broaderitem10.
  Wide512/1024-bit payloads3normal repeats<=10s/256MiB.
- Record commands/environment/inputs/products/output hashes, connected callback+
  destination storage, target/operand/frame counts and actual endpoint payloads.
  Callback arguments/notes are requests, never live observations. Explicit
  controls may contain extra empty Applies; compare physical frames separately.
- Retain Task162243process matrix and4dynamicallocationprobes. New72regular+
  6wide+3guide ->324matrix/example processes, subject to measured design refinement.
- Expected73suites/1189methods eachgenerator after20newCLI;4039inputs/38products,
  15normal/15leakSAN native,63gates using prior61 plus newSANCLI plus production budget fault injection. Actual C++quality.
- Build first; reuse only exact unchanged source/product + passing log evidence.
  Every changed product and wholegenerator/package must execute current code.
- One fresh whole-parent review, one Critical/Important TDD pass, defer Minors/
  no rereview. Preserve declined-behavior rulings and concrete costs.
- Merge directparentmaster, verify original24checks plus newnormal/staticCLI
  (26checks), identicalinputs and20copiednormalproducts. Push,detach,deletebranch,
  archiveonlyownledger,continueoriginalinventory;coverage1excluded.
- Use grouped policy + commit-message-storyteller on finalized selections.

## Review Focus

- Overwritten downstream masks and duplicate alias bits must not cause stale
  callback invocations or silently resolve distinct-target conflicts.
- Body/nested reset must cancel only pending staged work, retain completed
  frames, and not resurrect goals through references to outer group records.
- Inferred and explicit requests sharing a callback must preflight together
  while preserving direct explicit behavior and first effective source identity.
- Ordinary feedback cycles without a callback must retain their existing native
  behavior; structural tracing is not a new general graph-validity gate.
- Source actions, private marker bookkeeping and modeled-history rejection must
  survive callback bodies with getters, caught errors, split files and local
  reset insertion without mutable old operations or duplicate invocation.

- Established logical callback declarations must not suppress explicit requests,
  including no-care automatic groups; read/miscompare history must not seed them.
  Task2's test_multiple_targets_and_explicit_requests covers established/changed/
  explicit/no-care combinations; test_logical_and_observation_boundaries covers
  actual versus requested data and old-observer-cursor rejection.

### Task 1: Transparent dependencies, staged queue and preflight
**Files:** Modify src/pattern/pattern_runtime.cc, pattern_runtime.h,
pattern_readback.cc, icl_pattern_generation_main.cc, Makefile,
tests/src/pattern_runtime_test.cc.
Create tests/src/pattern/test_downstream_write_callbacks.py.
Only concrete guard/consumer failures justify src/pattern/pattern_readback.cc or
icl_pattern_generation_main.cc changes; record the interface refinement first.

**Interfaces:** Private WriteCallbackSource(const BitReference&) ->
optional<BitReference>, with immutable-path cache and selected-bit work budget.
Prepare effective callback prerequisites inside ExecuteCallbacks(Operation),
reusing its existing request record and dispatcher. Private staged operations
associate original stream indices with ordinary payloads; reset generation/
consumption follows existing group boundaries. Public Runtime methods unchanged;
Operation gains trailing default-false callback_prerequisite actual-invocation
provenance as justified by the verified consumer refinement below.
Task2 consumes this behavior through unchanged generator commands/SourceActions.

- [x] task-start/read brief, recordBASE. Add6native tests:
      TestDownstreamCallbackDispatch (endpointp9 before desten1, original marker
      immutable, callback once, original ordinary goal eventually scheduled);
      TestDownstreamCallbackMasks (selected/sparse/inverted/ascending and effective
      last-wins, compatible merge and distinct-target conflict);
      TestDownstreamCallbackReset (before Apply cancellation and body/nested reset,
      completed inner frames retained, caught-error reset retry);
      TestDownstreamCallbackPreflight (missing binding/arity/cycle before effects,
      explicit/inferred coalescing and conflict);
      TestDownstreamCallbackGoals (unrelated caller p3 still rejects bodyp9,
      compatible9 works, valid ordinary feedback remains unchanged);
      TestDownstreamCallbackSpans (source owns staged and dispatched operations,
      origins/stacks and once-only arguments, no old-entry mutation).
      Addfirst8CLI: test_direct_and_forwarded_paths, test_sparse_inverted_segments,
      test_effective_masks_and_no_care, test_multiple_targets_and_explicit_requests,
      test_body_apply_and_reset, test_caller_goals_and_ownership,
      test_preflight_and_failure_recovery, test_read_and_unsupported_mode_guards.
      Expected actual missing-dispatch RED against immutable parent, no weakened
      ordinary invalid-target/history/physical/merge guards.
- [x] Implement selected-bit WriteCallbackSource through transparent port/Alias/
      selectable-data sources. Cache immutable path results, preserve cumulative
      inversion and index mapping; leave ordinary cycles to native execution.
      In Command classify/stage dependent ordinary members without early native
      queue consumption, preserve full original markers and normalized args.
      ExecuteCallbacks computes effective per-target last-wins first, infers
      compatible masks and checks explicit/inferred conflicts before any effects.
      Reuse binding/arity/cycle and body dispatch. Append staged native requests
      only after surviving prerequisites and logical postconditions, before their
      final outer Apply. Body reset generation cancels stale staged records.
      Add compact actual-prerequisite markers and reject their provenance in the
      readback provider and final CLI publication guard, as proved by the actual
      false-acceptance negative. Expected native6+CLI8GREEN, pure/mixed explicit
      streams unchanged.
- [x] Run allRuntime normal/static; new8CLI normal/static; existing40callback,
      17mixedgroup,16mixedalias,23endpoint,10primarycallback,15queued,
      30observation and14feedback CLI each variant. Run changed-range C++format/
      tidy/C++17. Expected allpass with only the verified trailing provenance field API change;
      named failures only
      if an existing expectation becomes obsolete while preserving real negatives.
- [x] Bind exact source/product/log hashes in task1receipt. Finalize selected
      diff, groupedstorytellercommit, task-done with
      python3 /tmp/downstream-write-callbacks-task1-check.py.
      Expected core independently testable before expanded qualification.

### Task 2: Replay, scale, review and integration
**Files:** Extend tests/src/pattern/test_downstream_write_callbacks.py.
Create tests/src/pattern/benchmark_downstream_write_callbacks.py.
Modify README.md, docs/pattern-generation.md, docs/pattern-model-coverage.md,
docs/remaining-work.md and thisplan. Existing main/readback/source code changes
only for real test-first consumer failures, with design/ledger refinement.

**Interfaces:** Existing generator/source-action/local-reset APIs from Task1.
Benchmark reuses existing hash/command-count helpers, records its own connected
hardware dimensions and actual PIO writes; no request notes as observations.

- [x] Addlast8CLI before implementing missing replay/benchmark:
      test_prefix_namespace_and_split_inputs, test_once_only_arguments_and_order,
      test_reset_rewrite_retains_procedure_bytes, test_repeated_and_returned_sites,
      test_nested_and_canceled_groups, test_logical_and_observation_boundaries,
      test_scale_receipts_and_physical_controls, test_wide_dependency_budget.
      Expected16total; source/flat/generated/reset replay agree and original
      procedure bytes survive. Reproduce/fix only actual consumer regressions.
- [x] Implement72regular processes: current inferred source/flat/generated at
      16/64/128 (27), current explicit-before/unused source controls (18),
      parent matching explicit-before/unused (18), SANinferred16source/flat/
      generated (9); four value groups, threerepeats. Wide512/1024x3normal (6).
      Verify exact callback arguments, endpointp/en writes and native frame order;
      derive zeroCSU only after rejecting unexpected iScan. Record empty Applies
      separately for explicit controls, do not assert identical source-workload
      speedup. Full input/output/binary/environment provenance and original
      connected storage/target/operand counts. Expected all bounds unchanged.
      Preserve Task162243matrix+4allocationprobes; author-check3new guide commands.
- [x] Write complete model/PDL/commands and observed endpoint versus logical
      callback explanation, reconcile current support and unfinished inventory.
      Expected no broaditem4completion or coverage1 claim; guideauthorchecked,
      independentreader only if actually performed.
- [x] Commit reviewableimplementation, dispatch onefreshwholebranchreview on
      parentb9e59e2..HEAD with this ReviewFocus/template/ledger rulings. Regrade
      declined behavior, one verified blockerfixpass; no rereview. Build all,
      execute full63gates/73x1189each/15normal15SAN/sourcepackage/actualquality,
      source/product hashes and all measurement receipts. Expected exact4039input/
      38products and no in-scope blocker; retain unavailable tools as unrun.
- [x] Merge into master, run26integrationchecks on identicalinputs/20copiedproducts,
      push/detach/delete ownbranch/archive ownledger with117stashes preserved.
      task-done: python3 /tmp/downstream-write-callbacks-completion-check.py.
      Expected clean pushedmaster/onlymaster; continue broader inventory.

### Verified Task1 refinement
The real modeled automatic-history negative falsely accepted after core dispatch.
Add src/pattern/pattern_runtime.h's trailing default-false bool
Operation.callback_prerequisite and compact invocation markers; update
src/pattern/pattern_readback.cc to reject actual invocation before any new delta.
Copy the new metadata in projection construction; pure and canceled requests keep
false. Native dispatch asserts actual provenance without marking ReadCallBack.
Sparse positives use minimal selected hardware; shared-enable whole-register
collateral control rejects identically before/after inference.

Task2 verified compatibility refinement: track only successful callback logical
postconditions and controlling iState, clear on Reset, and filter matching
inferred-only requests. An actual parent-valid explicit-before control otherwise
runs callbacks twice. Explicit request keys must be retained even for automatic
no-care groups. Existing History storage avoids repeated full-stream scans;
this is no native scheduler preview or observation cache.


Final review: 0Critical/5Important/1Minor. All five Important cases reproduced
before fixes (four CLI methods/eight subcase failures and the compiled production
budget test). One fix pass addresses them; no rereview. The additional
AccessTogether fixture initially lacked an established prerequisite and failed
identically on immutable parent and head; declaring value9 makes both pass.
The Minor exact postcondition-pair benchmark oracle is deferred, increasing the
running Minor ledger from14 to15; payload identity remains functional-test work.
Expected final counts:73suites/1189methods per generator,4039inputs/38products,
63gates,20 downstream CLI methods,15quality and26 merged-result checks.


Task163 passes63 final qualification gates:15 native and15 leak-enabled sanitizer suites,73 suites/1189 methods in each generator variant,15 C++ quality checks, source packaging and4039 input/38 product hashes. The72 regular downstream processes peak at4.2884s/373760KiB; six512/1024-bit runs peak at1.1308s/121236KiB. All324 new/preserved matrix and guide processes plus4 dynamic allocation probes pass. Independent review found0Critical/5Important/1Minor; one test-first pass fixes all five Important findings. The benchmark checks postcondition counts, widths and allowed names; exact ordered register/value pairs remain a deferred Minor. Functional regressions check identities and payloads. All26 merged-result checks pass on identical4039 inputs and20 copied qualified executables, with117 stashes preserved. Broader inventory and excluded coverage item1 remain open.

Integration:26 checks passed on identical4039 inputs/20 products; direct-parent merge d8874747ea84cef45fd5f66c70308f52df78c92c.
