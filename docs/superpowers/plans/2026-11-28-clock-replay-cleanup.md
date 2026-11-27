# Clock replay cleanup implementation plan (Task211)

> **For agentic workers:** Use superpowers:executing-plans inline with one fresh whole-branch final review.

**Goal:** Remove the owned active script on wrapper exception paths.
**Architecture:** Preserve existing file creation and native replay; wrap write/copy/run in try/finally, unlinking only the owned active path. Retain scratch diagnostics and propagate the exception.
**Tech stack:** Existing Python standard library, published shell block and retained native artifacts; no dependency or producer change.
**Spec:** docs/superpowers/specs/2026-11-28-clock-replay-cleanup-design.md.

## Constraints

- Parent master d27afc7a5881ecb2aa25d6a64d7afc644e08e298; branch codex/clock-replay-cleanup.
- Keep 4135 qualified inputs and 48 products identical, assertion preflight, five replay forms and 10s/512MiB limits.
- Private mirrored tests under tests/docs/ are archived before source packaging. Preserve 117 stashes and all unrelated state.
- One inherited finding is a candidate closure: 26 minus 1 = 25. Broader inventory and excluded item1 remain open.

## Review focus

Check real file removal on both scratch-copy failure and child-wait interruption,
retained diagnostics and exception propagation. Preserve the optimization guard,
exact original/derived driver relationship, commands, native outputs and
canonical evidence. No broad file cleanup or archived-driver changes.

### Task 1: complete wrapper resource cleanup

**Files:** docs/clock-evidence-reproduction.md; docs/remaining-work.md; this spec/plan; private tests/docs/.task211-* probes and tests/.task211-* QA helpers; owned build/clock-replay-cleanup-20261128 evidence.
**Interfaces:** Execute the published Python block using actual owned files; inject only boundary exceptions. Consume retained Task209 drivers/manifests, Task210 optimization probe and parent qualification manifests; produce final resource cleanup and exact verification receipts.

- [x] Protect parent identities and guide bytes. Write mirrored lifecycle probes, run old command and observe both real active files left behind. Archive leaked source and clean only each probe's owned path. Expected: two assertion failures, no test setup errors.
- [x] Add the smallest try/finally change. Both lifecycle probes and both optimization preflight checks pass. Run the full published normal command and optimized rejection; preserve exact native outputs and canonical hashes. Archive mirrored test sources before packaging.
- [x] Record verified scope and run source build/install, exact four-member package checks, identities and whitespace. Read selected diff, commit under grouped policy and run task-done with tests/.task211-check.py.
- [x] One fresh final review; regrade findings, one Critical/Important repair wave if needed, no rereview; defer Minors.
- [x] Merge master, verify merged artifacts/identities, push, archive only own ledger/helpers and retire the branch while retaining the detached worktree.

Completed at `c1f2cd9`, merged and pushed as `23c65a2`. Independent review:
0 Critical, 0 Important, 1 new Minor issues. One inherited resolution and
26 remaining findings. Seven qualification gates and three merged-result
checks pass; 117 stashes preserved. Evidence:
`build/clock-replay-cleanup-20261128/`.
