# Merged Callback Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: superpowers:executing-plans.
> Execute inline in the retained isolated worktree; no implementer agents.
> Reuse the user's delegated choices, question opt-out and integration instructions.

**Goal:** Execute supported callbacks within valid iMerge calls using serial fallback.

**Architecture:** Reuse Runtime callback dispatch and Merger::Block serialization.
Mark callback provenance as a batching barrier; preserve ordinary neighboring
batching and existing source, queue, reset, reservation and history consumers.

**Tech Stack:** Existing C++17/Tcl/Z3/Python3/Make, no new dependency or public API.

**Spec:** docs/superpowers/specs/2026-11-24-merged-callbacks-design.md

## Global Constraints

- Direct parentmaster6a311dd -> codex/merged-callbacks in existing clean isolated
  .worktrees/bsdl-defaults. Preserve117stashes and unrelated worktrees/artifacts.
- Design02aa103; fresh builder says nothing important left. Choices ours, not
  specific user design approval. Inline execution is the existing chosen method.
- Parent explicit/inferred serial controls pass; both merged variants reproduce
  the guard. Discovery and immutable parent receipts live under
  build/merged-callbacks-20261124/{parent,discovery};4039baseline inputs verified.
- IEEE1687-2014 7.9.18 printed176–178 permits serialization and requires call Apply
  ordering, empty entry/return queues and no reset in called procedures.
- Keep whole callback-containing streams serial, with logical/actual provenance
  barriers. Ordinary independent streams remain batchable; no callback speedup.
- Preserve nested merging, note hoisting, call identity, arguments, source spans,
  once-only evaluation, caller goals/reservations and preflight/cycle failures.
- No reset recovery inside active merges; callback body reset remains forbidden.
  Modeled iMerge histories, actual read/automatic histories and physical callback
  output retain guards. Broader raw/opaque merges remain item8; coverage1excluded.
- Source/flat/generated and local-reset rewritten replay must agree, including
  literal procedure definitions. Refine actual failing consumers before fixing.
- New tests/benchmarks in tests/src/pattern; maintain existing native tests.
- Reuse connected-storage fixture/parser; four value groups,16/64/128normal,
  sanitizer16, three source/flat/generated repeats and512/1024-bit wide payloads.
  Limits10s/512MiB regular,10s/256MiB wide; parent256 is outside selected envelope.
- New benchmark checks exact ordered callback register/value postconditions,
  actual PIO frames, logical/actual/generated distinctions and full provenance.
- Preserve Task163324matrix/example processes and4 dynamic allocation probes;
  new72regular+6wide+3guide ->405matrix/example processes, subject to measured
  refinement. Expected74suites/1205methods each generator after16newCLI,
  4041inputs/38products,15normal/15leakSAN native,64gates and28mergedchecks.
  Quality count follows actual changed C++ files; no invented pass counts.
- One fresh whole-branch final review; one test-first blocker fix pass; defer
  Minors with costs, no rereview. Reuse exact passing evidence only when source,
  dependencies, binary and logs remain identical. No source edits during builds.
- Merge into master, verify merged identical inputs/20copied qualified products,
  push, detach retained externally managed worktree, delete only own branch,
  archive only own ledger and continue original inventory. Grouped storyteller
  policy applies to every finalized selected commit diff or merge range.

## Review Focus

1. Empty or read-only callback bodies lacking iState must remain explicit merge
   barriers; test native logical and actual markers independently.
2. Neighboring ordinary calls must retain batching, while nested callback call
   streams retain complete Apply order and note hoisting; test both together.
3. Actual callback tail work must settle before the merged iCall returns; test
   implicit Apply, returned/repeated call sites and unapplied ordinary returns.
4. Failed/cyclic callbacks or forbidden resets must not publish artifacts or
   accidentally clear merge/queue state; test errors before and after body work.
5. Source rewriting must retain complete merge ownership and procedure bytes,
   including nested, repeated, split-input and once-only expressions; compare
   source/flat/generated/rewritten outputs, not notes as device observations.

---

### Task 1: Dispatch and serial merge barriers

**Files:** Modify src/pattern/pattern_runtime.cc, src/pattern/pattern_merge.cc,
tests/src/pattern_runtime_test.cc, tests/src/pattern_scheduler_test.cc,
tests/src/pattern/test_downstream_write_callbacks.py,
tests/src/pattern/test_callback_execution.py,
tests/src/pattern/test_mixed_callback_aliases.py and Makefile.
Create tests/src/pattern/test_merged_callbacks.py.

**Interfaces:** Existing Runtime::Execute/Finish/operations/SourceActions and
MergeOperations(const Netlist&,const vector<Operation>&,bool). No new API.
Produces callback-expanded ordered merge streams for Task2 replay/measurement.

- [x] Add native TestCallbacksInMerge: explicit/read/inferred callbacks run within
      valid called procedures, markers retain context, reset remains forbidden.
      Add TestCallbackMergeBarriers before old scheduler tests: two independent
      ordinary call streams combine to one Apply; logical callback or actual
      callback-execution markers retain two Applies, flags and original payloads.
