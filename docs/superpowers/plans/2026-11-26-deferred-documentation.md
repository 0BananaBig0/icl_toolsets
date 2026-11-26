# Deferred documentation corrections implementation plan

> **For agentic workers:** Use superpowers:executing-plans inline.

**Goal:** Correct six deferred documentation/help findings without changing behavior.
**Spec:** docs/superpowers/specs/2026-11-26-deferred-documentation-design.md
**Architecture:** Existing comments, prose and shared benchmark description only.
**Tech Stack:** Markdown, C++ comments, Python docstring, existing native/Python checks.

## Global Constraints

- Start at pushed master5d70298; reuse .worktrees/bsdl-defaults.
- Preserve117stashes, siblings, historical numbers and original broad exclusions.
- No API declarations, scheduling logic, fixtures, matrices or limits change.
- Use source facts and retained rows; do not run earlier completed-task checkers.
- One final reviewer; record all declined areas; no rereview.

## Review Focus

- Current support statements must keep the remaining timed/physical/topology boundaries.
- Parent hardware comparisons and actual observations must not be conflated.
- SO expectations, returned miscompares and sanitizer RSS must keep their distinct meanings.
- C++ preprocessing and Python logic must remain identical after documentation edits.

### Task 1: Correct deferred public guidance

**Files:** src/pattern/pattern_scheduler.h, pattern_readback.h;
tests/src/pattern/benchmark_modeled_child_prefix.py;
docs/pattern-generation.md, pattern-model-coverage.md, remaining-work.md;
this plan and its spec.
**Interfaces:** Existing Apply observation callback, ReplayPrimary route metadata,
MakeModeledReadback borrowed-netlist lifetime and shared benchmark help text.

- [x] Step1: Capture baseline preprocessing, AST, help and historical evidence;
  read source/paragraphs for all six findings. Expected: concrete misleading
  guidance is identified without inventing new unsupported behavior.
- [x] Step2: Correct comments, shared help and current usage/measurement prose.
  Expected: six findings addressed; historical numbers and guards retained.
- [x] Step3: Compare preprocessing/AST, run help and existing focused native/CLI/
  reporting checks; inspect changed ranges and source diff. Expected: equivalent
  executable logic and accurate documented behavior; no redundant new tests.

Implementation verification: python3 /tmp/deferred-documentation-check.py

## Branch review and integration

- [x] Step4: Commit grouped diff and obtain one fresh final review. Grade every
  finding/declined area; repair blockers once and defer new Minors.
  Expected: no unresolved Critical/Important; exact deferred-count reconciliation.
- [ ] Step5: Merge/push, run merged checks, detach reused tree, delete owned
  branch and archive own ledger. Expected: clean pushed master,117stashes,
  31 inherited deferred Minors unless review adds a new one; broader work open.

Completion command: python3 /tmp/deferred-documentation-completion.py
Self-review: one documentation batch; no behavioral task interfaces.
