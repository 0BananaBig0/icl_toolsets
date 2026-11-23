# Opaque Child Raw Scan Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox tracking. Inline execution is the existing delegated method.

**Goal:** Retarget one declared-width opaque child on a native scalar scan route.

**Architecture:** Reuse OpaqueScanQueue completion/stability state and a shared
immutable private scan-interval expansion. Native ScanScheduler frames remain
the authority for surrounding registers; private captured/stored state remains
unknown while controlling fill is retained.

**Tech Stack:** Existing C++17/Z3/Tcl, Python3 unittest/Make; no dependencies.

**Spec:** docs/superpowers/specs/2026-11-23-opaque-child-raw-design.md

## Global Constraints

- One scalar opaque leaf, numeric explicitly sized DefaultLoadValue, current
  width equal to declared width, complete static scalar root route; existing
  topology/clock/reset/aggregate1MiBit guards remain. Broader8/6 and coverage1 pending.
- Opaque data never observed from expected/request/default/stable bits. Original
  IDs/model exports remain intact; private names/IDs never escape public PDL/
  step/observation APIs or become publicly addressable.
- Prior stable/reset-default data authorizes every preparation CSU; current
  explicit raw data is consumed once on its final raw CSU. Unknown prior length
  rejects preparation; no extra CSU for empty Apply.
- Preserve native copies, rollback and subscriber-failure commit; alias/inversion,
  logical/raw histories, status, scopes, reset and source replay remain correct.
- New tests under tests/src/pattern/; one focused native opaque target wired into
  normal/COV checks; total15native/15leakSAN. Preserve117stashes/unrelated worktrees.
- Normal widths8/64/256/1024 andSAN8/64, parent3+5, four groups/three repeats/
  source-flat-generated <=10s/512MiB. Fresh getters in generated probes; preserve
  1024callback10s and16Kalias256MiB bounds; fresh hashes/outputs/no speedup claim.
- Both whole generators, selected SAN histories/corpus/producer/protocol/Make,
  changedC++quality/source package required. One final independent review,
  one Critical/Important TDD fix pass, deferMinors/no rereview. Merge/push/delete.

## Review Focus

- A queued non-stable request must not be consumed by an earlier preparation CSU.
- Private writes must not leak through generated operations, native targets,
  Value/state or observation vectors, including native copies.
- Unknown-length state must survive a failed transaction and subscriber failures
  must retain already committed native/queue state.
- Alias/inverted wrappers and restored flat-origin scopes must keep exact
  child/root offsets and surrounding register data.
- A generated performance run must actually construct the lazy observer and
  check returned parent bits plus opaque x, rather than replay literal notes.

### Task 1: Reusable scalar opaque completion

**Files:** Modify src/pattern/pattern_opaque.h/.cc and Makefile.
Create tests/src/pattern/pattern_opaque_test.cc.

**Interfaces:** Add
std::vector<Operation> OpaqueScanQueue::Complete(const Operation& apply).
It completes one required physical CSU, including implicit scalar/named defaults/
stable payloads, and commits queue tracking only on success. Existing Apply
validates options, returns empty for an empty queue, otherwise calls Complete.
No change to NormalizeOpaqueScan/ReadScanFields/queued payload limits.

- [x] Write TestScalarCompletionDefaultStableUnknown and
      TestCompletionFailureAndCopies in the new native suite. After reset:
      default8'b0 yields iScan client8 -si00000000 -soxxxxxxxx -stable.
      Explicit8-bit0xa5/-so0x3c/-stable followed by Complete reuses0xa5 with
      all-x expected; non-stable0x5a makes later implicit Complete reject.
      Copy/reset/canceled invalid requests and named-chain behavior remain isolated.
      Assert empty Apply still emits no CSU even after reset/stable.
- [x] Add pattern-opaque-check, build/pattern_opaque_test and matching COV target/
      registration by reusing readback-test link objects/flags; no Runtime linkage.
      Run focused native RED: missing Complete API/required implicit behavior.
- [x] Factor existing completion, share scalar/named stable/unknown handling, keep
      validation-before-mutation and existing payload limits. Expected GREEN.
- [x] Build normal/static generator + focused native; run native and existing
      test_opaque_root_scan.py/test_opaque_parallel_scan.py dynamic/static.
      Run affected C++format/tidy/C++17. Expected all pass, root bytes unchanged
      and1MiBit root/repeated-reset tests remain within their existing checks.
- [x] Commit and task-done using python3 /tmp/opaque-child-raw-task1-check.py,
      verifying exact executed source/product/log hashes. Expected focused contract.

### Task 2: Native private opaque interval and per-CSU queue contracts

**Files:** Modify src/pattern/pattern_opaque.h/.cc,
src/pattern/pattern_scheduler.h/.cc, tests/src/pattern/pattern_opaque_test.cc.

