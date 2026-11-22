# Bound TAP Modeled Readback Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking.

**Goal:** Supply actual bound-TAP IR/DR observations and count modeled mismatches during normal and physical-PDL replay.
**Architecture:** Reuse ReplayTapStep's pre-edge observation and existing lane decoder. Native single-TAP replay owns its TDO stream, decodes requested registers once after acceptance, and delivers outside fallback catches. Runtime and emitted generation share observation-accepting scheduling.
**Tech Stack:** Existing C++17/Z3/Tcl, Python unittest, no dependencies.
**Spec:** docs/superpowers/specs/2026-11-22-modeled-tap-readback-design.md

## Global Constraints
Default generation unchanged. Existing topology, reset, physical-shape, binary-load and event-work guards remain.
Raw/interface/chain histories, independent TAP phases, timed histories, callbacks and device transport remain open.
No observation from expected values. First requested observation per Apply; tentative delivery until successful Apply.
New maintained tests under tests/src/pattern/. Preserve117 stashes and unrelated worktrees/ledgers. Coverage1 excluded.

## Review Focus
- Native mismatch handling must not choose a different schedule between Runtime and output generation.
- Physical replay must use the same reset/exact-CSU mode in the model adapter and output generator.
- Implicit IR setup must not inflate explicit read history or failure counts.
- Speculative native/recursive paths must own observations and never retry after subscriber errors.
- Synthetic bypass/IR returns and unequal multiple-TAP lanes must use actual pre-edge TDO semantics.

### Task 1: Complete bound-TAP observation paths
**Files:** Modify src/pattern/pattern_scheduler.{cc,h}, pattern_readback.{cc,h}, icl_pattern_generation_main.cc, Makefile; tests/src/pattern_readback_test.cc, tests/src/pattern/test_modeled_scan_readback.py; create tests/src/pattern/test_modeled_tap_readback.py.
**Interfaces:** Observation keeps existing vector<BitReference>,string signature. Add MakeModeledReadback(const Netlist&,bool physical), retain one-argument factory. Internal Generate gains trailing bool modeled_readback=false and GenerateMerged receives/forwards it.
- [ ] Write literal-result tests: IR captures01/X metadata, DR stored9 versus0 counts2, old9 before new6, sparse aliases, DataRegister captures, unknown masks, reset/reservations, implicit setup, multiple unequal common-phase TAPs, physical exact-CSU and wrong expected data, default diagnostics, no-query and failure publication. Extend native event/TDO oracle and subscriber-exception checks.
- [ ] Run new tests against parent c6cea04 generator/native source. Expected: bound-TAP guard or absent callback failures, retained as RED.
- [ ] Lift General Replay's interval-sampling loop into a local callback taking tick and const NetworkState&, invoke it in both standalone shift loop and ReplayTapStep; emit mapped observations for either route. Verify output references agree with actual TAP returns, including supported synthetic selections.
- [ ] Pass Observation through every TryNativeTapScan caller. Buffer actual pre-edge ReadTdo stream only in observer mode; preserve known-expected verification without observer. Decode each expected register interval from return order/suffix into declaration order after accepted state checks. Deliver owned vectors outside fallback catch. No full-path scans per tick.
- [ ] Add physical-mode factory overload and ModelReadback mode state; forward physical to Reset/Apply. Admit bound TAPs while retaining storage/operation guards.
- [ ] Forward modeled mode through all Generate/GenerateMerged/reset-verification/recursive calls; output scheduling uses non-storing observer in modeled mode, matching Runtime scheduling. Preserve default physical diagnostics; modeled mismatch data counts rather than causing a verification fallback.
- [ ] Run focused new/existing TAP, standalone, PIO/data and native observer tests with dynamic/static generators. Expected: all pass, default controls unchanged and original/flat/generated output stable.
- [ ] Commit implementation/tests.

### Task 2: Review, full qualification and integration
**Files:** Update README.md, docs/pattern-generation.md, pattern-model-coverage.md, remaining-work.md.
**Interfaces:** Consumes Task1 behavior; publishes runnable model example and bounded current evidence.
- [ ] Run example and equivalent parent/current single/multiple TAP scale controls with larger supported widths; record command/input/time/RSS/hash evidence.
- [ ] Run required full native/sanitizer, dynamic/static generator, affected format/tidy/strict C++17 and source-packaging checks. Expected: zero failures, no unsupported suite represented as run.
- [ ] Obtain one fresh independent whole-branch review; fix Important/Critical findings in one test-first verified pass, no rereview; ledger all dispositions.
- [ ] Update current docs/inventory and record actual counts/limits; commit.
- [ ] Merge into master, verify source/product hashes and integrated focused tests, push, detach reused worktree and delete merged task branch; archive only own ledger.
