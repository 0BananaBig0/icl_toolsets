# Standalone TCK idle replay

> Use superpowers:executing-plans for authorized inline implementation.

**Goal:** Execute literal iRunLoop TCK cycles on standalone IJTAG clients, using
their actual clock/control bindings and shared hardware state. Preserve the
existing single-TAP idle behavior, atomic failures and bounded fixed-point
acceleration. This closes a concrete idle-command gap; system-clock/absolute
time, multi-TAP planning and general scan-derived event synthesis remain required.

**Source:** IEEE1687 §7.9.17(c,e,f) requires at least the requested TCK cycles,
does not require a TAP, and forbids unapplied reads/writes. Existing Runtime
already validates queued operations; the generator already retains normalized
literal counts. No new frontend syntax or dependencies.

## Design for independent review

Reuse EffectiveScanInterfaces, register_ports_, client_pins_/client_pin_bits_,
client_clocks_, EffectiveScanControl and NetworkState::Clock/Reset. Do not
treat a ScanRegister value as a new observation or advance hidden storage merely
because it is reachable in the scan graph.

For standalone idle, clone the committed NetworkState. Establish idle levels on
owned CaptureEn/ShiftEn/UpdateEn controls and TCK roots; keep current SelectPort
bits and ordinary primary inputs. Each cycle samples every bound child clock,
toggles owned TCK roots through rising and falling levels, computes effective
per-register controls with inactive implicit capture/shift/update defaults,
then uses NetworkState::Clock on the correct edge. Batch all equivalent root
clock bits before reading child clocks; do not invent intermediate cross-client
edges from iteration order. Explicit constant or derived
child controls still determine actual events. Unknown clock/enables remain
unknown; inverted child-clock edges keep the existing unsupported diagnostic.

Use a private standalone-idle pulse helper rather than changing scan-frame
ClientPins semantics. Sample and apply each edge in common state, reset-settle
and check reservations after relevant transitions. No one-scheduler-per-client
partition: controls and reset effects may cross client boundaries. A design with
no declared TCK/control pins still has the implicit TCK; its idle cycles create
no scan events but retain the existing Clock/Reset DataRegister/reset settlement. Never use ApplyInputs to
normalize protocol levels: that would create an unrelated primary write event.

Generalize the existing Idle loop to operate on either a copied TapExecution or
a copied standalone NetworkState. Reuse its complete-state fixed-point check,
cycle/work/time bounds, reset/control-event fill bookkeeping and commit-on-success.
TAP mode still requires explicit reset; standalone mode has no TAP FSM and must
not gain an implicit reset requirement. Zero count remains a no-op. Never discard
a changing-state cycle merely to accelerate a wait.

Carefully review the choice to retain SelectPort state while driving protocol
enables inactive. iState SelectPort assumptions and last selected clients must
not be silently replaced; uninitialized selection can remain unknown when an
inactive enable masks it. The helper must preserve unowned bits of vector pins
using the trial state, not the original scheduler snapshot.

## Verification and scope

Files: src/pattern/pattern_scheduler.cc, existing native scheduler tests, existing
tests/src/pattern/test_client_scan_controls.py and hidden-client event tests,
current guide/model/spec and this plan. A new test script, if needed, belongs
under tests/src/pattern/ and must join the existing shared dynamic/static/COV
suite recipe.

- [x] Review actual edge/control behavior and the idle-level contract before code.
- [x] Failing tests: no-TAP positive waits, no implicit reset, zero/huge counts,
  count/order preservation, ordinary storage unchanged, explicit hidden capture/
  update events, gated/unknown clocks, rising/falling reset effects, primary/data
  dependencies, selected-client retention and no new observation history.
- [x] Test preserved reservations at each edge, failure rollback/publication,
  loop resource limits, and original/flat/generated/reset-source replay.
- [x] Implement with existing event/state helpers; compare native per-edge
  expectations and any feasible vendor probe without claiming broad equivalence.
- [x] Measure stable large waits and a changing-state bounded case; run focused
  dynamic/static, native/CLI sanitizers, full regression, style and independent
  review. Keep unmodeled external BIST behavior and system-time semantics open.
- [x] Record evidence and commit.

## Research boundaries

The multi-TAP constructor currently rejects multiple TAPs; symbolic frames,
IR/DR selection and concrete replay have one active TAP pointer. TapExecution
itself can observe multiple controllers, but that alone is not scheduling
support. Do not remove the guard without joint state/event planning.

Shared named-chain ports are not a safe shortcut: §6.4.16(q) allows each port in
only one Chain within an interface. Existing separately selected client groups
already exercise shared storage. General intermediate observations require
their own legal topology and common-state event proof; no duplicate-input
fixture is being accepted merely to remove the independent-chain guard.

Design review confirms omitted TCK is permitted by §6.4.6.17 (one global
free-running test clock) and §6.7. Preserve IsDriven as well as SelectPort bits:
a source-defined or explicitly assumed x is not the same as an absent select.
Omitted selection must follow active connectivity under §6.4.6.11b/c; the
existing ActivePath helper alone ignores ancestor selection and is not yet a
sufficient proof. This point remains under review before implementation. Test
selected versus mux-disconnected hidden storage with omitted SelectPort and
constant capture/update, plus an explicitly disabled ancestor. Do not choose
unconditional fallback selection.

