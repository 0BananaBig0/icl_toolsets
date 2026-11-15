# Static Child Scans Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Retarget modeled child-interface raw payloads through the qualified static root chain.

**Architecture:** Cache local subpath profiles, retain a per-interface setup queue,
and assemble one full physical root payload from actual controlling values.
Reuse native transactional replay and existing reset/control qualification.

**Tech Stack:** C++17, existing Z3/Tcl frontend, native and Python unittest checks.

**Spec:** docs/superpowers/specs/2026-11-15-static-child-scans-design.md

## Global Constraints

Existing1Mbit/4096register/depth128 limits. No dependencies or native phase changes.
Do not change frozen Task96 Root files. New scripts belong beneath tests/.
Complete required independent review, leak/fullserial and source-matched remote
qualification before commit, fast-forward integration and task branch deletion.

## Review Focus

- Child SI termination must precede parent Source forwarding; validate contiguous root membership.
- Local SI load parity and local SO comparison parity differ; preserve physical root pin behavior.
- Independent target setup must survive same-target overwrite; overlaps must fail explicitly.
- Surrounding values come from controlling fill; capture/readback must not choose scan-in payloads.
- Failed scans and invalid/discarded assignments preserve state and published artifacts.

### Task97: Static child-interface payload insertion

**Files:** Modify src/pattern/pattern_scheduler.{h,cc} and src/pattern/icl_pattern_generation_main.cc.
Maintain tests/src/pattern_scheduler_test.cc and tests/src/pattern/test_modeled_root_scan.py.
Create tests/cases/ours/pattern_generation/static_child_scans/{model.icl,source.pdl,cases.json,README.md}.
Update docs/pattern-generation.md and docs/pattern-model-coverage.md after evidence.

**Interfaces:** NormalizeScan validates logical target/width/data. Existing ApplyScan
continues to work; new ApplyScans(vector<Operation>) returns one normalized root operation.
Generate queues by validated interface name and clears all targets after Apply/Reset.

- [x] Independent design/builder review.
- [x] Add meaningful CLI/native tests and observe intended RED using Task96 binary.
- [x] Implement local profile qualification, parity/position conversion and atomic root replay.
- [x] GREEN focused and connected checks; no deadline or assertion weakening.
- [x] Source-matched vendor comparison and representative scale measurements.
- [x] Independent final review, style/naming/offline and required immutable leak/fullserial/remote checks.
- [x] Commit, fast-forward master, delete completed task branch and preserve subsequent work.

RED2 actual11 hashes match: build0/state0/opaque12=0; native scheduler1 and5 CLI methods1 at the intended root-only scan target guard. Initial RED build2 was a test API arity error and does not count as semantic RED; corrected before production edits.

GREEN actual13 hashes match: build/native state/native scheduler/modeled24/opaque12 all0. Independent final review found no blocking defect or required coverage gap. Subsequent edits add standard iterator include and braces only; final source-matched qualification will include them. Root Task96 snapshot remains frozen and its full serial regression runs separately.

Vendor boundary: Tessent2026.2 rejects genuine internal child.local iScan in this
mode, requiring active internal in-system-test inference; original exit1 is
retained. Generated full-root PDL passes0. A separate hand-calculated full-root
reference (head101 + childA5 + tail10011, masked comparison at offset3) also
passes0 and exports byte-identically to generated child lowering. Actual9
submitted,10original/normalized prepared and4physical-reference input hashes
match. This is a physical reference probe, not a strict child directional cert.

Twenty source-matched scale commands pass with matching replay hashes:
root1M .31-.37s peak106092KiB; child1M .31-.37s peak106076KiB;
1024 queued8-bit children .19-.37s peak99604KiB;97levels .06-.12s peak64064KiB.
Root and whole-chain child canonical hashes match each other. No speedup claim.
Final remote qualification submitted41 hashes; Root Task96 fullserial still runs.

Fullserial first run exit2 in33.067s at corpuscheck: reused Leaf identity duplicated the hierarchical fixture module. Renamed only the new fixture and matching native helper to ChildLeaf; no policy exception. Focused corpus audit now0 (2546files,0duplicate-file groups,26reviewed repeated-module groups). Original immutable snapshot retained; corrected13-file snapshot is /tmp/static-child-task97-reviewed2-files. Required affected checks and fullserial rerun follow.

Verified Task97: required leak-enabled build/native7/main69/modeled24/opaque12/clock51/client25/reset12 all0. Full serial make check passes in 1346.48929021199s. Final remote52 statuses all0 and actual41 source/test/fixture hashes match the corrected immutable snapshot. Local15 connected checks, formatting/naming and53 fresh offline directional goldens pass. Independent review found no blocking defect. Native literal shifts check unequal widths and eight SI/child-SO/root-SO polarity combinations in original/flat, exact comparison offsets, register/pin state and rollback. Twenty scale commands pass with matched hashes: root/child1M .31-.37s/~106092KiB,1024queued children .19-.37s/~99604KiB,97levels .06-.12s/~64064KiB. Tessent rejects original internal-child iScan in this mode (exit1 retained), accepts generated root output0; a separate hand-calculated physical root reference also passes0 with byte-identical exports. Actual9 submitted/10 prepared/4 physical-reference hashes match; no strict child directional certificate claim. Commit13 files, fast-forward master, delete completed branch and preserve the next raw-idle draft. Initial fullserial failed corpuscheck because the new fixture duplicated Leaf; renamed only the new fixture/native helper to ChildLeaf, corpus audit0 and affected leak checks plus fullserial rerun0, no policy exception. General mixed/opaque/mux/TAP/parallel/resource/event/reset/clock/lifecycle, strict coverage and final vendor/platform requirements remain open. No push.
