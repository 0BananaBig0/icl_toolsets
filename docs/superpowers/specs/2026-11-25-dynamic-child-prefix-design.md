# Preserve dynamic modeled child histories through generated prefixes

Task178 advances original item6 after Task177(master19af5bc). Existing native
dynamic scalar child requests already execute and expose actual histories.
When a nine-bit child request closes its local SIB, source data/miscompare
remain0b101001011 and status6, but generated-prefix child getters fail.
The current post-CSU static geometry cannot describe the earlier return word.

Retain the complete pre-CSU route geometry for the most recent successful
dynamic scalar raw transaction. Reuse RawProfile/RawSegmentFor rather than
reconstructing from changed state. Share the immutable profile across scheduler
copies; keep projection caches and traversal budgets local to each copy.
Commit geometry only with successful raw state, including source preparation,
and preserve existing subscriber/rollback behavior. Keep one profile per
scheduler, no per-child model snapshot or root-word copy. Static, opaque, TAP
and parallel behavior retain their existing contracts.

Use a distinct quoted standard comment for dynamic request intent:
modeled dynamic child raw history ROOT CHILD 0bEXPECTED. It carries expected
mask and identity only. Reuse the existing bounded child-note parser, adding
a dynamic discriminator. Static notes remain static; dynamic notes require
the committed dynamic geometry. This keeps older renderers' existing inert
static annotations unchanged and permits exact native parent comment controls.

Reuse Task177 canonical root provenance, neutral history/status callbacks,
default regeneration and actual-root projection. Width, offset and inversion
come from the route that executed the CSU, including inner routes that change
width, stable children whose parent route changes, and post-update closures.
No extra CSU, fabricated observation or repeated status accounting. Keep
unknown data/masks/status, previous root/child histories, cancellation/reset,
typed duplicates, invalid/inert metadata, merge and local-reset contracts.
Metadata still requires an adjacent actual canonical root scan in its source.

New tests under tests/src/pattern cover nested variable-width children,
offset/parity changes, equal-width route changes, closed/inactive children,
unknown returns, local stable claims, copied/failed scheduler paths, metadata
boundaries and generated-prefix local-reset verification. A native test pins
committed old geometry and once-only model deltas.

Measure source/flat/generated live getter replays with1/2/8/32dynamic children
at8/64/256bits and1/2/8children at1024bits, plus sanitizer samples. Native parent
inputs add only the new dynamic comments. Independently check actual child
words/masks, root words, failure counts, scan/CSU shape and complete outputs.
Keep existing1Mbit aggregate and10s/512MiB bounds, with single-wide256MiB
controls. Work scales with visited route objects plus addressed/returned bits;
no repeated full-root normalization or per-child full-route copies.

Use delegated inline execution, test-first changes, full normal/static/native/
SAN/corpus/quality/package evidence and all existing workload controls. One
fresh final reviewer, one blocker-fix wave, no rereview. Merge/push master,
delete the owned branch, preserve117stashes and30deferred Minors. This restores
already supported dynamic scalar histories; general independent clocks/TAPs,
other topologies/timing, real transport and excluded coverage1 remain open.
