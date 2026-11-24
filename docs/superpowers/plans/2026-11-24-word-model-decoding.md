# Whole-word model decoding implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking.

**Goal:** Reduce repeated per-bit Z3 model evaluation while preserving schedules.

**Architecture:** Keep private Decode and all callers. Convert model-evaluated
whole values and known masks using installed Z3, pad to width and overlay x.

**Tech Stack:** Existing C++17, Z3 4.13.3, Tcl and Python unittest; no dependency.

**Spec:** docs/superpowers/specs/2026-11-24-word-model-decoding-design.md

## Global constraints

- Preserve solver constraints, objectives, events, state coverage and limits.
- Preserve MSB-first declared width, leading zeros and unknown masks.
- Keep all117 stashes and unrelated worktrees/outputs; parent master92e6295.
- New scripts belong in tests/src/pattern; reuse existing fixture/replay helpers.
- One fresh final review and one blocker fix pass; merge/push/clean authorized.

## Review focus

- Z3 binary numerals omit leading zeros, especially zero and sparse high bits.
- Unknown masks and nonzero hidden data must preserve x at every unknown bit.
- Values crossing32/64-bit boundaries must never pass through machine integers.
- Serial/TAP/parallel replay shares Decode and must retain full native checks.
- Performance comparisons must use equivalent inputs, outputs and build modes.

### Task 1: Optimize and qualify shared model decoding

**Files:** Modify src/pattern/pattern_scheduler.cc, Makefile and current evidence
docs; create tests/src/pattern/test_word_model_decoding.py and
tests/src/pattern/benchmark_word_model_decoding.py. Maintain existing native tests
only if public scheduling checks need additional coverage.

Measured reporting gap: also create tests/src/pattern/test_benchmark_word_model_decoding.py
to run a real small generator matrix with a controlled elapsed clock. The test
must retain all eight rows and still report failure after a timing-bound miss.
It passes separately on normal/static/sanitizer products; it is not added to the
unchanged default generator-suite count.

**Interfaces:** Consume private Decode(model, SymbolicValue), returning string;
preserve that signature and every caller. Reuse benchmark_downstream_write_callbacks
fixture and benchmark_merged_callbacks.checked_frames for exact workload checks.

- [x] Step1: Add width/mask/serial replay correctness cases and benchmark with
  three parent/current64/128/256 runs, smaller sanitizer and wide512/1024 runs.
- [x] Step2: Run parent performance assertion; expect256 to exceed10s. Run
  correctness cases on parent to establish unchanged behavior before editing.
- [x] Step3: Decode whole known/value expressions once; skip literal model
  evaluation and value evaluation for wholly unknown words, pad and overlay x.
- [x] Step4: Run focused normal/static/sanitizer tests and measured benchmark;
  expect exact outputs, physical events and bounds specified by the design.
- [x] Step5: Run full required regression/corpus/quality/package checks, record
  reproducible evidence and current documentation, then commit selected changes.
- [x] Step6: Obtain one fresh whole-branch review; ledger/defer Minors and fix
  blockers in one test-first pass, rerunning affected required checks.
- [x] Step7: Merge into master, verify merged inputs/products and checks, push,
  safely delete task branch, archive its ledger and continue pending inventory.

Whole-task completion command: python3 /tmp/word-model-decoding-completion.py
Expected: exact qualified source/product hashes, successful required checks,
merged/pushed clean master, only owned branch removed and117 stashes retained.

Self-review: one shared interface and one task; no cross-task dependencies.
Prior full qualification remains valid only for unchanged sources/products.

Final measurement ruling: all 72 outputs match, but four 256-target source/flat
runs exceed the unchanged 10-second bound. Record the 36.8% median improvement
and retained failures; do not claim 256-target latency closure. The smaller
sanitizer matrix uses 16 targets after 64 exceeded 512MiB. Final 68 gates include
three standalone reporting regressions and fresh packaging, with exact unchanged
4,042 shared inputs/38 products and executed benchmark/new-test hashes.
