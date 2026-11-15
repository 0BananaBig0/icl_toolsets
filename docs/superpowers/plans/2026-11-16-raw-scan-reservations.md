# Modeled Raw Scan Reservations Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Respect existing known resource reservations during modeled raw CSUs.

**Architecture:** Reuse ResourceReservations and native CheckPreserved; collect
pre-route explicit IDs only when needed and refresh existing protected values.

**Tech Stack:** Existing C++17 scheduler/resources/generator, Python unittest.

**Spec:** docs/superpowers/specs/2026-11-16-raw-scan-reservations-design.md

## Global Constraints

Task102 Root frozen29 unchanged during gates. No opaque/IR/TAP/parallel/raw-merge,
closed-child prep, port/clock ownership or reset/clock widening. Unwritten takes
stay unrestricted until the existing write/state rule establishes protection.
1M active/queued bits,4096 registers,65536 graph/16M traversal bounds unchanged.
New test scripts beneath tests/ mirrored paths; no dependencies or RTL edits.

## Review Focus

- Raw assignments must not freeze unwritten single-stream takes.
- Footprints describe the pre-CSU addressed registers, not the changed post route.
- Preserve established surrounding values at every native phase; explicit SI
  overrides only addressed bits, and protected X stays an atomic error.
- Expected-only iState/SO must not establish controlling history.
- No-reservation scans avoid extra footprint/mask work; wide takes stay linear.

### Task103: Reserved Model State Across Raw Scans

**Files:** src/pattern/pattern_scheduler.cc/.h, pattern_resources.cc/.h,
icl_pattern_generation_main.cc; maintained tests/src/pattern_scheduler_test.cc,
tests/src/pattern/test_modeled_root_scan.py and existing resource/data-resource
suites; new uniquely named raw_scan_reservations fixture; generator/model guides.

**Interfaces:** ApplyScan/ApplyScans optional const map<size_t,string>& preserved={},
std::set<size_t>* assigned=nullptr; output untouched on failure. AssignedScans(const
std::set<size_t>&,const ScanScheduler&) updates only current Values masks. Private
known-value recording helper shared with Assigned; untouched mask positions x.

- [x] Builder review of semantics, phase checks, footprint/output/caller lifetime.

Builder rulings: storage-only raw CSU rejects established non-register masks;
unwritten takes remain free. Revalidate pre-route at Apply. A failed trial keeps
hardware/fill/caller footprint unchanged, while valid pre-state memoization is
allowed. No cache rollback or extra transaction engine.
- [x] Maintained native/CLI RED for take/release/state whitelist and stale history;
      single-stream unwritten take, explicit override, surrounding mask and X/atomicity.
- [x] Minimal preserved/footprint/resource/main integration; focused/native CLI GREEN.
- [x] Connected ownership/data/merge/call/reset checks,60offline/style/corpus, measured
      1M/1024/partial/no-owner scale and honest source-matched vendor qualification.
- [x] Independent final review and immutable leak/fullserial/remote/certificate gates.
- [x] Commit, fast-forward master, delete merged task branch and preserve future work.

Use frozen Task102 binary to establish intended RED before editing product. Do not
run local builds/tests concurrently with Root's timed sanitizer/full regression;
use remote after archived Task102 evidence or wait. Final make -j1 check exits0.

Post-review fix: one evaluator for a whole resolved Value target reuses native
SignalEvaluator::Bits through ReadBits, removing quadratic recorder reads. Maintained
1M setup/explicit-refresh regression RED at15.032s -> all58 CLI GREEN22.440s;
post-fix20connected/60offline/style/corpus all0. Local44 scale runs0, including
explicit wide assignment, all physical packets equal to Task102 reference after
excluding only iState metadata; .43-.50s owned1M source, .29-.31s1024children,
peak190936KiB. Remote scale/vendor/final immutable gates pending.

Task103 qualification: all20 local connected groups and60 prior offline certificates
pass after the performance fix; all14 remote groups pass with79 submitted inputs
verified. Both local and Rocky8 remote44-command scale runs pass, with all physical
packets equal across source/plain/flat/generated and equivalent Task102 no-owner
reference (only controlling iState metadata excluded from the packet hash). Remote
1M active bits: full protected surrounding .64-.65s source/.67s flat; explicit-wide
refresh .74-.76s source/.78s flat; partial32-bit take .47-.48s;1024 child requests
with512 shared aliases .44s. Peak195112KiB, including flat form. Current no-owner
baselines .30-.35s wide/.25s many; measurements describe reservation overhead.
Exact commands/environment/hash/memory records are in benchmark results.json.

Genuine Tessent2026.2 FORWARD run/strict record/strict check0 for
raw_scan_reservations. All source/artifact/job-input/console hashes verified before
importing one schema3 snapshot, directional offline total61. Initial forward run
added pre-write comparisons; the fixture now explicitly retains those observations
(0 thenA5) along with raw SO8 and finalA7. No comparison or checker rule removed.
Original REVERSE still1: vendor enforces previous iWrite bit3=1 after rawA5 and
rejects the next partial write. No reverse certificate or full vendor equivalence
is claimed. IEEE1687 §7.9.12(e) defines explicit SI controlling state like iWrite;
the implementation refreshes already-protected author-assigned storage, while
§7.9.19(d) prohibits software changes. Tessent v2023.4 iTake/iScan manual topics
and actual2026.2 behavior are recorded separately from that normative reading.

Final immutable leak/fullserial/remote gates and integration remain pending.

Verified Task103: leak-enabled native7/main69/modeled58/resources35/data9/queued15/observation20/opaque13/clocks51/client25/reset12/trace16/all61offline certificates all0. Full serial make check passes in 1464.4951526590157s. Final remote54 statuses0 and actual133 inputs match;13 source-matched GREEN2 tests are explicitly reused, with79 input hashes verified and golden registry checked freshly. Local20 connected/style/audit2553 files/zero duplicate/26reviewed groups0. Independent final review found one Important quadratic recorder read:1M setup/explicit-refresh regression RED at15.032s then all58 CLI GREEN22.440s; native ReadBits reuses existing SignalEvaluator::Bits and its memoization instead of whole-register copies per bit. No other findings or deferred minors. Both platforms44 scale commands0, exact commands/environment/memory/hashes archived. Remote1M full surrounding .64-.65s, explicitwide .74-.76s (.78s flat), partial32 .47-.48s,1024children/512aliases .44s; peak195112KiB. No-owner references .30-.35s/.25s; every physical packet/Apply hash matches after excluding only controlling iState metadata. Genuine Tessent2026.2 FORWARD run/strict record/strict check0 with all source/artifact/input/console hashes verified and one schema3 snapshot imported. Original REVERSE1 retains old iWrite bit3 after rawA5 and rejects later partial write; no reverse certificate or full equivalence claimed. Added author pre-write read0/A5 to retain vendor observations alongside SO8/finalA7; policy unchanged. Single-stream unwritten takes stay free; only previously protected explicit pre-CSU ScanRegister bits refresh, all surrounding values checked through native phases. Established port/non-ScanRegister ownership remains rejected; valid pre-state cache memoization allowed on failure, hardware/fill/caller footprint unchanged. Rulings: reuse immutable source evidence, review an uncommitted package before final commit, explicit SI controlling-state interpretation per IEEE7.9.12(e)/7.9.19(d) with vendor limitation recorded. No allocation-failure transaction expansion without concrete requirement. Commit26 files, fast-forward master, delete completed branch, preserve future mixed-root preparation work. Broader raw topology/ownership/events/reset/clock/lifecycle, strict coverage and final platform/vendor requirements remain open. No push.
