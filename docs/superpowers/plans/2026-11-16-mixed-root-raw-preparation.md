# Mixed Root Raw Preparation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Execute an ordinary access queue before a root raw scan in one source Apply.

**Architecture:** Separate immutable raw qualification from current route evaluation;
reuse ordinary Apply followed by existing transactional raw replay.

**Tech Stack:** Existing C++17 scheduler/generator, Python unittest, no dependencies.

**Spec:** docs/superpowers/specs/2026-11-16-mixed-root-raw-preparation-design.md

## Global Constraints

Task103 frozen26 Root files remain unchanged during gates. One million active/queued
bits,4096 registers,65536 graph vertices and16M route traversal work remain. Ordinary
setups precede one root raw target; reject mixed -together, reverse-order/child/
opaque/TAP/IR/parallel/raw-merge. No new solver, state engine or reset/clock model.
New test scripts beneath tests/ mirror source directories.

## Review Focus

- Future lengths must be checked after preparation without weakening pure raw setup validation.
- Failed preparation or raw replay must preserve published pattern/local-reset/debug ICL artifacts.
- Ownership changes during ordinary preparation must reach the final raw proof and refresh.
- Local-reset preview/call boundaries must not flush or duplicate source pending groups.
-1024 setups must not solve repeatedly or rebuild immutable qualification for every request.

### Task104: Ordered Ordinary Preparation and Root Raw Replay

**Files:** src/pattern/pattern_scheduler.cc/.h, icl_pattern_generation_main.cc; maintain
tests/src/pattern_scheduler_test.cc and tests/src/pattern/test_modeled_root_scan.py;
new unique mixed_root_raw_preparation fixture; generator/model coverage guides.

**Interfaces:** private const RawProfile& RawTemplate(); existing RawChain derives
current routes. Public/Impl Operation NormalizeScan(const Operation&,
bool defer_root_length=false), default behavior unchanged. No change to ApplyScans
preserved/footprint output. Main invokes Apply(accesses,false,Values), Assigned,
then ApplyScans(scans,Values,&assigned), AssignedScans and existing step renderers.

- [x] Builder check: immutable/static cache boundaries, strict versus deferred validation,
      local-reset previews and source ordering. Resolve routine choices in ledger.
- [x] Write maintained test_mixed_root_raw_prepares_future_route and
      test_mixed_root_raw_reservations_and_atomic_failures; compare root9 after
      write open1 to two-Apply reference, verify all artifacts remain prior on bad
      final length/stability and ownership conflicts. Run frozen Task103 binary:
      expected FAIL at existing mixed-queue guard.
- [x] Factor RawTemplate and defer only root length after an ordinary queue; keep
      default NormalizeScan behavior. Add native strict/deferred/cache assertions.
      Main executes and records the ordinary queue once before raw replay.
- [x] Run native and full modeled CLI suite: expected0; include source/flat/generated/
      local-reset replay, writes/reads, cancellation/overwrite/malformed operands,
      pending state and explicit source-order/root-only/-together boundaries.
- [x] Run connected ownership/data/reset/call/merge checks,61offline/style/corpus;
      measure1M/1024/repeated equivalent references, qualify genuine vendor source
      without dropping observations or changing strict checker policy. Expected0
      for supported paths; retain actual vendor limitations without false certificate.
- [x] Independent final review, one required TDD fix pass, immutable leak/fullserial/
      remote gates. Expected0 from all required available checks before completion.
- [x] Commit, fast-forward master and delete completed branch, preserving only future
      work and recovery stashes; only master plus active task branch remain.

### Ordinary preparation followed by a root raw scan (Task104)

An ordinary iWrite/iRead queue may precede a qualified root-client iScan in one
source iApply. Existing ordinary planning executes once, records newly established
reservations and renders its concrete preparation steps. Raw replay then checks
the resulting route length and uses refreshed ownership state. For example,
iWrite open1 followed by root iScan9 in one Apply can open a bypassed eight-bit
leaf, then load that nine-bit route. Generated PDL keeps both physical CSUs.

