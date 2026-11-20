# Completion Records Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Complete requested items 14 and 15 without changing executable behavior.
**Architecture:** Reuse Task 137 qualification and existing reference adapter tests; maintain one current remainder tracker.
**Tech Stack:** Existing Markdown, Python unittest and pinned external ANTLR parser.
**Spec:** docs/superpowers/specs/2026-11-20-completion-records-design.md

## Global Constraints
- Parent master 400d2ba; branch codex/completion-records.
- Strict coverage closure is excluded by user instruction; never alter thresholds.
- No product changes, global dependency installation or fabricated completion.
- Keep historical evidence and all remaining user items.

## Review Focus
- Current summaries must not claim physical vendor replay is still unqualified.
- Common-phase multi-TAP/relative-clock support must not be called unimplemented.
- Five reference tests must execute rather than skip.
- Older incomplete acceptance must remain represented in the current tracker.
- All file links and evidence counts must match current tracked files/artifacts.

### Task 1: Reconcile records and execute reference checks
**Files:** README.md; docs/pattern-generation.md; docs/pattern-model-coverage.md; docs/remaining-work.md; historical completion-plan introductions; own design/plan.
**Interfaces:** consumes build/reset-physical-replay-20261120/completion.json and existing tests/tests/test_antlr_diff.py; produces current item/status/evidence mapping.
- [x] Read Task 137 evidence and compare stale summaries; record exact discrepancies.
- [x] Run existing adapter tests using the pinned reference/runtime. Expected: six tests execute, no skips, exit 0.
- [x] Update current summaries, add the fourteen-item tracker and historical-plan pointers. Expected: completed features and pending requirements are distinguished.
- [x] Validate links, evidence values, runtime/product hashes and git diff. Expected: no executable changes or invalid links.
- [ ] Commit, task-done, fresh whole-branch review, necessary fix pass, integrate/push/delete branch.
**Verification:** ICL_REF_DIR=/tmp/icl-reference-completion-20261120 /tmp/icl-reference-runtime-20261120/bin/python tests/tests/test_antlr_diff.py
