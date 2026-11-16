# Native replay for qualified ordinary root accesses

Task105 addresses the measured ordinary-planner bottleneck left by Task104. A
write to a visible SIB in a modeled network with one million storage bits currently
hits symbolic reset/event budgets even when the active preparation chain is one
bit. A1024 distinct-SIB batch also hits the existing event budget. Raw native replay
already executes those qualified complete routes with exact eligibility and
per-phase preservation. Reuse that engine for ordinary accesses whose required
registers are already on the current qualified root route. No budget increase,
new solver, state evaluator, clock/reset semantics or hidden preparation sequence.

## Approach and qualification

Keep the existing ordinary solver as fallback. Add private TryNativeScan after
ordinary target/value/alias/AccessTogether/ownership parsing, before expensive
symbolic frame construction. Use RawTemplate/RawChain, current profile offsets/
parities and existing ReplayScan. Early skip TAP, primary/mixed/data-register,
end-state, non-root-compatible topology and nontrivial grouping. Unknown route,
unavailable target, unknown controlling fill or failed native eligibility returns
to the existing solver without hardware/fill mutation. No selection of a new path.
Native success is one fully replayed CSU; emitted ordinary ScanStep retains
logical iWrite/iRead, source ordering, comparisons and caller behavior.

Inputs are parsed full-register goals, expected masks, required register IDs,
preserved/desired-preserved masks and ordinary flags. Require every goal/expected
register on the active profile. Start physical SI from current controlling fill
and replace explicit goal data; require concrete0/1 loads for every active R.
Unknown ordinary write bits retain the existing controlling-fill semantics and
fall back when a concrete fill cannot be proved. Expected SO remains observer
data, with per-register suffix parity separate from SI path parity.

Preservation must match the existing ordinary contract, including sliced/aliased
writes and explicit_writes=false automatic preparation. Only source-authorized
bits may change; surrounding protected storage stays fixed through each phase.
Do not treat the whole physical root as an authored assignment. Existing
DesiredPreserved supplies the exact intended masks; use it in proof, or establish
that any reused raw desired mask is identical before replay. Port/non-ScanRegister
reservations fall back, rather than inheriting raw's storage-only rejection.
Unwritten takes do not become protected from automatic fill or reads.

Construct the entire ScanStep and maps before adoption: input/output names,
physical serial/serial_expected, active loads/writes and original expected masks.
Then replay once with existing native CSU proof. Keep transient control/reset-event
handling consistent with ordinary Apply. For qualified all-scan, inactive-direct-
reset routes and concrete loads, its start/end draining matches raw replay;
ordinary fill becomes active loads and inactive fill is unchanged. A failed attempt leaves native state/fill and output unchanged
(valid pre-state qualification caching is allowed) and the fallback sees original
state. Never try the fast path recursively from its own fallback.

## Verification and acceptance

TDD against frozen Task104:1M model with iWrite open1/iRead open0 before raw1M,
ordinary standalone visible-register accesses, and1024 distinct visible SIB writes.
Maintain small-step physical Native NetworkState references for prepared opening
and static/inverted/ascending aliases, partial masks, SI/SO parity, self/external
capture sources, reads and retained surrounding state. Prove fallback for inactive
targets, unknown fill/selector, unsupported controls/models, ports, grouped/end-state
operations and explicit_writes=false ownership conflicts. Existing unknown-input,
reset/call/merge/clock/owner/trace tests remain. Source/flat/generated/local-reset
forms and prior pattern/reset/debug artifacts stay consistent; diagnostic log
continues recording errors.

Measured existing limits and reference commands are preserved under mixed-root-
raw-{scale,scale2,probes,limits}-remote. Target linear native work in modeled state
plus active payload; preserve1M raw active bits,4096 R,65536 graph vertices and16M
route traversal limits. Measure one million bits,1024 distinct preparations,
partial protected fields,128 repeats and no-regression small networks. Compare
equivalent logical output where legacy succeeds; where it rejects, use independent
literal state/packet evidence and clearly distinguish manually authored raw
references from byte-equivalent ordinary output. No unmeasured speedup claim.

Root Task104 frozen28 stays immutable during final gates. No local heavy tests/
build/style during its timed run. Remote /tmp mutates only after Task104's evidence
is archived. No dependency/RTL edits; new test scripts mirror source under tests/.
Independent builder/final review, style/audit,63offline, leak/fullserial/remote
and honest vendor qualification precede commit/ff/delete branch. User authorized
continued inline work; executor selects native reuse for this concrete bottleneck.

Builder refinements: top-level Apply currently probes NeedsScanEvents before Solve
and fails the1M reset budget before any native opportunity. Make Solve event mode
std::optional<bool>, Apply passes nullopt, parse once and try native first; only
fallback derives event mode and regular-read handling. Existing internal explicit
bool calls retain behavior and fixed_loads constraints. Native dispatch only at
that top-level unresolved entry. Add private optional exact-desired override to
ReplayScan; raw defaults unchanged. Ordinary event draining already matches raw
for qualified inactive-reset/all-scan/concrete-load routes; no event-retention flag.
ScanStep.serial and serial_expected reverse the whole physical strings; SI parity
is profile.inverted!=suffix, expected SO parity is suffix. Preserve indexed endpoint
spelling, fill/load all active R, and normal together marking for write groups only.
