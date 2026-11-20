# BSDL model/default completion
Continue item11's audited BSDL portion using the bounded metadata reader and
existing shared ICL/TAP model. The user delegated implementation choices.
Finish the concrete eight audited requirement groups; item11's other source,
direction, alias, reference and corpus work remains tracked separately.

## Approach
Extend the reader's canonical identities and register metadata, then use existing
ICL mux/logic construction and event replay. A second VHDL parser or device
simulator would duplicate current code and invent unspecified hardware.
Pattern lists become a conservative per-bit consensus: differing allowed bits
remain unknown, with no claim to retain cross-bit correlations or identify a
particular manufacturer variant. Shared-opcode masks instead impose simultaneous
compatible constraints; reject contradictory cared bits, and preserve unknowns
outside the overlapping opcode region. Keep the existing nonoverlapping capture
mux and constant fast paths; emit shared per-bit muxes only when required.

## Requirements and authority
IEEE1149.1-2001(R2008), B.5.1/B.6.1: instruction/register identities are case
insensitive, but ICL signals retain their case. B.8.11 (printed153–155/PDF161–163):
accumulate repeated descriptions, parse alternative captures, validate private
names. Allones BYPASS is implicit; no-IDCODE reset selects it (7.2.1e/f).
B.8.12 (155–156/PDF163–164): alternative identification patterns and IDCODE low1.
B.8.13 (156–158/PDF164–166): per-instruction CAPTURES; mandatory associations
forbid CAPTURES; private instructions may omit associations; custom register
width/capture validation and shared storage. Reused width follows B.8.13.3c;
the printed BNF requires brackets while prose permits later omission. Document
this conflict rather than claiming full BNF consistency.

A private opcode with no supplied route must remain explicitly unknown, never
unassigned-opcode BYPASS or a fabricated register width. Public accesses and
instruction scans may coexist with unused private declarations; a selected
unknown private DR fails safely. Explicit private AccessLink routes remain valid.
No full BSDL conformance or boundary-cell hardware simulation is claimed.
Do not add new dependencies, resource thresholds, or enumerate IR complements.

## Verification and performance
Extend existing native BSDL/TAP and raw builtin CLI tests: original/flat/generated
and reset replay, invalid lists/widths/captures/private references, shared exact
and partial-opcode masks, selected private rejection, output preservation.
Observe RED before implementation. Retain existing input, metadata, model and
overlap work budgets; complexity bounded by retained metadata/overlap checks.
Reuse decoded capture groups rather than per-bit copies of opcode lists.
Focused native/dynamic/static first; required full generator/native, leak-enabled
sanitizers, style/C++17, package check, independent whole-branch review.
Coverage item1 stays excluded. Vendor conformance/equivalence needs separate
evidence; parsing or native round-trip success alone cannot establish it.
