# Current model guide facts implementation plan (Task217)

> **For agentic workers:** Use superpowers:executing-plans inline; one fresh whole-branch review.

**Goal:** Reconcile four local guide/tracker errata with current modeled behavior.
**Architecture:** Surgical prose/table edits, actual existing CLI/corpus facts,
then unchanged-input/product and exact source-package checks.
**Tech Stack:** Existing Python helpers, native CLI, make model-corpus-check/dist.
**Spec:** docs/superpowers/specs/2026-11-28-model-guide-facts-design.md.

## Global constraints

Parentmastered1b3d60d0490bf0469e882ec9f1a472b9a1e48d;branchcodex/model-guide-facts.
Only four Markdown files change.4135inputs/48products/117stashes stay identical.
Private fact script mirrors docs/pattern-generation.md under tests/docs and is
archived/removed before packaging. No new full/native/SAN/source-build claim.

## Review focus

- Walkthrough prose retains exact values and capture/update/status semantics.
- History scope acknowledges bounded support without promising transport.
- Unknown explicit loads versus retained known fill must match actual CLI output.
- Only the explicitly current audit table changes; dated measurements stay dated.
- Four packaged files and retained evidence must bind to final source bytes.

### Task 1: reconcile local model documentation

**Files:** docs/pattern-generation.md;docs/remaining-work.md;spec/plan;
private tests/docs/.task217-facts.py and tests/.task217-* helpers;
build/model-guide-facts-20261128 evidence.
**Interfaces:** Existing source/flat/generated CLI replay;current audit JSON;
Task216qualified4135inputs/48products. Produces prose consistent with these.

- [x] Run actual guide walkthrough and unknown-load/implicit-fill probe;refresh
  make model-corpus-check. Expected notes9/0b00001001/2/6/0,retained second-field
  fill0b10 after explicitX load,1032/44/687 audit outcomes.
- [x] Correct only four errata. Author fact/readability check expected all four
  claims agree with fresh output/source;no behavior change.
- [ ] Archive/remove private nested probe;make dist;verify four exact members,
  4135unchanged inputs/48products and whitespace. Commit selected diff;
  task-done runs python3 tests/.task217-check.py,expected all checks pass.
- [ ] Fresh whole-branch review;regrade/fix blocking findings once,ledger Minors.
- [ ] Merge master,verify merged identities/package/checks,push,archive owned
  workspace/helpers,retire branch and keep reusable detached worktree.
