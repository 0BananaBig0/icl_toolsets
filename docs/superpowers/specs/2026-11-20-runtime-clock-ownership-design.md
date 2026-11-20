# Bounded runtime, clock-name and ownership repairs
Continue concrete gaps within items5,6,7; none of these broad items is closed by
this increment. Reuse current C++17 runtime/history, clock mapping and raw replay.
The user delegated choices and authorized normal branch integration.

1. iClockOverride target and -source use the same associated-flat physical-name
decoding as iClock/-sck. Preserve literal collision precedence, non-flat rejection,
kind/index/prefix/origin/cycle checks and checked-version snapshots.
2. IEEE1687 8.4.1k–n/8.4.2e/8.4.3 (printed186–188/PDF200–202):
never-applied observation bits return x; a supplied partial history fills only
those bits. Applied reads without supplied observations remain unavailable,
distinct from never-read bits. Preserve aliases, inversion, formats and per-file
session history. Initial status is0 until an applied cared comparison makes
the count unavailable; do not infer samples from expected data or ResetValue.
A failed apply validation preserves both supplied histories and prior known/
unknown status. Catch/reset does not legalize invalid CLI operands.
iGetStatus-clear returns the current count before clearing; unknown counts
still require an observation provider. No device backend or full status support
is claimed. Comparison care follows the effective per-bit read queue after controlling
state and overlapping reads. Raw care follows normalized surviving interface/IR
keys, including grouped X masks; real comparisons remain unknown without readback.
3. Compatible held ports must not reject child scans merely by type. Reuse all
existing per-edge/final reservation checks; keep root-request exemptions exactly
as before. A child must reject conflicting selection/clock/output locks and
preserve state/files. Do not release or refresh port locks by implication.

No dependencies, timing defaults, scheduling thresholds or transport guesses.
Clock reuse is indexed. History uses bounded per-touched-node care strings and
one normalized entry per surviving raw key; all original operands are statically
validated before history commit. Child raw scans with cared port locks add one
linear netlist snapshot to prove all serial intermediate states invariant. If the
proof is inconclusive, existing per-edge replay enforces locks within the unchanged
one-million event-work limit; above it the operation rejects explicitly. Unprotected
scans and root exemptions keep their existing paths. No subprocesses are added.
Watch focused RED failures, then GREEN/native/dynamic/static/SAN, whole generator
regression, style/C++17, source package and one independent final branch review.
Item1 coverage stays excluded. Actual live observations, general clocks/lifecycle
and changing cross-instance ownership stay separately tracked.
