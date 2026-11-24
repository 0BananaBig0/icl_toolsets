# Connected callback latency implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking.

**Goal:** Reduce repeated primary model decoding and qualify256-target latency.

**Architecture:** Cache decoded word/mask pairs only inside one successful
primary solver model; keep solver construction and concrete replay intact.

**Tech Stack:** Existing C++17/std::map/Z3, GNU linker wrapping and Python.

**Spec:** docs/superpowers/specs/2026-11-24-connected-callback-latency-design.md

## Global constraints

- Parent master c56b8ea97eb7969150f7e898d427dbd9fa047e77; owned child codex/connected-callback-latency.
- Keep10s/512MiB regular and10s/256MiB wide, all actual misses and complete output checks.
- No objective, constraint, model, topology, API, dependency or solver limit changes.
- Cache keys live only with their model and retained frame expressions.
- Preserve117 stashes, ignored files and23 inherited deferred Minors.
- Native inline execution; one fresh final reviewer and one blocker fix pass.

## Review focus

- Cache entries must never survive a different model or reuse a freed AST ID.
- Mixed unknown masks and leading zeroes must keep exact widths and polarity.
- Cached results must not skip concrete state/fill comparisons or data events.
- Repeated callback postconditions and reservations must keep their frame order.
- Memory gains/costs and noisy parent timing must be reported honestly.

### Task 1: Qualify model-local primary decode reuse

**Files:** Modify src/pattern/pattern_scheduler.cc only in SolvePrimaryWrites;
create tests/src/pattern/benchmark_primary_decode_calls.py. Reuse existing
test_word_model_decoding.py, test_primary_pin_literals.py and both benchmarks.
Update docs/pattern-generation.md, docs/pattern-model-coverage.md and docs/remaining-work.md.

**Interfaces:** Keep Decode and all public signatures unchanged. Add a local
map<pair<unsigned,unsigned>,string> and lambda after primary solver.get_model;
replace only three primary Decode(model,value) calls with decode(value).

- [x] Step1: Add a real Z3_model_eval counter benchmark using linker wrapping,
  fixture(256,'automatic'), complete checked_frames output and <=600000 calls.
  Expected: parent fails only the work bound; native baseline timings remain
  archived even if all current12 controls happen to pass10s today.
- [x] Step2: Add the per-model map/lambda at the three primary replay sites.
  Expected: counter passes without changing any output, solver or validation.
- [x] Step3: Run word-model and primary-pin modules on normal/static/SAN, with
  the existing explicit wide-SAN skip; run the counter and inspect real count.
  Expected:3+10 functional methods per normal/static,3+9SAN and one wide skip.
- [x] Step4: Run complete matched72-word and378-merge process matrices using
  immutable parent/current/SAN products. Expected: all actual values, full
  outputs, Apply order and observations agree and all current bounds pass.
- [x] Step5: Run fresh15 normal/15 leak-enabled SAN native suites, full78-suite
  normal/static generator runs, affected CLI/corpus, three C++ quality checks,
  source packaging and allocation/reporting checks. Expected: exact qualified
  source/product hashes and all functional gates pass.
- [x] Step6: Document measurements and commit; request one fresh final review,
  re-grade findings, fix Critical/Important once test-first and defer Minors.
  Expected: no unresolved blocker; all declined judgments ruled with costs.
- [ ] Step7: Merge master, check identical inputs/products and affected behavior,
  push, delete owned branch and archive only this ledger. Expected: clean
  pushed master with117 stashes and no owned completed branch remaining.

Counter: Z3_PREFIX=<pinned-prefix> TCL_STATIC_LIB=<pinned-library> python3 tests/src/pattern/benchmark_primary_decode_calls.py
Completion: python3 /tmp/connected-callback-latency-completion.py

Self-review: one private optimization, existing fixtures/oracles and direct
model-scoped lifetime. No separate builder check is required for this small
design. The local cache adds bounded per-model storage, measured by the full
matrix; do not trade away replay or claimed scale to make fewer lines.
