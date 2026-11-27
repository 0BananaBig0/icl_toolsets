# Callback benchmark postconditions (Task207)

Close two recorded benchmark gaps: Task163's shared checker verifies only
postcondition names/widths/counts, and Task164's wrapper verifies ordered pairs
without their relative physical Apply placement. Correct production output
already exists; this task strengthens its measurement oracles.

During the existing physical_counts pass, collect each serialized iState target,
literal payload and number of completed nonempty physical frames. For fixture
value v in 1..4 and target index i, require r_i, its width-padded value, and
(v-1)*(targets+1)+i+1 completed frames. Each value has one callback frame per
target followed by its destination-enable frame. Ignore empty logical Applies
in that position count, preserving explicit controls. Unknown callback state is
not a device observation; these are serialized logical postconditions.

Keep the function's signature, returned metric fields and physical frame tuples
unchanged. Reuse its stronger guarantee in checked_frames; remove that wrapper's
duplicate postcondition scan while retaining ordered_postconditions_verified.
Alternative independent wrapper checks would leave other shared consumers weak.
Use the existing loop and fixture semantics rather than a general PDL parser.

Maintain a literal small fixture captured from the qualified current generator,
with positive unused/empty-Apply controls and payload/name/early/late corruptions.
The old checker must accept the relevant corruptions and the new checker reject
them. Put the new test under tests/tests/src/pattern and run it from the existing
pattern-scheduler-check target. No C++ producer, dependency, limit or API changes.

Expected workloads remain 16/64/128 four-bit targets and single 512/1024-bit
targets, plus the existing 256-target consumer. Added checking is linear in
output and target payload bytes; remove the wrapper's second output scan.
Preserve existing 10s/512MiB regular and 10s/256MiB wide caps. Run the three
complete existing benchmark matrices with three repeats, the model-evaluation
counter and bounded static source/flat/generated fixtures. Parent/current roles
use the same qualified engine because only Python oracles change; no speedup
or simultaneous maximum-size claim. Retain exact commands, reports and artifacts.

Verify all 48 products unchanged, package/install, Makefile and new metatests.
Do not repeat unrelated C++/frontend/native/event/vendor suites. Reuse standing
completion-program, opted-out design questions and inline authorization. One
fresh independent whole-branch review, one verified Critical/Important repair
wave if needed, no rereview. Inherit 30 findings, candidate 28; preserve 117
stashes, sibling ledgers, foreign worktrees and excluded item1. Merge, verify,
push and retire this owned branch before continuing.
