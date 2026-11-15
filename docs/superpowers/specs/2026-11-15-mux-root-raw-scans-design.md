# Root raw scans through gated binary SIB routes

Task100 follows frozen Task99. Extend modeled root iScan to a known active path
through binary ScanMux/SIB routing with properly gated descendant interfaces.
Do not merely remove the all-register restriction: bypassed registers can still
receive clocks and update, as hidden_client/network.icl demonstrates.

Reuse native capture/bulk-shift/update and concrete Match/source graph selection.
Cache immutable topology qualification, then derive the ordered parity-bearing
route from current held state at normalization and Apply. Keep the existing static
path cache and static child behavior. Dynamic requests are root-only; child
retargeting through a mux remains explicitly unsupported in this increment.
Require an explicit reset and known, unambiguous route before dynamic setup.
The supplied length must match the current active path. After a nonstable root
scan changes the route, subsequent modeled access uses the resulting known state.

Define binary precisely: a scalar selector with unambiguous0/1 choices.
Allow these ScanMux nodes whose selection is held update-register data,
constants or pure forwarding. Qualify the dependency path explicitly: serial-valued
ports, shift storage, phase-varying controls and mux-derived selectors do not count
as held data. SelectPort forwarding may use the same held data gating; CE/SE/UE/
TCK/reset retain pure root forwarding. No DataRegister, TAP, parallel chain,
conditional SO, arbitrary logic or internal-reset widening. Keep existing graph,
register and1Mbit active-path bounds.

Freeze one selected route for the complete CSU. Obtain full native capture,
shift and update control maps; every loaded register must receive the required
phase, and every omitted register must have zero effective capture/shift/update.
Reject bypassed-but-active storage atomically. Use full maps for native capture
and update. After proving omitted shift events are zero, pass only loaded shift
controls to the existing ShiftComplete API, whose load/control sets must match.
Sample update controls before the common update edge, so opening does not update
a previously disabled child and closing still updates a previously active child.
Preserve omitted held state and controlling fill; native shift-shadow retirement
is allowed and must match literal replay. Resets remain inactive/invariant.

For -stable, trial replay must leave the same ordered route/parity and eligible
phase controls as before the scan. A closed1-bit SIB scan loading1 opens a9-bit
route and must reject -stable. Opening may use a nonstable1-bit scan followed by
a stable9-bit scan. Validate this before adopting trial state/fill or publishing
output. Do not infer stability from unchanged width alone or from the flag itself.

The alternative one-step solver adds unnecessary work for this bounded linear
CSU and risks wide-vector cost/preparatory scans. Prefer conditional bulk replay
with explicit proof. Target linear work in modeled topology plus active payload;
normalization must not repeatedly rescan inactive storage per setup assignment.
Cache shared forwarding/selector identities, and measure shared selectors/aliases
rather than assuming their cost. Existing static1Mbit performance must remain
qualified. Use representative wide open paths, many gated registers and repeated
open/close sequences; record exact commands/time/RSS/hash comparisons. No speedup
claim without equivalent baseline.

Verification: closed->open->write->close->reopen with retained child data; held
selector versus changing shift bits; inactive common-control negative; unknown
selection and wrong length; invalid stable rollback including equal-width different
routes; source/flat/generated/local-reset replay. Native literal per-bit reference
compares every register and relevant pin, inactive state/fill and update ordering.
Existing static child/default/opaque behavior and strict certificates must remain.
Vendor probes must preserve genuine limitations. TDD, independent review and all
required immutable gates precede merge and completed-branch deletion.

Independent detailed builder review approves the division: immutable qualification
in RawChain, state-derived ordered/parity route helper, separate dynamic-root
normalization, one frozen route per ApplyScans and transactional full-map phase
checks in ReplayScan. Legal pure DataOut/ToSelect forwarding may be included only
with explicit held-data qualification; do not accidentally admit arbitrary ports.

Required refinements: check inactive resets for ALL modeled registers, not just
the current route. A post-update stability proof runs on another NetworkState
copy, so its ClientControls calls cannot alter committed pins/events. All failed
phase/stable checks precede state_.Restore/fill_ adoption and preserve shift
shadows, updated_, driven pins and event histories as well as held data. Add an
inactive-reset-active negative and a failed-stable valid retry with partial-X fill.
Measure shared selectors explicitly: Read creates a new evaluator and may copy a
wide register repeatedly, so cache proven forwarding identities and read selector
registers once per route derivation if the measurements require it.