**Interfaces:** Add immutable OpaqueChildModel (copied Netlist, original-node
boundary, opaque client/private-register identity/default) and
std::shared_ptr<const OpaqueChildModel> ExpandOpaqueChild(const Netlist&).
Return null when no eligible scan-only opaque-child route exists; preserve the
existing backend for unrelated PIO/data/TAP networks and its fail-closed checks
for unsupported opaque accesses. ScanScheduler owns/shares the model across copies
and exposes bool HasOpaqueChild() const for CLI capability checks. Reuse Task1
Complete for transaction-local native queue state. Public existing methods/
step types retain signatures.

- [x] Add native RED for3-bit head/8-bit opaque/5-bit tail. iReset; write head5/
      tail19; apply; raw child8 with0xa5/expected0x3c; observe childxxxxxxxx and
      actual parent5/19. Assert root/direct-child canonical payload offsets,
      copies/stable/default/unknown tracking and full-root -stable projection.
      Add native targets/privacy/Value tests, failed ownership/length rollback,
      observer throw after commit and empty-Apply/no-extra-frame assertions.
- [x] Implement one validated immutable expansion with preserved original IDs,
      x CaptureSource/reset/private post-CSU state and DefaultLoadValue controlling
      fill, scalar linked root controls, no opaque SO data/control side-channel.
      Existing RawTemplate/route checks decide whether the expanded static route
      is supported. Copies reuse owned immutable data and copy queue state.
- [x] Integrate ordinary parent register Apply and raw ApplyScans/ApplyRawScans.
      Validate original public targets before adding private goals; enforce prior
      stable/default in every ordinary/preparation frame, consume current raw
      data once, reject unsupported optimization/clock/physical modes.
      Filter private step names/observation IDs and remask private state on commit.
      Native raw preparation must either enforce prior known fill or fail in a
      trial; private register width never proves unknown opaque post-length.
- [x] Test mixed known-prior preparation+non-stable raw success; after non-stable
      completion, a new mixed group rejects preparation without prior knowledge
      and cannot silently reuse the current request. Test reset recovery and
      transactional native snapshots/subscriber commit. Expected RED→GREEN.
- [x] Run focused native/new checks plus existing scheduler/readback native,
      affected C++quality and relevant raw/root CLI compatibility on unchanged
      supported backends. Expected pass; no new CLI support claim until Task3.
- [x] Commit/task-done using python3 /tmp/opaque-child-raw-task2-check.py with
      exact executed inputs/products/logs. Expected native/state/queue/privacy.

### Task 3: Public CLI/readback, complete qualification and integration

**Files:** Modify src/pattern/icl_pattern_generation_main.cc,
src/pattern/pattern_readback.cc, src/pattern/pattern_opaque.cc, Makefile,
README.md, docs/pattern-generation.md, docs/pattern-model-coverage.md,
docs/remaining-work.md. Create tests/src/pattern/test_opaque_child_raw.py and
tests/src/pattern/benchmark_opaque_child_raw.py. Required native caller fixes
stay within Task2 files.

**Interfaces:** Consume HasOpaqueChild and original model for Runtime/exports.
Hierarchical register-free opaque models reach native validation/expansion rather
than the root-only opaque lowering/readback classifier. Clock/override/physical
calls fail closed for this backend. Existing source/reset rewriting and observer
APIs remain; no new CLI option or public private endpoint.

- [ ] CLI RED for parent-first and raw child access, actual parent/x opaque
      history, default/stable implicit traversal and non-stable follow-up rejection.
      Use immutable parent discovery probes. Add source/flat/generated/reset-source/
      split/prefix/namespace/inversion/alias/masks/status/reservations/error tests,
      nonempty outputs, atomic retained previous output/debug/reset artifacts.
- [ ] Update classifier/capability callers and required exposure guards only;
      register the suite in both generators. Expected all source/flat/generated
      replays preserve original target behavior and public operation privacy.
- [ ] Add benchmark with fresh invocation directories and actual four-group
      full-root probe data: expected parent head/tail bits around opaque x.
      Replace exactly four generated result notes with fresh getters at level1;
      compare canonical outputs and dimensions/tool/input/output hashes.
      Run specified normal/SAN grids and matched controls; expected all bounds.
- [ ] Document exact example/commands, visible controlling vs unknown actual
      state, width/stable/preparation constraints and all retained limits.
      Author-check example source/flat/generated; do not claim independent reader.
- [ ] Commit; one fresh final independent review of whole bf000ae..HEAD range.
      Regrade/resolve all declined behavior, one test-first Critical/Important
      fixpass, deferMinors/no rereview. Run full15native/15SAN, both whole
      generators, selected SAN/corpus/protocol/Make/quality/source packaging.
      Bind exact maintained-input/product/measurement receipts; expected all pass.
- [ ] Merge master, verify identical merged inputs/products and focused checks,
      push, detach preserved worktree/delete merged branch/archive only ownledger.
      task-done command python3 /tmp/opaque-child-raw-completion-check.py.
      Expected clean pushed master/onlymaster,117stashes and all receipts; continue
      original remaining inventory.
