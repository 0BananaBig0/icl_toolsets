# Native replay for already selected TAP data paths

Task112, implemented after Task111 merge/delete. The frozen Task111 source and its gates
remain immutable. User delegates routine choices and asks completion without
confirmation; executor selects the approach below. No implementation before
Task111 is merged and its branch deleted.

## Intent and evidence

The full-path raw TAP feature delegates to ordinary symbolic event planning.
An already selected64-register256-bit chain exceeded120 seconds in the prior
ordinary planner and15 seconds in the candidate probe. The same256 bits across
16 registers took3.28 seconds. Production-scale scheduling requirements call
for concrete replay and reuse of open paths, rather than event-expression
explosion for a transaction whose inputs are already fully known.

## Approaches

1. Simplify or constrain all symbolic event expressions. This preserves joint
planning but changes the general event solver and requires a separate measured
investigation; it still constructs per-tick symbolic histories.
2. Construct a candidate for an already selected, known complete TAP data path
and replay it through the existing TapExecution/Cycle machinery. This mirrors
TryNativeScan for non-TAP clients and leaves joint planning as fallback. Chosen
by executor because the failing geometry needs no retargeting or unknown data.
No new TAP FSM, event engine, serial walker or solver is introduced.

## Scope and architecture

Add a conservative private native TAP data attempt before NeedsScanEvents in
the existing optional native dispatch. Retain Solve's current PDL validation,
public synthetic-target rejection, grouping/connectivity and DesiredPreserved
before dispatch. One known-ready bound scalar root TAP; currently selected
instrument data path; no IR phase or synthetic builtin permission expansion.
All required registers must be in the path, all full loads from existing goals
or WriteFill must be known0/1. Existing gate excludes primary/DataRegister,
end-state and unsupported access groups. Unsupported reset/control/side-effect
cases return nullopt to the original planner without changing live state.

Reuse Task111 RawDataPath/TraceSerialPath, offsets and parity conventions.
Construct one full-path ScanStep with exact full load/write maps, requested
read masks, physical serial SI/SO, bound endpoints, instruction=false and
existing together semantics. Keep public signatures unchanged. Native attempt
receives already resolved goals/expected/required/preserved/desired maps.

Extract existing concrete TAP cycle block from Replay into one helper consumed
by both symbolic and native replay. Keep leading100, final shift exit,11/0
update/idle timing and writable/reset-held determination identical. Helper
accepts existing lane/input descriptors and reservation callback; it must also
allow native physical TDO mask checks at the sampling point before each shift
rising edge. Native checks are observational; never substitute requested SO
for captured hardware. Reuse NetworkState ReadBits scan mode and TapExecution,
with exact sample timing verified against independent literal edge tests.

Run speculative work only on copies of NetworkState/TapExecution/fill. Reuse
CheckPreserved through each real edge and its old-to-desired transition rules.
Require actual per-path writes equal requested loads and requested reads match
physical output. Conservatively fall back on off-path control/reset histories
that would require unimplemented fill bookkeeping; do not ignore them. Set
loaded-path fill from proven actual loads, preserve other fill, adopt complete
trial execution only after every check. If direct reset bookkeeping cannot be
reused cleanly, exclude direct resets from the native attempt; original joint
solver continues to support them. No new public rejection may replace fallback.
Raw exact-one-CSU proof and pre-CSU assigned sets remain unchanged and exercise
this same ordinary native attempt.

## Performance and checks

Expected work bounded by width times concrete netlist traversal, not symbolic
expression growth; retain existing event-work/structural limits without raising
budgets. The main acceptance is64 registers x4bits completing within15 seconds
on both measured platforms, with baseline120s timeout retained. Measure matching
flags, full artifacts, peak RSS and source/flat/generated replay. Cover also
16x16,64x1,1024 unrelated registers,512 forwarding aliases and128 packets.
Keep4096 inherited resource expectations unless source proves the existing
budget accepts that exact geometry; do not silently broaden guards.

TDD: maintained timeout regression must fail on saved Task111 binary, then pass
on candidate, plus literal pin/state/masked-capture checks. Verify unequal widths,
ascending indices, all SI/inter-register/SO parities, partial fill, owner refresh,
protected failure/retry and copy state. Test wrong physical SO, unknown fill,
inactive/direct-reset/off-path-event fallback, instruction preparation and
builtin/public-target restrictions. Existing IR/DR/builtin/multi-DR/native and
full leak-enabled SAN/remote/style/vendor gates remain required. New scripts
under tests/, no dependencies, no RTL. Independent builder and final review,
commit/ff master/delete completed task branch before starting later work.

Builder check: native_tap_data_builder_check reports nothing important left.

Independent final review: no Critical/Important/Minor source findings. One
conservative wording ruling: the existing ControlEvents log omits off-path
update-to-X. Complete copied hardware state is still adopted and existing
unplanned non-reset controlling fill is preserved, matching original behavior;
such an unlogged physical side effect alone need not force fallback. Logged
off-path/reset histories still do. Qualification gates remain required.
