# Primary callback scaling implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans inline, task-by-task.

**Goal:** Meet existing runtime/memory bounds for larger primary callback inputs.
**Architecture:** Reuse the primary solver and native replay; normalize gated fixed data differences to their two ranks, stream ordered objective bits into bounded 32-bit groups and omit constant chunks, without crossing the drive-count priority boundary.
**Tech Stack:** Existing C++17/Z3/Tcl scheduler and Python/native regression tools.
**Spec:** docs/superpowers/specs/2026-11-28-primary-callback-scaling-design.md

## Global Constraints

- Parent master e2657a6b3537b4216ed53f0c17c102b3ff652700; owned branch codex/primary-callback-scaling in .worktrees/bsdl-defaults.
- Preserve 117 stashes, 45 deferred Minors, foreign worktrees and sibling ledgers; item 1 excluded.
- Retain 256 primary ports/4096 bits, 1048576 queued bits, 262144 frame work, 2s solver timeout and 5s Tcl deadline.
- Keep data-deviation, drive-count and ordered unsigned pin-deviation priorities and exact native replay/publication checks.
- No new dependency, public format, API or eligible topology. New test scripts mirror source under root tests/; archive private nested helpers before packaging.

## Review Focus

- Partial/X goals must retain controlling fill; an explicit write still emits its required event.
- Constant chunks and packing across node/32-bit boundaries must not change unsigned order, storage, reset, routes or ownership.
- Multiframe and read-only cases must keep the old objective ordering and actual read origin.
- Abstract callback intentions must remain unknown actual data, including generated prefixes and once-only status clearing.
- New scale reports must validate actual bytes, identities, sizes and every required process rather than accepting a successful exit alone.

### Task 1: Reduce ordered objective overhead

**Files:** Modify src/pattern/pattern_scheduler.cc::SolvePrimaryWrites, tests/src/pattern/test_automatic_primary_readback.py, tests/src/pattern/benchmark_automatic_primary_readback.py, docs/pattern-generation.md and docs/remaining-work.md.
**Interfaces:** Keep SolvePrimaryWrites and ScanScheduler APIs unchanged. Reuse benchmark model/commands/hardware_output/replay_observations; optionally admit the static binary to the same measured matrix. No producer/consumer interface change.

- [x] Add a maintained 32-lane/8-bit and 1-lane/1024-bit source/flat/generated test using fresh observation queries, exact hardware and once-only callbacks. Run on the protected Task200 parent. Expected: 32-lane behavioral RED; the fresh baseline separately proves the 1024-bit memory RED.
- [x] Discard the measured ineffective pin substitutions. Reuse the scalar objective helper to batch adjacent nonconstant primary objective bits within 32 bits; flush before/after the drive-count objective and normalize gated constant differences to their two attainable ranks before packing; retain full symbolic differences. Expected: final focused GREEN meeting 32-lane runtime and 1024-bit memory bounds, with unchanged parent hardware, source/flat/generated output and limits.
- [x] Extend the existing maintained matrix with 16/32 eight-bit lanes and 512/1024 single-wide cases, normal/static exact replay and real parent default hardware. Keep the old sanitizer cases/configuration. Expected: 300 measured processes with static supplied, all required identities/histories/bounds checked; no parent modeled support claim.
- [x] Run focused normal/static ordinary/readback/callback/feedback controls, native scheduler/readback/state, affected leak-enabled CLI and 15 native sanitizer suites, changed-C++quality, fresh full make check, corpus/editor and source-package build/install. Expected: all final sources/products hashed, actual outcomes and optional reference skips recorded.
- [ ] Document the measured supported scope, stage/read the complete selected raw diff, write grouped commit and run task-done with the artifact checker. Expected: reproducible final qualification and preserved deferred work.

## Branch completion

- [ ] One fresh independent whole-branch review; regrade and fix Critical/Important findings in one RED→GREEN pass and a fresh applicable suite; defer Minors without re-review.
- [ ] Merge into master, verify the actual merged result, push, retire only the owned branch and archive only this plan workspace; continue the original inventory.
