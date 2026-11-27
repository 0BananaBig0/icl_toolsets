# Clock evidence replay guide implementation plan (Task210)

> **For agentic workers:** Use superpowers:executing-plans inline with one fresh whole-branch reviewer, starting with the isolated reader task.

**Goal:** Date the erratum and give maintainers verified isolated replay steps.
**Architecture:** Preserve canonical drivers; document a hash-checked scratch copy with redirected output and metadata-write tail omitted. Add explicit date fields to matching sidecars after backing up their undated bytes.
**Tech stack:** Existing retained native/trace tools and Python standard library; no producer or dependency change.
**Spec:** docs/superpowers/specs/2026-11-28-clock-evidence-replay-guide-design.md.

## Constraints

- Parent master cb5588f7feb1a1e43fa2c6ab201ce7f9b35a8e65; branch codex/clock-evidence-replay-guide.
- All 4135 qualified inputs and 48 products stay identical. No archived driver or original/corrected native payload changes; preserve undated sidecars.
- Existing 45 rows, nine native inputs, 270 native processes, 16 parser calls and 10s/512MiB caps; zero fresh vendor jobs.
- Keep 117 stashes and foreign/sibling state. Candidate 25 findings after closing two inherited follow-ups.

## Review focus

Can a maintainer start with only the guide and retained workspace? Check required
seed files, source digest and exact derivation, active script placement, success
and failure output, and preservation of canonical evidence. Distinguish original
and derived driver bytes, local correction date and new annotation date, native
replay and archived vendor attestation. Run the reader phase before reading the
implementation notes; use the same single fresh reviewer for technical review.

### Task 1: finish clock evidence handoff

**Files:** docs/clock-evidence-reproduction.md; docs/remaining-work.md; this spec/plan; matching Task209/Task194 correction sidecar metadata and owned build/clock-evidence-replay-guide-20261128 evidence. Private QA scripts stay under tests/ while active.
**Interfaces:** Consume Task209 exact drivers, two qualified manifests, receipt sidecars and original artifact data; produce dated sidecars plus a literal runnable guide and isolated replay receipt. No public producer interface changes.

- [x] Preserve parent identities, canonical drivers/evidence and exact undated sidecars. Add explicit local correction date/timezone while retaining original payload hashes and all other fields. Fact-check the correction date against the recorded Task209 execution date.
- [x] Write the maintainer guide using existing driver logic. Execute its actual code block in a fresh owned scratch directory; require 270 native processes/16 parser calls, matching output bytes and unchanged canonical hashes. Retain the executed derived source, derivation and both source hashes. This documentation-only change requires fact checks, not production TDD.
- [x] Record the author walkthrough separately, preserve all 4135/48 identities, and run source build/install, exact four-member package checks and whitespace. Read the selected patch, commit under grouped policy and run task-done with tests/.task210-check.py.
- [x] One fresh reviewer first receives only guide/reader task, then examines plan/spec/evidence. Regrade findings; one Critical/Important repair wave if needed, no rereview; defer Minors.
- [x] Merge master, verify merged evidence/identities/package, push, archive only own ledger/helpers and retire branch while retaining the detached worktree.

Completed at `8ab14ee`, merged and pushed as `3072118`. One fresh context
completed the isolated reader phase before technical review: 0 Critical,
1 Important and 2 Minor issues. One verified repair wave fixes the Important;
planned integration records close the reader-status Minor; 1 new Minor
remains deferred. Two inherited resolutions; 26 findings remain. Six
qualification gates and three merged-result checks pass. The final guarded
author walkthrough passes; the earlier isolated reader covers the unchanged
native body. No rereview; 117 stashes preserved. Evidence:
`build/clock-evidence-replay-guide-20261128/`.
