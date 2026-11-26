# Shared serial input for fixed scan lanes

This is a bounded increment of completion-program items 2 and 6. Two or more
interface-free lanes may share a physical ScanInPort bit when their storage is
disjoint, their routes are fixed and ungated, and their nonzero lengths agree.
All lanes participate in the same capture/shift/update. One physical input bit
has one stream; it cannot independently load incompatible words into the banks.
Capture sources and observations remain separate even when loads agree.

Reuse the existing graph, symbolic loads and concrete replay. Admit shared input
vertices and transparent input-only forwarding in inferred interface-free lanes,
then prove the bounded shape.
Concatenate each lane's load words in serial order with its actual inversion;
require equal streams for lanes sharing an input. This is linear in the lane
storage per frame and avoids constructing a quadratic per-clock symbolic history
for fixed routes. Concrete replay must reject contradictory streams as well.
Keep storage overlap, intermediate taps, unequal/empty shared lanes, gated or
switched shared routes, and named/client/TAP shared-input cases guarded.

Alternatives considered: merely relaxing the overlap check permits impossible
loads; per-clock symbolic histories already exist but are unnecessary for this
fixed-length case. Neither is the selected implementation.

Preserve original/flat/generated replay, explicit logical read masks, actual
captures, status clearing, omitted-load preservation, ownership and failed-output
safety. No new dependencies, resource-limit increases, transport or timing claims.
Verify independent bit-shift examples, split and inverted paths, separate input
groups, contradictory writes and retained boundaries. Measure 2/8/32 lanes at
4/64/256 bits with each four-form replay bounded by 10 seconds; retain existing
disjoint-lane behavior. Run the project's complete check, affected sanitizer
tests, C++ quality checks and source packaging before independent review.

Authorization: the approved completion-program design and repeated instructions
to proceed, finish items 2–15 except 1, and integrate each finished branch cover
this maintenance increment. Continue native execution in the existing isolated
worktree; merge into master, push and delete only this owned branch after review.

The review repair preserves control bits consumed only by unrelated logic.
For shared lanes, derive automatic load permissions from the scan network and
current requested write, read-route, port, reset and end-state dependencies.
Reuse the existing control traversal with a separate visited set; preserve the
existing permissions for independent lanes. Genuine requested data controllers
remain programmable without freeing their unrelated bits.
