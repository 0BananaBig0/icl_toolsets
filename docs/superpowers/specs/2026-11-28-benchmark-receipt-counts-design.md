# Benchmark receipt counts (Task222)

Close Task161's nominal zero-scan fields and Task167's failed-row observation
counts. A receipt must separate observed output from fixture expectations.
The merged-readback driver already parses actual_notes: use its length for
literal_notes, retain expected_literal_notes separately and rename the
uninstrumented provider_queries field to expected_provider_queries. No runtime
query measurement is claimed. In mixed-caller receipts, rename constant zero
csu_count/shifted_bits to expected_csu_count/expected_shifted_bits; existing
measured Apply/PIO/frame checks stay intact.

Approaches: adding native counters would expand APIs and rebuild scope; locally
counting unsupported physical syntax would still not measure executed CSU work.
Choose explicit expectation labels and reuse the already parsed literal notes.
Consumers are the two maintained test modules. Historical receipts remain
historical; the current tracker states the new schema and its measurement limit.

Strengthen the existing valid and missing-flat readback tests to check real
output-note counts, including empty failed rows, and expected count labels.
Update the mixed scale receipt test to distinguish constant expectations from
measured fields. Observe three assertion failures on old driver output before
editing drivers. Run both full modules normal/static/default-leak SAN: six
reports, 75 methods, zero skips. Keep 10s/512MiB regular and 10s/256MiB wide
bounds; no new wide SAN workload or performance claim.

Four Python files plus tracker/spec/plan change: preserve 4,131 unaffected of
4,135 inputs, all 48 products and 117 stashes. Package seven byte-exact members,
retain native/build/broad evidence by identity and run whitespace checks.
Parent master 85751d4c072ac43a119d16b6e0ee423a2dedebaf; branch codex/benchmark-receipt-counts. Standing question
opt-out and inline/merge/push authorization apply. One fresh whole-branch review,
one blocking repair wave if needed, no rereview. Ten inherited findings minus
two candidate closures leaves eight before new review findings. Archive only
owned ledger/helpers and retire the branch while preserving reusable worktree
and unrelated state. Broader original tasks remain open; item 1 stays excluded.
