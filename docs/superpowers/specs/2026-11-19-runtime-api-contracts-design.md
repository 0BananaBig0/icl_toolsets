# Runtime parse and context API contracts
Parent masterdb5f353; Task134. Continue the pending unchanged strict-coverage
work. Standing user direction delegates routine design/execution and immediate
qualified commit/merge/delete/push; reuse the existing isolated worktree.

Add four tests in existing tests/src/pattern_runtime_test.cc, reusing MakeDesign,
Fails and Require. No product change, new target/dependency, invented hardware
readback, invalid IR, failure injection or weakened100/90 gate is planned.
Baseline has16 zero-hit reachable lines49,88-91,125,551,569,573,627,661,703-704,
734-735,742 in pattern_runtime.cc. Existing-behavior coverage is the gap RED;
do not claim a failing product behavior. Required GREEN is real contract
assertions and observed hits at all16 locations. If an assertion reveals a real
product defect, investigate first and ledger any necessary scope correction.

Test constructor/parse failure: unknown top rejects intended diagnostic.
After a valid iNote previous file, a second file with iNote leaked followed
by an unclosed if body fails at EOF invalid.pdl:3 before any second-file operation
is published. Exactly the original note remains. Execute and Finish afterward
reject the failed runtime. This catches lost prevalidation/fail-stop semantics.

Test nested pending EOF: procedure queue writes reg without apply; Finish
reports the iCall queue frame and fails closed. Existing top-level EOF checks
remain unchanged. This catches lost delayed-error stack provenance.

Test fallback hierarchy: real Top -> Outer -> Plain/Typed instances.
Plain has a fixed data port; Typed adds VALUE=4'b10x1. Define emit procedures
in both modules. Runtime(design,"Top") successfully resolves a.plain.emit and
returns literal plain; a.typed.emit rejects the need for elaborated parameters
before its body executes. Runtime(design,elaborated netlist) executes that call
and emits literal0b10x1. A valid namespace binding to an absent module rejects;
unknown a.absent instance paths reject through both elaborated and fallback
lookup, requiring their intended diagnostics. These controls catch broken
nested resolution, silent parameter invention and accidental X conversion.

Test procedure namespace selection: define default/alternate emit procedures
for Top, select alternate with iUseProcNameSpace alternate, call emit, clear
with iUseProcNameSpace -, call emit. Exactly two notes alternate/default, no
warnings. This catches ignored selection and clearing. No live-readback or
simultaneous vector scan-return support is added or claimed.

Alternatives: CLI cases cannot cover the public non-elaborated constructor
path; fabricated IR/private-hook invocation is unnecessary. Real public Runtime
calls are the smallest faithful coverage. Fixed tiny models/scripts, bounded
memory, no expensive loop or per-case process; production performance unchanged.

Fresh format/tidy/strict17, full normal make check, full changed-runner leak
ASan/UBSan, focused and complete fresh-profile raw coverage, Rocky changed-runner
build/C++17/native checks. Reuse Task133 other qualification only with3993
unchanged runtime files and all ten local/remote product fingerprints. Archive
4239 tracked files+6 generated, compare every shipped tracked byte and4013
non-doc files against current remote source. No fresh remote CLI/vendor/
archive-build/install qualification claim. One independent final review, one
necessary fix pass; merge direct parent, delete own branch, archive ledger,
sync CodeGraph, push. Full coverage/release remains open on a failed raw gate.
