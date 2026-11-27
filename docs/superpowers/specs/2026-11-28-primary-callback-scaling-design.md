# Primary callback solver scaling (Task201)

Original item 10 includes the larger automatic-primary callback workloads left
open by the approved Task185 design. Three quiet rounds on the Task200 parent
reproduce a modeled-execution timeout at 32 eight-bit lanes. A single 1024-bit
lane succeeds but peaks near 301,000 KiB, above the existing 256-MiB single-wide
budget. Executed parent default-generation controls succeed. Actual histories
and matching generated hardware programs are checked wherever execution succeeds.

Reuse SolvePrimaryWrites and native replay. Per-apply diagnostics locate the
cost in repeated ordered optimization: 96 objectives per 32-lane apply and
66 per 1024-bit apply, including constant chunks. Preserve the feasible states
and objective preference while reducing their representation:

- Pack adjacent objective bits into unsigned objectives of at most 32 bits.
  Concatenation preserves the original lexicographic order. Omit chunks that
  simplify to constants, which cannot select among feasible models.
- A gated fixed nonzero data difference has only two attainable ranks: zero and
  that fixed value. Encode those ranks in one bit before packing. Fixed zero
  stays zero; symbolic differences retain their complete unsigned value.
- Keep data deviations, input-drive count and pin deviations in their existing
  separate priority groups. Stream bounded chunks and retain the scalar helper
  for the existing serial objective path.

The conditional rank mapping is strictly increasing over its attainable values.
The new invariant control requires preserving the earlier register to outweigh
fewer input drives, across 1-, 31-, 32-, 33- and 64-bit boundaries. Source, flat
and generated continuations query actual state afresh. The existing explicit-pin
and dead-trial-bit substitutions do not meet the measured gates and are discarded.
Partitioning would require a new proof for global events and ownership; raising
limits would change the contract. The selected changes stay within the planner.

Keep event, unknown-data, read-origin, reset, reservation, rollback, callback-order
and publication contracts. Preserve C++17, 256 primary ports, 4096 primary bits,
1,048,576 queued bits, 262,144 frame work, the 2-second solver timeout and the
5-second Tcl deadline. Public application APIs, syntax, eligibility and hardware
assumptions remain unchanged.

Qualify normal/static source, flat and generated replay at 8/16/32 eight-bit lanes
and single 512/1024-bit lanes, with actual data, masks, status and once-only
callbacks. Retain the smaller matrix and its configured sanitizer measurements;
run new functional cases with default leak-enabled instrumentation. Require
10 seconds/512 MiB per process and 256 MiB for normal/static single-wide cases.
Compare exact parent generated hardware where it succeeds. Failed parent
observations do not supply a speedup baseline.

Execute inline under the standing opt-out of design questions and completion
program authorization recorded by Task185. Use test-first repair, one fresh
whole-branch review and one Critical/Important repair pass, then merge, push and
retire the owned branch. Preserve 117 stashes, 45 deferred Minors, sibling ledgers
and excluded coverage item 1. Broader original items remain open.
