# Clock evidence receipt implementation plan (Task209)

> **For agentic workers:** Use superpowers:executing-plans inline, with one fresh independent whole-branch review.

**Goal:** Correct clock evidence scope and retain reproducible current drivers.
**Architecture:** Reuse literal retained inputs and five replay forms; bind fresh receipts to actual driver bytes and qualified producer hashes. Preserve original evidence and make historical/new claims explicit.
**Tech stack:** Existing native generator/trace tools, Python standard library, Make source packaging; no dependency or producer change.
**Spec:** docs/superpowers/specs/2026-11-28-clock-evidence-receipts-design.md.

## Constraints

- Parent master aeaf94fbf3115ae28655fc56648cb2bab87dc13f; branch codex/clock-evidence-receipts in .worktrees/bsdl-defaults.
- Keep all 4135 source/test inputs and 48 products identical; preserve 117 stashes and unrelated worktree state.
- Old driver bytes are unrecovered. Keep all old Task196 measurements and vendor records unchanged; preserve original Task194 native receipt before correcting only its vendor-scope field.
- Three repeats/all 45 original rows, five forms, exact old bytes,10s/512MiB; all 9 prepared native inputs (one native-only control) and 16 archived jobs. No fresh vendor/hardware or speedup claim.

## Review focus

Require actual process exits and output bytes, not a receipt merely restating
old booleans. Check every archived input/log/export and parsed trace, configured
clocks and actual 2026.2 version. Distinguish the retained historical parent from
the current producer. Copies must match the exact executed drivers. Preserve
the pre-correction receipt and explain missing original driver bytes.

### Task 1: repair clock evidence provenance

**Files:** docs/remaining-work.md and this spec/plan; owned build/clock-evidence-receipts-20261128 evidence; the single scope field in build/clock-period-serialization-20261127/qualification/receipt.json plus its correction sidecar. Private QA scripts under tests/ are archived after completion.
**Interfaces:** Consume original measure/results.json, content-addressed fixtures, native producer directories, vendor bundle/results/comparison records and Task208 input/product manifests. Produce retained measurement/attestation drivers, new exact receipts and correction provenance; no production interface changes.

- [x] Protect original receipt, parent 4135/48 identities and historical artifact hashes. Record the actual two evidence gaps; this documentation/artifact correction adds no production behavior requiring a TDD implementation cycle.
- [x] Implement the bounded private evidence driver using the existing five forms. Run 45 measurement rows/225 CLI processes and 9 prepared native cases/45 CLI processes; verify literal historical inputs, outputs and unchanged 10s/512MiB caps. Parse and compare all 16 archived exports, verifying their job inputs/logs/status/configuration/version. Save exact executed driver copies beside both fresh receipts.
- [x] Correct only the Task194 native receipt scope field after preserving original bytes; keep a before/after hash sidecar. Verify old Task196 evidence unchanged and current 4135/48 identities. Record results in tracker; run source build/install, exact three-member package checks and whitespace checks.
- [ ] Read the complete selected patch, commit under grouped policy and run task-done with tests/.task209-check.py. Expected: all artifact/identity/package checks exit0; candidate 25 remaining.
- [ ] One independent whole-branch review, one verified Critical/Important repair wave if needed, no rereview; defer Minors.
- [ ] Merge master, verify merged identities/evidence and package members, push, archive only own ledger/helpers, detach retained worktree and retire branch.
