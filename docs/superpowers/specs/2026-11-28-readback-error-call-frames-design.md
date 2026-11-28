# Deferred readback error call frames (Task214)

Close Task183's remaining provider-diagnostic finding. When an existing modeled
readback Process operation fails while serving a later getter, preserve that
operation's stored call_stack in the existing contextual error. Append each
frame using the established `\n  while ` format and stored order, matching the
normal generator and opaque lowering paths. Keep the failing operation origin,
underlying cause and callback-preservation prefix. Empty stacks keep identical
messages. Preserve sticky-error caching; a repeat read/status query must rethrow
the same contextual error without appending frames again.

Directly enrich the existing Process catch; introducing a shared exception
framework or changing validation/scheduling is unnecessary. Preflight validation
continues its existing separate path. There are no public API, protocol, native
observation, queue-order or successful-pattern changes. Error-only work is linear in total frame text; successful execution has no
added traversal or allocation.

First add native assertions on full cause/origin/ordered nested frames, repeat
read and status rethrows, and an empty-stack byte-exact control. Add one CLI
method with both direct/merged nested calls and three later getters. Use real
opaque first/second scans whose second Apply fails on unknown retained length;
preserve preexisting output/debug artifacts. Observe native and CLI RED before
changing production. Then add only the frame loop and verify GREEN.

Fresh qualification: full normal/static project check, all15 normal native
suites through that check, all15 default-leak sanitizer native suites, affected
readback/runtime/opaque CLI modules on sanitizer, changed C++ format/tidy/C++17,
source build/install and exact package members. Run default focused cases before
heavy work. Run sanitizer suites in isolation from heavy full regression to
avoid inherited bounded-solver contention; do not change limits or flags.
Capture commands, exits, input/product hashes and raw logs. There are4135 tracked
qualified inputs initially and48 products; rehash the changed inputs/rebuilt
products rather than claiming identities remain unchanged. No performance
speedup, fresh vendor/device access or broader support claim.

Standing question opt-out and inline authorization apply. Parentmaster
19b7e3ea2eeaad28f3839e897a33dd1456646ed6; branchcodex/readback-error-call-frames.
One fresh whole-branch review; one Critical/Important RED-to-GREEN repair wave,
no rereview. Inherited23 minus1 candidate =22 pending confirmation. Preserve117
stashes and unrelated worktree/sibling state. Merge, verify actual merged
products/tests, push, archive only owned helpers/ledger and retire the branch.
Original broader inventory and excluded strict coverage item1 remain open.
