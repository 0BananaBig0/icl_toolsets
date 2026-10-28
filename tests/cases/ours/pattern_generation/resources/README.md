# Resource reservation fixtures

Self-constructed ICL networks for IEEE 1687-2014 §§7.9.19–20, using a protected
SIB and downstream TDR. The Python runner supplies ownership, pending-write,
release, nested-call and merge scenarios. These are not vendor-qualified goldens.

`vendor.icl` and `vendor_{persistent,default,release}.pdl` are self-constructed
TAP-backed probes using `../access_link/simple.bsdl`; `oracle_cases.json` prepares
them through the standard comparison runner. `vendor_close_reopen.pdl` adds two
leaf writes, each with same-update SIB closure and intervening reopening.
All four cases have forward and reverse Tessent 2026.2 certificates under
`tests/golden/tessent_roundtrip/`. The select-qualified child must not receive
updates while bypassed. Apply boundaries, resolved values and TAP instruction
operations are retained; independent writes in one apply may reorder. See
`docs/tessent-goldens.md` for current evidence and `docs/tessent-oracle.md` for
the historical independent-source comparisons.
