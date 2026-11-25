# Modeled named-chain implicit-history replay

Original item6 still includes a reproduced continuity defect on already-supported
disjoint register-backed root chains. An explicit first-chain request leaves the
second history absent in source execution, but a generated prefix makes its
physical implicit scan explicit. A later second-chain request similarly replaces
first's old capture0000/mismatch1111 with implicit capture1001/mismatch0000.

## Approach

Keep the existing native simultaneous scan and actual observations. Mark implicit
parallel completions using existing raw_history_omitted intent and serialize that
intent into a distinct canonical named-modeled annotation. Reuse the existing
adjacent-note consumption, serial-merge ordering and comparison-before-history-
suppression behavior. No physical scan, clock, payload or comparison may disappear.

Distinct metadata is necessary for compatibility: old named raw-history omission
notes on modeled roots are informational, and activating them caused Task183's
verified call-order/history regression. Preserve that rule and scalar/opaque
metadata semantics, including historical generated prefixes and both note tags.

Alternatives: treating every emitted scan as explicit reproduces the defect;
accepting old named notes repeats the compatibility regression; adding a second
queue/history replay engine duplicates existing mechanisms. Choose one distinct
annotation within the established scope mechanism. The standing user opt-out of
design questions authorizes this bounded original-work repair inline.

## Acceptance

Use unequal modeled first4/second2 native chains, sparse/inverted captures, old
cared masks, known/unknown status, once-only clears, reset/cancel/latest queues,
multiple applies and all-explicit controls. Original, flat and live generated
continuations must match actual captures and explicit histories. Keep both
local-reset procedure/source bytes and default no-provider generation behavior.

Pin valid direct canonical metadata before merged calls with a discriminating
asymmetric canonical SI/control; invalid/malformed/old informational metadata
must preserve call-produced Tcl arguments and never suppress histories.
Retain existing physical/event/topology/route/width/resource/output checks.

## Scale and completion

Maintain the existing measured10s/512MiB bounds and256MiB wide controls. Exercise
1/2/8/32named chains at8/64/256bits plus1/2/8at1024, repeated source/flat/live
generated forms and sanitizer samples. Compare real native parent source controls
to candidate hardware words, flags, CSU boundaries, actual returns, masks and
clear counts. Any annotation-only comparison must validate the exact removed
intent; never normalize data or fabricate counts. Implicit-fill source controls
are required, rather than relying exclusively on all-explicit measurements.

Run focused RED→GREEN, native observer/queue checks, full normal/static generators,
30native normal/SAN, changed-C++quality, affected SAN CLI, retained/new matrices,
corpus, guides and packaging. Freeze qualified input/product hashes. Obtain one
fresh final review; repair blockers in one TDD pass, defer Minors, then verify
merge/push, delete only the owned branch and archive the own ledger. Preserve
117stashes,36inherited Minors, excluded1 and the broader unfinished inventory.


Eligibility reuses a lazily cached ScanScheduler::NormalizeScan with fixed
length validation for the new named-modeled form, rather than duplicating route
traversal. Existing replay link targets must link the actual required existing
solver objects. Measure the added validator state within the retained memory
bounds; do not promise zero cost or weaken native generation constraints.
