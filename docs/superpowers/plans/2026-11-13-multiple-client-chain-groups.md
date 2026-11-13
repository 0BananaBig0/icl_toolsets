# Named chain groups on independently selected clients

Task30 continues the approved multiple-interface scheduling requirement inline,
after Task29 verification. Reuse its per-client selection and common hardware
state. Each selected client's named chains must capture/shift/update together;
other clients remain disabled. IEEE6.4.16(q) still forbids duplicate ports among
chains within one interface.

Normalize each top-level client's routes into declared SI/SO pairs. Keep select
membership and per-frame client selection unchanged. Each chosen client enables
all its output roots, and the selected-root count equals that client's chain
count. Existing per-input length alignment, event solving, lane emission and
concrete shared-state replay remain responsible for the transaction.

A group can have several different declared inputs, so group-level input
membership alone is insufficient: swapped SI paths must fail. Propagate a
symbolic declared-input tag from each active root through active DAG edges;
each reached physical input must match its tag. Root tags are conditional on
the selected client because one physical output may have different declared
inputs in different clients. Reconvergent active paths with
different declared inputs become inconsistent. Different clients can still
share storage because only one client is selected per serial frame. This adds
O(vertices+edges+route declarations) constraints per frame, with existing graph,
solver, event and reset-work limits. No path enumeration or independent state
copies per client.

Retain the existing single-client named-chain path and all its golden behavior.
ShiftEn-only client selection, multi-TAP operation, intermediate storage taps
and arbitrary independently gated/serial-derived events remain separate work.

- [x] Add failing CLI/native cases for unequal chain lengths, vector lanes,
  different group sizes, shared storage across clients, read/write ordering,
  aliases, reservations, reset prerequisites and together obligations.
  Include wrong declared input, disabled output and duplicate-chain-port errors.
- [x] Normalize route membership, bind all selected roots, and constrain
  root-to-input correspondence through the shared DAG. Preserve physical replay,
  deterministic original/flat/generated results and failed-output protection.
- [x] Measure a representative multi-client/multi-chain workload. Run focused,
  full dynamic/static, leak-enabled sanitizer, style/naming and independent
  review checks. Qualify vendor behavior where supported and record limits.

All steps passed. Full dynamic/static make check exit0 is recorded in
build/multiple-client-groups-regression.log/.exit; the guide records focused,
sanitizer, scale and vendor evidence, including the corrected native reservation
test contract. Independent review found no Important/Critical findings.
