# Modeled merge readback implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking.

**Goal:** Return actual modeled merge histories while generating the same native
state trajectory and retaining batching without provider history requests.

**Architecture:** Treat valid native-model merge markers as event-free scope
transitions. Wrap the existing main readback callback with a cursor/latch and
thread a private serial decision through both GenerateMerged call sites.

**Tech Stack:** Existing C++17, Tcl, Z3 4.13.3 and Python unittest; no dependency.

**Spec:** docs/superpowers/specs/2026-11-24-merged-readback-design.md

## Global constraints

- Actual captured data, unknown masks, per-bit last-read planes and status/clear
  follow native model execution; no expected value or reset metadata substitutes.
- No Runtime/Operation API, user option, dependency or raised resource limit.
- Keep existing physical/opaque/read-callback/automatic-callback/timed guards.
- Preserve procedure bytes, once-only arguments, call identity, queue semantics
  and original/flat/generated/local-reset replay and failed-artifact protection.
- Track appended operations in linear time with one cursor and one latched bool.
- Direct parent masterc0735a5; preserve117 stashes and unrelated worktrees/files.
- New maintained scripts under tests/src/pattern; one native execution task,
  one fresh whole-branch review and one test-first blocker fix pass.

## Review focus

- A valid source-time read can differ after batching implicit DataRegister events.
- Cached/manual/zero-status and pre-merge queries must preserve existing batching.
- Marker processing must not create events or lose scoped ownership transitions.
- Callback reset-rewrite generation must use the same observed-order decision.
- Unsupported or failed histories must not expose partial deltas or replace artifacts.

### Task 1: Implement and qualify consistent modeled merge histories

**Files:** Modify src/pattern/pattern_readback.cc and its header contract comment;
src/pattern/icl_pattern_generation_main.cc; tests/src/pattern_readback_test.cc;
Makefile; maintained expectations in tests/src/pattern/test_merged_callbacks.py
and tests/src/pattern/test_raw_merge_calls.py, plus the necessary empty-merge
expectations in test_modeled_readback.py and test_modeled_data_readback.py. Create
tests/src/pattern/test_merged_readback.py and
tests/src/pattern/benchmark_merged_readback.py; update current evidence docs.

**Interfaces:** Preserve Runtime::Readback and MakeModeledReadback signatures.
Extend private GenerateMerged(netlist,operations,modeled_readback,combine=true).
Both callers pass the inverse of the main-local merge-history-request latch.
Retain existing malformed native iMerge-without-arguments and iClock negatives;
valid markers need only recognized begin/end arguments, not a second parser.

- [x] Step1: Add native tests for valid open/closed marker prefixes without
  data events, incremental raw/register planes and clear, and retained guards.
  Add20 CLI methods covering PIO/query timing, the ordering counterexample,
  no-request/cached/manual/pre-merge batching, unknown/sparse/inverted data,
  status/clear, read-before-write/branching, serial/TAP/raw/parallel histories,
  nested/repeated/returned calls, namespaces/prefix/split/once-only arguments,
  local-reset procedure preservation, reservations, explicit write callbacks,
  remaining callback/clock/opaque/physical guards and failed publication.
- [x] Step2: Run native readback and new CLI cases on parent. Expect valid
  merged observations to reject specifically on iMerge histories; retain logs.
- [x] Step3: Admit valid supported native markers in preflight and process them
  after reservations.Enter with no event. Keep malformed/physical/opaque guards.
- [x] Step4: Run native tests and the CLI ordering counterexample before changing
  rendering. Expect native positives to pass and the CLI replay probe to expose
  captured9 instead of source-observed0 if batching remains enabled.
- [x] Step5: Wrap the main readback callback with linear appended-prefix tracking.
  Add private combine parameter, pass the latch at initial and callback rewrite
  generation, and propagate serial errors without retrying the same trial.
- [x] Step6: Update the obsolete valid/empty-merge CLI history rejections to actual
  positive observations; retain their read/automatic/opaque/physical negatives.
- [x] Step7: Build normal/static/leak-enabled sanitizer variants and run focused
  native/CLI/legacy cases. Expect exact data, frame order, guards and replay.
- [x] Step8: Measure16/64/128-call PIO/raw histories, matched parent/current serial
  and batching controls, three forms/repeats, smaller sanitizer and512/1024-bit
  high-bit data. Use existing10s/512MiB regular and10s/256MiB wide limits; complete
  the matrix and retain misses. Record actual queries versus literal notes.
- [x] Step9: Run fresh15 normal/15 sanitizer native suites and full normal/static
  generator checks, affected sanitizer CLI, corpus, C++ quality and source package.
  Update current docs and commit; no native reuse for changed ModelReadback code.
- [x] Step10: Obtain one fresh whole-branch review, re-grade findings, fix blockers
  in one test-first pass and ledger Minors and declined-behavior costs.
- [ ] Step11: Merge into master, verify merged exact sources/products and relevant
  behavior, push, safely delete owned branch and archive only this plan's ledger.

Commands use the existing pinned Z3/Tcl paths. Focused CLI entry:
python3 tests/src/pattern/test_merged_readback.py build/icl-pattern-generation
Native entry: build/pattern_readback_test. Both must fail before implementation
and pass on the completed change; static and sanitizer use their existing paths.

Whole-task completion command: python3 /tmp/merged-readback-completion.py
Expected: required checks, complete measured outcomes and exact qualified hashes;
merged/pushed clean master, only owned branch removed and117 stashes retained.

Self-review: one task because native acceptance without matching rendering would
produce inconsistent observations. Both GenerateMerged callers and the two
existing valid-merge history negatives are included. Builder check reports
"nothing important left"; the recorded native ordering probe supplies the
counterexample and the unchanged no-query batching control.
