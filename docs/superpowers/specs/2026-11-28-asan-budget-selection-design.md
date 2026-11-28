# ASAN budget selection (Task221)

Close Task168's test-selection finding. ASAN_OPTIONS can be set while running a
normal or static generator; its presence must not skip their 1,024-bit budget
check. Identify the actual AddressSanitizer runtime from the existing --version
command with informational help=1 appended to a copied ASAN_OPTIONS value.
The installed runtime emits the exact AddressSanitizer flag-header line on
stderr. Normal/static products do not. Probe once per module, without guessing
from binary names, adding dependencies, changing native APIs or relaxing flags.
Retain the current wide memory/time limits and the sanitizer exclusion for this
unqualified large memory case.

Add a maintained regression in the existing test module: an eligible normal or
static build must execute the wide test in a child with ASAN_OPTIONS set, report
its actual metric and skip nothing. Do not recurse into the full suite. The same
meta-check is inapplicable to the intentionally excluded sanitizer wide case;
report both sanitizer skips explicitly. Observe the old decorator's false skip
as an assertion failure before changing it. Then run the full module under both
normal environments on normal/static binaries, and once on default-leak SAN:
five reports, 60 reported methods, 58 executed and two intentional SAN skips.
Record actual native version/probe responses for all three products.

Only tests/src/pattern/test_primary_pin_literals.py plus tracker/spec/plan change.
Preserve 4,134 unaffected qualified inputs, 48 products and 117 stashes from
Task220. Exact four-member source package and final hashes/whitespace qualify
bytes. Retain prior native/build/broad evidence by identity, with no fresh such
claim. The added cost is one --version subprocess per module plus one bounded
normal/static wide control. Existing 10s/256MiB/45s limits stay unchanged.
No performance improvement claim. Parent master c56190a3f6ac6e333c0c1ca229595e5142ad7075;
branch codex/asan-budget-selection. Standing question opt-out and inline,
merge/push authorization apply. One fresh review, one blocking repair wave if
needed, no rereview. Inherited 11 minus 1 candidate closure leaves 10 before
new findings. Archive only owned helpers/ledger, merge/verify/push and retire
branch while preserving the reusable worktree and unrelated state. Broader
original tasks remain open and item 1 stays excluded.

Review repair: require exit 0 from the version probe before classifying its
stderr. Report an inconclusive probe with its status and original stderr. A
harmless executable returns 2 for --version and marks any attempted wide command;
the maintained regression must fail before the fix and confirm no attempt after
the fix, without launching an unqualified wide sanitizer workload. Final five
reports contain 60 reported methods, 58 executed and two intentional SAN skips.
