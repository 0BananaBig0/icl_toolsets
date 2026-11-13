# Relative system-clock waits implementation plan

> Execute inline with superpowers:executing-plans under the existing autonomous
> authorization. Preserve independent review and use the existing isolated worktree.

**Goal:** Retarget checked relative system-clock waits to a physical root clock,
with exact cumulative frequency scaling and original/flat/generated replay.

**Architecture:** A small SystemClocks model traces applied clock routing and
remembers each checked target's effective root bit and reduced frequency ratio.
It emits root-relative PDL without changing modeled pins or storage. A conservative
fanout guard rejects clock roots that could affect unsupported non-clock consumers.
**Tech stack:** C++17, existing scheduler/value helpers, native and Python tests;
no new dependency. GCC/Clang unsigned128 arithmetic supports exact checked scaling.
**Spec:** [Approved generator design](../specs/2026-10-25-icl-pattern-generation-design.md),
IEEE1687-2014 §§6.4.6.19–20,6.4.13(c),7.9.15,7.9.17.

## Scope and decisions

Support scalar or explicitly indexed ClockPort/ToClockPort targets, forwarded
ports, non-inverted aliases in their paths, first-matching known ClockMux routes,
and positive fixed ToClockPort multipliers/dividers. Resolve to an unconnected
root ClockPort. Unknown or uncovered selectors, constant/inverted clock paths,
period oscillators and differential clocks remain unsupported in this increment.
Overrides, absolute time, clock events and tester-period normalization remain
mandatory later work; this is relative-duration retargeting, not event simulation.

An iClock inspects applied state without flushing queued reads/writes. A wait
requires that exact target bit to have passed iClock, even for zero cycles, and
an empty pending queue. Recheck its current effective root/ratio: require another
iClock after a changed tuple, but accept a different known route with the same
tuple. Independent design review agrees this proves duration, not phase continuity.
Emit iClock root and ceil(count*divider/multiplier) root cycles. Replay at the root
has ratio1, preventing double scaling. Retain original clock checks in reset-only
rewrites and copy checked-clock state into recursive reset previews.

Classify fanout edge roles. Only Source edges on clock ports/aliases and ClockMux
choice inputs are routing; SelectedBy and all non-clock consumers are unsafe.
Include unselected branches conservatively. Reverse propagation from unsafe uses
marks root bits once, rather than traversing each root's cone repeatedly. Reject
known0/1 reservations on root or forwarded clock bits for nonzero waits; preserve
all scheduler state. Unsupported inverted/differential branches also invalidate
an affected root. No arbitrary phase or terminal level is invented.

Use iterative cached path resolution, invalidated after hardware-state changes;
retain checked snapshots separately. Cache one tuple per encountered bit, not
whole paths per target. Cross-cancel ratio factors, reject uint64 overflow, and
use unsigned128 for ceil multiplication without intermediate wrap. Expected input:
4096 forwarding stages and repeated checks, plus wide indexed roots; bounded by
existing netlist limits. Measure CLI wall time/RSS and do not claim optimality.

## Files and steps

- Modify src/pattern/pattern_netlist.cc to materialize positive frequency constants
  independently of port width; emit these pos_int properties in decimal in
  src/pattern/pattern_flatten.cc, as required by the shared parser.
- Create src/pattern/pattern_clock.h/.cc with SystemClocks Check(target,scheduler),
  Wait(target,count,scheduler,preserved), Invalidate() and copyable checked state.
  Check returns the canonical root name; Wait returns root name and uint64 cycles.
- Modify src/pattern/icl_pattern_generation_main.cc for iClock and -sck emission,
  cache invalidation and recursive-preview state; preserve existing TCK behavior.
- Register the new solver source and tests/src/pattern/test_system_clocks.py in
  Makefile. Extend tests/src/pattern_scheduler_test.cc for native held clocks and
  unchanged state. Update help, generator guide and model ledger after validation.

- [x] Inspect normative/vendor sources and obtain independent design review.
- [x] Write and run failing CLI cases: direct root, nested ratios/rounding,
  indexed forwarding, known ordered wildcard mux and procedure/prefix binding.