- [x] Add first8 CLI methods before product edits: test_explicit_writes,
      test_inferred_writes, test_read_and_empty_callbacks, test_mixed_aliases,
      test_call_order_and_notes, test_nested_calls_and_merges,
      test_goals_and_reservations, test_protocol_rejections. Assert exact callback
      argument/order and endpoint frames; source/flat/generated agree. Negative
      reset/unapplied/cycle/failure cases preserve retained output artifacts.
      Register new suite beside downstream suite in Makefile. Convert the old
      called-downstream, explicit-callback and mixed-alias active-merge negatives
      to positives; retain direct-body
      writes and -together rejection assertions in its existing method.
- [x] Run first8 CLI and new native cases against unchanged implementation.
      Expected dispatcher rejection and two-Apply barrier assertion failures;
      fix test fixture errors before implementing.
- [x] Remove only the active-iMerge callback dispatcher rejection; make
      Merger::Batchable reject streams with logical or actual callback markers.
      Keep protocol, reset, ownership, failure and history rules unchanged.
- [x] Build normal/static generator and Runtime plus normal scheduler native.
      Run all Runtime cases both variants, scheduler native and first8 CLI both
      variants, existing merge and callback/downstream suites. Execute actual
      changed-line format/tidy/C++17 checks. Expected all current cases pass.
      Ledger any concrete consumer refinement and commit the verified core.

### Task 2: Replay, measurements, review and integration

**Files:** Extend tests/src/pattern/test_merged_callbacks.py; create
tests/src/pattern/benchmark_merged_callbacks.py. Modify README.md,
docs/pattern-generation.md, docs/pattern-model-coverage.md, docs/remaining-work.md
and this plan. Other consumer edits only for test-first failures with refinement.

**Interfaces:** Consume Task1 existing operations/source contracts and downstream
benchmark fixture, physical_counts, digest, counts and operand_bits helpers.
No new public runtime interface or independent scheduling engine.

- [x] Add last8 CLI methods first: test_prefix_namespace_split_inputs,
      test_once_only_arguments, test_reset_rewrite_preserves_procedures,
      test_repeated_and_returned_sites, test_failures_and_cycles,
      test_observation_and_physical_guards, test_scale_receipts,
      test_wide_receipts. Expected16total; all supported replay forms match,
      source procedure bytes remain unchanged, and known guards preserve files.
- [x] Implement matched benchmark using existing downstream fixture: place its
      four groups in one iProc called inside iMerge for merged mode; serial
      controls use the same body without iMerge. Retain unused controls. Current
      merged source/flat/generated16/64/128x3=27, current serial/unused18,
      parent serial/unused18, SANmerged16source/flat/generatedx3=9:72regular.
      Wide512/1024x3normal=6. Assert ordered register/value postconditions and
      exact reused physical frames; generated callbacks execute zero times.
      Record all dimensions, empty Applies, hashes, commands and environment.
      Run small receipt grid first; then full grids after review fixes.
- [x] Document complete model/program/commands with explicit serial fallback,
      actual versus logical histories and remaining boundaries. Author-run all
      three guide commands; no independent-reader claim without that work.
      Self-review plan/spec reach, then commit reviewable implementation.
- [ ] Dispatch one fresh whole-parent review on6a311dd..HEAD with ReviewFocus,
      code-reviewer template and ledger rulings. Regrade declined behaviors;
      one test-first Important/Critical fix pass, ledger Minors, no rereview.
- [ ] Build current normal/static/SAN products; run64expected gates including
      all native suites, both74x1205generator variants, existing SANselection
      plus new16CLI, corpus, production budget test, allocations, quality and
      source packaging. Verify actual input/product hashes and405matrix/example
      processes. Source-bound checker must assert actual counts and provenance.
- [ ] Merge direct parentmaster; run28merged checks on identical4041inputs and
      20copiedproducts, verify117stashes, commit integration evidence, push,
      detach/delete own branch and archive only own completed ledger.
      Task-done uses a source-bound completion checker verifying the complete
      qualification, clean pushedmaster, deleted task branch and receipts.
      Continue the broader inventory; no broad item4 completion claim.


Task1 verified: all Runtime cases normal/static, full native scheduler,378 CLI
compatibility methods,12 actual C++quality checks; exact9source/5product/28log
hashes in build/merged-callbacks-20261124/task1/receipt.json. Legacy explicit,
downstream and mixed-alias caller-merge rejection assertions are now positive
replay checks. Native passes reused only with exact source/dependency/product/log
proof after maintaining Python expectations; original failed attempts retained.


Task2 pre-review:16CLI pass normal3.027s/static3.696s/leakSAN8.141s; current
4C++sources retain12 actual quality checks. All products freshly built;
17 representative regular processes plus512/1024-bit runs pass(max4.76824s/
394780KiB), not the full repeated matrix. Three guide commands produce exact
identical frames/postconditions; source/flat invoke2callbacks, generated0 and
its notes are literal. Author checked, not an independent reader. No consumer
code refinement was needed. Full review/64gates/405matrix/integration remain.
