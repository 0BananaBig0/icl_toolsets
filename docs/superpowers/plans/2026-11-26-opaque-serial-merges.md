# Native opaque-child serial merges implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking.

**Goal:** Execute native opaque-child iMerge call groups with correct serial histories.
**Architecture:** Force existing serial merge traversal for opaque-child networks,
then reuse native frame scheduling and the modeled provider's actual observations.
**Tech Stack:** C++17, Tcl, Python unittest, existing Z3 and sanitizer products.
**Spec:** docs/superpowers/specs/2026-11-26-opaque-serial-merges-design.md

## Global Constraints

- Original items2–15 except1; parentmasterec3c2dd and117preserved stashes.
- Reuse isolated .worktrees/bsdl-defaults; no dependency or invented observation.
- Preserve native queue/protocol contracts and aggregate1048576bit bound.
- Native opaque-child merges serialize; standalone root/physical/event limits remain.
- Measurements10s/512MiB with existing256MiB single-wide controls.
- One inline implementer, one fresh final review and one TDD blocker-fix wave.
- Preserve34deferred Minors and accurate broader remaining inventory.

## Review Focus

The provider and generated output must execute the same serial call frames.
Care masks and status clears must not be recounted or synthesized from SI/SO.
Nested/direct-note order and reservations retain existing merger semantics.
Local-reset verification must retain procedure context and source operations.
Standalone roots, physical mode and unsupported event operations stay rejected.

### Task 1: Native opaque-child serial merge execution

**Files:** Modify src/pattern/icl_pattern_generation_main.cc and
pattern_readback.cc; extend existing native readback tests as needed, update the
obsolete opaque-child empty-merge rejection test, and Makefile. Create
tests/src/pattern/test_opaque_serial_merges.py, a focused benchmark and its
mirrored reporter under tests/tests/src/pattern. Reuse the existing measured
runner and helpers. Maintain the three current guide/inventory documents.
**Interfaces:** Preserve GenerateMerged and MakeModeledReadback public signatures.
Opaque-child GenerateMerged chooses combine=false. Existing MergeOperations
handles call streams; ModelReadback accepts valid markers only in the native
nonphysical provider and continues using actual ApplyScans observations.

- [x] Step1: Add opened two-call and automatically prepared two-call tests.
  Each requests opaque8SI165/60 SO0bx stable, then actualX8/mask0/status0.
  Compare sequential opaque control and source/flat/generated continuations.
  Frozen parent should reject merge histories/native merge. Expected: genuine RED.
- [x] Step2: Force serial opaque-child merge traversal and permit its valid
  provider markers, retaining root/physical boundaries. Run new focused and
  existing opaque/readback tests. Expected: identical serial frames and GREEN.
- [x] Step3: Pin mixed/nested/unequal, direct notes, namespace/reservations,
  cared unknown/status clears, old histories, default/local-reset, empty groups,
  invalid outputs and standalone/physical/event boundaries. Add measured
  sequential-parent controls/reporters first. Expected: truthful exact outputs.
- [x] Step4: Run focused then complete normal/static generators,30native,
  affected sanitizer CLI, changed-C++quality, corpus, packaging, old/new matrices
  and guides. Expected: passing exact input/product hashes and measured bounds,
  unchanged corpus identities/classes.
- [x] Step5: Read finalized staged patch, commit and obtain one fresh review.
  Repair blockers once test-first, qualify and ledger all rulings/Minors.
  Expected:0unresolved Critical/Important without rereview.
- [ ] Step6: Merge/push, verify merged results, detach reused tree, delete owned
  branch and archive own ledger. Expected: clean pushed master and117stashes.

Completion command: python3 /tmp/opaque-serial-merges-completion.py
Self-review: one shared serial traversal/native history flow; no task interfaces.

Review repair: the single Important direct-metadata ordering bug is resolved by
validated private runtime eligibility checks and existing deferred traversal.
Both real failures and informational-note argument evaluation are covered in
15 focused CLI methods. All99 post-repair gates pass freshly,88x1418 per
generator,30native,12quality,1944measurements/387live and20guides. No rereview;
0unresolved Critical/Important and34 inherited deferred Minors.
