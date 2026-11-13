# ShiftEn-only selectable standalone clients

Task31 continues the approved interface-selection requirement. IEEE1687-2014
6.4.16(e)(2) permits a client with ShiftEnPort and no SelectPort, while
6.4.16(m) requires unique selection. Section6.4.6.11 explains that absent
SelectPorts follow the active path; do not invent a persistent select pin.
ShiftEn must be asserted only in shift phases (§6.4.6.3).

Reuse per-frame client selection, declared route ownership and concrete replay.
For clients without SelectPorts, use their ShiftEn membership to determine
whether one client can be selected while others are disabled. A fallback
ShiftEn pin shared with a selected explicit-select client must also be driven;
account for all owners before proving uniqueness. Preserve phase qualification:
fallback ShiftEn is zero during capture/update/idle and selected during shift.
Keep explicit SelectPort behavior unchanged. Existing implicit selection in
the shared event/control evaluator remains responsible for missing SelectPorts.

Touch scheduler construction, symbolic phase pins and selection constraints,
concrete phase pins, native assertions, CLI tests, shared Make registration and
vendor fixtures/evidence. Expected scale is tens of clients within existing
graph/solver/event limits. Membership construction and phase assignment remain
linear in declared memberships; avoid a second state model or per-phase scan
of all declarations. Measure16clients and verify no single-pair regression.

- [x] Add failing CLI/native cases for separate ShiftEn clients, named groups,
  mixed selection types, vector lanes, shared selectors, reservations,
  reset prerequisites, readback and atomic failures.
- [x] Implement fallback membership and phase-qualified pins using existing
  scheduler helpers; prove inactive clients cannot be selected accidentally.
- [x] Run focused and full dynamic/static checks, leak-enabled sanitizers,
  naming/style and independent review; obtain a Tessent fixture certificate
  if accepted and report the exact qualification.

All steps passed. Full regression exit0 is recorded in
build/shift-enable-clients-regression.log/.exit. Native/new11/old17+11
leak-enabled sanitizers, naming/style and independent source/certificate
review pass. The guide records commands, scale, vendor evidence and limits.
