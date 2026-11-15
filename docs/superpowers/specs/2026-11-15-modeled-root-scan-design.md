# Modeled root raw scans

Next increment after verified Task93. Its opaque topology and existing branches
remain unchanged until the required gates and task integration finish.

Goal: execute iScan against a single static modeled root client and preserve
native register state for subsequent ordinary iRead/iWrite on later Applies.
Raw expected SO remains comparison data, never simulated hardware readback.

Approaches: per-bit native replay is straightforward but copies full vectors on
every shifted bit, giving quadratic work for a wide register. A second simulator
would duplicate reset/control/update rules. Choose native capture/update/idle plus
an explicitly proven full-chain bulk shift, reusing existing SetRegister and the
native event and controlling-fill history. No solver invocation per raw bit.

Supported boundary: one explicit root non-TAP client, scalar direct root controls,
unconditional rightmost-register SO, and every modeled register in one static,
non-inverted complete SI-to-SO chain. No hierarchy, muxes, aliases, DataRegisters,
extra modeled storage, internal reset cones, computed control or clock gating.
Require a preceding explicit iReset, exact total modeled width and supported
length1..1048576. Initial reset is never invented. Resources, merge, state, clock,
override and mixed raw/modeled accesses in the same Apply remain rejected for
this increment. Sequential raw and ordinary modeled Applies are allowed.

Algorithm: validate/canonicalize every queued request immediately; validate the
immutable topology once. At Apply, copy native state, run existing client capture
controls and rising Clock. One native shift Clock settles updated_ bookkeeping
and clock pin state; every register's final shift vector is then replaced with
its full SI slice, retaining its update vector. The complete static path and
fixed direct controls prove that omitted intermediate shift edges cannot affect
reset, capture, update, routing, external modeled state or events. Drive the final
physical SI value. Native falling update and idle handling then produce values,
events and fill history. Adopt the trial only after all checks pass. A failure
leaves state and queued publication artifacts untouched.

For path registers walked from SO to SI, assign rightmost SI payload slices first;
register vectors retain declaration order, including ascending numeric indices.
SO expected bits are output as supplied with no native state assumptions. Known
update/reset events adjust controlling fill using the same rules as ordinary
scheduler replay. Main preserves source operation order for local-reset replay.

Verification: native comparison against true per-bit replay for small unequal
register chains, ascending indices, X inputs, repeated scans and resets; failed
length/unsupported-topology rollback; raw-to-read/write state observations; all
source/flat/generated/local-reset forms; atomic unsupported combinations; modeled
vendor reference with actual input hashes; 65536/1048576-bit runtime and memory.
The native proof must include shift/update shadow bookkeeping and pin end state.
Full sanitizer/native/CLI and serial make check remain required before integration.

Independent review accepts the structural proof with direct inactive reset
validation, final SI retention, a native bookkeeping edge, partial-X fill merging
and initial exclusion of every iApplyEndState directive. Add production
NetworkState::ShiftComplete(loads, controls) as the actual bulk operation so its
entire snapshot/events can be compared to per-bit replay. It validates every
load/control/reset before mutation, then uses native Clock and SetRegister. The
scheduler still proves invariant complete overwrite and uses a transaction copy;
the primitive alone cannot establish those structural prerequisites.
