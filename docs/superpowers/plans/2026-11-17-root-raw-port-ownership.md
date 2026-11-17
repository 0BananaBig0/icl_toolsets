# Root Raw Port Ownership Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Accept the five measured root raw/port-reservation source gaps.
**Architecture:** Pass one private root-request flag into existing ReplayScan;
validate all masks, exempt only explicit root port effects from automatic-change
checks, and retain original ownership for later retargeting.
**Tech Stack:** Existing C++17/Python/Make/gcov/Clang SAN/Tessent2026.2/SSH.
**Spec:** docs/superpowers/specs/2026-11-17-root-raw-port-ownership-design.md

## Global Constraints

Parent master21c6a46; task/root-raw-port-ownership. Seven files below only.
No public API/header/state framework/dependency/per-bit loop or broad ownership
policy. Child/TAP guards unchanged; masks validated before filtering; owners and
AssignedScans stay unchanged. Actual100%/90% raw coverage gate is not waived.
Preserve previous full/coverage/vendor/package evidence. No push.

## Review Focus

- Root exemption must derive from the explicit requested client, after overlap
  validation; a child covering the same storage is not automatically a root.
- Invalid port masks must still fail before filtering and publish no state.
- Raw effects must not release or refresh old port reservations; later ordinary
  errors and explicit release/rebase recovery must remain observable.
- Register unknown/protected bits, assigned footprint, child guards and trial
  rollback remain intact; no hidden parent preparation exemption.
- Full-shift acceleration and wide reservation limits remain unchanged; report
  actual performance, coverage and scope without broader equivalence claims.

### Task 126: Preserve ownership around direct root raw effects

**Files:** src/pattern/pattern_scheduler.cc; tests/src/pattern/test_modeled_root_scan.py;
tests/src/pattern_scheduler_test.cc; docs/pattern-generation.md;
docs/pattern-model-coverage.md; this plan and its spec.
**Interfaces:** private Impl::ApplyScans/ReplayScan only; existing
ResourceReservations::Values/AssignedScans and native TestRawScanReservations.

- [x] Record base/brief/callers, references and before-file/product snapshots.
      Extend CLI positives/late errors/release and child boundary, native state/
      history/mask rollback checks and focused selector first. Expected actual
      CLI/native RED at the current storage-only guard.
- [x] Pass a private direct-root flag after interval validation; validate all
      masks, omit port IDs from held checks only for that explicit root scan,
      preserve storage protection and owner values. Expected focused GREEN.
- [x] Format/tidy changed C++ sections and strict C++17. Run full changed CLI
      dynamic/static, focused native/SAN/CLI/main, existing wide/scale limits;
      build current normal products. Expected all relevant gates0.
- [x] Freeze seven selected files/runtime hashes. Run complete normal integration,
      then fresh-profile complete raw coverage serially. Expected normal0; report
      actual strict result with raw compiler exception edges and thresholds intact.
- [x] Qualify current native and51 generator suites per dynamic/transferred-static
      build remotely; match submitted/source hashes and scoped fresh vendor export
      replays. Expected actual statuses0 and exact trace/input/product evidence.
- [x] Record evidence/limits, freeze candidate, one independent final review and
      necessary fix pass; final verification, merge master/delete branch,
      task-done/archive/sync. No push or blanket model/coverage completion claim.

Pre-flight: root flag is private producer/consumer in one source file; callers
and request identity inspected. Owner/assignment APIs unchanged and tested.
Design/plan self-review complete. Standing user delegation covers routine choices.


### Direct root raw port ownership (2026-11-17)

Direct requests for the qualified standalone root client now accept the five
port-reservation source cases measured in the preceding audit. The explicit
root CSU may change ports in that network; their established reservations keep
their earlier values and still constrain later ordinary retargeting. For example,
a taken select assumed 0 may be driven by the root raw scan, while a subsequent
ordinary write still fails until its ownership constraint is explicitly changed
or released. This separates explicit iScan effects from retargeter preparation
under IEEE1687-2014 section7.9.12 and the bounded Tessent 2026.2 observations.
No general child/TAP ownership policy is implied.

