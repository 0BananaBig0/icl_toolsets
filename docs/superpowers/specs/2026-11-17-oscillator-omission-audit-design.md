# Oscillator omitted-source audit

Record the outstanding factor-only oscillator question using current source,
IEEE1687-2014 and seven isolated Tessent2026.2 probes. Documentation only: retain
the native explicit-source guard and all production/test/build behavior.

IEEE7.9.15b and7.9.16c-e, printed174-175/PDF188-189, establish oscillator roots
and optional fields/defaults. They must not be presented as a prohibition proved
by vendor failure. The v2023.4 iClockOverride page describes different omitted
field behavior; identify its release and distinguish measured2026.2 behavior.

Reuse the existing unchanged oscillator override driver/model, in fresh owned
remote/tmp directories and patterns-ijtag/analysis with an open pattern set.
Fresh explicit-source control succeeds. Multiplier-only2, divider-only3 and
factor-only after a source override fail at the subsequent fresh iClock, with an
undriven-source diagnostic. An optionless override after an explicit source
fails at iClockOverride syntax, not at iClock. Do not conflate failure stages.

All21 fetched input hashes must match submitted files. Preserve raw exit1
negative results, logs and absence of exports, alongside successful control
exit0/export. These are bounded probes, not formal directional certificates or
general model equivalence. No new maintained script: the existing Tcl driver is
copied unchanged; new PDL files are scratch input fixtures and Python helpers
are evidence orchestration. Existing native oscillator rejection method runs
dynamic/static/leak-enabled SAN; no unrelated suite reruns for docs-only work.

Change only generator guide/model-coverage docs and this plan/spec. Record exact
scope and the distinction between the native early error and vendor late error.
All3994 runtime inputs and20 local/remote product fingerprints remain unchanged.
Pre-portability Task120 raw96.5%/62.7% remains historical; strict100%/90% and
broader model/vendor gaps stay open. One independent final review, direct-parent
merge master/delete task branch/archive/sync, no push. Parentcee5dcf.

Adaptive stage check: two additional no-refresh cached-wait patterns succeed
in Tessent2026.2, with child4/child4 and child4/root12/root12 traces. The native
tool currently rejects both early (exit2); record this remaining compatibility
gap explicitly. Do not implement it within this documentation-only audit.
