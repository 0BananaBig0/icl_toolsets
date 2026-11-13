# System-clock override snapshots implementation plan

> Execute inline with superpowers:executing-plans under existing autonomous
> authorization. Prepare failing tests in isolation while Task51 verification runs;
> keep root production files frozen until Task51 full verification and commit.
> Implement in the isolated worktree after the focused Task51 checks pass. Preserve independent review.

**Goal:** Apply ordered scalar/indexed iClockOverride declarations while retaining
previously checked wait durations until another iClock, with canonical root replay.
**Architecture:** Extend SystemClocks with versioned per-target override entries.
A checked target stores its effective tuple and override version; applied-state
revalidation uses that version. Keep the shared netlist immutable. Current routing
independently governs safety and reservation checks.
**Tech stack:** C++17, existing runtime, scheduler, netlist and arithmetic helpers;
no dependency. Preserve signed64 ICL factor limits and checked uint64 PDL factors.
**Spec:** [Approved generator design](../specs/2026-10-25-icl-pattern-generation-design.md),
IEEE1687-2014 §7.9.16(c–e) and its explicit cached-scaling example, printed pp174–175.

## Scope and decisions

Each override invocation restores every omitted field from ICL, independently of
previous overrides. A supplied source must name one ClockPort/ToClockPort bit;
the target must name one ToClockPort bit. Explicit indices isolate vector bits.
Factors must be known positive uint64 integers. Reject malformed options and
unsupported oscillator/differential targets. Missing original Source still needs
a supplied valid source; absence alone must not invent a root.

IEEE's example retains old scaling after override until the next iClock, despite
broader introductory wording. Follow that concrete example. An old checked target
uses its recorded override version when revalidating later mux state; an override
alone must neither change the old wait nor cause a changed-route rejection.
Reset and procedure return do not erase overrides or checked snapshots; reset
invalidates applied-selector caches. Copy these states into local-reset previews.

Tessent v2023.4 documents different omitted-option rules: an omitted source keeps
the previous override, neither factor keeps both previous factors, and one factor
sets the other to1. Document this discrepancy rather than silently implementing
vendor behavior as IEEE semantics. Probe the current vendor separately if useful;
matching a source with all options explicit does not validate omitted defaults.

Retain per-target vectors of (version, source, multiplier, divider), using binary
search for the latest entry at or before a checked version. Store one version ID
per checked target, not an entire override map or a path snapshot. Cache effective
routes for one requested version at a time; changing versions clears derived
routes, while changing hardware also clears selector values. This bounds retained
route memory; alternating old versions can require another traversal and must be
measured on the declared scale before acceptance.

Reject a newly introduced structural cycle at the override declaration, before
publishing that version. Starting at the proposed source bit, iteratively follow
current override/default forwarding and every ClockMux choice; reaching the
changed target bit proves a cycle. Existing graphs are acyclic, so an edge change
only needs this source-to-target reachability check. This does not require a known
mux selector or an operable clock. Retain Resolve's cycle check as well. Thus an
old checked wait cannot hide a newer cyclic declaration behind its old snapshot.
For this clock-only increment, reject an overridden target whose static downstream
cone can reach a non-clock consumer. Keep existing root restrictions. This ensures
new connections cannot add modeled events through a previously unsafe target.
Retain original plus introduced source edges for conservative fanout reachability,
deduplicating repeated edges. Reservation checks trace current override routing,
independently of an older duration snapshot, and reject potentially related unknown
routes conservatively. Original unsafe paths that an override removes may remain
conservatively rejected; removing that limit is later qualification work.

Normalized output omits override declarations after checks/waits have become root
commands. Reset-only output retains the original override commands and procedure
text. Do not mutate shared model properties or invent physical periods/phases.
Absolute time, oscillators, differential/inverted paths and hardware clock events
remain mandatory work outside this increment.

## Files and steps

Modify src/pattern/pattern_clock.h/.cc for Override(operation, numeric context),
versioned entries, checked-version route lookup and current-version reservations.
Reuse existing ResolvePdlTarget/PdlNumber validation, factor defaults and checked
ratio arithmetic. Modify src/pattern/icl_pattern_generation_main.cc to call
Override for the already qualified operation, omitting it only from normalized
output. Update help and the guide/model ledger. Extend the existing
 tests/src/pattern/test_system_clocks.py and native clock section in
 tests/src/pattern_scheduler_test.cc; no new runner or registration is needed.

- [x] Read IEEE/vendor rules and obtain independent conceptual review.
- [x] Add failing original/flat/generated/reset-only CLI cases for explicit factor
  and source changes, omitted fields returning to ICL defaults, and old check →
  override → old wait → new check → new wait. Verify RED on Task51's unsupported
  command, not an unrelated fixture or parser error.
