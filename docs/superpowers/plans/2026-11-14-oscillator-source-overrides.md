# Explicit oscillator source overrides

Execute inline under existing autonomous authorization and the approved generator
design. Keep Task53 root production files frozen for integration verification;
implement in the existing isolated worktree. Preserve independent source review.

## Scope and decisions

IEEE1687-2014 §7.9.16 permits a ToClockPort target and an explicit replacement
ClockPort/ToClockPort source. Extend the existing version history to a target
whose original definition is Period. A replacement source supplies the effective
reference; its supplied/default factors apply to that source, not to the original
Period. A checked older version retains its original intrinsic oscillator root.
Resolve considers Period terminal only when that bit has no override at the
requested version. Other vector bits remain independent oscillator roots.

Keep original metadata immutable. Reuse existing prospective cycle detection,
version caches, current reservation tracing and normalized root replay. Explicit
self-reference is a cycle, not an intrinsic-root marker. Require explicit -source
for Period targets in this increment, including after an earlier override. Omitted
source restoration/factor-only semantics need separate qualification; do not
invent frequency multiplication on a self-root. Differential targets remain open.

Independent conceptual review supports this bounded interpretation. A source-only
Tessent2026.2 probe accepts old oscillator check/wait2, explicit replacement by
root*2, old wait2, new check/wait2. Its export keeps two oscillator2 waits then
one root1 wait. Retain exact submitted/fetched evidence and compare normalized
output once implemented; acceptance alone is not broad vendor equivalence.

Connected files: pattern_clock.cc (Override and Resolve), existing clock CLI and
native scheduler tests, help/guide/model ledger. No dependency, new representation,
grammar or test runner. Complexity and retained memory remain those of Task52's
version history; existing4096-stage/1024-wait scale check must still pass.

## Steps

- [x] Inspect source, IEEE rules, independent conceptual review and vendor probe.
- [x] Add failing original/flat/generated/reset-only cases for old/new snapshots,
  indexed targets, replacement oscillator source, immutable defaults and cycles.
- [x] Remove only the explicit-source Period restriction; qualify terminal roots
  against the requested override version. Preserve specific omitted-source failure.
- [x] Native tests cover current held root/descendant identity versus old duration,
  failed-cycle rollback, other vector bits and unchanged storage/pin state.
- [x] Run focused checks, scale and independent review; integrate only after Task53
  passes and is committed. Run full dynamic/static and relevant leak checks,
  compare vendor exports, document limits and commit verified work.

Three positive methods fail against Task53's unsupported guard (RED). All41 clock
methods pass after implementation in11.277s (11.33s wall,94896KiB maximum RSS).
Native source/flat tests cover old/current reservations, adjacent bits, original
metadata, state and failed-cycle rollback; full scheduler passes. Existing4096
stages/1024 alternating-version waits take6.66197s under concurrent verification.
Naming and independent source review pass. Our generated commands exactly match
the fetched source-only Tessent2026.2 export; submitted input hashes match.
Comparison and command retained in build/oscillator-override-vendor-probe.
Root full integration and selected leak checks are running; no completion claim.

Full integration passes in build/oscillator-override-final-full.log/.exit0,
1086.789s wall:41 dynamic/static suites,51 directional/seven legacy goldens,
2523 ICL334/119 rules and editor6 in181.938s; five optional ANTLR tests skipped.
Leak-enabled native scheduler/flatten/netlist and main35/clock41/client18/reset12
checks pass. Naming, formatting and independent source review pass. The exact
Tessent2026.2 probe trace matches our normalized output; no formal certificate,
strict coverage or platform refresh is claimed.