Check reservations after idle normalization/reset settlement and after each
rising/falling edge; a final-cycle-only check can miss transient violations.
Keep driven-pin flags, shift storage, pending event sets, fill and held-map
progress on the trial copy. Reset-at-rising/falling tests must pin existing
Clock snapshot ordering; do not add a pre-capture Reset that changes the
captured value. Consume event sets consistently before fixed-point comparison.

Final design ruling for omitted selection: first compute EffectiveScanControl
with fallback.select=0 and1. If all resulting capture/shift/update controls are
identical, selection is masked and no route proof is needed. Otherwise require
ActivePath(trial) to prove one route and require every sourced or already-driven
SelectPort bit in EffectiveScanInterfaces to be known1. Cache those interface
bit identities once; re-read their trial-state values and re-evaluate the route
on each edge because reset/update can change them. Unsourced/never-driven selects
retain existing implicit inference; explicitly assumed x is not implicit.
Use the proven route's register membership for fallback1/0. If proof is absent
or a defined select is0/x, reject with a dedicated implicit-selection diagnostic.
Do not turn all selections into a global zero: an unrelated disabled interface
must not suppress a live client's events. This conservative guard may reject
valid selective networks; broader ancestor-aware selection inference remains
required. Explicitly selected interfaces remain governed by their real values.

Review-required regressions: top select1 plus a proven route succeeds; top
select0 or explicit x with selection-dependent hidden constant controls rejects
and rolls back; select0 with inactive enables succeeds without route inference;
an unrelated disabled interface documents conservative rejection. Include a
known mux-bypass route where omitted-select hidden storage stays inactive.
All idle level changes preserve unowned vector bits from the trial snapshot.
Use Drive for pin levels, then existing Clock ordering for an edge; settle Reset
when normalizing idle controls, not as an extra pre-capture phase.

Implementation/test progress: isolated worktree /tmp/icl-standalone-idle-20261113
is based on91f5f2a. Native red and16-method CLI red fail for the expected no-TAP
diagnostic. Initial CLI green passed behavior cases but exposed a sized-reset
fixture width mistake at65536bits; corrected to unsized0. The16-method focused
CLI run passes in6.655s (6.70s wall,325096KiB RSS); three wide stable-wait replay
forms took1.452s. A seventeenth method covers conservative rejection with an
unrelated disabled interface and acceptance when enables mask selection; it
passes in0.125s. No full regression or independent implementation review yet.

The native source builder initially inserted Port sel inside CaptureEnPort,
because the substring search was not bounded to ScanInterface; corrected the
fixture and moved new tests first for fast failure. Added no invented primary
DataRegister write, unowned vector clock bits and transient reservation checks.
Reset-edge assertions are being checked against existing Clock/Reset semantics:
Reset changes held update storage, not the captured shift bits. A reset on the
rising edge can therefore be followed by a later falling-edge update after reset
has deasserted. Do not alter that model to satisfy an incorrect test expectation.


Implementation review completed with no correctness findings. Added the one
requested coverage case: a hidden DataRegister clocked write updates fill used
by a subsequent X write, even after event-free primary assumptions change.
Native scheduler tests pass after that addition. Correct reset-edge expectations
are rising core001/observer100 and falling core000/observer000: reset modifies
held storage, and the captured shift bits survive until the following update.
All17 focused CLI methods pass (6.786s); the wide stable case takes1.440s.
Changed-source naming/style pass. The eighteenth method replays the new vendor
fixture. Fresh exact forward comparison succeeds after explicitly requesting
the initial reset-value read; the first mismatch is preserved, not waived.
Root integration is complete and final full/sanitizer checks are running.

The initial integrated full run exited2 on an obsolete main-CLI assertion that
all positive standalone waits must fail. Replaced only that assertion with a
success/output-count check; TAP-reset, pending-operation, unknown/overflow and
system-clock/time failures remain tested. The focused method passes. Retained
build/standalone-idle-final-full.log/.exit(2); the corrected full run uses
build/standalone-idle-reviewed-full.log/.exit. The first sandbox background
launcher died before creating a make log; the persistent host job supplied the
recorded first full run. No product code changed after independent review.

Leak-enabled final ASan/UBSan validation passed on2026-11-14: native scheduler,
main31 (22.620s), client18 (27.554s), hidden-client8 (43.923s), and multiple-client17
(18.522s). Evidence build/standalone-idle-final-sanitizer-all.exit(0) and its
per-suite logs/exits. The65536-bit UINT64_MAX case took5.713s for three sanitized
replays. Integrated formatting passes. Corrected full regression remains running.

Independent follow-up review verified the new certificate's source/input, local
and vendor artifact, result and console hashes against snapshot.json and the
fresh evidence. The registry has exactly one matching standalone_idle entry;
reset/read0/write9, waits3/5 and later apply groups match the unchanged comparator.
No findings. Both dynamic/static39-suite runs and51directional/seven legacy
standards pass; grammar334/334 and scanner119/119 across2519ICL files pass.
The full command is waiting only for its editor integration suite.

Final full regression completed on2026-11-14 with exit0, including editor6 in
178.871s: build/standalone-idle-reviewed-full.log/.exit. All39 generator suites
ran once dynamically and once statically;51directional/seven legacy standards
pass. Native/CLI sanitizers, style/naming, scale checks and independent product/
certificate review pass. Strict coverage and later-change Rocky8 qualification
remain unmet/unrun; other outstanding model requirements remain required.
