# Raw scan execution inside merge calls

Task166 advances inventory8. The user delegated implementation choices and
authorized repository work, commits, parent merges, pushes and branch cleanup.
Direct parent: clean pushed master ee8a2b3f8119104a7de18bfd491a38dc1920da23.
This is the executor's small-change design, not a separate user design approval.

## Intent and evidence

Allow valid raw iScan operations inside called procedures of iMerge, using
existing native raw execution and complete serial fallback. Serial calls already
work. The same two calls wrapped in iMerge currently fail before scheduling with
"unsupported operation with modeled raw scans: iMerge". Reproduction and serial
control are retained at /tmp/icl-raw-merge-discovery-frt2inmn/results.json.

GenerateMerged's raw preflight rejects merge markers. Merger::Batchable already
rejects streams containing iScan and preserves each complete stream in call
order. Runtime already validates direct-body commands, empty queues at entry
and return, nested merges and the no-reset rule. Reuse these responsibilities.
IEEE1687-2014 clause7.9.18, printed176–178 (opened /tmp/item11-1687.txt:9135–9210),
permits complete serialization and requires per-call Apply order, reservations,
empty queues and no reset in merged called procedures.

## Approaches and decision

1. Admit validated iMerge markers through the existing modeled-raw preflight;
   retain native validation and the merger's complete serial fallback. Choose
   this smallest change. Ordinary neighboring calls remain eligible for batching.
2. Implement parallel/interleaved raw planning and conflict resolution. This
   requires additional topology/event proofs; it is broader optimization work.

## Contracts and reach

Change the raw preflight in src/pattern/icl_pattern_generation_main.cc and its
necessary tests/docs. Preserve Runtime, Merger, native state/clock/TAP models,
public APIs, resource limits and dependencies unless an actual regression shows
a necessary caller fix. No assumed captures, device data or transport.

Cover existing modeled scalar root/child, disjoint named-chain and bound single
TAP IR/DR raw routes. Preserve original frame order, once-only evaluated values,
prefixes, queue overwrite/default/stable behavior, reservations and reset rules.
Prove source/flat/generated and caller-local-reset replay, with immutable
procedure definitions. Unsupported topologies, invalid raw lengths and mixed
IR/DR groups must still reject without replacing existing output artifacts.

Keep opaque-child merge, opaque-root merge, modeled iMerge histories and raw
physical-PDL guards. This increment does not implement histories, new raw
topologies or vendor/device equivalence and does not complete inventory8.

## Verification and performance

Write failing CLI cases before changing preflight. Compare complete merged
outputs with matched serial calls and check actual scan payloads and Apply
groups. Cover nested/repeated calls, namespaces/once-only arguments, ordinary/raw
ordering, parallel chains, TAP transitions, stable/default/overwrite, notes/waits,
reservations and protocol/publication failures, plus existing unsupported guards.

Measure three source/flat/generated repeats at16/64/256 calls and512/1024-bit
raw words, matched serial current/parent controls and a smaller sanitizer case.
Keep10s/512MiB regular and10s/256MiB wide bounds; retain misses and complete
measurements without claiming unmeasured scale or global optimality. Record
actual scans, shifted bits, Apply groups, input/product hashes and environment.

Run full normal/static generator checks, relevant leak-enabled sanitizer CLI,
corpus, C++ quality and source packaging. Reuse Task165 native suite executions
only with exact native dependency/build-rule, binary and raw-log provenance;
otherwise run them fresh. One independent final review and one test-first blocker
fix pass; ledger Minors. Merge into master, verify merged inputs/products, push,
delete only this branch, archive only its ledger and continue pending inventory.
