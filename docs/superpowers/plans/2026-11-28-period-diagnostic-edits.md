# Period diagnostic spans and edit plan (Task206)

> **For agentic workers:** Use superpowers:executing-plans inline with one fresh independent whole-branch review.

**Goal:** Correct known-unit diagnostic ranges and maintain actual LSP edit coverage.
**Architecture:** Choose a failing operand with the existing unit predicate; reuse the real client for full-sync edits.
**Tech stack:** C++17 and existing Python unittest/LSP client; no dependency.
**Spec:** docs/superpowers/specs/2026-11-28-period-diagnostic-edits-design.md.

## Constraints

- Parent master b0c0b3e01d9370957d5a24e34f57741e8b33abcb; branch codex/period-diagnostic-edits in .worktrees/bsdl-defaults.
- Change only src/pdl/pdl_document.cc, tests/src/test_pdl_tools.py and task docs. Preserve period acceptance, runtime normalization and public APIs.
- Inherit 32 findings, candidate 30; preserve 117 stashes, sibling ledgers, foreign worktrees and excluded item1.
- Rebuild 48 products and qualify the affected frontend/diagnostic scope; do not repeat or claim unrelated full native event matrices.

## Review focus

Literal unit ranges beside known/dynamic values, empty braces, invalid value
ranges, both PDL levels and successive versioned valid/invalid edits must work.
Task1 covers each with literal spans and a controlled stale-edit rejection.

### Task 1: diagnose the correct operand after real edits

**Files:** src/pdl/pdl_document.cc, tests/src/test_pdl_tools.py; docs/remaining-work.md and this spec/plan.
**Interfaces:** Reuse IsTimeUnit, NormalizeClockPeriod and Client.notify/wait. Add only a small shared LanguageServer.change(text, version) wrapper used by the existing matrix.

- [x] Extend known-operand cases with known bad units and value controls. Add literal range assertions; one didOpen then didChange through the matrix, asserting versions. Run against protected parent products: fail on unit ranges, not tooling errors.
- [x] On normalization failure, choose words[4] only for a concrete invalid separate unit, otherwise words[3]. Rebuild frontend; the real matrix passes. Controlled stale edits must pass the old regression and fail the updated one with no tooling errors.
- [x] Run complete normal/static frontend and focused generator diagnostics, default leak-enabled methods, three C++ quality checks, source build/install and exact 4133-input/48-product identity checks. Resolve only the two named findings after verification.
- [x] Mark implementation checks, read the complete selected patch, commit with grouped policy and run task-done using tests/.task206-check.py.

## Branch completion

- [ ] One independent whole-branch review; regrade findings and declined areas, one Critical/Important RED-to-GREEN repair wave if needed, defer Minors without rereview.
- [ ] Merge master, verify affected merged behavior and qualified identities, push, archive only this task's ledger/helpers, retire this branch and continue the original inventory.
