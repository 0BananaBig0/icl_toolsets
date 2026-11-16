# Root Raw Queue Order Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Accept root raw scans before or between ordinary queued accesses.

**Architecture:** Reuse existing root deferred normalization, two pending queues
and ordinary-then-raw Apply. Keep strict native API and child mixed boundaries.

**Tech Stack:** Existing C++17/Python/Tcl tooling, no dependencies.

**Spec:** docs/superpowers/specs/2026-11-16-root-raw-queue-order-design.md

## Global Constraints

Task105 frozen26 remains immutable. No local heavy work during its gates; shared
remote candidate changes only after its archived final gates. Keep1M active bits,
4096 registers,65536 graph vertices and16M traversal work. No clock/reset/solver,
RTL or dependency changes. All new scripts under tests/ mirror source paths.

## Review Focus

- Geometry validation must use the state after preparation regardless of source order.
- Root-only eligibility must retain child/opaque/TAP boundaries in both orders.
- Reset cancels geometry, while malformed/options/immutable qualification stays early.
- Alias/masked overwrites and returned owner lifetime must use the existing queues.
- Source/flat/generated/local-reset output and failed publication must agree.

### Task 106: Order-independent Root Raw Queue

**Files:** src/pattern/icl_pattern_generation_main.cc; maintain
 tests/src/pattern/test_modeled_root_scan.py and benchmark_native_ordinary_scans.py;
 new tests/src/pattern/benchmark_root_raw_queue_order.py and root_raw_queue_order
 fixture; model/generator guides and two strict vendor snapshots as qualified.

**Interfaces:** Existing ScanScheduler::NormalizeScan(Operation,bool=false),
Apply(vector<Operation>,bool,preserved), ApplyScans(vector<Operation>,preserved,assigned).
Private root-scan predicate consumes the existing resolved netlist target. No new
scheduler API, no public compatibility change. Benchmark main accepts an optional
scenario provider, defaulting to existing cases.

- [x] Maintain failing permutations/future-route/overwrites/call-owner/cancellation/
      boundary tests against frozen Task105 and record exact source hashes.
- [x] Identify root client once per queue decision, allow modeled accesses after
      root scans and defer every root queued length. Keep child mixed rejection.
- [x] Full modeled GREEN, connected ownership/reset/state/call/trace, style/audit/
      offline; performance40commands using canonical Task105 full-byte references;
      actual strict vendor evidence with all observation comparisons retained.
- [x] Independent final review, necessary TDD fix pass if any, frozen sanitizer/
      fullserial/remote checks all0.
- [x] Commit, fast-forward master, delete completed branch and retain one active
      task branch plus recovery artifacts. No push.

Self-review: every stated scope and review-focus condition has a maintained test
or required gate above; no new state engine or timing contract is introduced.

Task105 merged28734374435deb724423b3524e22fdef53e3774f and completed branch deleted. Its final benchmark helper already provides the default scenario callback and matching reference/candidate --debug options; this task reuses that interface. No benchmark_native_ordinary_scans.py diff remains. GREEN2 source174/15statuses0, main69/modeled72; genuine Tessent BOTH directions strict0; matched-option remote40 all0 with complete artifacts equal canonical Task105 output. Independent review no findings. Local gates pending.

Local20 connected0 (native7 explicitly reused from identical Task105 engine/test source;13 CLI/trace groups fresh), main69/modeled72,67offline0. Corpus2556/zero duplicate files/26 reviewed repeated-module groups. Style format0/tidy2 with required ICL_FORMAT_VERSION build macro0; first tidy invocation omitted that macro, no product defect. Local40 matching-option commands0, complete artifact hashes equal canonical Task105; remote matched40 also0 (.25s1M,.73-.74s1024distinct,.03s128repeats,.49-.50s partial32). Both vendor directions actual run/strict record/check0; source/artifact/input/console hashes verified and local replay0 before importing two schema3 certificates. Independent review no findings; benchmark matching-option delta approved separately. Runtime product remains16-line front-end delta with no scheduler/API changes. Freeze25 and run final gates next.

Verified Task106: root raw setup may precede or interleave ordinary access setup; existing ordinary preparation executes before final raw CSU. Every queued root defers current geometry/reset readiness to Apply; immutable qualification/options/payload bounds stay early, reset cancels execution and strict native default remains. Only16-line source front-end change, no scheduler/evaluator/reset/clock/API change. Root predicate retains child mixed rejection in both orders. Maintained RED5 methods/8 intended failures; GREEN2 all15 statuses0, main69/modeled72, actual174 inputs verified;12 unchanged functional groups reused with explicit169-input native-engine ancestry. Sole old expectation updated to accept canceled no-reset raw and separately reject executed no-reset raw; no product fix after independent review. Independent final review no Critical/Important/Minor; only actual gate/performance/vendor evidence was declined and verified by executor. Local20 connected0 with native7 source-identical Task105 full checks reused;13 fresh CLI/trace groups. Corpus2556/zero duplicate files/26 reviewed repeated-module groups0. Format/tidy with required version macro0. All67offline0. Genuine Tessent2026.2 BOTH directions actual run/strict record/check0, all source/artifact/jobinput/console hashes verified before importing two schema3 snapshots, local replay0. Local40 and remote matched-option40 scale commands0 with complete output hashes equal canonical Task105 explicit queue order, covering1M physical bits,1024distinct/512aliases,128 repeats and1M partial32 held state. Remote .25s1M,.73-.74s distinct1024,.03s128repeats,.49-.50s partial32; same-option canonical times equivalent. Earlier unequal-option evidence retained and superseded. Final leak-enabled sanitizer/native7/main69/modeled72/owner/data/state/clock/reset/trace/all67offline0; full serial make check0 (elapsed in build/root-raw-queue-order-final-full.wall). Final remote54 all0, actual187 hashes match frozen source;14 unchanged functional groups reused from source-matched GREEN2 with174 input verification/explicit benchmark+registry exclusions and inherited reuse records retained, fresh offline covers registry, separately measured benchmarks/main/engine hashes verified. Commit25, fast-forward master and delete branch; preserve mixed-child preparation drafts and recovery artifacts. A pre-existing nested symbolic-fallback route-cache issue discovered during Task107 is corrected and independently reviewed in that dependent task; its producer changes remain outside this frozen front-end snapshot. No push. Mixed child/opaque/IR/TAP/parallel/merge and broader reset/clock/ownership/strict coverage/release acceptance stay open.
