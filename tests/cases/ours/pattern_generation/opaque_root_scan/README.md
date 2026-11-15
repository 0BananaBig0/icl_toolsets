# Opaque root scan-interface probes

Self-constructed from IEEE1687-2014 6.4.5,6.4.6.2,6.4.16 and7.9.12.
The unsourced ScanOutPort conceals real storage; it is not a wire from si.
DefaultLoadValue establishes an8-bit stable reset configuration. Cases exercise
stable payloads, a caller-specified length after a non-stable access, grouped
binary data and access without an invented implicit reset. Vendor qualification
and retargeting support must be recorded separately.
The visible_* files form a separate reference-only model with one explicit
ScanRegister. Tessent2026.2 exports visible_stable successfully; all three opaque
cases fail with an unconfigured scan-path diagnostic and -stable is ignored.
Prepared input hashes and raw logs are retained under Root build/opaque-root-reference-remote
and build/visible-root-reference-remote. These are probes, not certificates.
The dedicated opaque path remains separate from modeled scanning. The visible
fixture also has a static modeled chain; ../modeled_root_scan exercises native
modeled replay and later ordinary register accesses.
