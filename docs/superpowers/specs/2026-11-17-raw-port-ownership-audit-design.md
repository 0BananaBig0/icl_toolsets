# Established port ownership during raw scans

Audit the recorded unsupported established-port reservation boundary at clean
master26ce06913f65b0011ba58b122056e09eeebb27b5. No product/test/Makefile changes.
IEEE1687-2014 section7.9.19 and Tessent2023.4 iTake describe preservation after
applied writes and (vendor) iState. Measure current Tessent2026.2 separately.

Reuse the existing qualified ReservedRawTop eight-bit ScanRegister/client model
and unchanged run.tcl driver from raw-scan-reservations-roundtrip2 evidence.
Add only scalar DataOutPorts hold (constant1) and held (source value[0])
to the probe model, recording these additions. Eight initial isolated fresh-process
programs cover baseline; selected/inactive SelectPort state; constant held
output; constant and transient-changing SI; unchanged and changing observed
register bit. Use explicit reset and valid existing PDL commands. Run current
native dynamic/static tools on identical input bytes and preserve logs/statuses
and output artifacts. Fresh vendor invocations in own lianghuaxiao /tmp have120s
limits, serial execution, submitted/fetched input hashes and completion anchors.

Separate reservation establishment failures, raw-command acceptance, later apply
failure, successful export and native rejection stages. Do not infer a general
IEEE prohibition from vendor behavior or implement a policy from these probes.
Preserve exact runtime3994/ten local product hashes and reuse unchanged Task123
full/SAN/strict-failed coverage and Task124 platform evidence by hash. No fresh
full regression/coverage/performance/certificate claims. Record bounded native
gaps and conflicts; one independent review then merge into master/delete branch.
No push, dependency, user installation, remote shared source mutation or RTL edit.
