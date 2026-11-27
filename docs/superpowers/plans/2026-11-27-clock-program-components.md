# Independent Clock Programs Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Accept independent ordinary clock paths in period-bearing programs.

**Architecture:** Reuse the sparse per-bit clock fanout graph to identify only
newly supported components for ordinary lowering, retaining existing rendering
elsewhere and copying the prepared policy with existing clock state.

**Tech Stack:** Existing C++17, Z3, Tcl and Python unittest; no new dependencies.

**Spec:** docs/superpowers/specs/2026-11-27-clock-program-components-design.md

## Global Constraints

Preserve all previously accepted output bytes, checked versions, per-bit identity,
existing validation timing, resource/hardware guards and immutable procedures.
Future source edges affect analysis only, never live resource/fanout state.
Keep sparse O((V+E+P)log V) analysis and unchanged10s/512MiB measured bounds.
No grammar, time/event engine, physical timing or new dependency changes.
Preserve117 stashes and41 inherited Minors; item1 remains excluded.

## Review Focus

- Previously accepted complete and representable partial period programs retain exact output spelling, order and logical waits.
- Connections in either direction, including future source changes, prevent lowering any period-connected candidate.
- Different vector bits and segmented or associated-flat names retain physical bit identity through policy preparation and copies.
- Preparing policy does not publish definitions, add future live resource edges or change invalid-program diagnostic ordering.
- Large declarations stay sparse; reset previews reuse prepared policy and bounded vendor references cannot certify raw omissions or timing.

### Task 1: lower independent unsupported clock components

**Files:** Modify src/pattern/pattern_clock.h, src/pattern/pattern_clock.cc,
src/pattern/icl_pattern_generation_main.cc, tests/src/pattern_scheduler_test.cc
and tests/src/pattern/test_clock_period_overrides.py,
tests/src/test_icl_pattern_generation_main.py;
update docs/pattern-generation.md and docs/remaining-work.md.

**Interfaces:** Add SystemClocks::RetainClockPrograms(const vector<Operation>&)
and bool RetainsClockProgram(const string&) const. Consume existing private Bit,
Target/ClockTargetName/Name, BuildSafety and fanout_. Produce a copied set of only
the newly supported independent bits to lower; empty means retain old rendering.
Generate's existing bool retain_clocks still gates the policy and passes through
recursive reset previews; real operations keep existing validation.

- [x] Add five-form mux/alias/inverted independent-clock regressions with exact
  root-relative waits, vector/procedure/reset cases, future bridge and connected
  guards, diagnostic/sentinel preservation, and a nine-point scale method.
  Keep the existing already period-connected unrepresentable-source negative
  fixtures and all accepted-output assertions unchanged.
  Run new positives on the protected parent. Expected: retained-source failure.
- [x] Extend Generator.test_help_version with the independent root-relative
  lowering description. Run on the protected parent. Expected: missing phrase.
- [x] Add native policy/snapshot tests with exact per-bit decisions, old/new root
  counts, copy independence, future resource-edge absence and sparse wide roots.
  Build/run --clock-snapshots before production edits. Expected: new API missing.
- [x] Implement temporary graph analysis and per-target rendering, reusing live
  fanout and name helpers while keeping future edges local. Expected: native and
  normal/static period modules pass, with old accepted output unchanged.
- [x] Run focused legacy/serialization/oscillator/frontend/readback/reset checks.
  Expected: all pass and all five replay forms agree.
- [x] Measure27 new matrix points and18 matched parent/current controls with
  actual artifact capture and complete coordinate checks; run bounded configured
  explicit-reference vendor pairs. Expected: all10s/512MiB bounds and traces pass.
- [x] Freeze final inputs; run full project, affected leak-enabled sanitizers,
  changed-C++ checks, corpus, editor and source packaging. Expected: all exit0;
  record actual counts/skips and source/product identities.
- [x] Update evidence documentation; read the complete final selected raw diff,
  commit grouped messages and run task-done with the final evidence checker.
  Expected: all source/product, actual control files, matched pairs and matrix
  coordinates agree before the fresh independent review.

## Branch completion

- [x] One independent whole-branch review; grade all findings/declined areas,
  one Critical/Important test-first repair pass if needed; defer Minors.
- [ ] Merge into parent master, verify merged inputs/products and focused checks,
  push, detach the retained worktree, safely delete only this owned branch and
  archive only this plan's workspace. Expected: clean aligned master/origin and
  unchanged117 stashes; continue the remaining inventory.
