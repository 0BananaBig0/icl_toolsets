# Read-callback histories implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking.

**Goal:** Observe actual native endpoints through invoked read-callback bodies.

**Architecture:** Reuse existing expanded operations and masked ordinary
observer; remove only read-callback-specific guards. Reuse the benchmark runner.

**Tech Stack:** Existing C++17/Tcl/Z3/Python, no new dependency.

**Spec:** docs/superpowers/specs/2026-11-24-read-callback-histories-design.md

## Global constraints

- Parent master249150971ffb6fb4c3674fd6d21e23085fb560a0; owned child codex/read-callback-histories.
- Expected <D> is never observed data; retain logical callback unknown masking.
- Preserve automatic, physical, timed and unexpanded callback guards.
- Keep10s/512MiB regular and10s/256MiB wide; preserve all misses.
- Preserve117 stashes/ignored files/24 inherited Minors; native inline, one final review.

## Review focus

- Read callback/reset metadata cannot fabricate logical register observations.
- Nested or caught callback failures must retain actual queue/reset semantics.
- Mixed ordinary operations must keep their caller frames and reservations.
- Provider-observed merge/reset rewrites must retain the source trajectory.
- Parent controls must lower the same endpoint events, including real wide captures.

### Task 1: Qualify invoked read-callback endpoint observations

**Files:** Modify src/pattern/icl_pattern_generation_main.cc and
src/pattern/pattern_readback.cc; create tests/src/pattern/test_read_callback_histories.py
and tests/src/pattern/benchmark_read_callback_histories.py. Modify Makefile,
tests/src/pattern/benchmark_merged_readback.py and affected obsolete negatives.
Update the native read-callback reset expectation in tests/src/pattern_readback_test.cc
to require four actual zero endpoint bits, mismatch mask1001 and clearable status2.
Update docs/pattern-generation.md, docs/pattern-model-coverage.md and docs/remaining-work.md.

**Interfaces:** Preserve public APIs. Keep main's automatic prerequisite check
and physical rejection; ModelReadback rejects physical callback_execution but
no longer rejects callback_read_execution alone. Existing runtime flags remain.
Benchmark main(make_fixture=fixture,wide_calls=1,applies_per_call=1) preserves default behavior;
fixture(...,owner=None) permits equivalent parent/current fixture construction.

- [x] Step1: Add12 methods named test_actual_endpoint_and_status,
  test_body_getter, test_no_query, test_unknown_endpoint,
  test_sparse_inverted_request, test_nested_write_callback,
  test_namespaced_callback, test_selectable_and_serial_endpoints,
  test_reset_and_retry, test_split_and_local_reset,
  test_merged_calls, test_unsupported_boundaries. Assert actual9/6 captures,
  mismatch2+2/clear0, partial/unknown masks and original/flat/generated agreement.
  Logical callback getters must remain unavailable; failed output/debug/reset
  artifacts stay unchanged. Expected: immutable parent rejects the new positives
  through the two reproduced guards; do not accept wrong failures.
- [x] Step2: Remove only the two read-specific guard terms. Expected: focused
  suite passes with actual endpoints; automatic/physical/timed negatives remain.
  Replace obsolete read-only rejection expectations with exact actual replay.
- [x] Step3: Add the reusable factory/width arguments with unchanged defaults;
  add the small callback factory and retain all reporting tests. Expected:
  direct parent/native callback PIO outputs match with two Apply records per
  call; raw routes remain direct scan-only controls, with real high-bit data
  captured on the second wide scan. Focused tests cover serial read callbacks.
- [x] Step4: Run complete378-process callback matrix and existing378-process
  merge matrix, plus72 word controls and four guide processes. Expected: full
  actual lists/frames/source-flat-generated replay match, all current bounds pass.
- [x] Step5: Run fresh15 normal/15 leak-enabled SAN native suites, full generator
  normal/static, affected CLI/corpus, C++ quality, package and existing work/
  allocation/reporting probes. Expected: all functional gates and exact hashes pass.
- [ ] Step6: Document measured behavior and commit; one fresh final review,
  re-grade/fix blockers once test-first and ledger Minors/declined judgments.
  Expected: no unresolved Critical/Important finding.
- [ ] Step7: Merge into master, verify identical qualified inputs/products and
  affected behavior, push, delete owned branch and archive only this ledger.
  Expected: clean pushed master/117 stashes retained.

Focused: python3 tests/src/pattern/test_read_callback_histories.py build/icl-pattern-generation
Completion: python3 /tmp/read-callback-histories-completion.py

Self-review: one small guard-lowering task using shared observer/runtime. No
separate builder check; the supplied standard and native probes define the
data/observation boundary. Existing execution choice and question opt-out apply.
