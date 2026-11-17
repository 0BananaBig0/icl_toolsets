# Scheduler C++17 portability

Makefile requires -std=c++17. Clang rejects ApplyScans' overlap-error lambda
under -Werror=c++20-extensions because it captures the structured binding name.
The isolated unchanged-source compiler probe exits1 at scheduler1504.

In only the first latest-request loop, iterate the map entry and bind its first
member to an ordinary const reference named name. The second member is unused
in this loop. Keep the lambda, diagnostic, interval checks/order and later
request-processing loop unchanged. No copy, allocation, new API or runtime
control flow. For C clients, interval work remains O(C log C)/O(C) space;
the existing queued-payload cap is one megabit. No performance improvement claim.

Use the existing worktree and native/CLI runners. No new maintained test script
or framework: compiler RED->GREEN directly verifies this syntax-only correction;
existing raw-client tests verify diagnostics, preservation and accepted requests.
Keep the compiler flag specific to the probe, not global build flags.

Verify strict C++17 syntax, changed-section format/tidy, Root dynamic/static
generator rebuild and all ten product fingerprints. Run existing automatic-child,
multiple-client-interface and chain-group CLI suites for both variants and a
focused leak-enabled overlap check. Rebuild the remote dynamic generator from
the one changed source, verifying3994 input hashes and product fingerprints.
If product bytes differ, qualify the changed artifacts with broader existing
generator suites before reuse. Exact byte identity permits reuse of unaffected
Task120/119/116/117 runtime and platform evidence; no fresh vendor/performance
claim. Preserve old generators and raw coverage evidence.

Task120's complete raw measurement remains explicitly the last pre-portability
measurement:17331/17961lines,29262/46677branches, strictFAILED100%/90%. Do not
claim post-edit source coverage or reinterpret those reports against shifted
lines. Global strict coverage and model/vendor gaps stay open.

Change only scheduler source, the two generator/coverage docs and this plan/spec.
One independent final review and necessary correction pass, then merge direct
parent master/delete branch/archive/sync; no push. Parent4104f49. User standing
routine delegation applies to this bounded portability correction.