- [x] Implement versioned overrides and successful checked snapshots. Keep target
  bit isolation, source qualification, exact arithmetic and output atomicity.
- [x] Test procedure/reset lifetime, independently checked targets at different
  versions, mux changes revalidated against the checked version, pending-queue
  behavior and a state-bearing reset-only preview containing an override.
- [x] Reject introduced cycles, invalid source/target kinds, missing sources,
  malformed/duplicate options, zero/X/negative/oversized factors, ratio/count
  overflow and new unsafe fanout. Assert old output/reset/debug preservation.
- [x] Native tests cover current-version reservation dependencies while duration
  uses an older snapshot, unrelated stopped clocks and unchanged hardware state.
- [x] Measure4096-stage paths with1024 repeated/alternating checked versions and
  indexed-bit overrides, including old-duration/current-reservation alternation
  on every wait. Compare with Task51's measured baseline; require bounded
  memory and completion within the existing45s CLI-case limit. If repeated
  retracing misses that target, improve invalidation/caching before acceptance.
- [x] Run focused tests, independent source review, formatting/naming, full
  dynamic/static regression and relevant leak-enabled sanitizers. Record exact
  evidence and vendor differences, then commit verified work.

## Review focus

Old checks must survive later declarations. Omitted options must not inherit a
previous override. New edges must not bypass event/fanout/reservation protection.
Cycle detection must include current override paths. Canonical replay must scale
once and preserve all observable waits. Tests above pin each requirement.

Task51 production files remain frozen for full verification. Eight Task52 CLI
cases in the isolated worktree fail on the unsupported command. Root tests retain
the17-method Task51 suite. This plan does not close the broader clock model.

Independent plan review identified the old-snapshot/current-cycle gap. Ruling:
validate structural cycles atomically at every source-edge change, including a
restored ICL default, instead of relying on a later new iClock or reservation.
The source-to-target walk includes all mux choices and needs no applied selector.
A two-node introduced cycle after an old check is a required rejection regression.

Execution ruling: continue Task52 in the isolated worktree while Task51's root
regression runs. The dependency's product/source checks, sanitizer checks and vendor
probe pass; only full integration is pending. Do not import Task52 changes to root
or integrate them until Task51 passes and is committed. Preserve the Task51 root
snapshot and save/rebase only Task52 file changes when advancing the worktree.

Isolated implementation: per-bit version history and checked snapshots are active;
30 CLI methods pass (Task51's17 plus13 override methods). Eight initial override
methods fail on the old unsupported command, retained as RED evidence. A positive
unconnected ToClockPort established by -source exposed an existing flat serializer
bug: empty ToClockPort bodies violate the shared grammar. Extend the existing
DataRegister empty-body semicolon rule only to ToClockPort; the five-form regression
now passes. This adds src/pattern/pattern_flatten.cc to the connected implementation
scope. First build failed only because a scripted replacement missed a wrapped
signature; corrected build passes.

Native current-reservation/old-duration, moved-root protection, failed-cycle
rollback and unchanged-state assertions pass before the broader native suite.
One-version cache benchmark:4096 legal forwarding instances,219071 ICL bytes,
1024 waits alternating checked-duration and current-held-clock versions take4.32s
locally. Keep the bounded cache; the upgrade trigger remains the measured45s
workload limit. Full native scheduler completes; independent source review is clean. Formatting,
naming and main35 checks pass. Leak-enabled native scheduler/flatten/netlist and
main35/clock30/client18/reset12 checks pass, with the4096-stage version benchmark
taking21.4063s. Final full dynamic/static integration passes.

Tessent2026.2 direct probes: no-option override rejected (retained failure);
partial-factor source/normalized exports match10/10/5 root waits. A separate
changed-defaults probe first overrides both source and factors, then omits fields:
its exports match root10,other35,root10,root25 and restore ICL defaults. Both vendor
exports have identical commands; redundant iClock checks in our output are removed.
Exact submitted-input hashes match fetched copies; comparison.json and logs are
in build/clock-override-vendor-probe/changed-defaults. Current measured behavior
differs from the older2023.4 manual. No formal vendor certificate is claimed.

Final integration: build/clock-override-final-full.log/.exit0,1050.603s wall:
41 dynamic/static suites,51 directional/seven legacy goldens,2523 ICL334/119
rules, editor6 in182.629s; five optional ANTLR tests skipped. All selected
leak-enabled native/CLI checks pass, including30 clock methods. Source review,
formatting and naming pass. Strict coverage and Rocky8 are not refreshed.
Task53 remains isolated; root production code is the verified Task52 snapshot.
