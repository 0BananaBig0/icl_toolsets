# Hierarchical static root scans

Next bounded increment after Task95. Goal: allow a root iScan over the complete
static modeled chain when registers and forwarded scan/control/reset ports live
in nested instances. The emitted operation still targets the explicit root client;
child-interface retargeting and partial-chain payload insertion remain separate.

Current evidence: RawChain rejects every nonroot scope, counts all explicit client
interfaces as duplicates and forwards only aliases. Native serial graph/state
already follow bound ports. TapExecution::RegisterInterfaces associates registers
only with local interfaces; EffectiveScanControl uses those local controls and
fallbacks. Therefore merely removing the scope/count checks would accept undriven
child controls. Existing ClientControls samples all declared interface clocks and
performs native capture/shift/update; retain it unchanged.

Choose typed forwarding qualification plus the existing bulk path. Flattening into
a second model would duplicate identity/reset/state handling, and a full serial
solver would reintroduce per-bit wide-vector work. Keep the original netlist and
names for state and later ordinary accesses.

Boundary: one explicit root non-TAP single-chain client, all modeled ScanRegisters
on one complete root-SI/root-SO path, pure aliases and scalar forwarded scan/control/
reset ports only. Nested client interfaces may be explicit or implicit; no host,
parallel-chain, mux, DataRegister, extra storage, conditional SO or end-state
widening. Root physical pins remain direct and unique by function. Nested declared
input controls/clocks require actual Source bindings, not native undriven-port
fallbacks. Reset forwarding includes existing elaborator-inferred parent reset
connections; qualification checks those actual Sources. Omitted optional ports
keep their existing implicit model semantics.

Qualify every forwarded control/clock to the corresponding physical root function
with positive total polarity. Qualify every register reset to a root ResetPort or
supported virtual reset through pure forwarding; retain actual inactive-reset
checks after explicit iReset. No storage-, serial-, gate- or unknown-clock-dependent
control/reset path qualifies. Native phase checks still require each actual capture,
shift and update event. This establishes invariant controls while shifting; it does
not establish electrical timing or CDC signoff.

Trace scalar ScanInPort/ScanOutPort Source links in the existing iterative serial
walk, preserving accumulated inversion, actual register rightmost bits, bit-key
cycle checks and all-register coverage. Cache forwarded terminals/parity for
control/reset qualification so repeated nested paths are not rescanned for every
register. Keep register load slicing, native ShiftComplete, final SI, partial-X
fill, publication atomicity and original local-reset operation order unchanged.

No changes to ownership, merges, mixed pending accesses, clock operations, TAP/IR,
opaque networks or pattern lifecycle. Root raw scans require an explicit reset,
exact modeled length and the existing1Mbit/4096register/graph resource bounds.

Verification: one/two-level and repeated parameterized leaves; serial aliases and
inversion across bindings; explicit/implicit nested clients; ordinary reads/writes
and procedure calls after root raw scan; original/flat/generated/local-reset forms.
Reject unbound/inverted/foreign controls or clocks and shifting-dependent resets
atomically. Native literal per-bit comparisons and state/fill/rollback assertions;
source-matched vendor export and wide/many/deep measurements. Required independent
review, sanitizer/native/allCLI/fullserial gates precede feature commit, integration
and completed-branch deletion.

Independent builder review accepts the boundary. It requires structural rejection
of declared unbound child clocks/controls despite native fallbacks; exact scalar
Source widths/indices, no Enable/computed behavior, exact physical root identity;
positive control/TCK parity, separately composed active-low reset parity with the
actual inactive check; role-independent terminal/parity cache and per-use role
validation. CaptureSource may remain storage-dependent because capture occurs
once before complete overwrite. Tests include native-auto-driven unbound child
TCK, wrong root identity and correctly forwarded active-low reset.
