# Resource reservation fixtures

Self-constructed ICL networks for IEEE 1687-2014 §§7.9.19–20, using a protected
SIB and downstream TDR. The Python runner supplies ownership, pending-write,
release, nested-call and merge scenarios. These are not vendor-qualified goldens.

`vendor.icl` and `vendor_{persistent,default,release}.pdl` are self-constructed
TAP-backed probes using `../access_link/simple.bsdl`; `oracle_cases.json` prepares
them through the standard comparison runner. They are local-regression inputs
and freshly executed Tessent 2026.2 references, not equivalence goldens. The
select-qualified child must not receive updates while bypassed. Strict trace
comparison retains bitwise assumptions and TAP instructions; the release case
also has a differing final SIB value. See `docs/tessent-oracle.md` for evidence.
