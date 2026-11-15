# TCK idle cycles in modeled raw-scan programs

Task98 follows the frozen Task97 snapshot. IEEE1687-2014 §7.9.17 permits idle
clock cycles between operations. The existing Generate iRunLoop handler parses
known64-bit cycle counts, invokes ScanScheduler::Idle and preserves the requested
count in output. Runtime rejects waits with unapplied setup operations. Idle
already replays native rising/falling edges transactionally, maintains reset/fill
history and uses complete-state fixed-point acceleration for large counts.

The current global modeled-raw whitelist rejects every iRunLoop before that
handler can run, including otherwise qualified static root/child scans. Remove
only this unnecessary command barrier by adding iRunLoop to the existing whitelist.
Keep topology qualification, reset requirements, pending queue rejection, clock
selection/count validation and all native Idle logic unchanged. Genuine opaque
root lowering retains its separate command restrictions because it has no native
state/clock model. System-clock/time/independent-TAP support is not widened.

Reuse the existing handler and native Idle implementation; a second wait engine
or a raw-specific no-op would lose event/state semantics. No public API, model,
dependency, new budget or solver change. Runtime/memory stay bounded by existing
per-edge limits and fixed-point proof, independent of large literal idle counts
once a stable complete state is reached.

Verification: root and child raw scans followed by0/one/many TCK cycles and later
ordinary reads/writes; repeated raw scans around waits; explicit and implicit local
controls; partial-X controlling fill survives idle shift-shadow invalidation;
source/flat/generated/local-reset replay; large64-bit count retains output and
completes via existing stable-state proof. Pending raw setup, unknown/invalid
counts, unsupported system-clock targets and genuinely opaque waits remain atomic
failures. Native literal idle edges compare full state/fill to optimized Idle.
Record source-matched vendor acceptance/limitations and representative timings.
Independent review and immutable leak/fullserial/remote gates precede integration.
