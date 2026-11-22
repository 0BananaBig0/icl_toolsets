# Modeled Raw Scan and Chain History Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking.

**Goal:** Supply truthful variable-width raw interface/chain history and execute root opaque named-chain queues.
**Architecture:** Shared canonical scan field layout and opaque description feed lowering/adapter code. Concrete raw replay exports owned actual words; Runtime validates separate raw deltas and serves latest applied histories. Opaque data stays unknown.
**Tech Stack:** Existing C++17/Z3/Tcl/Python unittest, no dependencies.
**Spec:** docs/superpowers/specs/2026-11-22-modeled-raw-readback-design.md

## Global Constraints
No observed values from expected or prior raw input. Existing scalar canonical output/defaults stay stable.
Named parallel scans/getters require explicit -chain; default aliases the unnamed scalar chain only.
Keep1MiBit request/queued payload limits, direct synthetic-target guards, exact CSU/reset/ownership/stable checks.
Opaque-root parallel cases only: no hidden child, general modeled parallel/multi-TAP raw, raw merge, timed histories, callbacks or transport enabling.
New maintained test scripts under tests/. Preserve117 stashes and other worktrees/ledgers. Coverage1 excluded.

## Review Focus
- Child/root offsets and inversion parity must survive ReplayScan invalidating borrowed profiles.
- Raw literal-x TAP proof may validate structure but never supply actual observations.
- IR/DR share latest interface history while pending validation remains mode-specific.
- Raw deltas require a preceding applied width and full atomic validation before publishing any planes.
- Named opaque chains retain their own widths/history and never infer storage from prior -si.

### Task 1: Canonical raw layout and opaque named-chain execution
**Files:** Modify src/pattern/pattern_opaque.{h,cc}, pattern_runtime.cc, Makefile; maintain tests/src/pattern/test_opaque_root_scan.py; create tests/src/pattern/test_opaque_parallel_scan.py.
**Interfaces:** Add ScanFields {bool instruction; size_t target,length,input,expected; string chain;} and ReadScanFields(const Operation& normalized). Add OpaqueScanModel {string client; map<string,string> defaults;} (empty key for scalar), DescribeOpaqueScans(const Netlist&) and NormalizeOpaqueScan(z3::context&,const OpaqueScanModel&,const Operation&). Add OpaqueScanQueue(model) with Add(context,op), Reset(op), Apply(op) and Pending() accessors for shared pending/stable/default lifecycle. Keep both existing NormalizeScan overloads.
- [ ] Write failing opaque parallel tests with two disjoint vector SI/SO endpoints, per-chain sized defaults, unequal4/7-bit requests, queued replacement and reset cancellation, order/note/together/empty Apply, omitted default/stable chains without repeat comparisons, unknown-length rejection, -stable, payload bounds/invalid discarded requests and malformed roles/defaults. Check source/flat/generated/reset-source bytes and legacy default alias.
- [ ] Run new module against parent11f15cd; expect existing single-chain opaque guard failures, retain RED.
- [ ] Implement generic canonical named-chain layout while legacy scalar entry strips default. Describe only the already validated strict root-opaque class, extending vector data endpoints/per-chain defaults; reuse elaborator role/uniqueness checks. Normalize selected chain names without repeated whole-network scans.
- [ ] Lower latest operations per canonical chain under1MiBit. Complete missing parallel chains from reset default or last -stable data, clear implicit expected bits, reject unknown lengths and emit deterministic requests with one Apply. Preserve empty Apply/reset/note behavior. Update Runtime's raw-care consumer to layout fields and cached chain lookup now, preserving default-alias/known-zero-status behavior before Task2 adds history.
- [ ] Run new/legacy opaque tests and relevant frontend cases with dynamic/static generators; expect pass and legacy canonical controls unchanged. Commit.

