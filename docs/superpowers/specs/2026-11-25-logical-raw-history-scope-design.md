# Logical raw history scope after lowering

Task175 advances inventory6. Both parent and Task174 expose a physical root
history after a child-only scan even though source execution has none. If an
earlier logical root history exists, lowering also overwrites its old masks.
Retain the Task174 reviewer probes for both cases as baseline evidence.

Carry logical observation scope on Operation with a default-false
raw_history_omitted flag. The native scalar raw lowering sets it on the
physical root result when the logical request queue excludes that root.
Do not alter hardware requests, captured values, comparisons or global failure
accounting. Explicit root scans retain ordinary histories.

Render a standard quoted comment immediately before each omitted-history
physical scan. Its text is the fixed prefix `modeled raw history omitted `
followed by the exact canonical iScan command and arguments. This includes
target, length, input, comparison and stable option. Share one text constructor
between renderer and Runtime; compare against the existing string rather than
parsing or copying arbitrary annotation fields. No extra dependencies.

Runtime accepts only a matching comment/status annotation immediately before
the next recorded root-client data iScan. Mark that scan and the consumed note;
the renderer skips the consumed original note and regenerates one canonical
comment, keeping replay idempotent. Only complete canonical scan shape and spelling can be consumed.
Unknown/malformed/mismatched/stale notes stay informational. Unconsumed typed
reserved notes render with one inert informational prefix, preserving their
tag and visible text so normalization/elision cannot activate them later. No actual data is carried. Cancellation by reset and
ordinary intervening operations cannot transfer the omission to another scan.

Runtime validates all queued raw operands and comparison care as before, but
does not invalidate a marked scan's logical raw history. The model provider
still samples and counts every comparison, then omits only publication of that
root's raw-history delta. Existing child identity notes restore explicit opaque
child histories unchanged. This preserves absent and previous root histories
without retaining extra snapshots or undoing provider observations.

Post-CSU rollback would need snapshots in Runtime and provider and would be
fragile across lazy readback boundaries. Replaying child scans would duplicate
hardware actions. A pre-scan scope comment preserves the original logical
request contract using existing Operation metadata and observation paths.

Cover opaque and modeled scalar child lowering, one/two children, absent and
previous root history, sparse masks, status/clear in the suffix, copies and
subscriber commit, resets/canceled queues, malformed/stale comments, both tags,
source/flat/generated continuation and provider-unavailable behavior. Explicit
root requests and whole-output replay remain unchanged except new comments.
The inherited in-prefix status-clear loss is a separate subsequent inventory6
fix, not part of this scope change. Preserve29deferred Minors and excluded1.

Retain1Mbit and existing10s/512MiB bounds (single-wide controls256MiB).
Annotation construction/comparison is O(root bits), once per physical scan;
no per-child full-root copies. Qualify representative existing matrices with
matched parent comments, exact complete outputs/scan counts and live getters.
Full normal/static/native/SAN/corpus/quality/package checks and exact hashes
are required before one fresh final review and authorized merge/push/cleanup.
