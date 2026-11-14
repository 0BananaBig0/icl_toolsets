# Physical reset mux solver work

> Execute inline under existing autonomous authorization. This is mandatory
> unfinished performance work exposed by Task67, not an acceptance waiver.

Goal: retain the unchanged 32-register feedback ring, a real top-level ResetPort,
standard forcing muxes, source/flat/generated replay, and the existing 2s solver
check/15s scheduling limits. Keep exact ternary reset snapshots, transient events,
controlling history, fill, reservations and unsigned deterministic ordering.

Baseline: the Task67 isolated build/direct-control-driver-ring-ablation contains
18 quiet source runs (three repeats) with inputs, commands, binary hashes, wall,
RSS and outputs. Bare32 on62d6915 passes with/without iReset (0.516–0.767s).
Physical-root mux32 fails on both62d6915 and the direct-driver guard binary.
A source-less ToReset helper mux32 passes on both (4.831–5.133s), matching the
bare-with-reset output. The original ring had direct feedback without an explicit
global branch; the helper is a legitimate fixture adaptation preserving tested
post-reset behavior, not whole-model equivalence or a physical-root fix.

Failed ablations retained in build/direct-control-driver-* logs: mux-only
simplify, Boolean two-row bitmask evaluation and scan-load MinimizeBits all fail
unchanged32 source or flat checks. All three product changes were discarded.
Do not repeat them without new evidence. NeedsScanEvents is gated off for this
standalone ring (no TAP/client controls); its250ms probe is not the cause here.

Affected code: symbolic value construction and canonical references; scheduler
initial assignments, SCC reset closure/named snapshots, frame constraints and
objective encoding; existing symbolic/state/scheduler/feedback tests. Use current
source to choose the smallest fix after collecting diagnostic evidence. No new
public API, metadata, dependencies or diagnostic settings in the product.

- [x] Preserve reviewed Task67 checkpoint while its root full/SAN checks run.
- [x] Retain physical32 RED; capture solver reason, construction/check timings,
  assertions/objectives, reset-pass counts and representative expression shape.
  Compare physical/implicit/bare sources with identical goals and snapshots.
- [x] Check actual constant/knownness normalization, graph/order and named reset
  constraints before choosing a change. Avoid optimization by incidental variable
  ordering or repeated whole-expression simplification through deep mux chains.
- [x] Implement one justified candidate, preserving linear/bounded graph work and
  the existing resource checks. Keep a meaningful regression for the failure.
- [x] Measure repeated physical32 source/flat/generated results. Check relevant
  larger rings, reverse reset chains, symbolic deep muxes, primary1024/4096 and
  multi-TAP16/32; record commands, scale, environment, hashes, wall/RSS and limits.
- [x] Independent review, focused native/CLI, leak-enabled sanitizer checks, full
  serial regression, documentation and a verified combined67/68 commit.

The helper-only adaptation later fails full replay too; it is not an acceptance
workaround. Captured formatting variants produce byte-identical first SMT dumps
(2,754,636 bytes, SHA79b37d...),4228 assertions/34 objectives/65 reset passes,
yet checks lie near2000ms. The necessary fix is now combined with67: fully known
scalar selectors with exhaustive disjoint0/1 rows use direct Select, preserving
branch X data, either row order and declared-width rejection. Identical arm
knowledge masks are reused, keeping deep selector chains compact. Other mux cases
retain generic consensus. Product scheduler source is unchanged from62d6915.

Native width RED/GREEN, symbolic variable-selector identity,243 ternary vector
combinations and deep closed-form proofs pass. Before-mask-reuse27 repeated32
source/flat/generated runs all pass, including physical roots/reversed rows;
successful baseline output/flat hashes match. Final-source repeat, native/full/SAN,
style, broader connected checks and independent review remain before completion.
Deep old selector4096 timed out after60s without a completed measurement; known
mask reuse reduces measured4096 selector construction2.945s to0.023–0.027s.

Final production source hashes match the reviewed67/68 snapshot. The confirmed
27 replay runs, deep512/4096 proofs and full connected native7/CLI checks pass;
physical source0.617–0.667s/flat0.566–0.617s retain the unchanged limits. Native
scheduler includes primary1024/4096, reverse reset and rollback checks; optimized
multiple-TAP22 includes unchanged16/32 scales. Format/naming and independent
review pass. Full root/SAN integration remains pending. This necessary fix shares
Task67's commit; the earlier separate-commit wording is superseded without losing
the separate RED/GREEN or diagnostic evidence.

Root native7 and main52/clock41/client18/reset12 leak-enabled ASan/UBSan all pass.
All29 source/test hashes match the final reviewed snapshot; root serial full
regression is running. Later Task69 edits are confined to the isolated checkout
and are not part of this frozen integration snapshot.

Final integration succeeds: build/direct-control-driver-final-full.log/.exit0,
1808.877704s wall with make-j1check. All41 dynamic and41 static generator suites,
51 directional goldens,2523 ICL334/119 grammar/scanner rules, LSP62 and editor6
in220.236s pass. Optional ANTLR5 skipped. All29 root source/test SHA256 values
match the reviewed snapshot. Required native7/main52/clock41/client18/reset12
leak-enabled ASan/UBSan, focused proofs, repeated scale/loading checks, naming/
style and independent source/fixture/documentation review pass. Reviewer wording
corrections distinguish baseline SAT/unknown and an attempted60s timeout from an
unattempted run. Product scheduler objects also match the measured worktree.
Task69 draft stays isolated and is excluded from this commit. Structural/
operational, runtime/TAP/lifecycle, strict coverage, full vendor certification and
refreshed platform goals remain mandatory; this does not close the whole project.
