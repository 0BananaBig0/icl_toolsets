# Dependency-guided scheduling implementation plan

> Execute inline with Superpowers TDD, independent review and full regression.

**Goal:** Configure larger serial IJTAG networks without searching every scan
horizon, following the user's shared-reset/select dependency guidance.
**Architecture:** Reuse the elaborated netlist, serial graph, symbolic values,
scan solver and concrete replay. Solve one combinational prerequisite snapshot,
then greedily batch reachable controllers until targets are accessible. Keep
the existing bounded scheduler as fallback and preserve atomic state updates.
**Tech stack:** existing C++17, Tcl and Z3; no new dependency or CLI option.
**Spec:** `docs/requirements.md` (Pattern scheduling scalability) and
`docs/superpowers/specs/2026-10-25-icl-pattern-generation-design.md`.

## Scope and interfaces

Modify `src/pattern/pattern_scheduler.cc` only for product behavior. Factor its
serial-path constraints into `SelectScan` shared by the original solver and the
snapshot planner. Add internal bounded-solve parameters and a private
`TryGreedy` result; public `ScanScheduler` interfaces stay unchanged.

Use the fast path for write-only applies on networks with at least 32 registers.
The current bounded path remains the small-model optimization oracle and handles
reads or configurations the heuristic cannot justify. This threshold is an
implementation choice, not an IEEE restriction. No global minimum is promised.
Single-TAP IR selection remains in the existing scan solver. Both original
`AccessTogether` and `iApply -together` obligations remain enforced at the final
apply. All configuration attempts run in an isolated scheduler state and commit
only after successful final replay.

## Tasks

- [x] Add persistent deep-chain and wide-tree ICL/PDL fixtures under
  `tests/cases/ours/pattern_generation/dependency_scheduling/`. Add new tests under
  `tests/src/pattern/` and integrate them into the existing regression.
  A 40-level chain must produce 41 DR scans; 64 independent gates and targets
  must share two DR scans. Check values, concrete replay, generated-PDL and
  flattened-ICL idempotency. Record the baseline failures/timings before edits.
- [x] Share serial viability/observation constraints, preserving enables,
  inversion, disconnected paths and overlapping mux selector checks.
- [x] Solve a desired controller snapshot with optional control writes;
  require target access and reset release, including resets of changed
  controllers. Minimize changed controllers and keep deterministic ties.
- [x] Batch accessible, reset-released controllers toward that snapshot.
  Pin prerequisite control loads to avoid undoing earlier progress. Stop on
  nonprogress; fall back without changing live state. Finish using the original
  operation group and its original simultaneous-write obligations.
- [x] Cover reconvergent/conflicting selectors, user writes to selectors,
  reset-controlled selectors, aliases, unknown initial controls, single TAP,
  fallback and atomic failure. Preserve existing small-model optimality tests.
- [x] Measure representative scale and limits, run independent review, focused
  checks, full dynamic/static regression and ASan/UBSan with leaks enabled.
  Update customer/developer docs with actual scope and remaining gaps.

## Review focus

- Desired selector values describe access before the target update; an explicit
  target write may intentionally close a SIB afterward.
- A control is writable only when its old reset state is deasserted. A release
  in this update cannot retroactively enable another target in the same update.
- Shared reset depth does not establish simultaneous scan accessibility.
- Any speculative partial schedule must leave original state and controlling
  fill untouched if it fails or falls back.
- Source/flat node numbering must not change decisions. Generated patterns must
  remain idempotent, including applies containing explicit prerequisite writes.

## Performance criteria

No pattern enumeration and no horizon loop in the greedy path. One snapshot
query, then at most one batch per progressing dependency layer; each batch uses
at most one DR scan plus a TAP instruction scan when necessary. Retain bounded
solver queries and a whole-attempt deadline. Measure 40 nested SIBs and 64
independent SIB/TDR pairs, then a larger representative case if those pass.
Report graph construction, solving and full CLI timings separately when available.

## Evidence and rulings

- Red: the old scheduler rejected the 40-level case at 32 scans and closed all
  selectors in the wide case (`build/dependency-scheduling-red.log`).
- Ten focused tests pass, including an independent finite-state oracle, deep and
  wide topologies, larger 512-register input, TAP select gating, alias grouping,
  explicit selector closure/reopening, unknown initial state, and slice/X writes.
  Native `TestGreedyRollback` checks state after a failed final apply and a later
  successful request. Log: `build/dependency-scheduling-reviewed.log`.
- Independent review found final partial writes being resolved against trial
  fill after prerequisite writes. The failing regression is recorded in
  `build/dependency-scheduling-partial-red.log`. Fixed by retaining original
  alias operations and appending canonical writes for the original resolved
  goals before final solving. Slice and X-mask tests now pass.
- Already-accessible requests bypass prerequisite solving. Deep generated-PDL
  replay improved from 11.17 s to 2.56 s in these runs; this measurement covers
  the same fixture and dynamic executable, with other jobs potentially active.
- Three full CLI measurements: 40-level source 0.31–0.34 s / 74,364–74,704 KiB
  peak RSS; 64-pair source 0.23–0.24 s / 75,460–75,784 KiB. The 53,661-byte,
  256-pair input took 2.97 s in the focused suite. Environment: g++ 16.2.1 `-O2`,
  Linux 6.18.33.2 WSL2. Commands and repeats are in
  `build/dependency-scheduling-measurements.log`; no global speedup or optimality
  claim follows from these specific topologies.
- ASan/UBSan with `detect_leaks=1`: all ten new cases, 26 existing CLI cases and
  native scheduler tests passed outside the sandbox; Z3 is not instrumented.
  Logs: `build/dependency-scheduling-sanitize.log` and
  `build/local-reset-rewrite-sanitize-host.log`.
- Final full `make check BISON=/usr/bin/bison -j4` exited 0 with the existing
  Honza reference directory/venv. Dynamic/static suites passed; all seven offline
  vendor goldens match; 334/334 grammar and 119/119 scanner rules exercised;
  both editors checked 38,639 ICL and 811 PDL files. Corpus audit found zero
  duplicate-file groups. Log: `build/dependency-scheduling-regression.log`.
  `make formatcheck` and `git diff --check` also passed.
- Ruling: use the heuristic only for write-only applies with at least 32
  registers; retain bounded fallback for smaller/mixed-read or incompatible
  configurations. This costs heuristic coverage in those cases and does not
  replace the outstanding full model/iMerge requirements.
