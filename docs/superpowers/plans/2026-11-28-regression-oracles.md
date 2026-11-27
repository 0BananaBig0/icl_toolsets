# Regression oracle maintenance plan (Task203)

> **For agentic workers:** Use superpowers:executing-plans inline, with one fresh independent whole-branch review.

**Goal:** Strengthen three recorded regression oracles and verify one already-completed guard correction.
**Spec:** docs/superpowers/specs/2026-11-28-regression-oracles-design.md.
**Architecture:** Reuse existing native/CLI tests, helper APIs and build objects. Controlled faults remain temporary; production behavior and generator binaries are unchanged.
**Tech stack:** Existing C++17/Z3/Tcl/Python and native test tools; no dependency changes.

## Constraints

- Parent master b8ad1ae3a34a0bbbeb06bd50f2b292c6aadb69ee; branch codex/regression-oracles in .worktrees/bsdl-defaults.
- Preserve 117 stashes, foreign worktrees, sibling ledgers and excluded item1. Inherit 38 Minors; candidate remainder 34.
- Change only the three named maintained test files plus task documentation. No production or public interface changes.
- Keep existing scan/callback actual-data, unknown-return, ownership, rollback and timing contracts.
- Place private QA scripts under project-root/tests; archive them before packaging if nested. Do not package fault-injection programs.

### Task 1: qualify the recorded test contracts

**Files:** tests/src/pattern_readback_test.cc, tests/src/pattern/pattern_opaque_test.cc, tests/src/pattern/test_fixed_scan_fanout.py; docs/remaining-work.md and this task's spec/plan.
**Interfaces:** Keep all APIs and test helpers. Readback clear must share state with preview; private payloads preserve byte order; first-group writes retain every requested target/value. Current shared-input guards remain unchanged.

- [x] Record all four finding identities and verify current source/history. Run protected current native controls and controlled old-oracle fault cases before maintained edits.
- [x] Add the shared-clear check, asymmetric private payloads and complete first-group fanout write assertions. Require actual production GREEN and fault-injected oracle rejection; distinguish these controls from a production bug reproduction.
- [x] Verify the existing shared-input test rejects unrelated diagnostics and passes against actual normal/static generators; record the Task192 historical fix without a redundant edit.
- [x] Run affected full native/normal/static suites, default leak-enabled native checks, changed-test C++ quality and source distribution build/install. Bind 4133 input paths and 44 products; require 4130 unchanged inputs and 40 unchanged products, including every generator.
- [x] Record actual qualification and the four dispositions, stage/read the complete raw patch, commit with grouped messages and run task-done with the artifact checker.

## Branch completion

- [ ] One independent whole-branch review; regrade findings and declined areas, fix Critical/Important issues in one verified repair wave, defer Minors without rereview.
- [ ] Merge into master, verify the actual merged result, push, retire only this branch and archive only this plan workspace. Complete the checkboxes with verified progress and continue the original inventory.
