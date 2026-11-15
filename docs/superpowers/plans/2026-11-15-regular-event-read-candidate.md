# Bounded regular-event read candidate implementation plan

Execute inline in the existing isolated worktree; Root Tasks88/89 source/tests
and certificates remain immutable during final sanitizer/full checks.

Goal: avoid building a full per-clock symbolic shadow history for read-only
accesses that can already satisfy the existing strict regular-event model.
Measured current32 gated four-bit reads cost13.47–13.72s/512236KiB from original/
flat and5.86s generated replay;16 take1.36–1.46s. No global latency target exists.
Assume a five-second command budget for this32-register representative, retaining
the existing solver and fallback budgets; measure the baseline before code.

Approaches: retaining full events keeps the measured cost; a new event-DAG/cache
would add another model. Prefer a bounded isolated attempt with the existing
Solve(events=false) constraints and concrete replay before the full-event path, restricted to stable controls.
Limit to pure iRead operations for this task. Mixed/writes retain their path.

Safety argument to verify: ConstrainScanControls(events=false) requires actual
capture/shift/update to equal selected normal phases and forbids hidden events.
Only a complete replayed candidate may be adopted. A failed attempt must leave
all live state, fill, controller and ownership-related behavior untouched.
Full model remains the fallback for suppressed captures, scan-driven selector
prerequisites, hidden activity, reservations and complex event histories.

Reach: src/pattern/pattern_scheduler.cc Apply/helper only; existing native/CLI
tests for read behavior, end states, reset/ownership and hidden events; existing
control-mux source/flat/generated benchmark; guide/model ledger/this plan. No
dependency, public option, extra event semantics or parser/schema change.

- [x] RED exact32-model benchmark under5s with current qualified binary, actual
  source/input/binary hashes, environment/time/RSS.
- [x] Independently review model-equivalence and trial rollback/adoption before
  code. Inspect existing Impl copy/trial adoption rather than inventing another.
- [x] Add focused correctness cases for normal read success and fallback:
  declared CaptureSource, expected-value comparison, owned CE0 rejection,
  no-CaptureSource held reads, scan-driven prerequisite, hidden clients, pending
  reads/group obligations and end-state/restoration.
- [x] Try at most2 Solve frames on a copy for pure reads only when events are
  needed; retain current solver timeout and catch only bounded no-schedule
  failure; publish only complete concretely replayed steps and all changed
  mutable scheduling state. Otherwise run the existing full-event Solve.
- [x] Same5s benchmark GREEN, representative larger scale and matched canonical
  outputs where unchanged; no global optimality or linear whole-planner claim.
- [x] Connected/native/CLI, existing53 offline certificates, style and independent
  source review. Advance only after verified88/89 commit; final immutable gates.

Review focus: strict normal-event equivalence including inactive/hidden storage;
serial-dependent control/reset assumptions already supported; isolated failed
trials and mutable control/end-state copies; grouped/direct/routed reads and IR;
fallback overhead and unchanged limits; deterministic output/certificates.

Independent design review identified that final-state replay alone does not prove
intermediate event equivalence. Revised approved boundary: standalone clients
only, nonempty pure reads; reject any register reset and any storage/ScanInPort
in the existing control_seen_ transitive closure (all selectors, enables and bound
client controls/clocks). Remaining phase-dependent controls depend only on stable
nonserial inputs, literals and synthetic client phase pins; strict phase checks
then apply throughout each phase. Do not widen this boundary without event proof.
Catch std::runtime_error only around the isolated optional trial: current Solve
uses that type for unknown/time limits and replay rejection; full Solve remains
authoritative for diagnostics. Allocation errors are not swallowed. Adopt state,
fill, execution and mutable control masks/seen; global reset/readiness are copied
and remain unchanged by this standalone path. Independent review accepted this
boundary and adoption. No new dependency traversal or event model.

RED exact benchmark on qualified88/89 remote binary: timeout124,5.02s,277476KiB;
actual source/input/binary hashes retained. New maintained32-read test also exceeds
5s. First unittest reported TimeoutExpired as error; express the same budget as
an assertion failure before implementation. Both baseline attempts preserved.

First candidate build0, client25 failure1 and exact benchmark124 at5.02s:
review found ResetValue infers a direct virtual reset, so the overbroad reset
rejection excludes the exact measured workload. Preserve this failed pilot.
Refined independently accepted guard permits only deasserted direct missing-node
virtual reset references: require global_resets_00, virtual kind, no inversion.
All physical/literal reset references remain excluded. Standalone Reset sets
both concrete and symbolic virtual resets00; Solve/Replay do not mutate them.
This preserves the original benchmark and exact baseline inputs. Added no-reset
reservedCE0 held-read variant exercises strict-trial failure/full-event success.

Second candidate exact source benchmark passes0 in0.24s/79524KiB. The maintained
original and flat paths pass5s, but generated PDL includes explicit primary writes
and remains outside the deliberately pure-read candidate. Its five-second pilot
fails; retain normal45s generated correctness timeout rather than claiming an
unimplemented mixed-write speedup. Same PDL equality remains required. Other24
client methods including both held-read variants pass. Pilot evidence retained.

Advanced onto verified41a2fa0 after full88/89 exits0 (1381.145s/editor174.760s).
Saved32draft files/full diff/hashes and retained stash under/tmp/regular-event-
read-task90-before-advance-files. Restored only Task90 source/test/plan; verified
88/89 docs/certificates retained. Final focused remote build/client25/exactscale
all0; source/test2hashesmatch. Same source candidate26commands all0 and canonical
PDL/flat hashes equal per size.16 baseline1.58–1.62s→.14s;32 baseline15.43–15.59s/
512256KiB→.26s/80496KiB;64candidate.60–.61s/98328KiB. Baseline binary hash matches
qualified88/89 baseline; candidate source matches final focused snapshot. No64
baseline claim. Connected/style/offline and final immutable gates remain pending.

Local connected native7/main69/clock41/client25/reset12/multipleTAP22/feedback10 all0; format/naming0 and53offline directional certificates0. Independent final reviewclean. Integrate thisfivefile snapshot onto41a2fa0 and freeze production/tests for required leak-enabled sanitizer/full gates. Final12hashremote verification remains in progress.

Final Task90 remote50 statuses all0 with actual12 source/test hashes matching frozen submission. Required leak-enabled build/native7/main69/clock41/client25/reset12 all0. Root source/test2 unchanged; full serial check now running in build/regular-event-read-final-full.*.

Verified Task90 checkpoint: required leak-enabled build/native7/main69/clock41/
client25/reset12 all0. Full serial make check exits0 in1345.397s,41 dynamic and
41 static generator suites,53 directional goldens,2525ICL334/119rules,LSP62 and
editor6 in181.414s. Five optional ANTLR tests remain skipped. Remote50 statuses
all0 with actual12 source/test hashes matching. Source/test2 stayed immutable
through final gates. Connected, style/naming,53offline certificates, independent
review and26 matched performance commands pass. Branch cleanup removed44 already
integrated task references and preserved116 draft files; all old draft paths and
plans exist in master. The developer guide now requires completing each task by
integration and deleting its merged branch. Commit thissixfile snapshot on the
Task90 branch, fast-forward master and delete that branch; restore only Task91
future files onto its own active branch. No push. Broader event/clock/resource,
vendor, strict coverage and final platform requirements remain mandatory.
