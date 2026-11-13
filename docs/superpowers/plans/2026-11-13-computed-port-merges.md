# Connected and computed port merge ownership

Continuation of the approved pattern-completion work. Execute inline on the
existing branch under the standing authorization; preserve independent review.

## Behavior and scope

Independent iMerge calls that write/read connected data ports should be eligible
for the existing ordered batching algorithm. A port name must not hide shared
physical state: retain whole-object ownership of the target and every reachable
combinational dependency, stopping at registers and unconnected top-level inputs.
Include mux selectors/branches, one-hot contributors and output enables. Keeping
all branches is conservative: shared inactive inputs may prevent batching.
Preserve logical-port controlling history, per-call apply order, AccessTogether
and resource/timing barriers, and existing fresh-state serial fallback.

Reuse ResolvePdlTarget, ValidatePdlAccessTarget and PdlDataValue. Walk only
dependencies reachable from the current stream, with one visited set shared by
all its targets; do not walk every netlist node per bit. Bound total traversal
work across a merge pass and fall back to serial scheduling when exhausted.
No new dependency or optimizer framework. The expected small/medium workload is
tens to hundreds of calls; measure 64 independent port streams and a shared
dependency case. This is greedy optimization, not a global-minimum guarantee.

Files: src/pattern/pattern_merge.cc; shared Make generator suite;
tests/src/pattern/test_merge_computed_ports.py; existing primary-merge test name
if its unchanged overlap assertion needs a clearer description; generator guide
and coverage ledger.

## Steps

- [ ] Reproduce serial-only behavior with independent connected/output-port
  tests. Cover aliases/masks, repeated apply order, shared sources, mux/logic/
  one-hot dependencies, output enables, reads, constants, invalid operands,
  resource barriers and output preservation. Replay source, flat ICL and output.
- [ ] Extend ownership traversal using the existing netlist edges and validation.
  Keep unsupported operations as barriers and bounded-work fallback explicit.
  Run focused new and existing merge/port suites.
- [ ] Measure representative scale; run relevant native/full dynamic/static
  regression and leak-enabled sanitizers, style/naming and independent review.
  Record exact results and conservative ownership limits. Do not claim this
  completes timing/resource optimization or the outstanding model requirements.