### Task 2: Concrete raw observations, typed history and adapter
**Files:** Modify src/pattern/pattern_scheduler.{h,cc}, pattern_runtime.{h,cc}, pattern_readback.cc, icl_pattern_generation_main.cc; extend tests/src/pattern_readback_test.cc and pattern_runtime_test.cc; create tests/src/pattern/test_modeled_raw_readback.py; update obsolete raw-rejection assertions in existing tests.
**Interfaces:** Append ScanReadbackUpdate {string target,chain,read,miscompare;} and vector<ScanReadbackUpdate> scans{} after existing ReadbackUpdates fields. Add ScanScheduler::RawObservation = function<void(const Operation&,const string&)> and five-argument ApplyScan/ApplyScans overloads, retaining old four-argument entry points.
- [ ] Write native Runtime provider tests for valid raw formats/width changes and invalid target/chain/width/characters/duplicate keys, mixed valid/invalid bit+raw deltas and atomic publication. Write CLI literal known root9/mask1001/count2, normal/raw history independence, child parity/disjoint intervals, queue/reset/latest width, IR/DR latest, BYPASS/IDCODE, literal-x input, opaque unknown named chains, default alias/prefix/file/once-only, status/clear, failures/output preservation, subscriber-error follow-up tests.
- [ ] Run against Task1 binary/native source; expect missing raw history/observation failures. Retain actual RED and distinguish fixture/build errors.
- [ ] Snapshot scalar root register offsets/suffixes and requested child slices before replay. Capture shift storage through the existing certified hook, reconstruct MSB-first actual words and project latest requested interfaces. Keep ShiftComplete.
- [ ] Thread internal observation through raw TAP binary/literal replay. Reconstruct binary actuals from real ordinary observations and saved request layout; literal replay samples actual TDO. Use a non-storing observer only for structural proof when modeled comparisons are enabled. Preserve exact step and stable-route checks.
- [ ] Buffer external RawObservation in public ApplyScans; notify after accepted state/assigned/preparation commit and cache cleanup. Preserve callback committed-state semantics and four-argument ABI entry points.
- [ ] Add Runtime raw history keyed by qualified interface/chain, cache client/chain metadata once (including supported implicit unnamed TAP), invalidate latest applied widths, preserve reset history, parse -chain/format and reuse formatting. Reject raw getters before any applied width. Validate all provider deltas first; duplicate canonical raw keys reject atomically; old register history behavior remains.
- [ ] ModelReadback detects strict opaque backend without constructing its hardware scheduler. Mirror pending raw/normal ordering, preparation/together rules, reservations, reset and unsupported histories; return x for opaque data. Share mismatch/count logic and coalesce latest raw deltas. Never publish missing/failed read results.
- [ ] Output generation supplies non-storing raw observation in model mode, keeping adapter/generation mismatch decisions aligned. Update help for new raw history and modeled physical comparison behavior; this also completes the previously deferred help requirement.
- [ ] Run focused new/existing raw, TAP, standalone, PIO/data and native Runtime/readback tests with dynamic/static generators; expect all pass and original/flat/generated outputs stable. Commit.

### Task 3: Qualification, review and integration
**Files:** README.md, docs/pattern-generation.md, pattern-model-coverage.md, remaining-work.md.
**Interfaces:** Publishes runnable known/native and unknown/opaque chain examples with measured supported scale.
- [ ] Execute examples and parent/current scale controls for existing scalar/TAP cases plus new opaque named chains, repeated Applies and larger payloads; record commands/sizes/time/RSS/hashes and actual limits.
- [ ] Run full native/sanitizer, dynamic/static generator, affected format/tidy/strict C++17 and source-package gates; expect no failures or falsely claimed unavailable suites.
- [ ] Obtain one fresh independent whole-branch review; fix Important/Critical findings in one test-first verified pass, no rereview. Ledger dispositions and any remaining Minor findings.
- [ ] Update current documentation/inventory with actual results and remaining scope; commit.
- [ ] Merge into master, verify integrated source/product hashes and focused checks, push, detach reused worktree and delete merged branch; archive only own ledger.