RawTemplate caches immutable qualification; RawChain caches current selected
geometry. Deferred setup validates root identity, graph/port/control/reset roles,
options, numeric bounds and payloads, with selected geometry/reset readiness
checked at Apply after preparation. Pure raw setup stays strict. Reset can cancel
a valid deferred queue. Writes/reads after a raw setup, child preparation and mixed
-together remain rejected in this increment. No new solver or reset/clock model.
Native raw failure preserves separately successful preparation; generator failure
preserves pattern/local-reset/debug artifacts. Diagnostic --log records the error.

Native strict/deferred/cache/copy/rollback tests and65 modeled CLI methods pass;
all20 local connected groups,14 remote groups/84 input hashes, style, corpus2554
files/zero duplicates/26reviewed module groups and63 offline certificates pass.
Independent final review found no Critical/Important/Minor issue. Genuine
Tessent2026.2 BOTH directions run/strict record/strict check0, two schema3 snapshots
imported with source/artifact/input/console hashes verified. Explicit pre-write
read0, rawSO1 and final leafA5/open1 comparisons retained; checker policy unchanged.

Scale acceptance distinguishes the inherited ordinary solver from raw replay.
Both platforms33 supported-workload commands0, complete generated bytes equal to
Task103 explicit-boundary references:16,384active bits,1024 queued writes to the
same SIB, and128 prepare/raw-close groups. Remote source repetitions3.31-3.34s,
.05s and.47s respectively; peak75464KiB. Local16K2.14s/queued.03-.07s/repeat.28-.30s,
peak75436KiB. These measurements show boundary handling adds no demonstrated
scaling change; no speedup is claimed. Queued1024 is not1024 distinct instruments.

The broader ordinary-planner performance requirement remains open: exact1M active
bits exceeds existing reset-work accounting (storage plus model nodes);1,047,552
bits reaches existing scan-solver limits;65,536bits timed out at45s in BOTH saved
reference and candidate, and1024 distinct gated preparations hit the existing
scan-event resource bound in BOTH. Rejections/timeouts and original model/PDL/
commands remain archived under mixed-root-raw-scale{,2}/probes/limits evidence.
Raw-only1M support remains verified by the maintained Task103 regression. The
upgrade trigger is workloads exceeding the characterized ordinary-planning range
or requiring those distinct preparations; address that planner separately, without
waiving its guards or claiming full project scale acceptance here.

Final immutable leak/fullserial/remote gates and integration remain pending.

Verified Task104: leak-enabled native7/main69/modeled65/resources35/data9/queued15/observation20/opaque13/clocks51/client25/reset12/trace16/all63offline certificates all0. Full serial make check passes in 1460.4493277180009s. Final remote54 statuses0 and actual151 inputs match;13 unchanged functional groups reused from GREEN2 with84 inputs verified, separately measured benchmark/changed golden registry excluded explicitly. Local20 connected/style/audit2554 files/zero duplicate/26reviewed groups0. Independent final review no Critical/Important/Minor. Exact desired reservation refresh order, cold-copy/cache invalidation, strict pure normalization, deferred root validation, pending state/owner return/reset cancel and publication checks pass. Pattern/reset/debug publication is atomic; diagnostic log records errors by existing contract. Both platforms33 supported scale commands0 with complete output byte hashes equal to Task103 explicit-Apply references:16K active3.31-3.34s remote source,1024 writes to SAME SIB .05s,128 prepare/raw-close groups .47s; peak75464KiB. Local2.14s/.03-.07s/.28-.30s. Maximum1M/near1M ordinary budget rejections and65K45s timeouts/1024 DISTINCT-SIB event-budget failures occur in both saved reference and candidate; retained artifacts and exact commands, no guard raised or broader optimization waiver. Raw-only1M remains verified; next native ordinary optimization addresses this real inherited bottleneck. Genuine Tessent2026.2 BOTH directions run/strict record/strict check0 with all source/artifact/input/console hashes verified; two schema3 snapshots imported and every pre-write/raw/final comparison retained. One ordinary preparation queue precedes root raw replay in a source Apply; generated output keeps physical preparation and final raw CSU. No duplicate solver or state engine, immutable RawTemplate factored from current RawChain. Reverse-order/child/mixed-together/opaque/TAP/IR/parallel/merge and broader reset/clock/port ownership stay explicitly unsupported. Commit28 files, fast-forward master, delete completed branch and preserve native ordinary optimization work. Strict coverage/final platform/full vendor and remaining functionality acceptance stay open. No push.
