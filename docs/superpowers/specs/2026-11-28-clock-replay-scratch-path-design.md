# Clock replay scratch location (Task212)

Resolve Task211's deferred early-location finding. Print the newly allocated
scratch path immediately after mkdtemp returns, before receipt creation,
manifest copies or active-script setup. Preserve the existing flush, exception
propagation, finally cleanup, assertion preflight and all native replay work.
Moving the existing print is sufficient; an additional logger or error handler
would duplicate this responsibility. Existing reader steps remain the same.

Use the standing opt-out of design questions and inline execution authorization.
The direct parent is master at 4e1f8d26ece787aa1ce7286ab06832d54afe2515;
use branch codex/clock-replay-scratch-path in the retained isolated worktree.
No C++/maintained tests/Makefile/dependencies or archived driver modifications.

Execute the actual published code on real owned active/scratch files, injecting
only driver-copy failure or child-wait interruption. Require printed exact
scratch location, preserved original exception and removal of the active path.
Observe one failing copy case and one passing wait case before moving the print;
both must pass afterwards. Retain the existing two optimization preflight checks.
Private mirrored probes belong under tests/docs/ and are archived before packaging.

Verify the complete published code is byte-identical to Task211 after removing
exactly the one moved print line. Verify all 4135 qualified inputs, 48 products
and 2617 protected canonical files unchanged. These identities preserve Task211's
270-native/16-parser replay and source build/install evidence for unchanged work;
do not label those historical results as newly executed. Run make dist and check
exact bytes of guide/tracker/spec/plan in the new source archive. No runtime or
memory change is expected from moving one flushed message; retain native limits.

One fresh whole-branch review, one Critical/Important RED-to-GREEN repair wave if
needed, no rereview. Candidate ledger closure: inherited26 minus1 =25. Preserve
117 stashes, unrelated worktree state and sibling ledgers. Merge, verify, push,
archive only owned helpers/ledger and retire only this branch. Original inventory
and excluded coverage item1 remain open.
