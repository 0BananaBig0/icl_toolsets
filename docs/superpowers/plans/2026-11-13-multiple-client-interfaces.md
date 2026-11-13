# Independently selected standalone client interfaces

Task29 continues the approved shared-controller completion work inline.
IEEE1687-2014 6.4.16(m) requires each interface to be uniquely selectable;
6.4.16(q) forbids reusing a port in multiple named Chains. Preserve those rules.

Support multiple top-level standalone client interfaces with one SI/SO pair
each and explicit SelectPort controls. Reuse the existing route DAG, symbolic
event transitions, concrete replay and shared hardware state. One interface
is selected per serial frame; other interface select controls are inactive.
Shared internal registers are modeled once, and actual capture/shift/update
events determine their changes. Do not concatenate independently planned states.
Existing single-interface named chains retain their simultaneous behavior.
Multiple TAPs, multi-chain/multi-client combinations and intermediate register
taps remain separate work.

Record top-level client SI/SO/select membership during construction. Share one
symbolic selection per client and frame across route selection and phase pins.
Require exactly one selected client in serial frames, and validate the reached
input against that client's declared SI. Shared select pins are driven from
their selected memberships, with constraints proving every other interface
disabled. Preserve common clock/phase events and hidden-storage planning.
Concrete replay decodes the same selection before driving pins in each phase.
No additional solver or execution backend.

Expected workload: the existing 4096-register/65536-vertex bounds, with tens
of interfaces in representative tests. Membership and route processing should
remain linear in declarations plus DAG edges per frame; existing solver/event
work budgets and fresh-state fallback remain. Measure a moderate multi-client
case and report resource limits without claiming unreachability.

- [x] Add failing native state/lane tests and CLI cases for independent clients,
  shared storage, unequal/vector paths, inverted paths, reads/writes, aliases,
  reservations, reset dependencies, declared-input mismatch and unselectable
  controls. Verify original/flat/generated replay and atomic failures.
- [x] Extend frame selection, client pins, input constraints and concrete replay
  through existing helpers; keep existing single-client results unchanged.
- [ ] Run focused dynamic/static tests, full regression, leak-enabled sanitizers,
  style/naming and independent review. Probe Tessent where supported, record
  the exact evidence, and update the model/requirement ledger.

## Reset and review findings

Selection changes must settle asynchronous resets before route/control/capture
evaluation. Match concrete simultaneous reset passes; a single topological pass
loses transient assertions. The scan reset DAG depth plus a conservative count
of reset-bearing data registers bounds the passes. Constructor and per-horizon
work are capped at1,048,576 bit/object units; exhaustion reports a resource limit.
Each definite assertion updates controlling fill unless a later successful
write replaces it. Unknown reset does not create a known controlling write.
Parallel frames retain held selection pins.

Seventeen CLI cases include both review reproductions, transient DataRegister
history with initialized/unknown sources, and a512-stage resource-limit case.
Native state checks cover actual SI/SO streams, capture comparisons, selector
pins, untouched input lanes and rollback. The corpus initially rejected repeated
fixture module definitions; use distinct module names. The original full run
failed that policy, and the next run was stopped for a required small extraction
of data-fill verification after Solve exceeded the800-statement lint threshold.
Final full regression passed in build/multiple-client-regression-complete.log
and .exit(0), including42 directional standards and seven legacy standards.
Native/new17/existing9+8 leak-enabled sanitizers and naming/style/review passed.
Neither earlier run is completion evidence.

Verified evidence: [generator guide](../../pattern-generation.md#independently-selected-client-interfaces-2026-11-13).
