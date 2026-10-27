# Apply-group vendor round trips

Execute inline with TDD, independent review and full regression. This applies
the user's corrected acceptance rule and supersedes the earlier requirement
that independently generated local/vendor schedules agree.

## Contract and design

- Forward: generate B locally from A, have Tessent consume B and export C.
  Compare B/C by ordered apply groups, irrespective of write order within a group.
- Reverse: have Tessent consume A and export B; generate C locally from B with
  the same ICL/BSDL. Compare B/C by the same policy. Record each direction;
  either successful direction qualifies that case, not all possible networks.
- Never require the forward result to match Tessent's independent source run.
  Preserve formatter source-vs-formatted PDL/testbench certification unchanged.
- Extend the existing static PDL trace helper with an opt-in ICL-aware apply-group
  mode. Reuse Design/Netlist/ResolvePdlTarget/PdlDataValue; compare resolved bits,
  last effective overlapping assignments and explicit apply options/boundaries.
  Keep assumptions/resets/waits and unsupported commands visible. No Tcl eval.
- Add a focused round-trip harness under tests/, reusing staging, vendor scripts,
  integrity checks and immutable publication from existing oracle helpers.
  Preserve legacy certificates; new records state their direction/policy and
  offline checks use stored vendor output, without SSH/licenses.
- TDR/SIB selection uses pre-update state. A controlling SIB may close in the
  same update that writes the TDR. Keep concrete replay and test this explicitly.
  Do not hide extra TAP operations or mark unsupported reverse input as passing.

## Tasks

- [x] Comparator tests first: reordered independent writes equal; bit/whole-vector
  representations equal; changed values, apply order/options, conflicting last
  writes, assumptions and waits differ; dynamic input rejected.
- [x] Implement opt-in trace mode using existing elaboration/value code; preserve
  old trace output and certificates. Bound aggregate expansion and input size.
- [x] Add forward/reverse staging, certification and offline replay tests, including
  differing independent schedules, immutable input hashes and rejected changes.
- [x] Execute the two directions using fresh or already hash-verified Tessent
  evidence where the exact staged inputs are unchanged; retain blocked cases.
- [x] Check same-update select closure via existing concrete scheduler tests and
  an explicit regression. Independent review, full make check, leak-enabled
  sanitizers, docs/README/requirements update. No blanket equivalence claim.

## Scale

Use bit expansion bounded by the existing netlist width limits and a trace-level
budget. Canonical maps cost O(bits log bits) per apply group; no pairwise command
comparison and no hardware schedule enumeration. Existing 8/16-bit vendor probes
supply qualification; include a larger synthetic comparison case.

## Outcome

Completed with six directional certificates, seven unchanged legacy standards,
13 trace and seven new harness tests. Independent review findings were reproduced
and fixed. Full make check passed (build/apply-group-final-regression.log), as did
ASan/UBSan with leak detection enabled (build/apply-group-sanitizer-tests.log).
Source-intent correctness remains a separate obligation; AccessTogether comparison
and traces over 256 Ki expanded bits are explicitly unsupported. No new licensed
run: exact previously executed Tessent inputs/exports were hash-verified again.