- [x] Implement the model and frequency elaboration; pass these cases and their
  source/flat/generated/reset-only comparisons.
- [x] Cover missing iClock (including zero), pending queue, unknown/uncovered or
  changed routes, equal effective reroutes, state changes via apply/reset/iState,
  invalid factors/overflow, unsupported fanout and inactive unsafe branches.
  Assert rejection preserves existing output/reset/debug files.
- [x] Native tests cover held root/forwarded bits, unrelated/X reservations,
  unchanged scheduler storage/pins and failed-wait state. CLI covers iClock with
  pending writes and multiple targets normalized to one root.
- [x] Measure4096-stage/repeated-check and wide/indexed cases; run focused tests,
  source review, formatting/naming, complete dynamic/static regression and relevant
  leak-enabled sanitizers. Record exact limitations and commit verified work.

## Review focus

ClockMux must use declaration order, never data-mux consensus. Hidden fanout and
alias bit positions must not hide event effects. Canonical output must scale once.
Cache invalidation must include automatic reset prerequisites and recursive previews.
Reservations must protect clock identities independently of checked tuple equality.
Each is covered by the steps above; full clock/event conformance remains open.

Implementation evidence: initial7-method CLI RED fails on unsupported iClock/-sck.
The first green run exposes binary factor serialization rejected by pos_int;
corrected decimal emission passes. Expanded fixtures initially used two unsized
selectors and a state-bearing reset-preview without reset establishment; those
fixture errors are retained and corrected. Final17 methods/main35 pass. Review
finds unrelated held inactive clocks were over-rejected: system-clock-held-red.exit1
proves it, and system-clock-held-green-2.exit0 proves the fix plus full native suite.
The initial fix failed to insert the fanout graph due a wrapped replacement; the
existing forwarded-hold assertion caught it (held-green.exit1), now corrected.
Native netlist/flatten and source formatting/naming pass. No source exemptions or
frequency overflow relaxations were added. A new tests/src/pattern/probe_system_clocks.tcl
runs original and normalized PDL against Tessent in the explicitly authorized
remote /tmp/icl-system-clocks-20261114. The probe exits0; source and normalized
exports match our runtime trace (iClock root, waits7/10). Submitted bytes/hashes,
fetched evidence and exact comparison commands are retained in
build/system-clock-vendor-probe. It is not a stored directional certificate.
All selected leak-enabled checks pass: native scheduler/flatten/netlist plus
main35/clock17/client18/reset12. Full dynamic/static integration passes; exact final evidence follows.

Integration corrections: the first full run (system-clock-final-full.exit2) found
pattern_clock.cc missing from tests/coverage.py. Added both .cc and inline .h;
all7 coverage-manifest tests pass, with no exclusions or threshold changes.
Confirmed full rerun is system-clock-confirmed-full.*. Positive ratio/scale
fixtures now use child ToClockPorts rather than local ToClockPort sources under
§6.3.12(e). The17-method corrected suite passes in3.586s/94096KiB;4096 forwarding
instances/219051bytes/1024checks/fiveforms take0.982595s; wide65536/256forwardedbits
case0.233540s. Corrected leak-enabled17 passes in10.111s. Root imports occurred
before either clock-suite invocation in the confirmed full run. Product code is
unchanged by these fixture corrections. A proposed missing-Source factor regression
proved the existing shared frontend already rejects it; no redundant validator or
test was added. Its diagnostic-mismatch probe log is retained, not a product RED.

Final confirmed full exit0:993.511s,41 dynamic/static generator suites,51 directional/
seven legacy goldens,2523 ICL334/119 rules, editor6 in179.043s, five optional ANTLR
skips. Corrected hierarchical17-method clock fixtures ran in both builds; final
leak-enabled corrected17 also passes. Source review/required fix/native regression,
formatting/naming and7 coverage-manifest checks pass. No pending Task51 verification;
strict coverage, later platform qualification and remaining model work stay open.
