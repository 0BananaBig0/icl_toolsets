# Readback diagnostic ancestry implementation plan (Task215)

> **For agentic workers:** Use superpowers:executing-plans inline with one fresh whole-branch review.

**Goal:** Distinguish recorded operation frames from live getter unwinding.
**Architecture:** Label/indent the recorded frames inside the existing catch; preserve live Runtime formatting and all locations, including recursion.
**Tech Stack:** Existing C++17 provider, native assertions and Python CLI tests.
**Spec:** docs/superpowers/specs/2026-11-28-readback-diagnostic-ancestry-design.md.

## Global constraints

Parentmasterb002ea8ae51d3310d46c2ccdaa9e77e3d12ba09f; branchcodex/readback-diagnostic-ancestry.
Only recorded-frame rendering changes. Preserve empty-stack message, cause,
origin, callback prefix, caching/type and all execution/validation logic. No
Runtime/Makefile/API/dependency changes. Preserve117 stashes and existing limits.
Retain full214 evidence only for byte-proven unchanged behavior, never as a new
full run. Rehash4135 inputs/48 rebuilt products and check six package members.

## Review focus

- Recorded frames must end by indentation before live unwinding starts.
- Identical recursive frames are meaningful; retain every occurrence in both roles.
- All three getters, direct/merged nesting and finished/active calls must preserve locations.
- Empty stacks and cached error rethrows must remain byte-exact and stable.
- Source-fragment identity and fresh affected checks must support the retained-evidence scope.

### Task 1: distinguish diagnostic frame roles

**Files:** src/pattern/pattern_readback.cc; tests/src/pattern_readback_test.cc;
tests/src/pattern/test_opaque_root_serial_merges.py; tracker/spec/plan; private
tests/.task215-* QA helpers and build/readback-diagnostic-ancestry-20261128/.
**Interfaces:** Existing stored Operation.call_stack and live InvokeProcedure
unwinding; no protocol or public type changes.

- [x] Protect parent source/manifests. Update native expected role blocks and
  repeated-frame control; update completed-call CLI and add active direct/merged
  getters at depths0/2. Build/run old producer: native RED and19CLI methods with
  missing-role assertion failures, no setup errors. Preserve actual diagnostics.
- [x] Add recorded-block header only for nonempty stacks and four-space frame
  indentation. Native and whole CLI GREEN; exact empty/cached error controls and
  actual recursive frame multiplicities/locations/artifacts pass.
- [ ] Prove fragment identity, run normal/SAN native readback/runtime/opaque6,
  nine CLI modules in3variants27reports, six C++ quality checks, source build/
  install, six exact package members, final4135/48 identities and whitespace.
  Read selected diff, commit, then task-done with python3 tests/.task215-check.py;
  expected all scoped gates pass with214 broad evidence explicitly retained.
- [ ] One fresh whole-branch review; regrade, one blocking repair wave if needed,
  no rereview; ledger Minors and updated inherited count.
- [ ] Merge master, verify merged products/source/tests, push, archive owned
  workspace/helpers, retire branch and preserve reusable detached worktree.
