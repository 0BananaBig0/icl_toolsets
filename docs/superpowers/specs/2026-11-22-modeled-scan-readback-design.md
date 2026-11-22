# Task150: standalone modeled scan readback

## Intent and scope
Continue inventory item6: --model_readback returns actual modeled ScanRegister and scan-accessed DataRegister observations, sparse history and per-bit mismatch status through existing standalone/client serial paths. Defaults and schedules remain unchanged. Bound TAPs, raw scans, merge mode, timed histories, callback execution, interface/chain histories and real-device transport remain open. Existing topology, event-work and clock guards stay in force.
The user delegated implementation choices and opted out of further confirmation. The approach below is the implementer's choice.

## Approaches
Sample the existing concrete replay's serial-return wires before each shift edge and decode established chain order and inversion. This reuses timing/topology semantics. Capture snapshots miss downstream reset/enable corruption; a second simulator duplicates existing code. Choose replay sampling.

## Contract and integration
Pass General Solve's optional observation callback into Replay. Retain PioObservation as a compatibility alias of generic Observation with the existing vector<BitReference>, string signature. Delivery is tentative until Apply succeeds; publish no partial failed Apply.

For each selected path, calculate register intervals in serial return order. Collect only directly selected read registers and physical capture registers used by selected DataRegister comparisons. Each interval records offset, width, suffix inversion and destination string. After input Drive and ClientControls(shift), before Clock(rising), batch-read active wanted outputs with scan=true. Decode known bits using suffix inversion, preserve x, and place returned LSB-first bits in declaration/MSB-first order. Advance cursors monotonically; ignore unequal-lane trailing read clocks. With no observer skip all observation sampling/allocation.

Add the logical target Key to CaptureComparison at its single creation site. Project selected comparisons from observed physical scan cells to logical DataRegister/DataOut bits, applying route inversion. Emit direct ScanRegister logical bits too. Keep post-replay PIO observations only for parallel frames; serial observations use returned bits. The adapter retains first observation per requested bit within one Apply, latest history across Applies.

Allow ScanRegister and DataRegister storage in ModelReadback, retaining TAP and operation guards. Migrate both old note forms to 'modeled register/PIO readback; no live device'; update help/assertions. No history query keeps unsupported networks lazy.

## Acceptance
Tests: simple/inverted serial chains, declaration directions, sparse aliases, unknowns, count/clear, read-before-write, multiple Applies, DataRegister projection, unequal lanes, client controls, reset-held downstream corruption, setup frames, atomic failures and default/no-query controls. Reuse fixtures/native binary; new maintained Python tests under tests/src/pattern/.

Bookkeeping is linear in path intervals plus sampled lane ticks and observed widths, with no full path scan per tick or dependency. Benchmark equivalent parent/current single-chain and larger multi-register inputs with/without history queries; report scale/time/limits.

## Verification and delivery
Focused red/green; existing readback/scheduler tests; required native/sanitizer and dynamic/static regressions; affected format/tidy/strict compilation; packaging. One fresh independent final review and one verified fix pass. Update current docs and inventory without closing broad item6. Commit, merge into parent master, push, detach reused worktree, delete this task branch. Preserve stashes, other worktrees and unrelated ledgers.
