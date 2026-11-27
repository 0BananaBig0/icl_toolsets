# Write-only logical read scope (Task200)

Repair the already-approved ordinary-read contract in
2026-11-26-ordinary-read-scope-design.md and original item6: incidental physical
captures must not become logical reads. A write-only preparation currently emits
an unrequested all-X read of a reset-held register. Its original never-read
history is xx; replay changes read history to 11 and miscompares to 00. A logical
read with automatic preparation also gains this read on its next regeneration.
Exact parent reproductions are in Task199's replay-diagnosis evidence.

Use Solve's existing logical reconstruction for write-only applies too. Retain
requested X reads, known previous histories, selected frames, implicit IR capture
checks and exact physical/raw contracts. Do not remove a required native event
or make an otherwise unexpressible transaction silently disappear. Existing
all-reset-held writes already reject; this does not add support for them.

Approaches: reuse the existing scheduler scope information (selected); filtering
all-X reads in the CLI loses explicit X requests, while new history metadata
would duplicate existing request/frame information. No public API, format,
dependency, resource bound or hardware assumption changes.

Additional work is linear in operations and emitted comparisons, within the
existing bounded scheduler. Check 16/128-register preparations and existing
sparse 256/512/1024-bit cases, with 10s/512MiB bounds and no speedup claim over
incorrect parent histories. Execute inline, review once, repair blockers once,
merge/push/retire the owned branch, preserving 117 stashes and 44 inherited Minors.


Qualification consumer: the transitive_releases forward certificate changes by
one removed unrequested all-X read. Fresh Tessent2026.2 exports of the corrected
program and exact prior schema3-certified program are byte-identical. Refresh
only that snapshot with the existing schema4 reference-vendor-equivalence policy,
retaining prior provenance, both fresh jobs and the distinct native logical
trace. All other directional snapshot bytes remain unchanged. Rerun the full
project check after this certificate update; reuse other checks only with exact
unchanged relevant inputs and product hashes.


The single review repair retains every previously generated implicit IR capture
comparison when read_scope is empty, including independent mixed write-only
TAP groups. Read-containing mixed groups keep their established logical scope.
The additional test checks the cared tap2 comparison in the final apply group
for independent/shared TMS wiring in default and modeled modes, with exact
source/flat/generated replay. The review's overview-prose Minor is deferred.
