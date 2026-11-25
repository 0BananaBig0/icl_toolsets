# Preserve status clears through generated prefixes

Task176 advances inventory6. Task174 review proves that a successful
iGetStatus -clear inside a prefix returns5 and clears source status, but its
generated prefix retains only the printed5. Appended status then returns5
instead of0. Task175 root-history scope is complete and does not fix this.
IEEE1687-2014 section8.4.3(b), printed188, requires returning the current count
and then clearing it. iGetStatus is level1; generated output remains level0.

Record every successful source clear, including provably zero clears, as an
ordered internal Operation with command iGetStatus and arguments -clear plus
the observed canonical decimal count. Preserve callback and invocation context.
The frontend still accepts only standard iGetStatus syntax. Failed/unknown
queries emit no clear. Internal events add no hardware request or observation.

Render the internal event as a quoted standard comment:
iNote -comment "modeled readback status clear 5". Both comment/status replay
tags are accepted. A shared bounded parser accepts only canonical uint64
counts. Runtime compares the count with actual available status before
performing a clear and recording the internal event. Provably zero status
requires no device observation. Unknown/unavailable/mismatched/malformed notes
stay informational and cannot fabricate a clear. Render unconsumed reserved
notes with one inert informational prefix, preventing later promotion after
normalization or elision. Valid events regenerate exactly once.

The model provider processes an internal clear at its ordered stream position,
clearing only known failure counts; unknown remains unknown. Actual data and
miscompare histories stay intact. Already performed source clears may be seen
again by the lazy provider before any later comparisons; clearing known zero
again is harmless. Events are scheduling barriers inside calls. Retain existing
rejection of direct level1 operations in iMerge; do not turn a direct merge
note into a new unsupported event. Called clear events retain call-stack depth
and prevent batching across their status boundary. Ordinary note ordering
keeps IEEE7.9.18 behavior.

Reuse the existing internal Operation stream, readback provider and renderer.
Level1 output would change the transcript contract; ordinary iNote-only events
could be hoisted across call comparisons. No dependency, hardware transport,
Tcl grammar or RTL change. Local-reset source rewriting preserves procedures
and clear evaluation order. Queued but unapplied accesses are not flushed.

Acceptance covers PIO/scalar raw/opaque-child and existing bound TAP observations,
nonzero/zero/repeated clears, later mismatches, reset persistence, pending queues,
unknown/no-provider behavior, ordinary and malformed notes, both tags,
source/flat/generated continuations, namespaces/local resets and called merge
barriers. Actual counts and histories agree, no extra CSU or failure recount.
Canonical note parsing is O(text length), bounded before numeric conversion;
clear processing is O(1), with no per-event root snapshots.

Maintain the multiple-child benchmark's exact parent controls and live getters.
The old parent carries matching clear comments. When replacing a generated
literal with a live clear getter, remove its matching recorded clear comment
so the getter recreates that event once and still returns the pre-clear count.
Keep complete whole-output/scan/count comparisons and unchanged10s/512MiB
bounds, including single-wide256MiB controls. No speedup claim.

Full normal/static/native/SAN/corpus/quality/package verification and exact
hashes precede one fresh review, one test-first blocker-fix pass and authorized
merge/push/branch cleanup. Retain29deferred Minors and excluded coverage1.
Broader modes, modeled child-local histories and real transport stay open.

Review clarification: validating a rejected annotation must not publish readback
history, cache a known count in Runtime, or mark a merge as explicitly observed.
A separate optional status-preview callback shares the same lazy model and
retains unpublished deltas until an ordinary readback call. Repeated previews
advance only new operations; later history assignments suppress the appropriate
unpublished plane. Existing ordinary provider callers keep their signatures.
Without a preview callback, only already provable zero annotations can activate.
Accepted nonzero annotations still perform the ordinary clear observation.
Clears before the first call hardware or after the last call hardware may be
peeled outside the batch, preserving their boundary and same-side batching.
Interior events remain barriers. Buffering is bounded by the existing latest
per-bit/per-chain delta maps; it adds no model copy or root-state snapshot.
