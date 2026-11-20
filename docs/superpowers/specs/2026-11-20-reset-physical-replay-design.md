# Physical reset-pattern replay
The outstanding vendor reset export describes shifted bits, including loads discarded by local reset. Add explicit `--physical_pdl`; ordinary `iWrite` remains a final update objective.

Reuse the concrete single-root TAP path and event simulator. Each nonempty apply executes exactly one CSU, checks captured serial output before update, shifts the requested binary loads, and commits actual reset/control events and fill only on success. Preserve physical load values in generated PDL. Output is replayed with the same option. No reset prerequisites, merging, or final-state restoration may introduce transactions.

The initial supported scope is a reset-initialized single bound TAP, directly driven common capture/shift/update/clock controls, known binary scan loads and explicit scan-register reads. Reject unsupported routes, mixed data access, AccessTogether aliases, raw scans, end-state declarations and source reset rewriting clearly. Preserve resource checks. Unknown comparisons remain masks. No new dependency or RTL changes.

Acceptance: both recorded reference and rewritten vendor exports replay on original and flattened models; ordinary mode continues rejecting their conflicting final goals; physical output has identical loads and packet count and is idempotent; wrong compares and unsupported shapes preserve existing artifacts. Tests must be self-contained under tests. Check normal, static, sanitizer, full regression, raw coverage, C++17 and source package. The existing failed 100% line/90% branch gate remains disclosed.

Performance: one native traversal per serial clock; retain existing 1,048,576 event-work limit. This is exact replay, not bounded symbolic planning. Measure the vendor case and a repeated larger pattern.
