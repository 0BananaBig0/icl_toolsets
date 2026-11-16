# Qualified Native Ordinary Scans Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Execute already-accessible qualified ordinary root accesses without full symbolic frame expansion.

**Architecture:** Reuse immutable/current raw profiles and native replay after ordinary parsing; retain symbolic fallback and original ScanStep rendering.

**Tech Stack:** Existing C++17 scheduler/native state, Python unittest; no dependencies.

**Spec:** docs/superpowers/specs/2026-11-16-native-ordinary-scans-design.md

## Global Constraints

Task104 Root frozen28 immutable. No new solver/evaluator, reset/clock/ownership
semantics or raised limits. Keep1M active bits/4096 registers/65536 graph/16M route
work. Only currently active qualified root storage; unsupported cases retain
solver fallback. New scripts beneath tests/ mirror source. No local heavy work
during timed Task104 gates; remote only after its archived qualification.

## Review Focus

- Physical root fill must not become author permission to alter surrounding reservations.
- SI and expected-SO parity must remain independent across aliases and mixed index directions.
- Native failure must leave the original state/history available to symbolic fallback.
- Ordinary write X, automatic writes and port reservations must retain prior contracts.
- Native ScanStep rendering/history must match ordinary callers and generated replay.

### Task105: Qualified Single-CSU Native Path

**Files:** src/pattern/pattern_scheduler.cc (private helper and eligibility dispatch);
pattern_scheduler.h only if a justified native proof interface needs it; maintain
tests/src/pattern_scheduler_test.cc and tests/src/pattern/test_modeled_root_scan.py;
new native_ordinary_scans fixture and focused benchmark under tests/src/pattern/;
generator/model guides. Native replay changes only for justified ordinary event/
mask reuse, preserving existing raw defaults.

**Interfaces:** private std::optional<std::vector<ScanStep>> TryNativeScan(
const std::vector<Operation>& operations, const std::map<size_t,std::string>& goals,
const std::map<size_t,std::string>& expected, const std::set<size_t>& required,
bool together, const std::map<size_t,std::string>& preserved,
const std::map<size_t,std::string>& desired_preserved).
Consume existing DesiredPreserved,
RawChain/RawTemplate and ReplayScan/NetworkState controls; no new public PDL API.
Return one fully constructed/replayed step or nullopt before live adoption.

- [x] Builder check event draining, desired-mask proof and exact fallback boundaries;
      settle routine decisions and record rulings before tests/product.
- [x] Maintained RED1M visible-SIB write/read preparation and1024 distinct visible
      writes against frozen Task104; expect existing reset/event/solver limits.
      Add native independent literal references and mask/parity/history cases.
- [x] Implement qualified fast attempt before symbolic frames, with transactional
      native replay and unchanged ordinary rendering; reuse any existing bitstring
      transformation instead of a second parity engine.
- [x] Native and complete modeled suite GREEN; preserve fallback, resources,
      unknowns, source/flat/generated/reset replay and atomic publication.
- [x] Connected owner/data/call/reset/clock/merge/trace,63offline/style/audit;
      reproducible1M/1024distinct/partial/repeated and small-network measurements;
      genuine strict vendor checks with actual limitations retained.
- [x] Independent final review and one necessary TDD fix pass, immutable sanitizer/
      fullserial/remote gates all0 before integration.
- [x] Commit, fast-forward master and delete completed branch; preserve only future
      work/recovery artifacts, master plus one active task branch.

Final vendor fixture explicitly retains open1 at every later apply, preserving all original comparisons; BOTH Tessent2026.2 directions run/strict record/strict check0. Genuine job/input/source/artifact/console hashes verified before importing two schema3 snapshots; current post-style source reproduces both. The first forward probe retained an additional open1 observation and strict comparison rejected it; evidence archived without normalization. Required style delta: missing braces fixed; existing access-group validation/ordering extracted unchanged from Solve to satisfy800-statement limit. Independent delta review no findings, declined list empty. Final style3 passes;40 local scale commands rerun on post-style source all0. Final gates must rerun functional groups freshly because product source changed after GREEN2.

Verified Task105: frozen26 files and scheduler9c8f3f2698c151a8cd9501912878382985e3908d5c7b3a4ddaf57d9cf7ce54e6. Leak-enabled native7/main69/modeled68/resources35/data9/queued15/observation20/opaque13/clocks51/client25/reset12/trace16/all65offline certificates all0. Full serial make check passes; elapsed seconds recorded in build/native-ordinary-final-full.wall. Final remote54 fresh statuses all0 and169 original input hashes match. After immutable full completion, only the benchmark helper was corrected to give reference/candidate matching --debug options and a default scenario-provider callback for reuse; unchanged other-module AST and168 unchanged functional inputs proved. Original26 snapshot retained; corrected helper qualified by fresh local40/remote40 commands and independent delta review, no runtime product change. No functional reuse after necessary style extraction. Final local/remote40 matched-option scale3 commands each all0, candidate source/flat/generated complete artifact hashes match; equivalent16K and128-repeat ordinary reference bytes match. Maximum1M visible preparation,1024distinct SIB/512alias preparation and1Mpartial32 now pass without raised guards; their final raw packet hashes match manually authored raw references because legacy large ordinary programs reject. Independent literal1M retained-state and eight unequal-width/ascending/inverted SI-SO alias clocks prove hardware and exact observation/reservation masks. Native attempt reuses RawChain/ReplayScan, exact DesiredPreserved and lazy top-level event dispatch; internal fixed event calls unchanged, no duplicate solver/evaluator. Unknown fill/inactive/data/TAP/end-state/group/port cases retain symbolic fallback. Existing access-group validation/context/order extracted unchanged to satisfy style limit, independently reviewed; clang-format/tidy pass. Initial connected20 and GREEN2 fourteen groups all0; current post-style sanitizer/full/remote gates run fresh. Corpus2555 files, zero duplicate-file groups,26 reviewed repeated-module groups. Genuine Tessent2026.2 BOTH directions run/strict record/check0, all source/artifact/jobinput/console hashes verified before two schema3 snapshots imported; current source reproduces both. First forward probe failed due retained open1 observation; final source explicitly retains that observation on every later apply, without deleting comparisons or normalizing trace evidence. Independent final and necessary style-delta reviews no Critical/Important/Minor, declined-to-judge lists empty. Commit26, fast-forward master and delete completed branch; preserve root raw queue-order drafts. Raw-order/child/opaque/IR/TAP/parallel/merge and broader reset/clock/ownership/strict coverage/final release acceptance stay open. No push.
