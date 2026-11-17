# Direct root raw scans with established port reservations

Implement the bounded source-acceptance gaps measured by Task125. A directly
requested standalone root iScan controls its complete qualified modeled network
(IEEE1687-2014 section7.9.12); its explicit effects may change ports with earlier
established takes. Those port reservations are neither released nor refreshed:
subsequent ordinary retargeting still sees the original constraints, matching
six successful Tessent2026.2 probes and two later-Apply failures. Serial-input
write eligibility remains unchanged. Older2023.4 iTake/iScan/manual limitations
and measured2026.2 behavior remain separately identified.

Only the existing ReplayScan concrete reservation check changes. Pass one private
boolean from ApplyScans identifying a direct request for the root client, after
existing route/length/overlap validation. Validate every original reservation
mask using DesiredPreserved before filtering. During this explicit root CSU,
exclude port IDs from the held map checked for automatic changes. Retain every
ScanRegister check, original DesiredPreserved/ResourceReservations values and
AssignedScans register-only footprint. Keep the storage-only guard for child
requests, unsupported kinds and all other backends; no public API/header/cache
change. Existing trial replay and publication preserve failure atomicity.

Update the existing modeled-root CLI methods and native reservation function:
original/flat/generated/reset replay for all five vendor-positive source gaps,
late ordinary failures, explicit release/rebase recovery, aliases and unchanged
child rejection. Native source/flat tests prove port history retained, assignment
footprint excludes ports, malformed port masks still reject atomically and
release recovers. Add a test-only --raw-reservations selector for that existing
function; default full scheduler tests retain it. Real CLI/native RED first.

Expected size/hot path stays the current raw profile (scalar ports, existing
wide register/route limits). Filtering costs at most one ordered-map erase per
protected port; existing full-shift acceleration and register masks stay intact.
No per-bit replay loop, new dependency or timing model. Verify existing wide and
reservation workloads against established limits; no unmeasured speedup claim.

Seven selected files: scheduler.cc, existing CLI/native tests, generator guide,
model ledger, this spec and matching plan. Run focused/full CLI dynamic/static,
focused leak-enabled ASan/UBSan native/CLI/main, changed-section format/tidy and
strict C++17; complete normal integration then fresh full raw coverage, serial
heavy local work. Qualify updated native and both generators remotely on current
runtime hashes; reuse old unrelated products only by exact fingerprint. Refresh
scoped Tessent normalized/export replays using the unchanged driver. No general
port/child/TAP ownership, full vendor equivalence or new formal certificate.
Strict100% line/90% raw branch gate stays unchanged and its actual result visible.
One independent final review and necessary correction pass; merge parent master,
delete task branch and archive/sync. No push.
