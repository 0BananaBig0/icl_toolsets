# Standalone opaque-root serial merges implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking.

**Goal:** Execute standalone opaque-root merge calls while retaining explicit-only histories.
**Architecture:** Existing serial merge traversal precedes opaque queue lowering;
implicit completions use existing canonical omission intent and actual observations.
**Tech Stack:** C++17, Tcl, Python unittest, existing Z3 and sanitizer products.
**Spec:** docs/superpowers/specs/2026-11-26-opaque-root-serial-merges-design.md

## Global Constraints

- Original items2–15 except1; parentmaster1296385 and117preserved stashes.
- Reuse isolated .worktrees/bsdl-defaults; no dependency or invented observation.
- Preserve C++17, public signatures, note syntax and1048576-bit aggregate limit.
- Serialize eligible roots; physical/event/unsupported topology limits remain.
- Measurements10s/512MiB and256MiB single-wide controls; preserve34Minors.
- One inline implementer, one fresh final review and one TDD blocker repair wave.

## Review Focus

- Implicit default/stable lane fills must not invent or overwrite explicit histories.
- Valid direct omission notes must execute before hardware; invalid notes stay informational.
- Nested/scoped calls and local-reset verification must retain source context.
- Width changes, canceled queues and unknown status must not weaken completion rules.
- Unsupported modes and later failed calls must preserve existing output artifacts.

### Task 1: Opaque-root serial merge execution and history scope

**Files:** Modify src/pattern/pattern_opaque.cc, pattern_readback.cc and
pattern_runtime.cc; extend tests/src/pattern/pattern_opaque_test.cc and
tests/src/pattern_readback_test.cc as needed. Update obsolete root merge rejection
cases, Makefile and the three current guide/inventory documents. Create
tests/src/pattern/test_opaque_root_serial_merges.py, a measured benchmark and its
mirrored reporter under tests/tests/src/pattern.
**Interfaces:** Preserve LowerOpaqueScans, OpaqueScanQueue, GenerateMerged and
MakeModeledReadback signatures. Use MergeOperations(...,false) only after existing
root eligibility checks; retain actual observations. Reuse Operation's
raw_history_omitted intent for implicit completion and normalized named-root replay.

- [x] Step1: Add scalar two-call150/105 and named first4/second7 call tests,
  source/flat/generated history scope and prior cared-mask checks. Run against
  frozen/current parent. Expected: genuine unsupported-merge and implicit-history RED.
- [x] Step2: Reuse serial traversal and admit root provider markers; preserve
  implicit omission intent through named-root normalization. Run focused/native
  checks. Expected: exact native frames, actualX returns and explicit-only GREEN.
- [x] Step3: Pin direct notes and informational metadata, nested/unequal calls,
  namespaces, variable widths/reset/cancellation, unknown masks/status/clears,
  defaults/local-reset, invalid outputs/modes and payload limits. Add measured
  parent controls and reporter tests first. Expected: truthful replay and bounds.
- [x] Step4: Run full normal/static generators,30native, affected SAN CLI,
  C++quality, corpus, packaging, old/new matrices and guides. Expected: verified
  exact input/product hashes, unchanged corpus classes and passing measured bounds.
- [x] Step5: Read final staged patch, commit and obtain one fresh final review.
  Repair blockers once test-first; ledger every ruling and Minor. Expected:
  0unresolved Critical/Important with no rereview.
- [ ] Step6: Merge/push, verify merged checks, detach reused tree, delete only
  owned branch and archive own ledger. Expected: clean pushed master,117stashes.

Completion command: python3 /tmp/opaque-root-serial-merges-completion.py
Self-review: one connected root queue/lowering/provider flow; no task interfaces.
