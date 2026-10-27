# Dependency scheduling fixtures

Self-constructed IJTAG networks for the user's root-to-leaf scheduling requirement.
`deep.icl` has 40 initially closed nested selectors before an eight-bit target.
It needs 40 prerequisite scans and one target scan. `wide.icl` has 64 serially
connected, independent SIB/TDR segments: all gates are initially accessible, so
one scan can open all gates and one scan can write all targets. No iMerge,
readback, proprietary models or hidden temporal behavior is assumed.

The independent regression model checks old-state accessibility, requested
values and minimal scan counts for these specific topologies. It does not
claim a general optimality proof for the heuristic scheduler.

The test driver also constructs variants of these fixtures with protected
storage and multi-bit protected selectors. It checks 40-level scheduling,
partial/X/full explicit changes, forbidden intermediate values, fixed-point
replay, and single-TAP control protection. These variants are self-constructed;
they are not vendor equivalence goldens.
