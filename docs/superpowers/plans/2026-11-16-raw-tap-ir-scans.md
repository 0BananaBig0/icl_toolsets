# Raw TAP IR Scans Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Support known-binary root iScan -ir through one bound TAP.

**Architecture:** Qualify existing BSDL/client-TAP binding, reuse ordinary IR Apply
transactionally, require one exact IR CSU, render valid leading-ir raw PDL.

**Tech Stack:** Existing C++17/Tcl/Z3/Python and TapExecution; no dependencies.

**Spec:** docs/superpowers/specs/2026-11-16-raw-tap-ir-scans-design.md

## Global Constraints

Single bound TAP and direct scalar root client-TAP, explicit or inferred unnamed.
Known SI only; all existing global bounds unchanged. No DR/multi-TAP/parallel/
child-TAP/merge extension or new state/controller/clock/reset engine. Preserve
strict default API and phase/output transactionality. New scripts under tests/.
Task107 frozen22 immutable while its gates run; no competing local heavy work.

## Review Focus

- The ordinary backend must emit exactly the raw request's one IR CSU and masks.
- Numeric LSB-first shifts and final-bit exit edge must match independent pins.
- Root target and payload accounting must recognize leading-ir canonical shape.
- Expected comparisons cannot control IR fill or bypass reservation refresh.
- Inferred interfaces must be unambiguous and lookup must avoid per-item model scans.

### Task 108: Qualified Raw IR Transactions

**Files:** src/pattern/pattern_scheduler.cc/.h only if needed; main queue accounting;
maintain tests/src/pattern_scheduler_test.cc and tests/src/pdl_pattern_trace.cc/
test_pdl_pattern_trace.py; new tests/src/pattern/test_raw_tap_ir_scan.py,
benchmark_raw_tap_ir_scan.py, raw_tap_ir fixture and genuine snapshots; guides.

**Interfaces:** Existing NormalizeScan(Operation,bool defer_length=false) recognizes
leading-ir and returns valid leading-ir canonical Operation. Existing ApplyScan(s)
consumes it, using a private RawIrRequest (physical Operation, logical IR accesses,
register ID/width) and a copied scheduler. One-CSU qualification precedes impl
swap/assigned publication. Reuse the constructor's effective interface metadata.
No new public command or source runtime grammar variant.

- [x] Maintained native/CLI RED: bound4bit IR, explicit/inferred target, masks,
      stable, state/owner refresh, overwritten/canceled width, malformed input,
      mixed ordinary queue and scope errors; source/flat/generated/reset replay.
- [x] Implement metadata/normalization, leading-ir queue accounting and one-IR
      transactional ordinary proof with no additional loads/reads/writes.
- [x] Keep apply-group IR/default-chain/numeric trace comparison canonical; add
      independent literal TMS/TDI edges and retained storage/idle/reset tests.
- [x] GREEN focused/connected/style/corpus/offline; matching-option width/model/
      repeat measurements and actual strict vendor evidence, limitations explicit.
- [x] Independent final review, necessary TDD fix pass if needed; immutable
      sanitizer/fullserial/remote all0 then commit/ff master/delete task branch.

Self-review: tests cover all five review-focus classes. First scope deliberately
reuses existing IR planning; failed exact-CSU or resource proof rejects atomically.
Task-start records the Task107 merge base before implementation.

Preflight: existing single-lane ScanStep uses outer input/output/serial fields
and an empty lanes vector; populated lanes represents multiple lanes. Exact IR
proof must require that established representation, not vector size1. Observed
ordinary reference: one instruction step, TDI/TDO, serial1100, expected10xx,
one IR load/write/read. Canonical IR stays leading-ir; no runtime trailing-flag
variant or new public command is added.

Verified Task108: known-binary iScan -ir on a direct scalar root client-TAP bound to one BSDL TAP, explicit or inferred unnamed. Shared numeric/default-chain parsing retains leading-ir canonical PDL; source queue offsets account for IR target/payload. Reuses ordinary IR Apply on a copied scheduler, adopts full state and publishes assigned IR only after exact one-CSU/endpoints/serial/mask/load-write-read proof. Unknown SI, raw DR, multi-TAP/parallel/child-TAP and raw merge stay unsupported. Native literal LSB-first/final-exit/N+6-cycle/all-endpoint/idle/state/transaction checks, CLI7 source/flat/generated/reset/owner/partial/queue/cancel/call/scope groups and trace17 pass. Intended native/CLI RED precedes implementation; separate trace numeric/default-chain RED observed. Initial single-lane preflight assertion and private-field compile issue corrected; no unresolved product failure. Independent final review no Critical/Important/Minor; pending runtime gates declined and verified by executor. Format/tidy0. Full serial make check0 in1435.091s, editor6 in195.861s, corpus2558/zero duplicate files/26 reviewed repeated-module groups; full checkpoint included68 directional certificates. New registry and15 genuine snapshot files then checked freshly:70 offline locally and leak-enabled SAN, oracle/golden/roundtrip harness checks all0. Original22 SAN gates all0; initial additional offline attempt under ptrace failed for LeakSanitizer environment, approved untraced rerun passes. Final remote60 statuses0, actual3903 input hashes match;53 functional/performance checks reused from all0 GREEN56 with3888 exact unchanged runtime/test/benchmark inputs and explicit registry exclusion, new certificates/registry fresh. Remote extra harness checks initially lacked formatter/inventory executables; archived failures retained, built prerequisites and reran affected checks0 without source edits. Genuine Tessent2026.2 BOTH directions run/strict record/check0; all source/job-input/artifact/console hashes verified before importing15 schema3 files. Local and remote32 matched-option scale commands each:30 successes with complete source/flat/generated byte identity and canonical whole-IR ordinary equivalence,2 expected inherited4096 scan-event rejections, no guard increase. Remote4bit .05s,32bit .09-.11s,256bit2.68-2.71s,1024nodes .54s,128repeats3.32s; copy/proof overhead measured, no speedup or1M IR promise. Original frozen17 inputs unchanged throughout full/SAN/remote checks; later changes only documentation and separately verified certificates/registry. Commit34, fast-forward master and delete completed branch, retain recoverable worktree/backups/stashes. No push. Pure inaccessible child, opaque child, raw DR/TAP/parallel/multi-TAP/merge, broader reset/clock/ownership, strict coverage and final release acceptance remain open.