One private flag identifies the requested root client after interval validation.
ReplayScan validates every original reservation mask before omitting port IDs
from its held-state checks for that root operation. ScanRegister protections,
unknown-bit rejection and original reservation values remain intact; assignment
footprints still contain only explicitly addressed pre-route registers. Child
requests retain the storage-only guard, including a measured child interval
covering exactly all eight root storage bits. Invalid masks and failed later
retargeting preserve state and caller footprints; explicit release recovers.
No public API/header/cache/dependency or per-bit replay loop is added.

Existing CLI/native tests first fail at the old storage guard, then pass with
the bounded change. Original/flat/generated/reset replay covers five vendor
positive source gaps, inverted output aliases, retained later constraints,
release/reset/explicit rebase and child rejection. Full modeled-root suites pass
80 methods per dynamic/static variant. Leak-enabled ASan/UBSan passes the focused
native reservation function, all 80 modeled-root and 71 main methods. The native
function also covers source/flat owner history, register-only assignments,
invalid port mask validation before filtering and rollback. Its new test-only
--raw-reservations selector reuses the default suite function. Changed C++
format/tidy and strict C++17 checks all pass.

Complete normal integration passes in 1533.315018s,
including six editor tests in 174.471s. The existing raw-reservation benchmark
passes 88 rows across dynamic/static: 1,048,576 active bits and up to 1,025 registers,
wide/partial ownership and 1,024 protected masks, three-run old plain references,
current plain/reserved forms, flat and generated replay. Physical packets match
within every scenario. Maximum measured process time/RSS is 0.50s/203,020KiB for
dynamic and 0.48s/201,732KiB for static. Per-row commands, sizes, platform and binary
hashes are retained; no speedup claim. Filtering adds at most one ordered-map
erase per protected port; the existing full-shift acceleration is retained.

The fresh complete raw coverage recipe takes 1118.165168s.
All 58 Python groups/945 cases (51 generator groups/783 cases), native checks,
1916 regression and 2180 conformance cases pass. Strict coverage still fails:
17,336/17,966 lines (96.5%) and
29,268/46,679 raw branches (62.7%), against unchanged 100%/90%
thresholds. Make exits 2 at the final report. All 54 authored reports and raw
compiler exception/cleanup edges remain included; untaken edges comprise
12,233 throw and5,178 other edges. No filtering or waiver.
Optional ANTLR checks remain skipped when unavailable.

Rocky qualification passes all seven gates, including current native checks and
51 suites/783 cases per dynamic/transferred-static generator variant. All 3,994
submitted source/test hashes match the remote files. Only the two generator
product bytes change; the other eight products remain unchanged on each platform.
The transferred static generator matches the local fingerprint and lacks an ELF
interpreter. This is current generator qualification; five-tool installation and
source-archive qualification from Task 124 predate this implementation change.

All five normalized programs retarget successfully in fresh Tessent 2026.2 runs
using the unchanged driver; exports replay through the current native tool and
all raw payload traces match the preceding audit. After the review correction,
all 20 submitted program/driver/model hashes remain identical, so the first
Tessent results are reused and every export is replayed through the corrected
native binary. Canonical replay bytes match
the two select cases. The constant-output export removes an empty pre-scan
Apply; both observed-output exports add an ordinary pre-scan capture comparison.
Those differences are retained, so full ordered-group equality and a new formal
directional certificate are not claimed. Existing broader vendor/model gaps
remain open. Evidence is `build/root-raw-port-20261117/`: frozen inputs, raw
coverage/profiles, local/remote/vendor logs, 88 benchmark rows, equal-span child
checks and fingerprints. Only scheduler source and two existing tests change
among the 3,994 runtime inputs. No push.
The independent review found one Important missed caller: TryNativeScan passed
its desired-map pointer into the newly inserted boolean parameter. A one-line
caller correction now passes false and the exact desired map. A 65,536-bit
ordinary explicit update of a taken register bit timed out 15s on the reviewed
candidate and passed five forms in 0.393004s after the correction; the pre-feature
reference passed in 0.400905s. The new maintained wide regression runs in every
current 80-method suite. All source-dependent qualification above was repeated
after this fix; first-candidate evidence is retained before-review. No second
independent review was requested. Independent final review found one Important missed caller; the executor resolved it in one fix pass with RED/GREEN evidence and refreshed source-dependent qualification. No unresolved Critical or Important issues remain. The scoped root raw port feature is qualified for integration.
