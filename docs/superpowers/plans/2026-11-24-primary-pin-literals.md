# Literal primary-pin goals implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking.

**Goal:** Meet the existing1,024-bit PIO256MiB target without changing native
events, solver acceptance or actual observed values.

**Architecture:** Specialize only already-forced full binary primary-pin words
at horizon1 inside SolvePrimaryWrites. Keep every existing constraint and
concrete replay; reuse the existing complete merge-readback benchmark.

**Tech Stack:** Existing C++17/Tcl/Z3 4.13.3 and Python unittest; no dependency.

**Spec:** docs/superpowers/specs/2026-11-24-primary-pin-literals-design.md

## Global constraints

- Parent master6bbbae13fc7e719bed135da954d76204668d0e81; owned branch codex/primary-pin-literals.
- No API, option, solver/resource limit, topology, event or reservation change.
- Unknown internal goal bits, unforced pins and horizons>1 stay symbolic.
- Keep10s/512MiB regular and10s/256MiB wide; retain all parent/current misses.
- New tests under tests/src/pattern; reuse existing fixtures and benchmark.
- Native inline implementation, one fresh final review, one blocker fix pass.
- Preserve117 stashes, ignored build products and22 inherited deferred Minors.

## Review focus

- A final pin goal must not force intermediate values at horizons>1.
- Source X/slice writes must retain existing fill/unknown semantics.
- Pins needed only for read guards must remain solver-controlled.
- Literal words must preserve implicit data/reset and reservation events.
- Memory evidence must distinguish missed parent controls from current closure.

### Task 1: Qualify forced primary-pin literals

**Files:** Modify src/pattern/pattern_scheduler.cc inside SolvePrimaryWrites and
Makefile's existing generator-test macro. Create
tests/src/pattern/test_primary_pin_literals.py. Update the current generator,
coverage and remaining-work evidence docs. Reuse
tests/src/pattern/benchmark_merged_readback.py without changing its fixtures,
counter fields, inherited Minors or bounds.

**Interfaces:** Preserve every signature. Let goal=goals.find(id). Substitute
SymbolicValue::Literal(context_, goal->second).bits only when horizon==1,
goal!=goals.end() and goal->second.find_first_not_of("01")==std::string::npos.
Retain the drive Boolean, known mask, Choose, all constraints/objectives and
replay/commit. Otherwise construct the original bv_const.

- [ ] Step1: Add8 CLI methods using existing modeled-readback replay helpers:
  wide memory (actual1,024-bit alternating word,512 failing comparisons,
  peak RSS<=262144KiB and<=10s); zero/matching known goals; ascending/sliced
  pins and inverted observations (write upper2bits2 => q8/inverse7); unknown
  read then masked X fill; unforced output-enable control; protected other
  pin and iApplyEndState restoration; DataRegister9 with final source pin6
  requiring intermediate source9; and impossible tied-register/source goals
  preserving prior artifacts. All non-memory cases check original/flat/generated
  agreement and actual values/events. The large normal/static memory acceptance
  check is explicitly unrun under ASAN; existing16-call sanitizer measurements
  and small functional cases cover instrumentation.
  Method names: test_wide_memory_goal, test_zero_and_matching_goals,
  test_ascending_slices_and_inversion, test_unknown_and_partial_x_history,
  test_unforced_read_enable, test_reservations_and_end_state,
  test_intermediate_pin_value_in_later_horizon, and
  test_impossible_goals_preserve_artifacts.
- [ ] Step2: Run new tests on immutable parent. Expected: the wide memory case
  fails specifically on peak RSS>262144KiB while compatible functional controls
  pass. Retain raw timing/output; correct fixture errors before product edits.
- [ ] Step3: Implement the single guarded literal substitution. Expected: only
  the private pin-word construction changes; no repeated scan or new cache.
- [ ] Step4: Build and run the focused suite. Expected:8/8 normal/static and
  small sanitizer functional cases pass with unchanged event/observation bytes.
  If memory does not improve below256MiB, stop and return to diagnosis.
- [ ] Step5: Run the existing complete378-process benchmark with immutable
  parent, current and sanitizer binaries, three forms/repeats and512/1024-bit
  high-bit data. Expected: all actual observations/replays match controls;
  all current cases meet the existing bounds. Retain the parent's9 wide-PIO
  memory misses and the benchmark's resulting nonzero status; do not erase or
  raise those bounds. Record environment, hashes, runtime and RSS.
- [ ] Step6: Run fresh15 normal/15 leak-enabled sanitizer native suites, full
  normal/static generator checks, affected sanitizer CLI, corpus, C++ quality,
  source packaging and allocation/reporting controls. Expected: all required
  functional gates pass with exact qualified input/product hashes.
- [ ] Step7: Update measured docs and commit, request one fresh whole-branch
  review, re-grade findings and fix blockers in one test-first pass. Expected:
  no unresolved Critical/Important findings; ledger Minors and declined costs.
- [ ] Step8: Merge into master, verify identical qualified sources/products and
  affected behavior, push, delete only the owned merged branch and archive only
  this plan's ledger. Expected: clean pushed master and117 stashes retained.

Focused command: python3 tests/src/pattern/test_primary_pin_literals.py build/icl-pattern-generation
Whole-task completion: python3 /tmp/primary-pin-literals-completion.py

Self-review: one private optimization task carries both memory and event/replay
proof. Existing solver guards and both Runtime/generation callers stay intact.
No separate builder check is required for this small design. User delegation
and the already selected native execution method remain in force.
